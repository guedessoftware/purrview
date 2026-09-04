import QtQuick
import QtQuick.Controls

ToolButton {
    id: control

    required property var themePalette
    required property string iconName
    property string accessibleName: ""

    width: 48
    height: 48
    hoverEnabled: true
    Accessible.name: accessibleName
    ToolTip.visible: hovered
    ToolTip.text: accessibleName

    contentItem: PurrIcon {
        name: control.iconName
        color: control.enabled ? UiTheme.navigationText : UiTheme.navigationDisabled
        strokeWidth: 2.2
    }

    background: Rectangle {
        radius: width / 2
        color: control.down ? Qt.rgba(0, 0, 0, 0.72) : (control.hovered ? Qt.rgba(0, 0, 0, 0.62) : Qt.rgba(0, 0, 0, 0.42))
        border.color: control.hovered ? UiTheme.brandPink
                                      : Qt.rgba(1, 1, 1, 0.12)
    }

}
