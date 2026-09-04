import PurrView 1.0
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: viewerPage

    required property var controller
    required property var applicationController
    required property var themePalette
    required property bool fullScreen
    readonly property bool compactInfoPanel: width < 980
    readonly property real infoPanelWidth: Math.min(390, Math.max(300, compactInfoPanel ? width * 0.82 : 340))
    readonly property bool infoPanelOpen: controller.state.infoPanelVisible
    readonly property bool controlsShown: !fullScreen || controller.state.controlsVisible
    readonly property bool interactionBlocked: infoPanelOpen || viewerToolbar.interacting || filmStrip.interacting || imageCanvas.panning || imageContextMenu.opened || trashConfirmationDialog.opened
    property bool filmstripRevealed: true

    signal fullScreenRequested(bool enabled)
    signal aboutRequested()

    function captureVisualState() {
        controller.captureViewState(imageCanvas.panX, imageCanvas.panY, filmStrip.contentPosition());
    }

    function revealFilmstrip() {
        if (!controller.state.filmstripVisible || controller.folderModel.count === 0)
            return;
        filmstripRevealed = true;
        filmstripHideTimer.restart();
    }

    function dismissViewerState() {
        controller.state.notifyActivity();
        if (imageContextMenu.opened)
            imageContextMenu.close();
        else if (controller.state.infoPanelVisible)
            controller.toggleInfoPanel();
        else if (controller.selectedImageCount > 0)
            controller.clearSelection();
        else if (fullScreen)
            fullScreenRequested(false);
        forceActiveFocus();
    }

    focus: true
    Component.onCompleted: Qt.callLater(function() {
        controller.state.setFullScreen(fullScreen);
        controller.state.setInteractionBlocked(interactionBlocked);
        forceActiveFocus();
        imageCanvas.restorePan(viewerPage.controller.savedPanX, viewerPage.controller.savedPanY);
        filmStrip.restoreContentPosition(viewerPage.controller.savedFilmstripContentX);
    })
    onFullScreenChanged: controller.state.setFullScreen(fullScreen)
    onInteractionBlockedChanged: controller.state.setInteractionBlocked(interactionBlocked)
    Component.onDestruction: {
        viewerPage.captureVisualState();
    }

    Action {
        id: printAction

        text: viewerPage.controller.printActionText
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.openComposer()
    }

    Action {
        id: fitAction

        text: qsTr("Ajustar à janela")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.fitToWindow()
    }

    Action {
        id: actualSizeAction

        text: qsTr("Tamanho real")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.actualSize()
    }

    Action {
        id: rotateLeftAction

        text: qsTr("Girar para esquerda")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.rotateLeft()
    }

    Action {
        id: rotateRightAction

        text: qsTr("Girar para direita")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.rotateRight()
    }

    Action {
        id: infoAction

        text: qsTr("Mostrar informações")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.toggleInfoPanel()
    }

    Action {
        id: openLocationAction

        text: qsTr("Abrir localização")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.openCurrentFolder()
    }

    Action {
        id: copyPathAction

        text: qsTr("Copiar caminho")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: viewerPage.controller.copyCurrentPath()
    }

    Action {
        id: trashAction

        text: qsTr("Enviar para a lixeira")
        enabled: viewerPage.controller.imageCount > 0
        onTriggered: trashConfirmationDialog.open()
    }

    Item {
        id: canvasArea

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width - (viewerPage.infoPanelOpen && !viewerPage.compactInfoPanel ? viewerPage.infoPanelWidth : 0)
        clip: true

        ImageCanvas {
            id: imageCanvas

            anchors.fill: parent
            controller: viewerPage.controller
            themePalette: viewerPage.themePalette
            // The toolbar is deliberately an overlay; it must not push the image down.
            safeInsetTop: 0
            safeInsetBottom: 0
            onFullScreenToggleRequested: viewerPage.fullScreenRequested(!viewerPage.fullScreen)
            onUserActivity: viewerPage.controller.state.notifyActivity()
            onContextMenuRequested: {
                viewerPage.controller.state.notifyActivity();
                imageContextMenu.popup();
            }
            onOpenImageRequested: openImageDialog.open()
        }

        Behavior on width {
            NumberAnimation {
                duration: UiTheme.durationNormal
                easing.type: Easing.OutCubic
            }

        }

    }

    Image {
        visible: false
        source: viewerPage.controller.previousImageUrl
        asynchronous: true
        cache: true
        sourceSize: Qt.size(Math.ceil(canvasArea.width * 1.5), Math.ceil(viewerPage.height * 1.5))
    }

    Image {
        visible: false
        source: viewerPage.controller.nextImageUrl
        asynchronous: true
        cache: true
        sourceSize: Qt.size(Math.ceil(canvasArea.width * 1.5), Math.ceil(viewerPage.height * 1.5))
    }

    ViewerToolbar {
        id: viewerToolbar

        anchors.top: parent.top
        anchors.horizontalCenter: canvasArea.horizontalCenter
        anchors.topMargin: 18
        controller: viewerPage.controller
        themePalette: viewerPage.themePalette
        availableWidth: canvasArea.width
        fullScreen: viewerPage.fullScreen
        controlsShown: viewerPage.controlsShown || viewerPage.controller.state.toolbarPinned
        z: 5
        onPrintRequested: printAction.trigger()
        onFullScreenRequested: viewerPage.fullScreenRequested(!viewerPage.fullScreen)
        onTrashRequested: trashConfirmationDialog.open()
        onAboutRequested: viewerPage.aboutRequested()

        transform: Translate {
            y: viewerToolbar.controlsShown ? 0 : -8

            Behavior on y {
                NumberAnimation {
                    duration: UiTheme.durationNormal
                    easing.type: Easing.OutCubic
                }

            }

        }

    }

    NavigationButton {
        id: previousButton

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 18
        iconName: "chevron-left"
        accessibleName: qsTr("Imagem anterior")
        themePalette: viewerPage.themePalette
        visible: viewerPage.controller.folderModel.count > 1
        enabled: viewerPage.controller.canGoPrevious
        onClicked: {
            viewerPage.revealFilmstrip();
            viewerPage.controller.previousImage();
        }
        z: 5
        opacity: viewerPage.controlsShown ? 1 : 0
    }

    NavigationButton {
        id: nextButton

        anchors.right: canvasArea.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 18
        iconName: "chevron-right"
        accessibleName: qsTr("Próxima imagem")
        themePalette: viewerPage.themePalette
        visible: viewerPage.controller.folderModel.count > 1
        enabled: viewerPage.controller.canGoNext
        onClicked: {
            viewerPage.revealFilmstrip();
            viewerPage.controller.nextImage();
        }
        z: 5
        opacity: viewerPage.controlsShown ? 1 : 0
    }

    FilmStrip {
        id: filmStrip

        anchors.horizontalCenter: canvasArea.horizontalCenter
        anchors.bottom: canvasArea.bottom
        anchors.bottomMargin: 12
        controller: viewerPage.controller
        themePalette: viewerPage.themePalette
        availableWidth: canvasArea.width
        enabled: viewerPage.controller.state.filmstripVisible
        opacity: enabled && viewerPage.controller.folderModel.count > 0
                 && viewerPage.filmstripRevealed ? 1 : 0
        z: 6
        onInteractingChanged: {
            if (interacting)
                viewerPage.revealFilmstrip();
            else if (viewerPage.filmstripRevealed)
                filmstripHideTimer.restart();
        }

        transform: Translate {
            y: filmStrip.enabled && viewerPage.filmstripRevealed
               ? 0 : filmStrip.height + filmStrip.anchors.bottomMargin + 8

            Behavior on y {
                NumberAnimation {
                    duration: 360
                    easing.type: Easing.OutCubic
                }

            }

        }

        Behavior on opacity {
            NumberAnimation {
                duration: 260
            }

        }

    }

    Item {
        anchors.left: canvasArea.left
        anchors.right: canvasArea.right
        anchors.bottom: canvasArea.bottom
        height: 74
        z: 4

        HoverHandler {
            onHoveredChanged: {
                if (hovered)
                    viewerPage.revealFilmstrip();
            }
        }
    }

    Timer {
        id: filmstripHideTimer

        interval: 2300
        repeat: false
        running: viewerPage.filmstripRevealed
                 && viewerPage.controller.state.filmstripVisible
                 && viewerPage.controller.folderModel.count > 0
        onTriggered: {
            if (filmStrip.interacting)
                restart();
            else
                viewerPage.filmstripRevealed = false;
        }
    }

    Label {
        id: errorBanner

        anchors.horizontalCenter: canvasArea.horizontalCenter
        anchors.bottom: filmStrip.enabled && viewerPage.filmstripRevealed
                        ? filmStrip.top : parent.bottom
        anchors.bottomMargin: 14
        visible: viewerPage.controller.state.errorString.length > 0
        text: viewerPage.controller.state.errorString
        color: UiTheme.textPrimary
        padding: 10
        z: 9

        Timer {
            interval: 5200
            running: errorBanner.visible
            onTriggered: viewerPage.controller.clearError()
        }

        background: Rectangle {
            radius: UiTheme.radiusMedium
            color: Qt.rgba(0.48, 0.12, 0.14, 0.94)
            border.color: Qt.rgba(1, 0.55, 0.58, 0.65)
        }

    }

    ImageInfoPanel {
        id: infoPanel

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: viewerPage.infoPanelWidth
        x: viewerPage.infoPanelOpen ? parent.width - width : parent.width
        opacity: viewerPage.infoPanelOpen ? 1 : 0
        enabled: viewerPage.infoPanelOpen
        controller: viewerPage.controller
        themePalette: viewerPage.themePalette
        z: 10

        Behavior on x {
            NumberAnimation {
                duration: UiTheme.durationNormal
                easing.type: Easing.OutCubic
            }

        }

        Behavior on opacity {
            NumberAnimation {
                duration: UiTheme.durationFast
            }

        }

    }

    Shortcut {
        sequence: "Left"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.revealFilmstrip();
            viewerPage.controller.previousImage();
        }
    }

    Shortcut {
        sequence: "Right"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.revealFilmstrip();
            viewerPage.controller.nextImage();
        }
    }

    Shortcut {
        sequence: "+"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.zoomIn();
        }
    }

    Shortcut {
        sequence: "="
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.zoomIn();
        }
    }

    Shortcut {
        sequence: "-"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.zoomOut();
        }
    }

    Shortcut {
        sequence: "0"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.fitToWindow();
        }
    }

    Shortcut {
        sequence: "Ctrl+0"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.fitToWindow();
        }
    }

    Shortcut {
        sequence: "1"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.actualSize();
        }
    }

    Shortcut {
        sequence: "R"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.rotateRight();
        }
    }

    Shortcut {
        sequence: "Shift+R"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.rotateLeft();
        }
    }

    Shortcut {
        sequence: "Ctrl+P"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.openComposer();
        }
    }

    Shortcut {
        sequence: "Ctrl+A"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.selectAllFolderImages();
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+A"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.clearSelection();
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+C"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.copyCurrentPath();
        }
    }

    Shortcut {
        sequence: "Delete"
        onActivated: trashConfirmationDialog.open()
    }

    Shortcut {
        sequence: "I"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.toggleInfoPanel();
        }
    }

    Shortcut {
        sequence: "Alt+Return"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.controller.toggleInfoPanel();
        }
    }

    Shortcut {
        sequence: "F11"
        onActivated: {
            viewerPage.controller.state.notifyActivity();
            viewerPage.fullScreenRequested(!viewerPage.fullScreen);
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: viewerPage.dismissViewerState()
    }

    HoverHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onPointChanged: viewerPage.controller.state.notifyActivity()
    }

    Connections {
        function onCaptureVisualStateRequested() {
            viewerPage.captureVisualState();
        }

        function onNoticeRequested(message) {
            inlineNotice.show(message);
        }

        target: viewerPage.controller
    }

    Connections {
        target: viewerPage.controller.folderModel

        function onCurrentIndexChanged() {
            viewerPage.revealFilmstrip();
        }

        function onCountChanged() {
            viewerPage.revealFilmstrip();
        }
    }

    Connections {
        target: viewerPage.controller.state

        function onFilmstripVisibleChanged() {
            if (viewerPage.controller.state.filmstripVisible)
                viewerPage.revealFilmstrip();
            else
                viewerPage.filmstripRevealed = false;
        }
    }

    Menu {
        id: imageContextMenu

        onClosed: viewerPage.forceActiveFocus()

        MenuItem {
            action: printAction
        }

        MenuSeparator {
        }

        MenuItem {
            action: fitAction
        }

        MenuItem {
            action: actualSizeAction
        }

        MenuItem {
            action: rotateLeftAction
        }

        MenuItem {
            action: rotateRightAction
        }

        MenuSeparator {
        }

        MenuItem {
            action: infoAction
        }

        MenuItem {
            action: openLocationAction
        }

        MenuItem {
            action: copyPathAction
        }

        MenuSeparator {
        }

        MenuItem {
            action: trashAction
        }

    }

    Dialog {
        id: trashConfirmationDialog

        x: Math.round((viewerPage.width - width) / 2)
        y: Math.round((viewerPage.height - height) / 2)
        width: Math.min(520, viewerPage.width - 80)
        modal: true
        title: qsTr("Enviar para a lixeira?")
        standardButtons: Dialog.Cancel | Dialog.Ok
        closePolicy: Popup.CloseOnEscape
        onOpened: standardButton(Dialog.Ok).text = qsTr("Enviar para a lixeira")
        onAccepted: viewerPage.controller.trashCurrentImage()
        onClosed: viewerPage.forceActiveFocus()

        contentItem: Label {
            text: qsTr("Enviar “%1” para a lixeira?").arg(viewerPage.controller.currentFileName)
            color: viewerPage.themePalette.windowText
            wrapMode: Text.Wrap
            padding: UiTheme.spacingLg
            Accessible.name: text
        }

    }

    FileDialog {
        id: openImageDialog

        title: qsTr("Abrir imagem")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("Imagens (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.avif *.heif *.heic *.hif *.icns)"), qsTr("Todos os arquivos (*)")]
        onAccepted: viewerPage.applicationController.openDroppedInViewer(selectedFiles)
    }

    InlineNotice {
        id: inlineNotice

        anchors.horizontalCenter: canvasArea.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: filmStrip.enabled && viewerPage.filmstripRevealed
                              ? filmStrip.height + 34 : 24
        z: 40
    }

    DropArea {
        anchors.fill: parent
        z: 800
        onEntered: function(drag) {
            drag.accepted = drag.hasUrls;
            viewerDropOverlay.visible = drag.hasUrls;
        }
        onExited: viewerDropOverlay.visible = false
        onDropped: function(drop) {
            viewerDropOverlay.visible = false;
            if (drop.hasUrls) {
                viewerPage.applicationController.openDroppedInViewer(drop.urls);
                drop.acceptProposedAction();
            }
        }
    }

    Rectangle {
        id: viewerDropOverlay

        anchors.fill: parent
        anchors.margins: 24
        z: 801
        visible: false
        color: Qt.rgba(viewerPage.themePalette.highlight.r, viewerPage.themePalette.highlight.g, viewerPage.themePalette.highlight.b, 0.2)
        border.color: viewerPage.themePalette.highlight
        border.width: 2
        radius: 12

        Label {
            anchors.centerIn: parent
            text: qsTr("Solte as imagens para visualizá-las")
            color: viewerPage.themePalette.highlight
            font.pixelSize: 20
            font.weight: Font.DemiBold
        }

    }

}
