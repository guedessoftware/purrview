import QtQuick
import QtQuick.Controls

Column {
    id: root

    required property string labelText
    required property string valueText

    width: parent ? parent.width : implicitWidth
    spacing: 3
    visible: valueText.length > 0

    Label {
        text: root.labelText
        color: UiTheme.textMuted
        font.pixelSize: 11
    }

    Label {
        width: parent.width
        text: root.valueText
        color: UiTheme.textPrimary
        font.pixelSize: 13
        font.weight: Font.Medium
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
    }

}
