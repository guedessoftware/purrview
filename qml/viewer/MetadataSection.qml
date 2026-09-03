import QtQuick
import QtQuick.Controls

Column {
    id: root

    required property string title
    default property alias sectionData: rows.data

    width: parent ? parent.width : implicitWidth
    spacing: UiTheme.spacingMd

    Label {
        width: parent.width
        text: root.title
        color: UiTheme.brandPink
        font.pixelSize: 14
        font.weight: Font.DemiBold
    }

    Rectangle {
        width: parent.width
        height: 1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: UiTheme.brandBorder }
            GradientStop { position: 1; color: Qt.rgba(1, 1, 1, 0.04) }
        }
    }

    Column {
        id: rows

        width: parent.width
        spacing: UiTheme.spacingMd
    }

}
