import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    required property int itemIndex
    required property string imageFileName
    required property url imageThumbnailSource
    required property bool imageCurrent
    required property bool imageSelected
    required property bool imageValid
    required property int imagePixelWidth
    required property int imagePixelHeight
    required property var controller
    required property var themePalette

    function requestThumbnail() {
        root.controller.folderModel.requestThumbnail(root.itemIndex);
    }

    width: 76
    height: 76
    radius: UiTheme.radiusSmall
    color: imageSelected
           ? Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                     UiTheme.brandPurple.b, 0.22)
           : thumbnailHover.hovered ? UiTheme.floatingSurfaceHover
                                    : UiTheme.subtleSurface
    border.width: imageCurrent ? 2 : 1
    border.color: imageCurrent ? UiTheme.brandPink
                               : imageSelected ? UiTheme.brandPurple : UiTheme.borderSubtle
    Accessible.name: imageFileName
    Accessible.description: qsTr("%1%2, %3 por %4 pixels").arg(imageCurrent ? qsTr("Imagem atual") : qsTr("Miniatura")).arg(imageSelected ? qsTr(", selecionada") : "").arg(imagePixelWidth).arg(imagePixelHeight)
    Accessible.role: Accessible.ListItem
    Accessible.checkable: true
    Accessible.checked: imageSelected
    ToolTip.visible: thumbnailHover.hovered
    ToolTip.text: imageFileName
    ToolTip.delay: 500
    Component.onCompleted: requestThumbnail()
    onItemIndexChanged: requestThumbnail()

    Image {
        anchors.fill: parent
        anchors.margins: root.imageCurrent ? 4 : 5
        source: root.imageThumbnailSource
        sourceSize: Qt.size(144, 144)
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        smooth: true
        visible: status === Image.Ready
    }

    PurrIcon {
        anchors.centerIn: parent
        width: 24
        height: 24
        name: "image"
        color: UiTheme.textSecondary
        visible: root.imageValid && root.imageThumbnailSource.toString().length === 0
    }

    PurrIcon {
        anchors.centerIn: parent
        width: 20
        height: 20
        name: "warning"
        color: UiTheme.danger
        visible: root.imageThumbnailSource.toString().length === 0
                 && !root.imageValid
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 4
        width: 17
        height: 17
        radius: 9
        visible: root.imageSelected
        color: UiTheme.brandPurple
        border.color: Qt.rgba(1, 1, 1, 0.85)

        PurrIcon {
            anchors.centerIn: parent
            width: 11
            height: 11
            name: "check"
            color: root.themePalette.highlightedText
            strokeWidth: 2.4
        }

    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 2
        width: 22
        height: 3
        radius: 2
        color: UiTheme.brandPink
        visible: root.imageCurrent
    }

    HoverHandler {
        id: thumbnailHover
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.PointingHandCursor
        onClicked: function(mouse) {
            root.controller.state.notifyActivity();
            if ((mouse.modifiers & Qt.ControlModifier) !== 0)
                root.controller.toggleFolderSelection(root.itemIndex);
            else if ((mouse.modifiers & Qt.ShiftModifier) !== 0)
                root.controller.selectFolderRange(root.itemIndex);
            else
                root.controller.activateFolderIndex(root.itemIndex);
        }
    }

}
