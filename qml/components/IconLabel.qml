import QtQuick
import QtQuick.Controls

Row {
    id: root

    property string iconName: ""
    property alias text: label.text
    property alias color: label.color
    property alias font: label.font
    property real iconSize: 18
    property color iconColor: color

    spacing: 8

    PurrIcon {
        anchors.verticalCenter: parent.verticalCenter
        width: root.iconSize
        height: root.iconSize
        name: root.iconName
        color: root.iconColor
    }

    Label {
        id: label
        anchors.verticalCenter: parent.verticalCenter
    }
}
