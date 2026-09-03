import QtQuick
pragma Singleton

QtObject {
    readonly property color backgroundPrimary: "#101117"
    readonly property color backgroundSecondary: "#171923"
    readonly property color floatingSurface: Qt.rgba(0.075, 0.082, 0.105, 0.94)
    readonly property color floatingSurfaceHover: Qt.rgba(1, 1, 1, 0.1)
    readonly property color floatingSurfacePressed: Qt.rgba(1, 1, 1, 0.18)
    readonly property color textPrimary: "#f3f5ff"
    readonly property color textSecondary: "#aeb5c9"
    readonly property color textMuted: "#858ca0"
    readonly property color disabled: "#777e91"
    readonly property color borderSubtle: Qt.rgba(1, 1, 1, 0.14)
    readonly property color danger: "#ef8f95"
    readonly property color warning: "#d7ad72"
    readonly property color brandPurple: "#a978e8"
    readonly property color brandPink: "#ef79a4"
    readonly property color brandCoral: "#ff9478"
    readonly property color brandCyan: "#6fcbd5"
    readonly property color brandGreen: "#9ed58e"
    readonly property color panelSurface: Qt.rgba(0.075, 0.086, 0.145, 0.92)
    readonly property color panelSurfaceStrong: Qt.rgba(0.10, 0.11, 0.19, 0.98)
    readonly property color brandBorder: Qt.rgba(0.66, 0.47, 0.91, 0.36)
    readonly property int radiusSmall: 6
    readonly property int radiusMedium: 10
    readonly property int radiusLarge: 14
    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 12
    readonly property int spacingLg: 16
    readonly property int spacingXl: 24
    readonly property int toolbarHeight: 54
    readonly property int durationFast: 110
    readonly property int durationNormal: 160
    readonly property int durationSlow: 230
}
