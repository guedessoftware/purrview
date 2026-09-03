#include "viewer/ViewerState.h"

#include <algorithm>
#include <cmath>

namespace impage::viewer {

ViewerState::ViewerState(QObject* parent) : QObject(parent) {
    inactivityTimer_.setSingleShot(true);
    inactivityTimer_.setInterval(2700);
    connect(&inactivityTimer_, &QTimer::timeout, this, [this] {
        if (!fullScreen_ || interactionBlocked_) {
            return;
        }
        controlsVisible_ = false;
        cursorVisible_ = false;
        emit interactionChanged();
    });
}

ViewerState::ZoomMode ViewerState::zoomMode() const {
    return zoomMode_;
}

double ViewerState::zoomFactor() const {
    return zoomFactor_;
}

bool ViewerState::fitMode() const {
    return zoomMode_ == ZoomMode::Fit;
}

bool ViewerState::filmstripVisible() const {
    return filmstripVisible_;
}

bool ViewerState::infoPanelVisible() const {
    return infoPanelVisible_;
}

bool ViewerState::toolbarPinned() const {
    return toolbarPinned_;
}

QString ViewerState::errorString() const {
    return errorString_;
}

bool ViewerState::fullScreen() const {
    return fullScreen_;
}

bool ViewerState::controlsVisible() const {
    return controlsVisible_;
}

bool ViewerState::cursorVisible() const {
    return cursorVisible_;
}

bool ViewerState::interactionBlocked() const {
    return interactionBlocked_;
}

void ViewerState::updateFitScale(double scale) {
    if (zoomMode_ != ZoomMode::Fit || !std::isfinite(scale) || scale <= 0.0) {
        return;
    }
    setZoom(ZoomMode::Fit, scale);
}

void ViewerState::setCustomZoom(double scale) {
    if (!std::isfinite(scale)) {
        return;
    }
    setZoom(ZoomMode::Custom, std::clamp(scale, MinimumCustomZoom, MaximumCustomZoom));
}

void ViewerState::zoomIn() {
    setCustomZoom(zoomFactor_ * ZoomStep);
}

void ViewerState::zoomOut() {
    setCustomZoom(zoomFactor_ / ZoomStep);
}

void ViewerState::fitToWindow() {
    const bool modeChanged = zoomMode_ != ZoomMode::Fit;
    zoomMode_ = ZoomMode::Fit;
    if (modeChanged) {
        emit zoomChanged();
    }
    emit panResetRequested();
}

void ViewerState::actualSize() {
    setZoom(ZoomMode::ActualSize, 1.0);
    emit panResetRequested();
}

void ViewerState::resetForImage() {
    clearError();
    fitToWindow();
}

void ViewerState::requestPanReset() {
    emit panResetRequested();
}

void ViewerState::setError(const QString& error) {
    if (error == errorString_) {
        return;
    }
    errorString_ = error;
    emit errorChanged();
}

void ViewerState::clearError() {
    setError({});
}

void ViewerState::setFilmstripVisible(bool visible) {
    if (filmstripVisible_ == visible) {
        return;
    }
    filmstripVisible_ = visible;
    emit filmstripVisibleChanged();
    requestPanReset();
}

void ViewerState::toggleFilmstrip() {
    setFilmstripVisible(!filmstripVisible_);
}

void ViewerState::setInfoPanelVisible(bool visible) {
    if (infoPanelVisible_ == visible) {
        return;
    }
    infoPanelVisible_ = visible;
    emit infoPanelVisibleChanged();
}

void ViewerState::toggleInfoPanel() {
    setInfoPanelVisible(!infoPanelVisible_);
}

void ViewerState::setToolbarPinned(bool pinned) {
    if (toolbarPinned_ == pinned) {
        return;
    }
    toolbarPinned_ = pinned;
    emit toolbarPinnedChanged();
}

void ViewerState::toggleToolbarPinned() {
    setToolbarPinned(!toolbarPinned_);
}

void ViewerState::restoreZoom(ZoomMode mode, double factor) {
    if (mode == ZoomMode::Fit) {
        zoomMode_ = ZoomMode::Fit;
        zoomFactor_ = std::isfinite(factor) && factor > 0.0 ? factor : 1.0;
        emit zoomChanged();
        return;
    }
    if (mode == ZoomMode::ActualSize) {
        setZoom(ZoomMode::ActualSize, 1.0);
        return;
    }
    setZoom(ZoomMode::Custom, std::clamp(factor, MinimumCustomZoom, MaximumCustomZoom));
}

void ViewerState::setFullScreen(bool fullScreen) {
    if (fullScreen_ == fullScreen) {
        return;
    }
    fullScreen_ = fullScreen;
    controlsVisible_ = true;
    cursorVisible_ = true;
    if (fullScreen_) {
        inactivityTimer_.start();
    } else {
        inactivityTimer_.stop();
    }
    emit fullScreenChanged();
    emit interactionChanged();
}

void ViewerState::setInteractionBlocked(bool blocked) {
    if (interactionBlocked_ == blocked) {
        return;
    }
    interactionBlocked_ = blocked;
    if (blocked) {
        inactivityTimer_.stop();
        controlsVisible_ = true;
        cursorVisible_ = true;
    } else if (fullScreen_) {
        inactivityTimer_.start();
    }
    emit interactionChanged();
}

void ViewerState::notifyActivity() {
    const bool changed = !controlsVisible_ || !cursorVisible_;
    controlsVisible_ = true;
    cursorVisible_ = true;
    if (fullScreen_ && !interactionBlocked_) {
        inactivityTimer_.start();
    }
    if (changed) {
        emit interactionChanged();
    }
}

void ViewerState::setInactivityIntervalForTesting(int milliseconds) {
    inactivityTimer_.setInterval(std::max(1, milliseconds));
}

void ViewerState::setZoom(ZoomMode mode, double factor) {
    if (mode == zoomMode_ && std::abs(factor - zoomFactor_) < 0.0001) {
        return;
    }
    zoomMode_ = mode;
    zoomFactor_ = factor;
    emit zoomChanged();
}

} // namespace impage::viewer
