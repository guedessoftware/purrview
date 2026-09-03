#pragma once

namespace impage::core::units {

inline constexpr double MillimetersPerInch = 25.4;

[[nodiscard]] constexpr double millimetersToPixels(double millimeters, double dpi) {
    return millimeters * dpi / MillimetersPerInch;
}

[[nodiscard]] constexpr double pixelsToMillimeters(double pixels, double dpi) {
    return pixels * MillimetersPerInch / dpi;
}

} // namespace impage::core::units
