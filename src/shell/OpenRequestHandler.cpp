#include "shell/OpenRequestHandler.h"

#include "core/image/ImageFormatSupport.h"
#include "shell/ModuleManager.h"

#include <QFileInfo>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logOpenRequest, "impage.openrequest")

namespace impage::shell {

OpenRequestHandler::OpenRequestHandler(ModuleManager& modules, QObject* parent)
    : QObject(parent), modules_(modules) {}

bool OpenRequestHandler::handle(const desktop::OpenRequest& request) {
    QStringList accepted;
    QStringList rejected;
    for (const QString& path : request.files) {
        const QFileInfo file(path);
        if (!file.exists() || !file.isFile() || !file.isReadable()) {
            rejected.push_back(QStringLiteral("Não foi possível acessar: %1").arg(path));
        } else if (!core::isSupportedImageFile(file.absoluteFilePath())) {
            rejected.push_back(
                QStringLiteral("Formato de imagem não suportado: %1").arg(file.fileName()));
        } else {
            accepted.push_back(file.absoluteFilePath());
        }
    }

    if (!request.files.isEmpty() && accepted.isEmpty()) {
        const QString message = rejected.isEmpty()
                                    ? QStringLiteral("Nenhuma imagem válida foi informada.")
                                    : rejected.join(QLatin1Char('\n'));
        qCWarning(logOpenRequest) << message;
        emit errorOccurred(message);
        return false;
    }

    desktop::OpenMode mode = request.mode;
    if (mode == desktop::OpenMode::Auto) {
        mode = accepted.isEmpty() ? desktop::OpenMode::Composer : desktop::OpenMode::Viewer;
    }
    const QVariantList urls = desktop::fileUrls(accepted);
    const bool opened = mode == desktop::OpenMode::Composer ? modules_.showComposer(urls)
                                                            : modules_.showViewer(urls);
    if (!rejected.isEmpty()) {
        emit errorOccurred(rejected.join(QLatin1Char('\n')));
    }
    if (opened) {
        qCInfo(logOpenRequest) << "Handled" << accepted.size() << "file(s) in"
                               << (mode == desktop::OpenMode::Viewer ? "Viewer" : "Composer");
        emit requestHandled(request);
    }
    return opened;
}

} // namespace impage::shell
