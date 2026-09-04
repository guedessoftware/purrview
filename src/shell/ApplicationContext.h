#pragma once

#include "core/image/ImageSession.h"
#include "core/image/ImageMetadataService.h"
#include "core/image/ThumbnailCache.h"

#include <QObject>

namespace purrview::shell {

class ApplicationContext : public QObject {
    Q_OBJECT
    Q_PROPERTY(purrview::core::ImageSession* imageSession READ imageSession CONSTANT)
    Q_PROPERTY(purrview::core::ThumbnailCache* thumbnailCache READ thumbnailCache CONSTANT)
    Q_PROPERTY(purrview::core::ImageMetadataService* imageMetadataService READ imageMetadataService
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

} // namespace purrview::shell
