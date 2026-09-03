#pragma once

#include "composer/ComposerController.h"
#include "shell/ApplicationContext.h"
#include "viewer/ViewerController.h"

#include <QList>
#include <QObject>
#include <QVariantList>

#include <functional>
#include <memory>

namespace impage::shell {

class ModuleManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(Module currentModule READ currentModule NOTIFY currentModuleChanged)
    Q_PROPERTY(bool viewerActive READ viewerActive NOTIFY currentModuleChanged)
    Q_PROPERTY(bool composerActive READ composerActive NOTIFY currentModuleChanged)
    Q_PROPERTY(ModuleState viewerState READ viewerState NOTIFY moduleStateChanged)
    Q_PROPERTY(ModuleState composerState READ composerState NOTIFY moduleStateChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationChanged)
    Q_PROPERTY(int composerLoadCount READ composerLoadCount NOTIFY moduleStateChanged)
    Q_PROPERTY(qint64 composerLoadDurationMs READ composerLoadDurationMs NOTIFY moduleStateChanged)
    Q_PROPERTY(qint64 composerActivationDurationMs READ composerActivationDurationMs NOTIFY
                   moduleStateChanged)
    Q_PROPERTY(impage::composer::ComposerController* composerController READ composerController
                   NOTIFY composerControllerChanged)
    Q_PROPERTY(impage::viewer::ViewerController* viewerController READ viewerController NOTIFY
                   viewerControllerChanged)

  public:
    enum class Module { None, Viewer, Composer };
    Q_ENUM(Module)
    enum class ModuleState { NotLoaded, Loading, Ready, Active, Inactive, Error };
    Q_ENUM(ModuleState)

    explicit ModuleManager(ApplicationContext& context, QObject* parent = nullptr);

    [[nodiscard]] Module currentModule() const;
    [[nodiscard]] bool viewerActive() const;
    [[nodiscard]] bool composerActive() const;
    [[nodiscard]] ModuleState viewerState() const;
    [[nodiscard]] ModuleState composerState() const;
    [[nodiscard]] bool canGoBack() const;
    [[nodiscard]] int composerLoadCount() const;
    [[nodiscard]] qint64 composerLoadDurationMs() const;
    [[nodiscard]] qint64 composerActivationDurationMs() const;
    [[nodiscard]] composer::ComposerController* composerController() const;
    [[nodiscard]] viewer::ViewerController* viewerController() const;

    bool showComposer(const QVariantList& images = {});
    bool showComposer(const core::ComposerActivationContext& context);
    bool showViewer(const QVariantList& images = {});
    bool goBack();

    using ComposerFactory =
        std::function<std::unique_ptr<composer::ComposerController>(core::ImageSession&)>;
    void setComposerFactoryForTesting(ComposerFactory factory);

  signals:
    void currentModuleChanged();
    void composerControllerChanged();
    void viewerControllerChanged();
    void moduleStateChanged();
    void navigationChanged();
    void moduleLoadFailed(Module module, const QString& message);

  private:
    [[nodiscard]] bool ensureComposer();
    [[nodiscard]] bool ensureViewer();
    void activateModule(Module module, bool addToHistory);
    void setViewerState(ModuleState state);
    void setComposerState(ModuleState state);

    ApplicationContext& context_;
    std::unique_ptr<composer::ComposerController> composerController_;
    std::unique_ptr<viewer::ViewerController> viewerController_;
    Module currentModule_ = Module::None;
    ModuleState viewerState_ = ModuleState::NotLoaded;
    ModuleState composerState_ = ModuleState::NotLoaded;
    QList<Module> backStack_;
    ComposerFactory composerFactory_;
    int composerLoadCount_ = 0;
    qint64 composerLoadDurationMs_ = 0;
    qint64 composerActivationDurationMs_ = 0;
};

} // namespace impage::shell
