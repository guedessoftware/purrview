#include "core/render/PageRenderer.h"

#include "core/image/ImageFormatSupport.h"

#include <QFileInfo>
#include <QImageReader>
#include <QLoggingCategory>
#include <QPen>
#include <QTransform>

#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(logRender, "purrview.render")

namespace purrview::core {
namespace {

int quantizedDimension(qreal value) {
    constexpr int bucket = 256;
    const int pixels = std::max(1, static_cast<int>(std::ceil(value)));
    return ((pixels + bucket - 1) / bucket) * bucket;
}

QSize requestedDecodeSize(const QSize& sourceSize, QSize targetSize, int rotationDegrees,
                          PlacementMode placementMode) {
    if (!sourceSize.isValid() || !targetSize.isValid()) {
        return {};
    }
    if (rotationDegrees % 180 != 0) {
        targetSize.transpose();
    }

    QSize result;
    if (placementMode == PlacementMode::Stretch) {
        result = targetSize;
    } else {
        result = sourceSize.scaled(targetSize, placementMode == PlacementMode::Fill
                                                   ? Qt::KeepAspectRatioByExpanding
                                                   : Qt::KeepAspectRatio);
    }
    // Never spend CPU or memory upscaling while decoding. QPainter performs any
    // necessary enlargement at the final destination.
    if (result.width() > sourceSize.width() || result.height() > sourceSize.height()) {
        return sourceSize;
    }
    return result;
}

qsizetype imageCost(const QImage& image) {
    return std::max<qsizetype>(1, image.sizeInBytes());
}

} // namespace

PageRenderer::PageRenderer() {
    imageCache_.setMaxCost(DefaultMaximumCacheCostBytes);
}

bool PageRenderer::render(QPainter& painter, const DocumentModel& document,
                          const QRectF& targetRect, Purpose purpose, int pageIndex) {
    const PageLayout layout = layoutEngine_.calculate(document);
    if (!layout.isValid() || targetRect.isEmpty() || pageIndex < 0) {
        qCWarning(logRender) << layout.error;
        return false;
    }

    painter.save();
    painter.setClipRect(targetRect);
    painter.fillRect(targetRect, Qt::white);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const double scaleX = targetRect.width() / layout.pageWidthMm;
    const double scaleY = targetRect.height() / layout.pageHeightMm;
    const std::vector<ImageItem>& images = document.images();
    const std::size_t pageOffset = static_cast<std::size_t>(pageIndex) * layout.cellsMm.size();

    for (std::size_t index = 0; index < layout.cellsMm.size(); ++index) {
        const QRectF& cellMm = layout.cellsMm[index];
        const QRectF cellPixels(targetRect.x() + cellMm.x() * scaleX,
                                targetRect.y() + cellMm.y() * scaleY, cellMm.width() * scaleX,
                                cellMm.height() * scaleY);

        const std::size_t imageIndex = pageOffset + index;
        if (imageIndex >= images.size()) {
            if (purpose == Purpose::Preview) {
                QPen guide(QColor(184, 193, 207));
                guide.setStyle(Qt::DashLine);
                guide.setWidthF(1.0);
                painter.setPen(guide);
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(cellPixels);
            }
            continue;
        }

        const ImageItem& item = images[imageIndex];
        const qreal deviceScale = painter.device() == nullptr
                                      ? 1.0
                                      : std::max<qreal>(1.0, painter.device()->devicePixelRatioF());
        const QSize decodeTarget(quantizedDimension(cellPixels.width() * deviceScale),
                                 quantizedDimension(cellPixels.height() * deviceScale));
        const QImage image =
            loadImage(item.source, item.rotationDegrees, decodeTarget, item.placementMode);
        if (image.isNull()) {
            continue;
        }

        const ImagePlacement placement =
            placementEngine_.calculate(image.size(), cellMm, item.placementMode);
        if (!placement.valid) {
            continue;
        }

        const QRectF destination(targetRect.x() + placement.targetRect.x() * scaleX,
                                 targetRect.y() + placement.targetRect.y() * scaleY,
                                 placement.targetRect.width() * scaleX,
                                 placement.targetRect.height() * scaleY);
        painter.save();
        painter.setClipRect(cellPixels);
        painter.drawImage(destination, image, placement.sourceRect);
        painter.restore();
    }

    painter.restore();
    return true;
}

void PageRenderer::clearCache() {
    imageCache_.clear();
}

qsizetype PageRenderer::maximumCacheCostBytes() const {
    return imageCache_.maxCost();
}

qsizetype PageRenderer::currentCacheCostBytes() const {
    return imageCache_.totalCost();
}

void PageRenderer::setMaximumCacheCostBytes(qsizetype bytes) {
    imageCache_.setMaxCost(std::max<qsizetype>(1, bytes));
}

QImage PageRenderer::loadImage(const QString& path, int rotationDegrees, const QSize& targetSize,
                               PlacementMode placementMode) {
    const int normalizedRotation = ((rotationDegrees % 360) + 360) % 360;
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QSize sourceSize = reader.size();
    if (!reader.canRead() || !isImageSizeWithinLimits(sourceSize)) {
        qCWarning(logRender) << "Could not safely decode image for rendering";
        return {};
    }

    const QSize decodeSize =
        requestedDecodeSize(sourceSize, targetSize, normalizedRotation, placementMode);
    const QFileInfo file(path);
    const QString cacheKey = path + QChar(0x1f) + QString::number(file.size()) + QChar(0x1f) +
                             QString::number(file.lastModified().toMSecsSinceEpoch()) +
                             QChar(0x1f) + QString::number(normalizedRotation) + QChar(0x1f) +
                             QString::number(decodeSize.width()) + QLatin1Char('x') +
                             QString::number(decodeSize.height());
    if (const QImage* cached = imageCache_.object(cacheKey)) {
        return *cached;
    }

    if (decodeSize.isValid() && decodeSize != sourceSize) {
        reader.setScaledSize(decodeSize);
    }
    QImage image = reader.read();
    if (image.isNull()) {
        qCWarning(logRender) << "Could not decode image for rendering";
        return {};
    }
    if (normalizedRotation != 0) {
        QTransform transform;
        transform.rotate(normalizedRotation);
        image = image.transformed(transform);
    }

    const qsizetype cost = imageCost(image);
    if (cost <= imageCache_.maxCost()) {
        imageCache_.insert(cacheKey, new QImage(image), cost);
    }
    return image;
}

} // namespace purrview::core
