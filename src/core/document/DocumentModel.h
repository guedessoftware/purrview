#pragma once

#include "core/document/GridLayout.h"
#include "core/document/ImageItem.h"
#include "core/document/PageModel.h"

#include <vector>

namespace purrview::core {

class DocumentModel {
  public:
    [[nodiscard]] PageModel& page();
    [[nodiscard]] const PageModel& page() const;
    [[nodiscard]] GridLayout& grid();
    [[nodiscard]] const GridLayout& grid() const;
    [[nodiscard]] const std::vector<ImageItem>& images() const;
    [[nodiscard]] PlacementMode placementMode() const;
    [[nodiscard]] int imagesPerPage() const;
    [[nodiscard]] int pageCount() const;

    void addImage(ImageItem image);
    void clearImages();
    void setPlacementMode(PlacementMode mode);

  private:
    PageModel page_;
    GridLayout grid_;
    std::vector<ImageItem> images_;
    PlacementMode placementMode_ = PlacementMode::Fit;
};

} // namespace purrview::core
