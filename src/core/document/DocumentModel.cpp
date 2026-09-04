#include "core/document/DocumentModel.h"

#include <utility>

namespace purrview::core {

PageModel& DocumentModel::page() {
    return page_;
}

const PageModel& DocumentModel::page() const {
    return page_;
}

GridLayout& DocumentModel::grid() {
    return grid_;
}

const GridLayout& DocumentModel::grid() const {
    return grid_;
}

const std::vector<ImageItem>& DocumentModel::images() const {
    return images_;
}

PlacementMode DocumentModel::placementMode() const {
    return placementMode_;
}

int DocumentModel::imagesPerPage() const {
    return grid_.rows() * grid_.columns();
}

int DocumentModel::pageCount() const {
    const int capacity = imagesPerPage();
    if (capacity <= 0 || images_.empty()) {
        return 1;
    }
    return (static_cast<int>(images_.size()) + capacity - 1) / capacity;
}

void DocumentModel::addImage(ImageItem image) {
    image.placementMode = placementMode_;
    images_.push_back(std::move(image));
}

void DocumentModel::clearImages() {
    images_.clear();
}

void DocumentModel::setPlacementMode(PlacementMode mode) {
    placementMode_ = mode;
    for (ImageItem& image : images_) {
        image.placementMode = mode;
    }
}

} // namespace purrview::core
