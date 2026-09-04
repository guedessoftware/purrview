#pragma once

#include "core/document/DocumentModel.h"
#include "core/layout/ImagePlacementEngine.h"
#include "core/layout/LayoutEngine.h"

#include <QCache>
#include <QImage>
#include <QPainter>

namespace purrview::core {

class PageRenderer {
  public:
    enum class Purpose { Preview, Print };
    static constexpr int DefaultMaximumCacheCostBytes = 192 * 1024 * 1024;

    PageRenderer();

    [[nodiscard]] bool render(QPainter& painter, const DocumentModel& document,
                              const QRectF& targetRect, Purpose purpose, int pageIndex = 0);
    void clearCache();
    [[nodiscard]] qsizetype maximumCacheCostBytes() const;
    [[nodiscard]] qsizetype currentCacheCostBytes() const;
    void setMaximumCacheCostBytes(qsizetype bytes);

  private:
    [[nodiscard]] QImage loadImage(const QString& path, int rotationDegrees,
                                   const QSize& targetSize, PlacementMode placementMode);

    LayoutEngine layoutEngine_;
    ImagePlacementEngine placementEngine_;
    QCache<QString, QImage> imageCache_;
};

} // namespace purrview::core
