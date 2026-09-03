#include "core/layout/ImagePlacementEngine.h"

#include <algorithm>

namespace impage::core {

ImagePlacement ImagePlacementEngine::calculate(const QSizeF& imageSize, const QRectF& cellRect,
                                               PlacementMode mode) const {
    ImagePlacement result;
    if (imageSize.width() <= 0.0 || imageSize.height() <= 0.0 || cellRect.width() <= 0.0 ||
        cellRect.height() <= 0.0) {
        return result;
    }

    result.sourceRect = QRectF(QPointF(0.0, 0.0), imageSize);
    result.targetRect = cellRect;

    if (mode == PlacementMode::Fit) {
        const double scale =
            std::min(cellRect.width() / imageSize.width(), cellRect.height() / imageSize.height());
        const QSizeF fitted(imageSize.width() * scale, imageSize.height() * scale);
        result.targetRect =
            QRectF(cellRect.center().x() - fitted.width() / 2.0,
                   cellRect.center().y() - fitted.height() / 2.0, fitted.width(), fitted.height());
    } else if (mode == PlacementMode::Fill) {
        const double cellRatio = cellRect.width() / cellRect.height();
        const double imageRatio = imageSize.width() / imageSize.height();
        if (imageRatio > cellRatio) {
            const double cropWidth = imageSize.height() * cellRatio;
            result.sourceRect.setX((imageSize.width() - cropWidth) / 2.0);
            result.sourceRect.setWidth(cropWidth);
        } else {
            const double cropHeight = imageSize.width() / cellRatio;
            result.sourceRect.setY((imageSize.height() - cropHeight) / 2.0);
            result.sourceRect.setHeight(cropHeight);
        }
    }

    result.valid = true;
    return result;
}

} // namespace impage::core
