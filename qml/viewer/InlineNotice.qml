import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property string message: ""
    property real bottomInset: 24

    signal dismissed()

    function show(text) {
        message = text;
        opacity = 1;
        noticeTimer.restart();
    }

    width: Math.min(implicitWidth, parent ? parent.width - 48 : implicitWidth)
    implicitWidth: noticeLabel.implicitWidth + UiTheme.spacingXl * 2
    implicitHeight: noticeLabel.implicitHeight + UiTheme.spacingMd * 2
    radius: UiTheme.radiusMedium
    color: UiTheme.floatingSurface
    border.color: UiTheme.borderSubtle
    opacity: 0
    visible: opacity > 0
    Accessible.role: Accessible.AlertMessage
    Accessible.name: message

    Label {
        id: noticeLabel

        anchors.centerIn: parent
        width: Math.min(implicitWidth, root.parent ? root.parent.width - 88 : implicitWidth)
        text: root.message
        color: UiTheme.textPrimary
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
    }

    Timer {
        id: noticeTimer

        interval: 2600
        onTriggered: {
            root.opacity = 0;
            root.dismissed();
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: UiTheme.durationFast
        }

    }

}
