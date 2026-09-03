import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: shellWindow

    required property var applicationController
    required property var aboutInfo

    width: applicationController.savedWindowWidth
    height: applicationController.savedWindowHeight
    minimumWidth: applicationController.modules.viewerActive ? 640 : 860
    minimumHeight: applicationController.modules.viewerActive ? 480 : 600
    visible: true
    title: applicationController.windowTitle
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint
           | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint
           | Qt.WindowCloseButtonHint
    color: palette.window
    property string displayedSource: ""
    property bool maximizedBeforeFullScreen: false

    function setViewerFullScreen(enabled) {
        if (enabled && visibility !== Window.FullScreen) {
            maximizedBeforeFullScreen = visibility === Window.Maximized
            showFullScreen()
        } else if (!enabled && visibility === Window.FullScreen) {
            if (maximizedBeforeFullScreen)
                showMaximized()
            else
                showNormal()
        }
    }

    function currentSource() {
        if (applicationController.modules.viewerActive)
            return "ViewerPage.qml"
        if (applicationController.modules.composerActive)
            return "ComposerPage.qml"
        return ""
    }

    function loadCurrentModule() {
        displayedSource = currentSource()
        if (applicationController.modules.viewerActive) {
            moduleLoader.setSource(displayedSource, {
                "controller": applicationController.modules.viewerController,
                "applicationController": applicationController,
                "themePalette": Qt.binding(function() { return shellWindow.palette }),
                "fullScreen": Qt.binding(function() {
                    return shellWindow.visibility === Window.FullScreen
                })
            })
        } else if (applicationController.modules.composerActive) {
            moduleLoader.setSource(displayedSource, {
                "controller": applicationController.modules.composerController,
                "applicationController": applicationController,
                "themePalette": Qt.binding(function() { return shellWindow.palette })
            })
        } else {
            moduleLoader.source = ""
        }
    }

    function showCurrentModule() {
        if (displayedSource === currentSource())
            return
        moduleTransition.restart()
    }

    Loader {
        id: moduleLoader

        anchors.fill: parent
        opacity: 1
    }

    Connections {
        target: moduleLoader.item
        ignoreUnknownSignals: true

        function onViewerRequested() { shellWindow.applicationController.goBack() }
        function onFullScreenRequested(enabled) { shellWindow.setViewerFullScreen(enabled) }
        function onAboutRequested() { aboutDialog.open() }
        function onCloseRequested() { shellWindow.close() }
    }

    SequentialAnimation {
        id: moduleTransition

        NumberAnimation {
            target: moduleLoader
            property: "opacity"
            from: moduleLoader.opacity
            to: 0
            duration: 70
            easing.type: Easing.OutQuad
        }
        ScriptAction {
            script: shellWindow.loadCurrentModule()
        }
        NumberAnimation {
            target: moduleLoader
            property: "opacity"
            from: 0
            to: 1
            duration: 100
            easing.type: Easing.InQuad
        }
    }

    Connections {
        target: shellWindow.applicationController.modules
        function onCurrentModuleChanged() {
            shellWindow.showCurrentModule()
        }
    }

    Connections {
        target: shellWindow.applicationController
        function onWindowActivationRequested() {
            shellWindow.show()
            shellWindow.raise()
            shellWindow.requestActivate()
        }
        function onOpenRequestError(message) {
            openErrorDialog.message = message
            openErrorDialog.open()
        }
    }

    Dialog {
        id: openErrorDialog
        x: Math.round((shellWindow.width - width) / 2)
        y: Math.round((shellWindow.height - height) / 2)
        width: Math.min(520, shellWindow.width - 80)
        modal: true
        title: qsTr("Não foi possível abrir")
        standardButtons: Dialog.Ok
        property string message: ""
        onClosed: shellWindow.applicationController.clearOpenRequestError()
        contentItem: Label {
            text: openErrorDialog.message
            wrapMode: Text.Wrap
            color: shellWindow.palette.windowText
            padding: 18
        }
    }

    AboutDialog {
        id: aboutDialog
        aboutInfo: shellWindow.aboutInfo
    }

    Component.onCompleted: {
        loadCurrentModule()
        if (applicationController.savedWindowMaximized)
            showMaximized()
        if (applicationController.lastOpenRequestError.length > 0) {
            openErrorDialog.message = applicationController.lastOpenRequestError
            openErrorDialog.open()
        }
    }
    onClosing: applicationController.saveWindowState(width, height,
                                                       visibility === Window.Maximized)
}
