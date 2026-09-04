#pragma once

#include "core/document/DocumentModel.h"
#include "core/layout/PageLayout.h"

namespace purrview::core {

class LayoutEngine {
  public:
    [[nodiscard]] PageLayout calculate(const DocumentModel& document) const;
};

} // namespace purrview::core
