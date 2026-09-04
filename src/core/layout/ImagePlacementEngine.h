#pragma once

#include "core/document/ImageItem.h"

#include <QRectF>
#include <QSizeF>

namespace purrview::core {

struct ImagePlacement {
    QRectF targetRect;
    QRectF sourceRect;
    bool valid = false;
};

class ImagePlacementEngine {
  public:
    [[nodiscard]] ImagePlacement calculate(const QSizeF& imageSize, const QRectF& cellRect,
                                           PlacementMode mode) const;
};

} // namespace purrview::core
