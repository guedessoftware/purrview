#pragma once

#include <QPalette>

namespace purrview::ui {

[[nodiscard]] bool paletteIsDark(const QPalette& palette);
[[nodiscard]] QPalette createPurrViewPalette(bool darkMode = true);

} // namespace purrview::ui
