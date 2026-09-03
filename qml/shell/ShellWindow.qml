pragma ComponentBehavior: Bound

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
    property var displayedComponent: null
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

    function currentComponent() {
        if (applicationController.modules.viewerActive)
            return viewerComponent
        if (applicationController.modules.composerActive)
            return composerComponent
        return null
    }

    function showCurrentModule() {
        if (displayedComponent === currentComponent())
            return
        moduleTransition.restart()
    }

    Component {
        id: composerComponent

        ComposerPage {
            controller: shellWindow.applicationController.modules.composerController
            applicationController: shellWindow.applicationController
            themePalette: shellWindow.palette
            onViewerRequested: shellWindow.applicationController.goBack()
            onAboutRequested: aboutDialog.open()
            onCloseRequested: shellWindow.close()
        }
    }

    Component {
        id: viewerComponent

        ViewerPage {
            controller: shellWindow.applicationController.modules.viewerController
            applicationController: shellWindow.applicationController
            themePalette: shellWindow.palette
            fullScreen: shellWindow.visibility === Window.FullScreen
            onFullScreenRequested: function(enabled) {
                shellWindow.setViewerFullScreen(enabled)
            }
            onAboutRequested: aboutDialog.open()
        }
    }

    Loader {
        id: moduleLoader

        anchors.fill: parent
        opacity: 1
        sourceComponent: shellWindow.displayedComponent
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
            script: shellWindow.displayedComponent = shellWindow.currentComponent()
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
        anchors.centerIn: parent
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
            width: Math.min(520, shellWindow.width - 80)
        }
    }

    AboutDialog {
        id: aboutDialog
        aboutInfo: shellWindow.aboutInfo
    }

    Component.onCompleted: {
        displayedComponent = currentComponent()
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
