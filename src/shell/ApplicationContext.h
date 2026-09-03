#pragma once

#include "core/image/ImageSession.h"
#include "core/image/ImageMetadataService.h"
#include "core/image/ThumbnailCache.h"

#include <QObject>

namespace impage::shell {

class ApplicationContext : public QObject {
    Q_OBJECT
    Q_PROPERTY(impage::core::ImageSession* imageSession READ imageSession CONSTANT)
    Q_PROPERTY(impage::core::ThumbnailCache* thumbnailCache READ thumbnailCache CONSTANT)
    Q_PROPERTY(impage::core::ImageMetadataService* imageMetadataService READ imageMetadataService
                   CONSTANT)

  public:
    explicit ApplicationContext(QObject* parent = nullptr);

    [[nodiscard]] core::ImageSession* imageSession();
    [[nodiscard]] const core::ImageSession* imageSession() const;
    [[nodiscard]] core::ThumbnailCache* thumbnailCache();
    [[nodiscard]] core::ImageMetadataService* imageMetadataService();

  private:
    core::ImageSession imageSession_;
    core::ThumbnailCache thumbnailCache_;
    core::ImageMetadataService imageMetadataService_;
};

} // namespace impage::shell
