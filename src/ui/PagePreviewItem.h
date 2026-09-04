#pragma once

#include "composer/ComposerController.h"

#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

namespace purrview::ui {

class PagePreviewItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* controller READ controllerObject WRITE setControllerObject NOTIFY
                   controllerChanged)

  public:
    explicit PagePreviewItem(QQuickItem* parent = nullptr);

    [[nodiscard]] composer::ComposerController* controller() const;
    void setController(composer::ComposerController* controller);
    [[nodiscard]] QObject* controllerObject() const;
    void setControllerObject(QObject* controller);
    void paint(QPainter* painter) override;

  signals:
    void controllerChanged();

  private:
    composer::ComposerController* controller_ = nullptr;
};

} // namespace purrview::ui
