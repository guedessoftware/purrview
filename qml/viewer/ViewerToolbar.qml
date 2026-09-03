import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property var controller
    required property var themePalette
    required property real availableWidth
    required property bool fullScreen
    required property bool controlsShown
    readonly property bool interacting: toolbarHover.hovered || overflowMenu.opened
    readonly property bool compact: availableWidth < 820
    readonly property real activeOpacity: 0.94
    readonly property real idleOpacity: 0.50

    signal fullScreenRequested()
    signal printRequested()
    signal trashRequested()
    signal aboutRequested()

    implicitWidth: actions.implicitWidth + 18
    implicitHeight: UiTheme.toolbarHeight
    radius: UiTheme.radiusMedium
    color: Qt.rgba(UiTheme.panelSurfaceStrong.r, UiTheme.panelSurfaceStrong.g,
                   UiTheme.panelSurfaceStrong.b, 0.88)
    border.color: interacting || controller.state.toolbarPinned
                  ? UiTheme.brandBorder : UiTheme.borderSubtle
    opacity: !controlsShown ? 0 : (controller.state.toolbarPinned || interacting ? activeOpacity : idleOpacity)
    enabled: controlsShown

    Behavior on opacity {
        NumberAnimation {
            duration: UiTheme.durationNormal
            easing.type: Easing.OutCubic
        }
    }

    HoverHandler {
        id: toolbarHover
    }

    RowLayout {
        id: actions

        anchors.centerIn: parent
        spacing: UiTheme.spacingXs

        Image {
            Layout.preferredWidth: 30
            Layout.preferredHeight: 30
            visible: !root.compact
            source: "qrc:/qt/qml/Impage/assets/purrview.svg"
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
        }

        Label {
            visible: root.availableWidth >= 1020
            text: "PurrView"
            color: UiTheme.textPrimary
            font.family: "serif"
            font.pixelSize: 18
            font.bold: true
            font.italic: true
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 26
            visible: !root.compact
            color: UiTheme.borderSubtle
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "▦"
            tooltipText: root.controller.state.filmstripVisible ? qsTr("Ocultar miniaturas") : qsTr("Mostrar miniaturas")
            checkable: true
            checked: root.controller.state.filmstripVisible
            enabled: root.controller.folderModel.count > 0
            onClicked: root.controller.toggleFilmstrip()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 26
            color: UiTheme.borderSubtle
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: qsTr("Fit")
            tooltipText: qsTr("Ajustar à janela (0)")
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.fitToWindow()
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: qsTr("1:1")
            tooltipText: qsTr("Tamanho real (1)")
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.actualSize()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 26
            color: UiTheme.borderSubtle
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "−"
            tooltipText: qsTr("Diminuir zoom (-)")
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.zoomOut()
        }

        Label {
            Layout.preferredWidth: 54
            text: qsTr("%1%").arg(Math.round(root.controller.state.zoomFactor * 100))
            color: root.controller.imageCount > 0 ? UiTheme.textPrimary : UiTheme.disabled
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "+"
            tooltipText: qsTr("Aumentar zoom (+)")
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.zoomIn()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 26
            color: UiTheme.borderSubtle
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "↶"
            tooltipText: qsTr("Girar para esquerda (Shift+R)")
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.rotateLeft()
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "↷"
            tooltipText: qsTr("Girar para direita (R)")
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.rotateRight()
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "ⓘ"
            tooltipText: root.controller.state.infoPanelVisible ? qsTr("Ocultar informações (I)") : qsTr("Mostrar informações (I)")
            checkable: true
            checked: root.controller.state.infoPanelVisible
            enabled: root.controller.imageCount > 0
            onClicked: root.controller.toggleInfoPanel()
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "📌"
            tooltipText: root.controller.state.toolbarPinned
                         ? qsTr("Liberar transparência da barra")
                         : qsTr("Fixar barra visível")
            checkable: true
            checked: root.controller.state.toolbarPinned
            onClicked: root.controller.state.toggleToolbarPinned()
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 26
            color: UiTheme.borderSubtle
        }

        Button {
            id: printButton

            Layout.preferredWidth: root.compact ? 42 : 108
            Layout.preferredHeight: 38
            text: root.compact ? "⎙" : qsTr("⎙  Imprimir")
            enabled: root.controller.imageCount > 0
            font.weight: Font.Bold
            Accessible.name: root.controller.printAccessibleName
            ToolTip.visible: hovered
            ToolTip.text: root.controller.printAccessibleName + qsTr(" (Ctrl+P)")
            onClicked: root.printRequested()

            contentItem: Label {
                text: printButton.text
                color: "white"
                font: printButton.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: UiTheme.radiusSmall
                opacity: printButton.enabled ? (printButton.down ? 0.80 : 1) : 0.36
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: UiTheme.brandPink }
                    GradientStop { position: 1; color: UiTheme.brandCoral }
                }
            }
        }

        ViewerToolButton {
            themePalette: root.themePalette
            text: "⋯"
            tooltipText: qsTr("Mais opções")
            onClicked: overflowMenu.popup()
        }

    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: UiTheme.radiusMedium
        anchors.rightMargin: UiTheme.radiusMedium
        height: 2
        radius: 1
        opacity: root.interacting || root.controller.state.toolbarPinned ? 0.9 : 0.35
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: UiTheme.brandPurple }
            GradientStop { position: 0.5; color: UiTheme.brandPink }
            GradientStop { position: 1; color: UiTheme.brandCoral }
        }
    }

    Menu {
        id: overflowMenu

        MenuItem {
            text: root.fullScreen ? qsTr("Sair da tela cheia") : qsTr("Visualizar em tela cheia")
            onTriggered: root.fullScreenRequested()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Abrir localização")
            onTriggered: root.controller.openCurrentFolder()
        }

        MenuItem {
            text: qsTr("Copiar caminho")
            onTriggered: root.controller.copyCurrentPath()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Enviar para a lixeira")
            enabled: root.controller.imageCount > 0
            onTriggered: root.trashRequested()
        }

        MenuSeparator {
        }

        MenuItem {
            text: qsTr("Sobre o PurrView")
            onTriggered: root.aboutRequested()
        }

    }

}
