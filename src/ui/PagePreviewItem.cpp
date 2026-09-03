#include "ui/PagePreviewItem.h"

#include <QPainter>

namespace impage::ui {

PagePreviewItem::PagePreviewItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
}

composer::ComposerController* PagePreviewItem::controller() const {
    return controller_;
}

QObject* PagePreviewItem::controllerObject() const {
    return controller_;
}

void PagePreviewItem::setControllerObject(QObject* controller) {
    setController(qobject_cast<composer::ComposerController*>(controller));
}

void PagePreviewItem::setController(composer::ComposerController* controller) {
    if (controller_ == controller) {
        return;
    }
    if (controller_ != nullptr) {
        disconnect(controller_, nullptr, this, nullptr);
    }
    controller_ = controller;
    if (controller_ != nullptr) {
        connect(controller_, &composer::ComposerController::documentChanged, this,
                [this] { update(); });
        connect(controller_, &composer::ComposerController::currentPageChanged, this,
                [this] { update(); });
    }
    emit controllerChanged();
    update();
}

void PagePreviewItem::paint(QPainter* painter) {
    if (controller_ == nullptr) {
        painter->fillRect(boundingRect(), Qt::white);
        return;
    }
    controller_->paintPreview(*painter, boundingRect());
}

} // namespace impage::ui
