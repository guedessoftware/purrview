#include "ui/ThumbnailImageProvider.h"

namespace purrview::ui {

ThumbnailImageProvider::ThumbnailImageProvider(core::ThumbnailCache& cache)
    : QQuickImageProvider(QQuickImageProvider::Image,
                          QQuickImageProvider::ForceAsynchronousImageLoading),
      cache_(cache) {}

QImage ThumbnailImageProvider::requestImage(const QString& id, QSize* size,
                                            const QSize& requestedSize) {
    QImage image = cache_.imageForKey(id);
    if (size != nullptr) {
        *size = image.size();
    }
    if (!image.isNull() && requestedSize.isValid() &&
        (image.width() > requestedSize.width() || image.height() > requestedSize.height())) {
        image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return image;
}

} // namespace purrview::ui
