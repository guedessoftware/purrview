import QtQuick
pragma Singleton

QtObject {
    readonly property SystemPalette applicationPalette: SystemPalette {}
    readonly property bool darkMode: applicationPalette.window.hslLightness
                                     < applicationPalette.windowText.hslLightness
    readonly property color backgroundPrimary: darkMode ? "#101117" : "#f6f7fb"
    readonly property color backgroundSecondary: darkMode ? "#171923" : "#ffffff"
    readonly property color alternateBase: darkMode ? "#202331" : "#eef0f6"
    readonly property color toolTipBase: darkMode ? "#252839" : "#ffffff"
    readonly property color floatingSurface: darkMode
                                             ? Qt.rgba(0.075, 0.082, 0.105, 0.94)
                                             : Qt.rgba(1, 1, 1, 0.94)
    readonly property color floatingSurfaceHover: darkMode
                                                  ? Qt.rgba(1, 1, 1, 0.1)
                                                  : Qt.rgba(0.10, 0.08, 0.18, 0.09)
    readonly property color floatingSurfacePressed: darkMode
                                                    ? Qt.rgba(1, 1, 1, 0.18)
                                                    : Qt.rgba(0.10, 0.08, 0.18, 0.16)
    readonly property color subtleSurface: darkMode
                                           ? Qt.rgba(1, 1, 1, 0.055)
                                           : Qt.rgba(0.10, 0.08, 0.18, 0.055)
    readonly property color insetSurface: darkMode
                                          ? Qt.rgba(1, 1, 1, 0.045)
                                          : Qt.rgba(0.10, 0.08, 0.18, 0.055)
    readonly property color idleControlSurface: darkMode
                                                ? Qt.rgba(1, 1, 1, 0.06)
                                                : Qt.rgba(0.10, 0.08, 0.18, 0.06)
    readonly property color quietControlHover: darkMode
                                               ? Qt.rgba(1, 1, 1, 0.08)
                                               : Qt.rgba(0.10, 0.08, 0.18, 0.08)
    readonly property color quietControlPressed: darkMode
                                                 ? Qt.rgba(1, 1, 1, 0.14)
                                                 : Qt.rgba(0.10, 0.08, 0.18, 0.14)
    readonly property color insetBorder: darkMode
                                         ? Qt.rgba(1, 1, 1, 0.10)
                                         : Qt.rgba(0.10, 0.08, 0.18, 0.12)
    readonly property color sectionFade: darkMode
                                         ? Qt.rgba(1, 1, 1, 0.04)
                                         : Qt.rgba(0.10, 0.08, 0.18, 0.04)
    readonly property color textPrimary: darkMode ? "#f3f5ff" : "#171923"
    readonly property color textSecondary: darkMode ? "#aeb5c9" : "#4c5263"
    readonly property color textMuted: darkMode ? "#858ca0" : "#70778a"
    readonly property color disabled: darkMode ? "#777e91" : "#9499a8"
    readonly property color borderSubtle: darkMode
                                          ? Qt.rgba(1, 1, 1, 0.14)
                                          : Qt.rgba(0.10, 0.08, 0.18, 0.16)
    readonly property color danger: darkMode ? "#ef8f95" : "#b9404a"
    readonly property color warning: darkMode ? "#d7ad72" : "#8a5b1b"
    readonly property color brandPurple: darkMode ? "#a978e8" : "#8750cf"
    readonly property color brandPink: darkMode ? "#ef79a4" : "#c84f81"
    readonly property color brandCoral: darkMode ? "#ff9478" : "#d96851"
    readonly property color brandCyan: darkMode ? "#6fcbd5" : "#247f88"
    readonly property color brandGreen: darkMode ? "#9ed58e" : "#4e8244"
    readonly property color panelSurface: darkMode
                                          ? Qt.rgba(0.075, 0.086, 0.145, 0.92)
                                          : Qt.rgba(1, 1, 1, 0.94)
    readonly property color panelSurfaceStrong: darkMode
                                                ? Qt.rgba(0.10, 0.11, 0.19, 0.98)
                                                : Qt.rgba(0.98, 0.985, 1, 0.98)
    readonly property color brandBorder: darkMode
                                         ? Qt.rgba(0.66, 0.47, 0.91, 0.36)
                                         : Qt.rgba(0.53, 0.31, 0.81, 0.34)
    readonly property color buttonSurface: darkMode ? "#292c3d" : "#e9ebf2"
    readonly property color brightText: "#ffffff"
    readonly property color highlightedText: "#ffffff"
    readonly property color link: darkMode ? "#ef79a4" : "#a83f72"
    readonly property color linkVisited: darkMode ? "#c991ef" : "#7040a6"
    readonly property color paletteLight: darkMode ? "#464a61" : "#ffffff"
    readonly property color paletteMidlight: darkMode ? "#393d52" : "#f2f3f7"
    readonly property color paletteDark: darkMode ? "#0b0c11" : "#c3c7d1"
    readonly property color paletteMid: darkMode ? "#34384b" : "#d5d8e0"
    readonly property color paletteShadow: darkMode ? "#05060a" : "#858b99"
    readonly property color dialogOverlay: darkMode
                                           ? Qt.rgba(0.025, 0.028, 0.055, 0.72)
                                           : Qt.rgba(0.20, 0.22, 0.28, 0.34)
    readonly property color dialogSurfaceStart: darkMode ? "#1b1d30" : "#ffffff"
    readonly property color dialogSurfaceEnd: darkMode ? "#121420" : "#f1f3f8"
    readonly property color filmStripSurface: darkMode
                                              ? Qt.rgba(0.04, 0.04, 0.06, 0.40)
                                              : Qt.rgba(1, 1, 1, 0.56)
    readonly property color filmStripSurfaceHover: darkMode
                                                   ? Qt.rgba(0.07, 0.07, 0.09, 0.54)
                                                   : Qt.rgba(1, 1, 1, 0.72)
    readonly property color filmStripBorder: darkMode
                                             ? Qt.rgba(1, 1, 1, 0.18)
                                             : Qt.rgba(0.10, 0.08, 0.18, 0.20)
    readonly property color filmStripBorderHover: darkMode
                                                  ? Qt.rgba(1, 1, 1, 0.28)
                                                  : Qt.rgba(0.10, 0.08, 0.18, 0.30)
    readonly property color overlayText: "#ffffff"
    readonly property color navigationText: darkMode ? "#f3f5ff" : "#ffffff"
    readonly property color navigationDisabled: darkMode
                                                ? "#777e91"
                                                : Qt.rgba(1, 1, 1, 0.48)
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
