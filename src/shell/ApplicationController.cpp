#include "shell/ApplicationController.h"

#include <QFileInfo>
#include <QSettings>
#include <QUrl>

namespace impage::shell {

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent), moduleManager_(context_), openRequestHandler_(moduleManager_) {
    connect(&moduleManager_, &ModuleManager::currentModuleChanged, this,
            &ApplicationController::windowTitleChanged);
    connect(&moduleManager_, &ModuleManager::viewerControllerChanged, this, [this] {
        connectViewerTitle();
        emit windowTitleChanged();
    });
    connect(&openRequestHandler_, &OpenRequestHandler::errorOccurred, this,
            [this](const QString& message) {
                lastOpenRequestError_ = message;
                emit openRequestError(message);
            });
}

ApplicationContext* ApplicationController::context() {
    return &context_;
}

ModuleManager* ApplicationController::modules() {
    return &moduleManager_;
}

QString ApplicationController::windowTitle() const {
    if (moduleManager_.viewerActive()) {
        const auto* viewer = moduleManager_.viewerController();
        const QString name = viewer == nullptr ? QString() : viewer->currentFileName();
        return name.isEmpty() ? QStringLiteral("PurrView")
                              : QStringLiteral("%1 — PurrView").arg(name);
    }
    return QStringLiteral("PurrView");
}

int ApplicationController::savedWindowWidth() const {
    return QSettings().value(QStringLiteral("window/width"), 1180).toInt();
}

int ApplicationController::savedWindowHeight() const {
    return QSettings().value(QStringLiteral("window/height"), 780).toInt();
}

bool ApplicationController::savedWindowMaximized() const {
    return QSettings().value(QStringLiteral("window/maximized"), false).toBool();
}

QString ApplicationController::lastOpenRequestError() const {
    return lastOpenRequestError_;
}

bool ApplicationController::handleOpenRequest(const desktop::OpenRequest& request) {
    const bool opened = openRequestHandler_.handle(request);
    if (opened && request.activateWindow) {
        emit windowActivationRequested();
    }
    return opened;
}

void ApplicationController::openComposer(const QVariantList& images) {
    moduleManager_.showComposer(images);
}

void ApplicationController::openViewer(const QVariantList& images) {
    moduleManager_.showViewer(images);
}

void ApplicationController::openDroppedInComposer(const QVariantList& images) {
    (void)handleOpenRequest(requestFromUrls(images, desktop::OpenMode::Composer));
}

void ApplicationController::openDroppedInViewer(const QVariantList& images) {
    (void)handleOpenRequest(requestFromUrls(images, desktop::OpenMode::Viewer));
}

void ApplicationController::goBack() {
    moduleManager_.goBack();
}

void ApplicationController::saveWindowState(int width, int height, bool maximized) {
    QSettings settings;
    if (!maximized && width >= 860 && height >= 600) {
        settings.setValue(QStringLiteral("window/width"), width);
        settings.setValue(QStringLiteral("window/height"), height);
    }
    settings.setValue(QStringLiteral("window/maximized"), maximized);
}

void ApplicationController::clearOpenRequestError() {
    lastOpenRequestError_.clear();
}

desktop::OpenRequest ApplicationController::requestFromUrls(const QVariantList& images,
                                                            desktop::OpenMode mode) const {
    desktop::OpenRequest request;
    request.mode = mode;
    request.activateWindow = false;
    for (const QVariant& image : images) {
        const QUrl url = image.toUrl();
        request.files.push_back(url.isLocalFile() ? url.toLocalFile() : url.toString());
    }
    return request;
}

void ApplicationController::connectViewerTitle() {
    if (auto* viewer = moduleManager_.viewerController()) {
        connect(viewer, &viewer::ViewerController::currentImageChanged, this,
                &ApplicationController::windowTitleChanged, Qt::UniqueConnection);
    }
}

} // namespace impage::shell
