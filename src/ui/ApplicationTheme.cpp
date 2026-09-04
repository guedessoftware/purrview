#include "ui/ApplicationTheme.h"

#include <QColor>

namespace purrview::ui {

bool paletteIsDark(const QPalette& palette) {
    const QColor window = palette.color(QPalette::Active, QPalette::Window);
    const QColor text = palette.color(QPalette::Active, QPalette::WindowText);
    return window.lightnessF() < text.lightnessF();
}

QPalette createPurrViewPalette(bool darkMode) {
    QPalette palette;

    const auto color = [darkMode](const char* dark, const char* light) {
        return QColor(QString::fromLatin1(darkMode ? dark : light));
    };

    palette.setColor(QPalette::Window, color("#101117", "#f6f7fb"));
    palette.setColor(QPalette::WindowText, color("#f3f5ff", "#171923"));
    palette.setColor(QPalette::Base, color("#171923", "#ffffff"));
    palette.setColor(QPalette::AlternateBase, color("#202331", "#eef0f6"));
    palette.setColor(QPalette::ToolTipBase, color("#252839", "#ffffff"));
    palette.setColor(QPalette::ToolTipText, color("#f3f5ff", "#171923"));
    palette.setColor(QPalette::Text, color("#f3f5ff", "#171923"));
    palette.setColor(QPalette::Button, color("#292c3d", "#e9ebf2"));
    palette.setColor(QPalette::ButtonText, color("#f3f5ff", "#171923"));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::Highlight, color("#a978e8", "#8750cf"));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::Link, color("#ef79a4", "#a83f72"));
    palette.setColor(QPalette::LinkVisited, color("#c991ef", "#7040a6"));
    palette.setColor(QPalette::Light, color("#464a61", "#ffffff"));
    palette.setColor(QPalette::Midlight, color("#393d52", "#f2f3f7"));
    palette.setColor(QPalette::Dark, color("#0b0c11", "#c3c7d1"));
    palette.setColor(QPalette::Mid, color("#34384b", "#d5d8e0"));
    palette.setColor(QPalette::Shadow, color("#05060a", "#858b99"));
    palette.setColor(QPalette::PlaceholderText, color("#858ca0", "#70778a"));

    const QColor disabledText = color("#777e91", "#9499a8");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Highlight,
                     color("#4c3a62", "#c8b5df"));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);

    return palette;
}

} // namespace purrview::ui
