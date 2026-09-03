#include "ImpageVersion.h"
#include "platform/desktop/CommandLineParser.h"
#include "platform/desktop/SingleInstanceService.h"
#include "shell/ApplicationController.h"
#include "shell/ModuleManager.h"
#include "ui/ApplicationTheme.h"
#include "ui/PagePreviewItem.h"
#include "ui/ThumbnailImageProvider.h"
#include "viewer/ViewerState.h"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileOpenEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSettings>
#include <QStyleHints>
#include <QSysInfo>
#include <QTextStream>
#include <QUrl>
#include <QVariantList>

#include <functional>
#include <utility>

Q_LOGGING_CATEGORY(logStartup, "impage.startup")

namespace {

class PurrViewApplication final : public QApplication {
  public:
    using QApplication::QApplication;

    void setFileOpenHandler(std::function<void(const QString&)> handler) {
        fileOpenHandler_ = std::move(handler);
        for (const QString& file : std::as_const(pendingFiles_)) {
            fileOpenHandler_(file);
        }
        pendingFiles_.clear();
    }

  protected:
    bool event(QEvent* event) override {
        if (event->type() == QEvent::FileOpen) {
            const auto* openEvent = static_cast<QFileOpenEvent*>(event);
            const QString file =
                !openEvent->file().isEmpty() ? openEvent->file() : openEvent->url().toLocalFile();
            if (!file.isEmpty()) {
                if (fileOpenHandler_) {
                    fileOpenHandler_(file);
                } else {
                    pendingFiles_.push_back(file);
                }
                return true;
            }
        }
        return QApplication::event(event);
    }

  private:
    std::function<void(const QString&)> fileOpenHandler_;
    QStringList pendingFiles_;
};

void migrateLegacySettings() {
    QSettings currentSettings;
    if (currentSettings.value(QStringLiteral("identity/impageSettingsImported"), false).toBool()) {
        return;
    }

    QSettings legacySettings(QSettings::NativeFormat, QSettings::UserScope,
                             QStringLiteral("Impage"), QStringLiteral("Impage"));
    for (const QString& key : legacySettings.allKeys()) {
        if (!currentSettings.contains(key)) {
            currentSettings.setValue(key, legacySettings.value(key));
        }
    }
    currentSettings.setValue(QStringLiteral("identity/impageSettingsImported"), true);
}

} // namespace

int main(int argc, char* argv[]) {
    QElapsedTimer startupTimer;
    startupTimer.start();
    QCoreApplication::setApplicationName(QStringLiteral("PurrView"));
    QCoreApplication::setOrganizationName(QStringLiteral("PurrView"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("io.github.impage"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(IMPAGE_VERSION_STRING));

    QStringList arguments;
    arguments.reserve(argc);
    for (int index = 0; index < argc; ++index) {
        arguments.push_back(QString::fromLocal8Bit(argv[index]));
    }

    const impage::desktop::CommandLineResult commandLine =
        impage::desktop::parseCommandLine(arguments, QDir::currentPath());
    if (!commandLine.valid || commandLine.showHelp || commandLine.showVersion) {
        QTextStream stream(commandLine.valid ? stdout : stderr);
        stream << commandLine.output;
        return commandLine.valid ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    QQuickStyle::setStyle(QStringLiteral("Fusion"));
    PurrViewApplication application(argc, argv);
    bool darkMode = impage::ui::paletteIsDark(application.palette());
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const Qt::ColorScheme systemScheme = application.styleHints()->colorScheme();
    if (systemScheme != Qt::ColorScheme::Unknown) {
        darkMode = systemScheme == Qt::ColorScheme::Dark;
    }
#endif
    application.setPalette(impage::ui::createPurrViewPalette(darkMode));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QObject::connect(application.styleHints(), &QStyleHints::colorSchemeChanged, &application,
                     [&application](Qt::ColorScheme scheme) {
                         if (scheme != Qt::ColorScheme::Unknown) {
                             application.setPalette(impage::ui::createPurrViewPalette(
                                 scheme == Qt::ColorScheme::Dark));
                         }
                     });
#endif
    migrateLegacySettings();
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.impage.Impage"));
    application.setWindowIcon(
        QIcon(QStringLiteral(":/qt/qml/Impage/assets/purrview-window.png")));

    impage::desktop::SingleInstanceService singleInstance;
    const auto instanceResult = singleInstance.startOrForward(commandLine.request);
    if (instanceResult == impage::desktop::SingleInstanceService::StartResult::Forwarded) {
        return EXIT_SUCCESS;
    }
    if (instanceResult == impage::desktop::SingleInstanceService::StartResult::Error) {
        QTextStream(stderr) << QStringLiteral("PurrView: %1\n").arg(singleInstance.errorString());
        return EXIT_FAILURE;
    }

    qmlRegisterUncreatableType<impage::shell::ModuleManager>(
        "Impage", 1, 0, "ModuleManager", QStringLiteral("Managed by the PurrView shell"));
    qmlRegisterUncreatableType<impage::viewer::ViewerState>(
        "Impage", 1, 0, "ViewerState", QStringLiteral("Managed by ViewerController"));
    qmlRegisterType<impage::ui::PagePreviewItem>("Impage", 1, 0, "PagePreviewItem");

    impage::shell::ApplicationController shellController;
    QObject::connect(&singleInstance, &impage::desktop::SingleInstanceService::requestReceived,
                     &shellController,
                     [&shellController](const impage::desktop::OpenRequest& request) {
                         (void)shellController.handleOpenRequest(request);
                     });
    QObject::connect(&singleInstance, &impage::desktop::SingleInstanceService::protocolError,
                     &application, [](const QString& error) { qWarning().noquote() << error; });
    application.setFileOpenHandler([&shellController](const QString& file) {
        (void)shellController.handleOpenRequest(
            {.mode = impage::desktop::OpenMode::Auto, .files = {file}});
    });
    if (!shellController.handleOpenRequest(commandLine.request)) {
        shellController.openComposer();
    }
    qCInfo(logStartup) << "Application services ready in" << startupTimer.elapsed() << "ms";

    QQmlApplicationEngine engine;
    engine.addImageProvider(
        QStringLiteral("impage-thumbnail"),
        new impage::ui::ThumbnailImageProvider(*shellController.context()->thumbnailCache()));
    const QVariantMap aboutInfo{
        {QStringLiteral("applicationVersion"), QCoreApplication::applicationVersion()},
        {QStringLiteral("qtVersion"), QString::fromLatin1(qVersion())},
        {QStringLiteral("operatingSystem"), QSysInfo::prettyProductName()},
        {QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture()},
        {QStringLiteral("projectUrl"),
         QStringLiteral("https://github.com/guedessoftware/purrview")},
    };
    engine.setInitialProperties({
        {QStringLiteral("applicationController"), QVariant::fromValue(&shellController)},
        {QStringLiteral("aboutInfo"), aboutInfo},
    });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &application,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Impage/ShellWindow.qml")));
    qCInfo(logStartup) << "Shell loaded in" << startupTimer.elapsed() << "ms";

    return application.exec();
}
