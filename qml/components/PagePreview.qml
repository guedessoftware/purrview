import Impage 1.0
import QtQuick

Item {
    id: root

    required property var controller
    required property var themePalette
    property real zoomFactor: 1.0

    clip: true

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.darker(root.themePalette.window, 1.08) }
            GradientStop { position: 1.0; color: Qt.darker(root.themePalette.base, 1.18) }
        }
    }

    Item {
        id: pageFrame

        readonly property real pageRatio: root.controller.pageWidthMm / root.controller.pageHeightMm

        anchors.centerIn: parent
        width: Math.min(parent.width - 72, (parent.height - 72) * pageRatio)
        height: width / pageRatio
        scale: root.zoomFactor

        Behavior on scale {
            NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
        }

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: 5
            anchors.topMargin: 7
            color: Qt.rgba(root.themePalette.shadow.r, root.themePalette.shadow.g, root.themePalette.shadow.b, 0.34)
            radius: 2
        }

        Rectangle {
            anchors.fill: parent
            color: "white"
            border.color: root.themePalette.mid
            border.width: 1
        }

        PagePreviewItem {
            id: paintedPreview

            anchors.fill: parent
            controller: root.controller
        }

        Rectangle {
            readonly property var cell: {
                const capacity = Math.max(1, root.controller.rows * root.controller.columns);
                const pageOffset = root.controller.currentPage * capacity;
                const localIndex = Math.max(0, Math.min(capacity - 1, pageReorderMouse.targetIndex - pageOffset));
                const row = Math.floor(localIndex / root.controller.columns);
                const column = localIndex % root.controller.columns;
                const usableWidth = root.controller.pageWidthMm - root.controller.marginLeft - root.controller.marginRight - (root.controller.columns - 1) * root.controller.horizontalSpacing;
                const usableHeight = root.controller.pageHeightMm - root.controller.marginTop - root.controller.marginBottom - (root.controller.rows - 1) * root.controller.verticalSpacing;
                const cellWidth = usableWidth / root.controller.columns;
                const cellHeight = usableHeight / root.controller.rows;
                return {
                    "x": (root.controller.marginLeft + column * (cellWidth + root.controller.horizontalSpacing)) / root.controller.pageWidthMm * pageFrame.width,
                    "y": (root.controller.marginTop + row * (cellHeight + root.controller.verticalSpacing)) / root.controller.pageHeightMm * pageFrame.height,
                    "width": cellWidth / root.controller.pageWidthMm * pageFrame.width,
                    "height": cellHeight / root.controller.pageHeightMm * pageFrame.height
                };
            }

            visible: pageReorderMouse.reordering && pageReorderMouse.targetIndex >= 0
            x: cell.x
            y: cell.y
            width: cell.width
            height: cell.height
            color: Qt.rgba(root.themePalette.highlight.r, root.themePalette.highlight.g, root.themePalette.highlight.b, 0.18)
            border.color: root.themePalette.highlight
            border.width: 3
            radius: 3
        }

        MouseArea {
            id: pageReorderMouse

            property int sourceIndex: -1
            property int targetIndex: -1
            property point pressPoint
            property bool reordering: false

            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            cursorShape: reordering ? Qt.ClosedHandCursor : containsMouse ? Qt.OpenHandCursor : Qt.ArrowCursor
            onPressed: function(mouse) {
                sourceIndex = root.controller.imageIndexAtPagePosition(mouse.x / width, mouse.y / height, false);
                pressPoint = Qt.point(mouse.x, mouse.y);
                reordering = false;
                targetIndex = -1;
                mouse.accepted = sourceIndex >= 0;
            }
            onPositionChanged: function(mouse) {
                if (!pressed || sourceIndex < 0)
                    return ;

                const distance = Math.abs(mouse.x - pressPoint.x) + Math.abs(mouse.y - pressPoint.y);
                if (distance >= 8)
                    reordering = true;

                if (reordering)
                    targetIndex = root.controller.imageIndexAtPagePosition(mouse.x / width, mouse.y / height, true);

            }
            onReleased: function(mouse) {
                if (reordering && targetIndex >= 0)
                    root.controller.moveImagesToPosition(sourceIndex, targetIndex);

                sourceIndex = -1;
                targetIndex = -1;
                reordering = false;
            }
            onCanceled: {
                sourceIndex = -1;
                targetIndex = -1;
                reordering = false;
            }
        }

        Column {
            anchors.centerIn: parent
            visible: root.controller.imageCount === 0
            spacing: 14

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(150, pageFrame.width * 0.34)
                height: width
                source: "qrc:/qt/qml/Impage/assets/purrview.svg"
                fillMode: Image.PreserveAspectFit
                opacity: 0.46
                smooth: true
                mipmap: true
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: emptyLabel.implicitWidth + 30
                height: emptyLabel.implicitHeight + 20
                color: root.themePalette.button
                radius: 7

                Text {
                    id: emptyLabel

                    anchors.centerIn: parent
                    text: qsTr("Adicione imagens para começar")
                    color: root.themePalette.buttonText
                    font.pixelSize: 14
                }
            }
        }

    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 80, 430)
        height: errorText.implicitHeight + 30
        visible: root.controller.layoutError.length > 0
        color: root.themePalette.toolTipBase
        border.color: root.themePalette.highlight
        radius: 8

        Text {
            id: errorText

            anchors.fill: parent
            anchors.margins: 15
            text: root.controller.layoutError
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: root.themePalette.toolTipText
        }

    }

}
