#pragma once

#include <QPalette>

namespace impage::ui {

[[nodiscard]] bool paletteIsDark(const QPalette& palette);
[[nodiscard]] QPalette createPurrViewPalette(bool darkMode = true);

} // namespace impage::ui
