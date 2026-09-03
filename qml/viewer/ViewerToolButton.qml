import QtQuick
import QtQuick.Controls

ToolButton {
    id: control

    required property var themePalette
    property string tooltipText: text
    property bool emphasized: false

    implicitWidth: Math.max(42, contentItem.implicitWidth + 18)
    implicitHeight: 42
    hoverEnabled: true
    Accessible.name: tooltipText
    ToolTip.visible: hovered
    ToolTip.text: tooltipText
    ToolTip.delay: 450

    contentItem: Label {
        text: control.text
        color: control.enabled ? UiTheme.textPrimary : UiTheme.disabled
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: control.emphasized ? 13 : 15
        font.weight: control.emphasized ? Font.DemiBold : Font.Medium
    }

    background: Rectangle {
        radius: UiTheme.radiusSmall
        color: control.emphasized
               ? Qt.rgba(UiTheme.brandPink.r, UiTheme.brandPink.g,
                         UiTheme.brandPink.b, control.enabled ? 0.88 : 0.30)
               : control.checkable && control.checked
                 ? Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                           UiTheme.brandPurple.b, 0.28)
                 : control.down ? UiTheme.floatingSurfacePressed
                   : control.hovered ? UiTheme.floatingSurfaceHover : "transparent"
        border.color: control.hovered || (control.checkable && control.checked)
                      ? UiTheme.brandBorder : "transparent"
    }

}
