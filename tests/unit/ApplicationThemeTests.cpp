#include "ui/ApplicationTheme.h"

#include <QColor>
#include <QPalette>

#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

} // namespace

int main() {
    const QPalette palette = impage::ui::createPurrViewPalette();

    check(palette.color(QPalette::Window) == QColor(QStringLiteral("#101117")),
          "window uses the PurrView dark surface");
    check(palette.color(QPalette::WindowText) == QColor(QStringLiteral("#f3f5ff")),
          "window text remains readable on every desktop theme");
    check(palette.color(QPalette::Button) == QColor(QStringLiteral("#292c3d")),
          "native controls use a dark button surface");
    check(palette.color(QPalette::Highlight) == QColor(QStringLiteral("#a978e8")),
          "selection uses the PurrView brand accent");
    check(palette.color(QPalette::Disabled, QPalette::ButtonText) ==
              QColor(QStringLiteral("#777e91")),
          "disabled controls retain sufficient visual hierarchy");

    const QPalette lightPalette = impage::ui::createPurrViewPalette(false);
    check(lightPalette.color(QPalette::Window) == QColor(QStringLiteral("#f6f7fb")),
          "light mode uses the PurrView light surface");
    check(lightPalette.color(QPalette::WindowText) == QColor(QStringLiteral("#171923")),
          "light mode uses dark readable text");
    check(lightPalette.color(QPalette::Highlight) == QColor(QStringLiteral("#8750cf")),
          "light mode keeps a high-contrast brand accent");
    check(!impage::ui::paletteIsDark(lightPalette), "light palette is detected as light");
    check(impage::ui::paletteIsDark(palette), "dark palette is detected as dark");

    return failures == 0 ? 0 : 1;
}
