#include "shell/ApplicationContext.h"
#include "shell/ModuleManager.h"

#include <QApplication>
#include <QImage>
#include <QTemporaryDir>
#include <QUrl>

#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    purrview::shell::ApplicationContext context;
    purrview::shell::ModuleManager modules(context);

    check(modules.currentModule() == purrview::shell::ModuleManager::Module::None,
          "shell starts without an active module");
    check(modules.composerController() == nullptr,
          "composer backend is not constructed with the shell");
    check(modules.viewerController() == nullptr,
          "viewer backend is not constructed with the shell");
    check(modules.viewerState() == purrview::shell::ModuleManager::ModuleState::NotLoaded &&
              modules.composerState() == purrview::shell::ModuleManager::ModuleState::NotLoaded,
          "module lifecycle starts in NotLoaded state");

    QTemporaryDir directory;
    check(directory.isValid(), "temporary source directory is available");
    const QString sourcePath = directory.filePath(QStringLiteral("startup.png"));
    QImage source(64, 48, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(25, 100, 180));
    check(source.save(sourcePath), "startup image fixture can be saved");
    const QString secondPath = directory.filePath(QStringLiteral("selected.png"));
    source.fill(QColor(180, 80, 25));
    check(source.save(secondPath), "selected image fixture can be saved");

    const QList<purrview::core::ImageId> sessionIds =
        context.imageSession()->addImages({sourcePath, secondPath});
    check(context.imageSession()->selectImage(sessionIds.at(1)),
          "future module can prepare a shared selection");

    modules.showViewer();
    auto* firstViewerController = modules.viewerController();
    check(firstViewerController != nullptr, "viewer backend is created on demand");
    check(modules.composerController() == nullptr,
          "opening viewer does not construct composer backend");
    check(modules.currentModule() == purrview::shell::ModuleManager::Module::Viewer,
          "viewer becomes the active module");
    check(modules.viewerActive() && !modules.composerActive(),
          "shell exposes the active viewer state to QML");
    check(modules.viewerState() == purrview::shell::ModuleManager::ModuleState::Active &&
              modules.composerState() == purrview::shell::ModuleManager::ModuleState::NotLoaded &&
              modules.composerLoadCount() == 0,
          "Viewer-only startup leaves Composer services genuinely unloaded");
    check(firstViewerController->currentImageUrl() == QUrl::fromLocalFile(sourcePath),
          "viewer receives the current shared image");

    firstViewerController->setCustomZoom(1.6);
    firstViewerController->toggleInfoPanel();
    firstViewerController->toggleFilmstrip();
    firstViewerController->captureViewState(31.0, -17.0, 88.0);
    firstViewerController->openComposer();
    auto* firstController = modules.composerController();
    check(firstController != nullptr, "composer backend is created on demand");
    check(modules.currentModule() == purrview::shell::ModuleManager::Module::Composer,
          "composer becomes the active module");
    check(modules.composerActive() && !modules.viewerActive(),
          "shell exposes the active composer state to QML");
    check(modules.viewerState() == purrview::shell::ModuleManager::ModuleState::Inactive &&
              modules.composerState() == purrview::shell::ModuleManager::ModuleState::Active &&
              modules.composerLoadCount() == 1 && modules.composerLoadDurationMs() >= 0 &&
              modules.composerActivationDurationMs() >= 0,
          "Composer loads once, becomes active and records transition metrics");
    check(firstController != nullptr && firstController->imageCount() == 1,
          "preexisting shared selection is transferred to the composer");
    check(firstController != nullptr &&
              firstController->imageThumbnails().constFirst().toMap().value("source").toUrl() ==
                  QUrl::fromLocalFile(secondPath),
          "composer receives the selected image in session order");
    check(firstController != nullptr && firstController->imageSession() == context.imageSession(),
          "composer receives the shell-owned image session");

    context.imageSession()->clearSelection();
    check(firstController->imageCount() == 1,
          "inactive Viewer selection changes do not mutate the explicit composition");

    check(modules.goBack(), "back navigation returns from Composer to Viewer");
    check(modules.viewerController() == firstViewerController,
          "viewer backend is retained when returning from composer");
    check(modules.currentModule() == purrview::shell::ModuleManager::Module::Viewer,
          "shell can return to viewer in the same process");
    check(modules.viewerController()->currentImageUrl() == QUrl::fromLocalFile(sourcePath),
          "viewer current image survives module transitions");
    check(context.imageSession()->selectedCount() == 1 &&
              firstViewerController->state()->zoomFactor() == 1.6 &&
              firstViewerController->state()->infoPanelVisible() &&
              !firstViewerController->state()->filmstripVisible() &&
              firstViewerController->savedPanX() == 31.0 &&
              firstViewerController->savedPanY() == -17.0 &&
              firstViewerController->savedFilmstripContentX() == 88.0,
          "back navigation restores Viewer selection and visual navigation state");

    context.imageSession()->clearSelection();
    firstViewerController->openComposer();
    check(modules.composerController() == firstController,
          "composer backend is retained when reopened");
    check(modules.composerController()->imageCount() == 1 &&
              modules.composerController()
                      ->imageThumbnails()
                      .constFirst()
                      .toMap()
                      .value("source")
                      .toUrl() == QUrl::fromLocalFile(sourcePath),
          "second activation uses the current image when no selection exists");
    check(modules.composerLoadCount() == 1,
          "second activation reuses Composer without reinitializing its services");

    purrview::shell::ApplicationContext failureContext;
    purrview::shell::ModuleManager failureModules(failureContext);
    const QList<purrview::core::ImageId> failureImages =
        failureContext.imageSession()->addImages({sourcePath});
    Q_UNUSED(failureImages)
    failureModules.showViewer();
    failureModules.setComposerFactoryForTesting([](purrview::core::ImageSession&) {
        return std::unique_ptr<purrview::composer::ComposerController>{};
    });
    failureModules.viewerController()->openComposer();
    check(failureModules.currentModule() == purrview::shell::ModuleManager::Module::Viewer &&
              failureModules.composerState() == purrview::shell::ModuleManager::ModuleState::Error &&
              failureModules.viewerController()->state()->errorString() ==
                  QStringLiteral("Não foi possível abrir o módulo de impressão."),
          "Composer load failure leaves Viewer active with a recoverable message");

    purrview::shell::ApplicationContext externalContext;
    purrview::shell::ModuleManager externalModules(externalContext);
    externalModules.showComposer({QUrl::fromLocalFile(sourcePath)});
    auto* preservedComposer = externalModules.composerController();
    const int preservedImageCount = preservedComposer->imageCount();
    externalModules.showViewer({QUrl::fromLocalFile(secondPath)});
    check(externalModules.currentModule() == purrview::shell::ModuleManager::Module::Viewer &&
              externalModules.viewerController()->currentImageUrl() ==
                  QUrl::fromLocalFile(secondPath),
          "external Viewer request from Composer displays the requested first image");
    check(externalModules.goBack() && externalModules.composerController() == preservedComposer &&
              preservedComposer->imageCount() == preservedImageCount,
          "returning from an external Viewer request preserves the Composer document");

    return failures == 0 ? 0 : 1;
}
