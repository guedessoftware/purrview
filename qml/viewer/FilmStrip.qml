import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    required property var controller
    required property var themePalette
    property real availableWidth: parent ? parent.width : 760
    readonly property bool interacting: stripHover.hovered || thumbnailList.moving
    readonly property int itemCount: root.controller.folderModel.count
    readonly property real thumbnailWidth: 76
    readonly property real horizontalPadding: 20
    readonly property real naturalWidth: itemCount > 0
                                         ? itemCount * thumbnailWidth
                                           + Math.max(0, itemCount - 1) * UiTheme.spacingSm
                                           + horizontalPadding
                                         : 0
    readonly property real maximumWidth: Math.max(horizontalPadding + thumbnailWidth,
                                                  availableWidth - 96)

    function ensureCurrentVisible() {
        const itemIndex = root.controller.folderModel.currentIndex;
        if (itemIndex < 0 || thumbnailList.count === 0)
            return ;

        const item = thumbnailList.itemAtIndex(itemIndex);
        if (!item || item.x < thumbnailList.contentX || item.x + item.width > thumbnailList.contentX + thumbnailList.width)
            thumbnailList.positionViewAtIndex(itemIndex, ListView.Center);

    }

    function contentPosition() {
        return thumbnailList.contentX;
    }

    function restoreContentPosition(position) {
        Qt.callLater(function() {
            const maximum = Math.max(0, thumbnailList.contentWidth - thumbnailList.width);
            thumbnailList.contentX = Math.max(0, Math.min(maximum, position));
        });
    }

    activeFocusOnTab: true
    Accessible.role: Accessible.List
    Accessible.name: qsTr("Miniaturas da pasta")
    Accessible.description: qsTr("%1 imagens; use as setas para navegar e espaço para selecionar").arg(root.controller.folderModel.count)
    implicitHeight: 98
    implicitWidth: naturalWidth
    width: Math.min(naturalWidth, maximumWidth)
    radius: UiTheme.radiusLarge
    color: stripHover.hovered ? UiTheme.filmStripSurfaceHover
                              : UiTheme.filmStripSurface
    border.color: stripHover.hovered ? UiTheme.filmStripBorderHover
                                     : UiTheme.filmStripBorder
    clip: true

    Behavior on width {
        NumberAnimation {
            duration: UiTheme.durationNormal
            easing.type: Easing.OutCubic
        }
    }

    Keys.onLeftPressed: function(event) {
        root.controller.state.notifyActivity();
        root.controller.previousImage();
        event.accepted = true;
    }
    Keys.onRightPressed: function(event) {
        root.controller.state.notifyActivity();
        root.controller.nextImage();
        event.accepted = true;
    }
    Keys.onSpacePressed: function(event) {
        const index = root.controller.folderModel.currentIndex;
        if (index >= 0)
            root.controller.toggleFolderSelection(index);

        event.accepted = true;
    }
    Keys.onReturnPressed: function(event) {
        const index = root.controller.folderModel.currentIndex;
        if (index >= 0)
            root.controller.activateFolderIndex(index);

        event.accepted = true;
    }
    Component.onCompleted: Qt.callLater(ensureCurrentVisible)

    HoverHandler {
        id: stripHover
    }

    ListView {
        id: thumbnailList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        orientation: ListView.Horizontal
        spacing: UiTheme.spacingSm
        model: root.controller.folderModel
        boundsBehavior: Flickable.StopAtBounds
        reuseItems: true
        cacheBuffer: 720
        clip: true

        delegate: FilmStripItem {
            required property int index
            required property string fileName
            required property url thumbnailSource
            required property bool current
            required property bool selected
            required property bool valid
            required property int pixelWidth
            required property int pixelHeight

            itemIndex: index
            imageFileName: fileName
            imageThumbnailSource: thumbnailSource
            imageCurrent: current
            imageSelected: selected
            imageValid: valid
            imagePixelWidth: pixelWidth
            imagePixelHeight: pixelHeight
            controller: root.controller
            themePalette: root.themePalette
        }

        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AsNeeded
            height: 3
        }

    }

    WheelHandler {
        target: null
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: function(event) {
            root.controller.state.notifyActivity();
            const rawDelta = event.pixelDelta.x !== 0 ? event.pixelDelta.x : (event.pixelDelta.y !== 0 ? event.pixelDelta.y : event.angleDelta.y / 2);
            const maximum = Math.max(0, thumbnailList.contentWidth - thumbnailList.width);
            thumbnailList.contentX = Math.max(0, Math.min(maximum, thumbnailList.contentX - rawDelta));
            event.accepted = true;
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        width: 30
        height: 30
        running: visible
        visible: root.controller.folderModel.scanning && thumbnailList.count === 0
    }

    Connections {
        function onCurrentIndexChanged() {
            Qt.callLater(root.ensureCurrentVisible);
        }

        function onCountChanged() {
            Qt.callLater(root.ensureCurrentVisible);
        }

        target: root.controller.folderModel
    }

}
