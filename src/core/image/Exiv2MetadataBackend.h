#pragma once

#include "core/image/ImageMetadata.h"

#include <QString>

namespace purrview::core {

// Exiv2 is deliberately hidden behind this Core-only boundary. The Viewer never includes it.
[[nodiscard]] bool populateAdvancedMetadataWithExiv2(const QString& path,
                                                     ImageMetadata& metadata,
                                                     QString* warning = nullptr);

} // namespace purrview::core
