#pragma once

namespace purrview::core::units {

inline constexpr double MillimetersPerInch = 25.4;

[[nodiscard]] constexpr double millimetersToPixels(double millimeters, double dpi) {
    return millimeters * dpi / MillimetersPerInch;
}

[[nodiscard]] constexpr double pixelsToMillimeters(double pixels, double dpi) {
    return pixels * MillimetersPerInch / dpi;
}

} // namespace purrview::core::units
