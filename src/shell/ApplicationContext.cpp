#include "shell/ApplicationContext.h"

namespace purrview::shell {

ApplicationContext::ApplicationContext(QObject* parent) : QObject(parent) {}

core::ImageSession* ApplicationContext::imageSession() {
    return &imageSession_;
}

const core::ImageSession* ApplicationContext::imageSession() const {
    return &imageSession_;
}

core::ThumbnailCache* ApplicationContext::thumbnailCache() {
    return &thumbnailCache_;
}

core::ImageMetadataService* ApplicationContext::imageMetadataService() {
    return &imageMetadataService_;
}

} // namespace purrview::shell
