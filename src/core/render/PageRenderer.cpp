#include "core/render/PageRenderer.h"

#include <QImageReader>
#include <QLoggingCategory>
#include <QPen>
#include <QTransform>

#include <algorithm>

Q_LOGGING_CATEGORY(logRender, "impage.render")

namespace impage::core {

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
        const QImage& image = loadImage(item.source, item.rotationDegrees);
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

const QImage& PageRenderer::loadImage(const QString& path, int rotationDegrees) {
    const int normalizedRotation = ((rotationDegrees % 360) + 360) % 360;
    const QString cacheKey = path + QChar(0x1f) + QString::number(normalizedRotation);
    auto iterator = imageCache_.find(cacheKey);
    if (iterator != imageCache_.end()) {
        return iterator.value();
    }

    if (normalizedRotation == 0) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        QImage image = reader.read();
        if (image.isNull()) {
            qCWarning(logRender) << "Could not decode image for rendering";
        }
        return imageCache_.insert(cacheKey, std::move(image)).value();
    }

    const QImage source = loadImage(path, 0);
    if (source.isNull()) {
        return imageCache_.insert(cacheKey, {}).value();
    }
    QTransform transform;
    transform.rotate(normalizedRotation);
    return imageCache_.insert(cacheKey, source.transformed(transform)).value();
}

} // namespace impage::core
