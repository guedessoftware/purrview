import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property var controller
    required property var themePalette
    property real safeInsetTop: 0
    property real safeInsetBottom: 0
    property real panX: 0
    property real panY: 0
    property bool restoredPanPending: false
    property real restoredPanX: 0
    property real restoredPanY: 0
    readonly property bool panning: interaction.pressed && canPan
    readonly property bool quarterTurn: controller.rotation % 180 !== 0
    readonly property real sourcePixelWidth: Math.max(1, controller.currentPixelWidth)
    readonly property real sourcePixelHeight: Math.max(1, controller.currentPixelHeight)
    readonly property real rotatedPixelWidth: quarterTurn ? sourcePixelHeight : sourcePixelWidth
    readonly property real rotatedPixelHeight: quarterTurn ? sourcePixelWidth : sourcePixelHeight
    readonly property real safeWidth: Math.max(1, viewport.width - 64)
    readonly property real safeHeight: Math.max(1, viewport.height - 64 - safeInsetTop - safeInsetBottom)
    readonly property real contentCenterX: viewport.width / 2
    readonly property real contentCenterY: safeInsetTop + (viewport.height - safeInsetTop - safeInsetBottom) / 2
    readonly property real fitScale: Math.min(safeWidth / rotatedPixelWidth, safeHeight / rotatedPixelHeight)
    readonly property real effectiveScale: controller.state.fitMode ? fitScale : controller.state.zoomFactor
    readonly property real displayedWidth: rotatedPixelWidth * effectiveScale
    readonly property real displayedHeight: rotatedPixelHeight * effectiveScale
    readonly property bool canPan: displayedWidth > safeWidth + 0.5 || displayedHeight > safeHeight + 0.5

    signal fullScreenToggleRequested()
    signal userActivity()
    signal contextMenuRequested()
    signal openImageRequested()

    function resetPan() {
        panX = 0;
        panY = 0;
    }

    function clampPan() {
        const maxX = Math.max(0, (displayedWidth - safeWidth) / 2);
        const maxY = Math.max(0, (displayedHeight - safeHeight) / 2);
        panX = Math.max(-maxX, Math.min(maxX, panX));
        panY = Math.max(-maxY, Math.min(maxY, panY));
    }

    function restorePan(x, y) {
        restoredPanX = x;
        restoredPanY = y;
        restoredPanPending = true;
        if (displayImage.status === Image.Ready)
            applyRestoredPan();

    }

    function applyRestoredPan() {
        panX = restoredPanX;
        panY = restoredPanY;
        restoredPanPending = false;
        Qt.callLater(clampPan);
    }

    function applyWheelZoom(cursorX, cursorY, delta) {
        if (displayImage.status !== Image.Ready || delta === 0)
            return ;

        const oldScale = effectiveScale;
        const pointX = (cursorX - contentCenterX - panX) / oldScale;
        const pointY = (cursorY - contentCenterY - panY) / oldScale;
        const requestedScale = oldScale * Math.pow(1.0015, delta);
        controller.setCustomZoom(requestedScale);
        const newScale = controller.state.zoomFactor;
        panX = cursorX - contentCenterX - pointX * newScale;
        panY = cursorY - contentCenterY - pointY * newScale;
        clampPan();
    }

    Accessible.role: Accessible.Graphic
    Accessible.name: controller.currentFileName.length > 0 ? controller.currentFileName : qsTr("Nenhuma imagem aberta")
    Accessible.description: controller.imageCount > 0 ? qsTr("Imagem %1 de %2, %3 por %4 pixels%5.").arg(controller.currentIndex + 1).arg(controller.imageCount).arg(controller.currentPixelWidth).arg(controller.currentPixelHeight).arg(controller.currentImageSelected ? qsTr(", selecionada") : "") : qsTr("Área de visualização vazia")
    onFitScaleChanged: {
        if (controller.state.fitMode && displayImage.status === Image.Ready)
            controller.updateFitScale(fitScale);

    }
    onWidthChanged: Qt.callLater(clampPan)
    onHeightChanged: Qt.callLater(clampPan)

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0; color: UiTheme.backgroundSecondary }
            GradientStop { position: 1; color: UiTheme.backgroundPrimary }
        }
    }

    Item {
        id: viewport

        anchors.fill: parent
        clip: true

        Image {
            id: displayImage

            width: root.sourcePixelWidth
            height: root.sourcePixelHeight
            x: root.contentCenterX - width / 2 + root.panX
            y: root.contentCenterY - height / 2 + root.panY
            source: root.controller.currentImageUrl
            rotation: root.controller.rotation
            scale: root.effectiveScale
            transformOrigin: Item.Center
            asynchronous: true
            cache: false
            smooth: true
            mipmap: root.effectiveScale < 1
            autoTransform: true
            opacity: status === Image.Ready ? 1 : 0
            onStatusChanged: {
                if (status === Image.Ready) {
                    root.controller.reportCurrentImageVisible();
                    root.controller.updateFitScale(root.fitScale);
                    if (root.restoredPanPending)
                        root.applyRestoredPan();
                    else
                        root.resetPan();
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: UiTheme.durationFast
                }

            }

        }

        MouseArea {
            id: interaction

            property int mouseButton: Qt.NoButton
            property real previousX: 0
            property real previousY: 0

            anchors.fill: parent
            enabled: displayImage.status === Image.Ready
            acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.BackButton | Qt.ForwardButton
            hoverEnabled: true
            cursorShape: pressed && mouseButton === Qt.LeftButton && root.canPan ? Qt.ClosedHandCursor : (!root.controller.state.cursorVisible ? Qt.BlankCursor : (root.canPan ? Qt.OpenHandCursor : Qt.ArrowCursor))
            onPressed: function(mouse) {
                root.userActivity();
                mouseButton = mouse.button;
                previousX = mouse.x;
                previousY = mouse.y;
            }
            onPositionChanged: function(mouse) {
                root.userActivity();
                if (!pressed || !root.canPan)
                    return ;

                root.panX += mouse.x - previousX;
                root.panY += mouse.y - previousY;
                previousX = mouse.x;
                previousY = mouse.y;
                root.clampPan();
            }
            onClicked: function(mouse) {
                root.userActivity();
                if (mouse.button === Qt.BackButton)
                    root.controller.previousImage();
                else if (mouse.button === Qt.ForwardButton)
                    root.controller.nextImage();
                else if (mouse.button === Qt.RightButton)
                    root.contextMenuRequested();
            }
            onDoubleClicked: function(mouse) {
                if (mouse.button === Qt.LeftButton)
                    root.fullScreenToggleRequested();

            }
            onWheel: function(wheel) {
                root.userActivity();
                const delta = wheel.angleDelta.y !== 0 ? wheel.angleDelta.y : wheel.pixelDelta.y;
                root.applyWheelZoom(wheel.x, wheel.y, delta);
                wheel.accepted = true;
            }
        }

    }

    BusyIndicator {
        anchors.centerIn: parent
        running: visible
        visible: displayImage.status === Image.Loading
    }

    Column {
        anchors.centerIn: parent
        spacing: 10
        visible: root.controller.imageCount === 0

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 104
            height: 104
            source: "qrc:/qt/qml/Impage/assets/purrview.svg"
            fillMode: Image.PreserveAspectFit
            opacity: 0.72
            smooth: true
            mipmap: true
        }

        Label {
            text: root.controller.state.errorString.length > 0 ? root.controller.state.errorString : qsTr("Nenhuma imagem aberta")
            color: UiTheme.textPrimary
            font.pixelSize: 16
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Abrir imagem")
            Accessible.name: qsTr("Abrir uma imagem")
            onClicked: root.openImageRequested()
        }

    }

    Column {
        anchors.centerIn: parent
        spacing: 8
        visible: root.controller.imageCount > 0 && displayImage.status === Image.Error

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Não foi possível abrir esta imagem.")
            color: UiTheme.textPrimary
            font.pixelSize: 16
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.controller.currentFileName
            color: UiTheme.textSecondary
        }

    }

    Connections {
        function onPanResetRequested() {
            root.resetPan();
            if (root.controller.state.fitMode && displayImage.status === Image.Ready)
                root.controller.updateFitScale(root.fitScale);

        }

        function onZoomChanged() {
            Qt.callLater(root.clampPan);
        }

        target: root.controller.state
    }

}
