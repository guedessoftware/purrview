#include "shell/ModuleManager.h"

#include <QElapsedTimer>
#include <QLoggingCategory>

#include <exception>

Q_LOGGING_CATEGORY(logNavigation, "impage.navigation")

namespace impage::shell {

ModuleManager::ModuleManager(ApplicationContext& context, QObject* parent)
    : QObject(parent), context_(context) {}

ModuleManager::Module ModuleManager::currentModule() const {
    return currentModule_;
}

bool ModuleManager::viewerActive() const {
    return currentModule_ == Module::Viewer;
}

bool ModuleManager::composerActive() const {
    return currentModule_ == Module::Composer;
}

ModuleManager::ModuleState ModuleManager::viewerState() const {
    return viewerState_;
}

ModuleManager::ModuleState ModuleManager::composerState() const {
    return composerState_;
}

bool ModuleManager::canGoBack() const {
    return !backStack_.isEmpty() || currentModule_ == Module::Composer;
}

int ModuleManager::composerLoadCount() const {
    return composerLoadCount_;
}

qint64 ModuleManager::composerLoadDurationMs() const {
    return composerLoadDurationMs_;
}

qint64 ModuleManager::composerActivationDurationMs() const {
    return composerActivationDurationMs_;
}

composer::ComposerController* ModuleManager::composerController() const {
    return composerController_.get();
}

viewer::ViewerController* ModuleManager::viewerController() const {
    return viewerController_.get();
}

bool ModuleManager::showComposer(const QVariantList& images) {
    if (!ensureComposer()) {
        return false;
    }

    if (!images.isEmpty()) {
        composerController_->addImages(images);
    }
    activateModule(Module::Composer, true);
    return true;
}

bool ModuleManager::showComposer(const core::ComposerActivationContext& context) {
    if (!ensureComposer()) {
        if (viewerController_) {
            viewerController_->state()->setError(
                QStringLiteral("Não foi possível abrir o módulo de impressão."));
        }
        return false;
    }

    QElapsedTimer timer;
    timer.start();
    composerController_->activate(context);
    composerActivationDurationMs_ = timer.elapsed();
    qCDebug(logNavigation) << "Composer activation completed in" << composerActivationDurationMs_
                           << "ms for" << context.imageIds.size() << "image(s)";
    emit moduleStateChanged();
    activateModule(Module::Composer, true);
    return true;
}

bool ModuleManager::showViewer(const QVariantList& images) {
    if (currentModule_ == Module::Composer && !images.isEmpty() && composerController_) {
        composerController_->preserveDocumentImages();
    }
    if (!ensureViewer()) {
        return false;
    }

    if (!images.isEmpty()) {
        viewerController_->openImages(images);
    }
    if (currentModule_ == Module::Composer && images.isEmpty()) {
        QElapsedTimer timer;
        timer.start();
        viewerController_->restoreNavigationState();
        qCDebug(logNavigation) << "Viewer restore completed in" << timer.elapsed() << "ms";
    }
    activateModule(Module::Viewer, true);
    return true;
}

bool ModuleManager::goBack() {
    Module destination = Module::None;
    if (!backStack_.isEmpty()) {
        destination = backStack_.takeLast();
    } else if (currentModule_ == Module::Composer) {
        destination = Module::Viewer;
    }
    emit navigationChanged();

    if (destination == Module::Viewer && ensureViewer()) {
        QElapsedTimer timer;
        timer.start();
        viewerController_->restoreNavigationState();
        qCDebug(logNavigation) << "Viewer restore completed in" << timer.elapsed() << "ms";
        activateModule(Module::Viewer, false);
        return true;
    }
    if (destination == Module::Composer && ensureComposer()) {
        activateModule(Module::Composer, false);
        return true;
    }
    return false;
}

void ModuleManager::setComposerFactoryForTesting(ComposerFactory factory) {
    if (!composerController_) {
        composerFactory_ = std::move(factory);
    }
}

bool ModuleManager::ensureComposer() {
    if (composerController_) {
        return true;
    }
    setComposerState(ModuleState::Loading);
    QElapsedTimer timer;
    timer.start();
    try {
        composerController_ =
            composerFactory_
                ? composerFactory_(*context_.imageSession())
                : std::make_unique<composer::ComposerController>(*context_.imageSession());
    } catch (const std::exception& error) {
        qCWarning(logNavigation) << "Composer load failed:" << error.what();
    } catch (...) {
        qCWarning(logNavigation) << "Composer load failed with an unknown exception";
    }
    composerLoadDurationMs_ = timer.elapsed();
    if (!composerController_) {
        setComposerState(ModuleState::Error);
        emit moduleLoadFailed(Module::Composer,
                              QStringLiteral("Não foi possível abrir o módulo de impressão."));
        return false;
    }
    ++composerLoadCount_;
    qCDebug(logNavigation) << "Composer loaded in" << composerLoadDurationMs_ << "ms";
    setComposerState(ModuleState::Ready);
    emit composerControllerChanged();
    return true;
}

bool ModuleManager::ensureViewer() {
    if (viewerController_) {
        return true;
    }
    setViewerState(ModuleState::Loading);
    viewerController_ = std::make_unique<viewer::ViewerController>(
        *context_.imageSession(), *context_.thumbnailCache(), *context_.imageMetadataService());
    connect(viewerController_.get(), &viewer::ViewerController::composerActivationRequested, this,
            [this](const core::ComposerActivationContext& context) { showComposer(context); });
    setViewerState(ModuleState::Ready);
    emit viewerControllerChanged();
    return true;
}

void ModuleManager::activateModule(Module module, bool addToHistory) {
    if (currentModule_ == module) {
        if (module == Module::Viewer) {
            setViewerState(ModuleState::Active);
        } else if (module == Module::Composer) {
            setComposerState(ModuleState::Active);
        }
        return;
    }
    if (addToHistory && currentModule_ != Module::None &&
        (backStack_.isEmpty() || backStack_.constLast() != currentModule_)) {
        backStack_.push_back(currentModule_);
    }
    if (currentModule_ == Module::Viewer) {
        setViewerState(ModuleState::Inactive);
    } else if (currentModule_ == Module::Composer) {
        setComposerState(ModuleState::Inactive);
    }
    currentModule_ = module;
    if (module == Module::Viewer) {
        setViewerState(ModuleState::Active);
    } else if (module == Module::Composer) {
        setComposerState(ModuleState::Active);
    }
    emit currentModuleChanged();
    emit navigationChanged();
}

void ModuleManager::setViewerState(ModuleState state) {
    if (viewerState_ != state) {
        viewerState_ = state;
        emit moduleStateChanged();
    }
}

void ModuleManager::setComposerState(ModuleState state) {
    if (composerState_ != state) {
        composerState_ = state;
        emit moduleStateChanged();
    }
}

} // namespace impage::shell
