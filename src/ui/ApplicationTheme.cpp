#include "ui/ApplicationTheme.h"

#include <QColor>

namespace impage::ui {

QPalette createPurrViewPalette() {
    QPalette palette;

    palette.setColor(QPalette::Window, QColor(QStringLiteral("#101117")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#f3f5ff")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#171923")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#202331")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#252839")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#f3f5ff")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#f3f5ff")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#292c3d")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#f3f5ff")));
    palette.setColor(QPalette::BrightText, QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#a978e8")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#ffffff")));
    palette.setColor(QPalette::Link, QColor(QStringLiteral("#ef79a4")));
    palette.setColor(QPalette::LinkVisited, QColor(QStringLiteral("#c991ef")));
    palette.setColor(QPalette::Light, QColor(QStringLiteral("#464a61")));
    palette.setColor(QPalette::Midlight, QColor(QStringLiteral("#393d52")));
    palette.setColor(QPalette::Dark, QColor(QStringLiteral("#0b0c11")));
    palette.setColor(QPalette::Mid, QColor(QStringLiteral("#34384b")));
    palette.setColor(QPalette::Shadow, QColor(QStringLiteral("#05060a")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#858ca0")));

    const QColor disabledText(QStringLiteral("#777e91"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(QStringLiteral("#4c3a62")));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);

    return palette;
}

} // namespace impage::ui
