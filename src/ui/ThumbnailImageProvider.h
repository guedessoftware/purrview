#pragma once

#include "core/image/ThumbnailCache.h"

#include <QQuickImageProvider>

namespace impage::ui {

class ThumbnailImageProvider : public QQuickImageProvider {
  public:
    explicit ThumbnailImageProvider(core::ThumbnailCache& cache);

    [[nodiscard]] QImage requestImage(const QString& id, QSize* size,
                                      const QSize& requestedSize) override;

  private:
    core::ThumbnailCache& cache_;
};

} // namespace impage::ui
