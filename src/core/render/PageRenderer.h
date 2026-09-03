#pragma once

#include "core/document/DocumentModel.h"
#include "core/layout/ImagePlacementEngine.h"
#include "core/layout/LayoutEngine.h"

#include <QHash>
#include <QImage>
#include <QPainter>

namespace impage::core {

class PageRenderer {
  public:
    enum class Purpose { Preview, Print };

    [[nodiscard]] bool render(QPainter& painter, const DocumentModel& document,
                              const QRectF& targetRect, Purpose purpose, int pageIndex = 0);
    void clearCache();

  private:
    [[nodiscard]] const QImage& loadImage(const QString& path, int rotationDegrees);

    LayoutEngine layoutEngine_;
    ImagePlacementEngine placementEngine_;
    QHash<QString, QImage> imageCache_;
};

} // namespace impage::core
