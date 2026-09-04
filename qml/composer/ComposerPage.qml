pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: composerPage

    required property var controller
    required property var applicationController
    required property var themePalette
    property real previewZoom: 1.0
    readonly property var paperStatusLabels: [
        qsTr("A4 (210 × 297 mm)"),
        qsTr("A3 (297 × 420 mm)"),
        qsTr("A5 (148 × 210 mm)"),
        qsTr("Carta (215,9 × 279,4 mm)"),
        qsTr("Ofício (215,9 × 355,6 mm)"),
        qsTr("Foto (100 × 150 mm)")
    ]

    signal closeRequested()
    signal viewerRequested()
    signal aboutRequested()

    Action {
        id: addImagesAction
        text: qsTr("Adicionar imagens…")
        shortcut: "Ctrl+A"
        onTriggered: imageDialog.open()
    }

    Action {
        id: printAction
        text: qsTr("Imprimir")
        shortcut: "Ctrl+P"
        enabled: composerPage.controller.imageCount > 0
                 && !composerPage.controller.printDialogLoading
        onTriggered: composerPage.controller.printDocument()
    }

    Action {
        id: viewerAction
        text: qsTr("Voltar ao visualizador")
        shortcut: "Ctrl+Shift+V"
        enabled: composerPage.controller.imageCount > 0
        onTriggered: composerPage.viewerRequested()
    }

    Action {
        id: pasteImagesAction
        text: qsTr("Colar imagens")
        shortcut: "Ctrl+V"
        enabled: composerPage.controller.canPasteImages
        onTriggered: composerPage.controller.pasteImages()
    }

    Action {
        id: clearAction
        text: qsTr("Limpar imagens")
        shortcut: "Ctrl+Shift+Delete"
        enabled: composerPage.controller.imageCount > 0
        onTriggered: clearConfirmationDialog.open()
    }

    Action {
        id: selectAllImagesAction
        text: qsTr("Selecionar todas")
        shortcut: "Ctrl+Shift+A"
        enabled: composerPage.controller.imageCount > 0
        onTriggered: composerPage.controller.selectAllImages()
    }

    Action {
        id: clearImageSelectionAction
        text: qsTr("Desmarcar todas")
        enabled: composerPage.controller.selectedImageCount > 0
        onTriggered: composerPage.controller.clearImageSelection()
    }

    Action {
        id: duplicateImagesAction
        text: qsTr("Duplicar")
        shortcut: "Ctrl+D"
        enabled: composerPage.controller.selectedImageCount > 0
        onTriggered: composerPage.controller.duplicateSelectedImages()
    }

    Action {
        id: removeImagesAction
        text: qsTr("Remover da composição")
        shortcut: "Delete"
        enabled: composerPage.controller.selectedImageCount > 0
        onTriggered: composerPage.controller.removeSelectedImages()
    }

    Action {
        id: portraitAction
        text: qsTr("Orientação retrato")
        shortcut: "Ctrl+Shift+R"
        enabled: composerPage.controller.landscape
        onTriggered: composerPage.controller.landscape = false
    }

    Action {
        id: landscapeAction
        text: qsTr("Orientação paisagem")
        shortcut: "Ctrl+Shift+L"
        enabled: !composerPage.controller.landscape
        onTriggered: composerPage.controller.landscape = true
    }

    Action {
        id: fitAction
        text: qsTr("Encaixe: Ajustar (Fit)")
        shortcut: "Ctrl+1"
        onTriggered: composerPage.controller.placementMode = 0
    }

    Action {
        id: fillAction
        text: qsTr("Encaixe: Preencher (Fill)")
        shortcut: "Ctrl+2"
        onTriggered: composerPage.controller.placementMode = 1
    }

    Action {
        id: stretchAction
        text: qsTr("Encaixe: Esticar")
        shortcut: "Ctrl+3"
        onTriggered: composerPage.controller.placementMode = 2
    }

    Action {
        id: increaseRowsAction
        text: qsTr("Adicionar linha")
        shortcut: "Alt+Up"
        enabled: composerPage.controller.rows < 20
        onTriggered: composerPage.controller.rows++
    }

    Action {
        id: decreaseRowsAction
        text: qsTr("Remover linha")
        shortcut: "Alt+Down"
        enabled: composerPage.controller.rows > 1
        onTriggered: composerPage.controller.rows--
    }

    Action {
        id: increaseColumnsAction
        text: qsTr("Adicionar coluna")
        shortcut: "Alt+Right"
        enabled: composerPage.controller.columns < 20
        onTriggered: composerPage.controller.columns++
    }

    Action {
        id: decreaseColumnsAction
        text: qsTr("Remover coluna")
        shortcut: "Alt+Left"
        enabled: composerPage.controller.columns > 1
        onTriggered: composerPage.controller.columns--
    }

    Action {
        id: previousPageAction
        text: qsTr("Página anterior")
        shortcut: "PgUp"
        enabled: composerPage.controller.currentPage > 0
        onTriggered: composerPage.controller.currentPage--
    }

    Action {
        id: nextPageAction
        text: qsTr("Próxima página")
        shortcut: "PgDown"
        enabled: composerPage.controller.currentPage + 1 < composerPage.controller.pageCount
        onTriggered: composerPage.controller.currentPage++
    }

    Action {
        id: firstPageAction
        text: qsTr("Primeira página")
        shortcut: "Ctrl+Home"
        enabled: composerPage.controller.currentPage > 0
        onTriggered: composerPage.controller.currentPage = 0
    }

    Action {
        id: lastPageAction
        text: qsTr("Última página")
        shortcut: "Ctrl+End"
        enabled: composerPage.controller.currentPage + 1 < composerPage.controller.pageCount
        onTriggered: composerPage.controller.currentPage = composerPage.controller.pageCount - 1
    }

    Action {
        id: shortcutsHelpAction
        text: qsTr("Atalhos do teclado")
        shortcut: "F1"
        onTriggered: shortcutsDialog.open()
    }

    Action {
        id: organizationHelpAction
        text: qsTr("Como organizar as imagens")
        onTriggered: organizationHelpDialog.open()
    }

    Action {
        id: aboutAction
        text: qsTr("Sobre o PurrView")
        onTriggered: composerPage.aboutRequested()
    }

    Action {
        id: quitAction
        text: qsTr("Sair")
        shortcut: "Ctrl+Q"
        onTriggered: composerPage.closeRequested()
    }

    Shortcut {
        sequence: "Ctrl+O"
        context: Qt.WindowShortcut
        onActivated: addImagesAction.trigger()
    }

    Shortcut {
        sequence: "Ctrl+W"
        context: Qt.WindowShortcut
        onActivated: quitAction.trigger()
    }

    component HeaderActionButton: Button {
        id: headerButton

        property string iconName: ""
        property bool accent: false
        property bool iconOnly: false
        property real minimumButtonWidth: 0

        Layout.alignment: Qt.AlignVCenter
        implicitWidth: iconOnly ? 42
                                : Math.max(minimumButtonWidth,
                                           contentItem.implicitWidth
                                           + leftPadding + rightPadding)
        implicitHeight: 42
        leftPadding: iconOnly ? 12 : 18
        rightPadding: iconOnly ? 12 : 18
        topPadding: 10
        bottomPadding: 10
        spacing: 9
        hoverEnabled: true
        font.weight: Font.DemiBold

        contentItem: Item {
            implicitWidth: headerButton.iconOnly ? 18 : headerContent.implicitWidth
            implicitHeight: 20

            PurrIcon {
                anchors.centerIn: parent
                width: 18
                height: 18
                visible: headerButton.iconOnly
                name: headerButton.iconName
                color: headerButton.enabled
                       ? composerPage.themePalette.buttonText
                       : composerPage.themePalette.placeholderText
            }

            PurrIconLabel {
                id: headerContent
                anchors.centerIn: parent
                visible: !headerButton.iconOnly
                iconName: headerButton.iconName
                iconSize: 18
                spacing: headerButton.spacing
                text: headerButton.text
                color: headerButton.enabled
                       ? composerPage.themePalette.buttonText
                       : composerPage.themePalette.placeholderText
                font: headerButton.font
            }
        }

        background: Rectangle {
            radius: 8
            color: headerButton.accent
                   ? Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                             UiTheme.brandPurple.b,
                             headerButton.down ? 0.25
                                               : headerButton.hovered ? 0.18 : 0.11)
                   : headerButton.down ? UiTheme.quietControlPressed
                     : headerButton.hovered ? UiTheme.quietControlHover
                                            : UiTheme.idleControlSurface
            border.color: headerButton.accent
                          ? UiTheme.brandPurple
                          : headerButton.hovered ? UiTheme.brandBorder
                                                 : UiTheme.insetBorder
            border.width: 1
            opacity: headerButton.enabled ? 1.0 : 0.55
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        MenuBar {
            Layout.fillWidth: true
        Menu {
            title: qsTr("Arquivo")
            MenuItem { action: addImagesAction }
            MenuItem { action: pasteImagesAction }
            MenuItem { action: viewerAction }
            MenuItem { action: printAction }
            MenuSeparator {}
            MenuItem { action: clearAction }
            MenuSeparator {}
            MenuItem { action: quitAction }
        }

        Menu {
            title: qsTr("Layout")
            Menu {
                title: qsTr("Orientação")
                MenuItem { action: portraitAction }
                MenuItem { action: landscapeAction }
            }
            Menu {
                title: qsTr("Encaixe")
                MenuItem { action: fitAction }
                MenuItem { action: fillAction }
                MenuItem { action: stretchAction }
            }
            Menu {
                title: qsTr("Grade")
                MenuItem { action: increaseRowsAction }
                MenuItem { action: decreaseRowsAction }
                MenuSeparator {}
                MenuItem { action: increaseColumnsAction }
                MenuItem { action: decreaseColumnsAction }
            }
            Menu {
                title: qsTr("Páginas")
                MenuItem { action: previousPageAction }
                MenuItem { action: nextPageAction }
                MenuSeparator {}
                MenuItem { action: firstPageAction }
                MenuItem { action: lastPageAction }
            }
        }

        Menu {
            title: qsTr("Imagens")
            MenuItem { action: selectAllImagesAction }
            MenuItem { action: clearImageSelectionAction }
            MenuSeparator {}
            MenuItem { action: duplicateImagesAction }
            MenuItem { action: removeImagesAction }
        }

        Menu {
            title: qsTr("Ajuda")
            MenuItem { action: organizationHelpAction }
            MenuItem { action: shortcutsHelpAction }
            MenuSeparator {}
            MenuItem { action: aboutAction }
        }
    }

        ToolBar {
        Layout.fillWidth: true
        Layout.preferredHeight: 76
        background: Rectangle {
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                                   UiTheme.brandPurple.b, 0.08)
                }
                GradientStop { position: 1; color: composerPage.themePalette.window }
            }
            border.color: UiTheme.brandBorder
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            spacing: 10

            RowLayout {
                spacing: 8
                Layout.rightMargin: 16
                Layout.alignment: Qt.AlignVCenter

                Image {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    source: "qrc:/qt/qml/PurrView/assets/purrview.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                }

                Label {
                    text: "PurrView"
                    color: composerPage.themePalette.windowText
                    font.family: "serif"
                    font.pixelSize: 27
                    font.bold: true
                    font.italic: true
                }
            }

            HeaderActionButton {
                id: addImagesButton
                action: addImagesAction
                text: qsTr("Adicionar")
                iconName: "add-image"
                accent: true
                minimumButtonWidth: 150
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Adicionar imagens (Ctrl+A ou Ctrl+O)")
            }

            HeaderActionButton {
                id: clearButton
                action: clearAction
                text: qsTr("Limpar")
                iconName: "trash"
                iconOnly: true
                Accessible.name: qsTr("Limpar imagens")
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Limpar imagens (Ctrl+Shift+Delete)")
            }

            Rectangle {
                Layout.preferredHeight: 42
                Layout.preferredWidth: imageCountLabel.implicitWidth + 36
                Layout.alignment: Qt.AlignVCenter
                radius: 8
                color: UiTheme.idleControlSurface
                border.color: UiTheme.insetBorder

                Label {
                    id: imageCountLabel
                    anchors.centerIn: parent
                    text: composerPage.controller.imageCount === 1
                          ? qsTr("1 imagem")
                          : qsTr("%1 imagens").arg(composerPage.controller.imageCount)
                    color: composerPage.themePalette.windowText
                    font.weight: Font.Medium
                }
            }

            ListView {
                id: thumbnailList

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 80
                Layout.leftMargin: 6
                orientation: ListView.Horizontal
                spacing: 6
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: composerPage.controller.imageThumbnails

                onCountChanged: {
                    if (count > 0)
                        positionViewAtEnd()
                }

                delegate: Rectangle {
                    id: thumbnailDelegate

                    required property int index
                    required property var modelData
                    readonly property int pageIndex: Math.floor(
                        index / Math.max(1, composerPage.controller.rows
                                           * composerPage.controller.columns))
                    readonly property bool selected: Boolean(modelData.selected)

                    width: 51
                    height: 51
                    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                    radius: 4
                    color: pageIndex === composerPage.controller.currentPage
                           ? composerPage.themePalette.alternateBase
                           : composerPage.themePalette.base
                    border.color: selected ? composerPage.themePalette.highlight
                                           : composerPage.themePalette.mid
                    border.width: selected ? 2 : 1
                    opacity: thumbnailMouse.wasDragging ? 0.58 : 1.0
                    z: thumbnailMouse.wasDragging ? 10 : 0

                    Image {
                        anchors.fill: parent
                        anchors.margins: 3
                        source: thumbnailDelegate.modelData.source
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: true
                        sourceSize.width: 102
                        sourceSize.height: 102
                    }

                    MouseArea {
                        id: thumbnailMouse

                        property int sourceIndex: -1
                        property real originalX: 0
                        property bool wasDragging: false

                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        hoverEnabled: true
                        preventStealing: true
                        cursorShape: wasDragging ? Qt.ClosedHandCursor
                                                 : Qt.OpenHandCursor
                        drag.target: thumbnailDelegate
                        drag.axis: Drag.XAxis
                        drag.minimumX: 0
                        drag.maximumX: Math.max(0, thumbnailList.contentWidth
                                                  - thumbnailDelegate.width)
                        drag.threshold: 8
                        onPressed: {
                            sourceIndex = thumbnailDelegate.index
                            originalX = thumbnailDelegate.x
                            wasDragging = false
                        }
                        onPositionChanged: wasDragging = wasDragging || drag.active
                        onReleased: {
                            if (!wasDragging || sourceIndex < 0)
                                return
                            const originalIndex = sourceIndex
                            const slotWidth = thumbnailDelegate.width
                                            + thumbnailList.spacing
                            const destination = Math.max(
                                        0, Math.min(
                                            composerPage.controller.imageCount - 1,
                                            Math.floor((thumbnailDelegate.x
                                                        + thumbnailDelegate.width / 2)
                                                       / slotWidth)))
                            thumbnailDelegate.x = originalX
                            sourceIndex = -1
                            wasDragging = false
                            composerPage.controller.moveImagesToPosition(
                                        originalIndex, destination)
                        }
                        onCanceled: {
                            thumbnailDelegate.x = originalX
                            sourceIndex = -1
                            wasDragging = false
                        }
                        onClicked: function(mouse) {
                            if (mouse.button === Qt.RightButton) {
                                if (!thumbnailDelegate.selected)
                                    composerPage.controller.selectImage(
                                                thumbnailDelegate.index, false, false)
                                thumbnailMenu.popup()
                            } else if (mouse.modifiers & Qt.ControlModifier) {
                                composerPage.controller.selectImage(
                                            thumbnailDelegate.index, true, false)
                            } else if (mouse.modifiers & Qt.ShiftModifier) {
                                composerPage.controller.selectImage(
                                            thumbnailDelegate.index, false, true)
                            } else {
                                composerPage.controller.currentPage
                                        = thumbnailDelegate.pageIndex
                            }
                        }
                    }

                    ToolButton {
                        id: duplicateThumbnailButton

                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 3
                        width: 21
                        height: 21
                        z: 3
                        hoverEnabled: true
                        visible: thumbnailDelegate.selected
                                 || thumbnailMouse.containsMouse
                                 || hovered
                                 || removeThumbnailButton.hovered
                        Accessible.name: qsTr("Duplicar imagem")
                        ToolTip.visible: hovered
                        ToolTip.text: composerPage.controller.selectedImageCount > 1
                                      && thumbnailDelegate.selected
                                      ? qsTr("Duplicar imagens selecionadas")
                                      : qsTr("Duplicar imagem")
                        onClicked: {
                            if (!thumbnailDelegate.selected)
                                composerPage.controller.selectImage(
                                            thumbnailDelegate.index, false, false)
                            composerPage.controller.duplicateSelectedImages()
                        }

                        contentItem: Item {
                            PurrIcon {
                                anchors.centerIn: parent
                                name: "duplicate"
                                color: UiTheme.textPrimary
                                width: 12
                                height: 12
                            }
                        }

                        background: Rectangle {
                            radius: 6
                            color: duplicateThumbnailButton.down
                                   ? UiTheme.floatingSurfacePressed
                                   : duplicateThumbnailButton.hovered
                                     ? UiTheme.floatingSurfaceHover
                                     : UiTheme.floatingSurface
                            border.color: duplicateThumbnailButton.hovered
                                          ? UiTheme.brandPurple
                                          : UiTheme.filmStripBorder
                        }
                    }

                    ToolButton {
                        id: removeThumbnailButton

                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 3
                        width: 21
                        height: 21
                        z: 3
                        hoverEnabled: true
                        visible: thumbnailDelegate.selected
                                 || thumbnailMouse.containsMouse
                                 || hovered
                                 || duplicateThumbnailButton.hovered
                        Accessible.name: qsTr("Remover imagem da composição")
                        ToolTip.visible: hovered
                        ToolTip.text: composerPage.controller.selectedImageCount > 1
                                      && thumbnailDelegate.selected
                                      ? qsTr("Remover imagens selecionadas")
                                      : qsTr("Remover imagem da composição")
                        onClicked: {
                            if (!thumbnailDelegate.selected)
                                composerPage.controller.selectImage(
                                            thumbnailDelegate.index, false, false)
                            composerPage.controller.removeSelectedImages()
                        }

                        contentItem: Item {
                            PurrIcon {
                                anchors.centerIn: parent
                                name: "trash"
                                color: UiTheme.danger
                                width: 12
                                height: 12
                            }
                        }

                        background: Rectangle {
                            radius: 6
                            color: removeThumbnailButton.down
                                   ? UiTheme.floatingSurfacePressed
                                   : removeThumbnailButton.hovered
                                     ? UiTheme.floatingSurfaceHover
                                     : UiTheme.floatingSurface
                            border.color: removeThumbnailButton.hovered
                                          ? UiTheme.danger
                                          : UiTheme.filmStripBorder
                        }
                    }

                    ToolTip.visible: thumbnailMouse.containsMouse
                                         && !thumbnailMouse.wasDragging
                                         && !duplicateThumbnailButton.hovered
                                         && !removeThumbnailButton.hovered
                    ToolTip.text: qsTr("%1 — página %2\nClique: abrir página · Ctrl+clique: selecionar · Arraste: reordenar")
                                  .arg(thumbnailDelegate.modelData.name)
                                  .arg(thumbnailDelegate.pageIndex + 1)
                }

                ScrollBar.horizontal: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }

            HeaderActionButton {
                id: viewerButton
                action: viewerAction
                text: qsTr("Visualizar")
                iconName: "eye"
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Voltar ao visualizador (Ctrl+Shift+V)")
            }

            Button {
                id: printButton
                action: printAction
                text: qsTr("Imprimir")
                implicitHeight: 42
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 18
                rightPadding: 18
                topPadding: 10
                bottomPadding: 10
                font.weight: Font.Bold
                contentItem: Item {
                    implicitWidth: printContent.implicitWidth
                    implicitHeight: Math.max(20, printContent.implicitHeight)

                    PurrIconLabel {
                        id: printContent
                        anchors.centerIn: parent
                        iconName: "printer"
                        iconSize: 18
                        spacing: 9
                        text: printButton.text
                        color: UiTheme.brightText
                        font: printButton.font
                    }
                }
                background: Rectangle {
                    radius: 8
                    opacity: printButton.enabled ? (printButton.down ? 0.82 : 1.0) : 0.42
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0; color: UiTheme.brandPink }
                        GradientStop { position: 1; color: UiTheme.brandCoral }
                    }
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Imprimir (Ctrl+P)")
            }
        }
    }

        RowLayout {
        id: compositionArea
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 14

        Rectangle {
            Layout.preferredWidth: 338
            Layout.fillHeight: true
            Layout.leftMargin: 12
            Layout.topMargin: 12
            Layout.bottomMargin: 12
            color: UiTheme.panelSurface
            border.color: UiTheme.brandBorder
            radius: UiTheme.radiusLarge
            clip: true

            LayoutPanel {
                anchors.fill: parent
                controller: composerPage.controller
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 12
            Layout.rightMargin: 12
            Layout.bottomMargin: 12

            PagePreview {
                anchors.fill: parent
                anchors.bottomMargin: 42 + (composerPage.controller.pageCount > 1 ? 54 : 0)
                controller: composerPage.controller
                themePalette: composerPage.themePalette
                zoomFactor: composerPage.previewZoom
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 42
                height: 54
                visible: composerPage.controller.pageCount > 1
                color: composerPage.themePalette.window
                border.color: composerPage.themePalette.mid

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 14

                    Button {
                        action: previousPageAction
                        text: qsTr("Anterior")
                    }
                    Label {
                        text: qsTr("Página %1 de %2")
                              .arg(composerPage.controller.currentPage + 1)
                              .arg(composerPage.controller.pageCount)
                        color: composerPage.themePalette.windowText
                        font.weight: Font.DemiBold
                    }
                    Button {
                        action: nextPageAction
                        text: qsTr("Próxima")
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 42
                color: Qt.rgba(composerPage.themePalette.window.r,
                               composerPage.themePalette.window.g,
                               composerPage.themePalette.window.b, 0.96)
                border.color: UiTheme.brandBorder
                radius: 8

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 8
                    spacing: 10

                    Label {
                        text: qsTr("Papel: %1").arg(
                                  composerPage.paperStatusLabels[
                                      composerPage.controller.paperSize])
                        color: composerPage.themePalette.placeholderText
                        elide: Text.ElideRight
                    }
                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 18
                        color: composerPage.themePalette.mid
                    }
                    Label {
                        text: composerPage.controller.landscape
                              ? qsTr("Paisagem") : qsTr("Retrato")
                        color: composerPage.themePalette.placeholderText
                    }
                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 18
                        color: composerPage.themePalette.mid
                    }
                    Label {
                        text: qsTr("Grade: %1 × %2")
                              .arg(composerPage.controller.rows)
                              .arg(composerPage.controller.columns)
                        color: composerPage.themePalette.placeholderText
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        flat: true
                        implicitWidth: 38
                        Accessible.name: qsTr("Diminuir visualização")
                        enabled: composerPage.previewZoom > 0.5
                        onClicked: composerPage.previewZoom = Math.max(
                                       0.5, composerPage.previewZoom - 0.1)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Diminuir visualização")
                        contentItem: PurrIcon {
                            name: "minus"
                            color: composerPage.themePalette.buttonText
                        }
                    }
                    Label {
                        Layout.preferredWidth: 48
                        horizontalAlignment: Text.AlignHCenter
                        text: Math.round(composerPage.previewZoom * 100) + "%"
                        color: composerPage.themePalette.windowText
                        font.weight: Font.DemiBold
                    }
                    Button {
                        flat: true
                        implicitWidth: 38
                        Accessible.name: qsTr("Ampliar visualização")
                        enabled: composerPage.previewZoom < 1.5
                        onClicked: composerPage.previewZoom = Math.min(
                                       1.5, composerPage.previewZoom + 0.1)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Ampliar visualização")
                        contentItem: PurrIcon {
                            name: "plus"
                            color: composerPage.themePalette.buttonText
                        }
                    }
                    Button {
                        flat: true
                        implicitWidth: 42
                        Accessible.name: qsTr("Ajustar página")
                        onClicked: composerPage.previewZoom = 1.0
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Ajustar página")
                        contentItem: PurrIcon {
                            name: "fullscreen"
                            color: composerPage.themePalette.buttonText
                        }
                    }
                }
            }

        }
        }
    }

    DropArea {
        anchors.fill: parent
        z: 800
        onEntered: function(drag) {
            drag.accepted = drag.hasUrls
            dropOverlay.visible = drag.hasUrls
        }
        onExited: dropOverlay.visible = false
        onDropped: function(drop) {
            dropOverlay.visible = false
            if (drop.hasUrls) {
                composerPage.applicationController.openDroppedInComposer(drop.urls)
                drop.acceptProposedAction()
            }
        }
    }

    Rectangle {
        id: dropOverlay
        anchors.fill: parent
        anchors.margins: 24
        z: 801
        visible: false
        color: Qt.rgba(composerPage.themePalette.highlight.r,
                       composerPage.themePalette.highlight.g,
                       composerPage.themePalette.highlight.b, 0.20)
        border.color: composerPage.themePalette.highlight
        border.width: 2
        radius: 12

        Label {
            anchors.centerIn: parent
            text: qsTr("Solte as imagens para adicioná-las")
            color: composerPage.themePalette.highlight
            font.pixelSize: 20
            font.weight: Font.DemiBold
        }
    }

    Rectangle {
        anchors.fill: parent
        z: 1000
        visible: composerPage.controller.printDialogLoading
        color: Qt.rgba(composerPage.themePalette.shadow.r,
                       composerPage.themePalette.shadow.g,
                       composerPage.themePalette.shadow.b, 0.48)

        MouseArea { anchors.fill: parent }

        Rectangle {
            anchors.centerIn: parent
            width: loadingContent.implicitWidth + 48
            height: loadingContent.implicitHeight + 36
            color: composerPage.themePalette.window
            border.color: composerPage.themePalette.mid
            radius: 8

            ColumnLayout {
                id: loadingContent
                anchors.centerIn: parent
                spacing: 12

                BusyIndicator {
                    Layout.alignment: Qt.AlignHCenter
                    running: composerPage.controller.printDialogLoading
                }
                Label {
                    text: qsTr("Consultando impressoras…")
                    color: composerPage.themePalette.windowText
                    font.weight: Font.DemiBold
                }
            }
        }
    }

    FileDialog {
        id: imageDialog
        title: qsTr("Escolher imagens")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("Imagens (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.avif *.heif *.heic *.hif *.icns)"),
                      qsTr("Todos os arquivos (*)")]
        onAccepted: composerPage.controller.addImages(selectedFiles)
    }

    Menu {
        id: thumbnailMenu
        MenuItem { action: duplicateImagesAction }
        MenuItem { action: removeImagesAction }
        MenuSeparator {}
        MenuItem { action: selectAllImagesAction }
        MenuItem { action: clearImageSelectionAction }
    }

    Dialog {
        id: organizationHelpDialog
        x: Math.round((composerPage.width - width) / 2)
        y: Math.round((composerPage.height - height) / 2)
        width: Math.min(620, composerPage.width - 48)
        modal: true
        title: qsTr("Como organizar as imagens")
        standardButtons: Dialog.Ok

        contentItem: Label {
            text: qsTr("• Clique simples em uma miniatura: abre a página correspondente sem selecioná-la.\n\n"
                       + "• Ctrl+clique: marca ou desmarca imagens para ações em grupo. Shift+clique seleciona um intervalo a partir da última imagem marcada.\n\n"
                       + "• Faixa superior: clique, segure e arraste uma miniatura para mudar sua posição. Se ela estiver marcada, o grupo selecionado será movido junto.\n\n"
                       + "• Página: clique e segure uma foto no preview, arraste-a até outro quadro e solte para alterar a ordem diretamente na composição.\n\n"
                       + "• Ctrl+D duplica as imagens marcadas. Delete remove somente as ocorrências da composição; os arquivos originais não são apagados.")
            wrapMode: Text.WordWrap
            color: organizationHelpDialog.palette.windowText
        }
    }

    Dialog {
        id: messageDialog
        x: Math.round((composerPage.width - width) / 2)
        y: Math.round((composerPage.height - height) / 2)
        width: Math.min(440, composerPage.width - 48)
        modal: true
        title: heading
        standardButtons: Dialog.Ok
        property string heading: qsTr("PurrView")
        property string message: ""

        contentItem: Label {
            id: messageLabel
            text: messageDialog.message
            wrapMode: Text.WordWrap
            color: messageDialog.palette.windowText
            verticalAlignment: Text.AlignVCenter
        }
    }

    Dialog {
        id: clearConfirmationDialog
        x: Math.round((composerPage.width - width) / 2)
        y: Math.round((composerPage.height - height) / 2)
        width: Math.min(440, composerPage.width - 48)
        modal: true
        title: qsTr("Limpar imagens")
        standardButtons: Dialog.Yes | Dialog.Cancel
        onAccepted: composerPage.controller.clearImages()

        contentItem: Label {
            text: qsTr("Remover todas as imagens da composição?")
            wrapMode: Text.WordWrap
            color: clearConfirmationDialog.palette.windowText
        }
    }

    Dialog {
        id: shortcutsDialog
        x: Math.round((composerPage.width - width) / 2)
        y: Math.round((composerPage.height - height) / 2)
        width: Math.min(540, composerPage.width - 48)
        modal: true
        title: qsTr("Atalhos do teclado")
        standardButtons: Dialog.Ok

        contentItem: GridLayout {
            columns: 2
            columnSpacing: 24
            rowSpacing: 9

            Repeater {
                model: [
                    { keys: "Ctrl+A / Ctrl+O", label: qsTr("Adicionar imagens") },
                    { keys: "Ctrl+V", label: qsTr("Colar imagens da área de transferência") },
                    { keys: "Ctrl+Shift+V", label: qsTr("Voltar ao visualizador") },
                    { keys: "Ctrl+P", label: qsTr("Imprimir") },
                    { keys: "Ctrl+Shift+A", label: qsTr("Selecionar todas as imagens") },
                    { keys: "Ctrl+D", label: qsTr("Duplicar imagens selecionadas") },
                    { keys: "Delete", label: qsTr("Remover selecionadas da composição") },
                    { keys: "Ctrl+Shift+Delete", label: qsTr("Limpar imagens") },
                    { keys: "Ctrl+Shift+R / L", label: qsTr("Retrato / Paisagem") },
                    { keys: "Ctrl+1 / 2 / 3", label: qsTr("Fit / Fill / Stretch") },
                    { keys: "Alt+↑ / Alt+↓", label: qsTr("Adicionar / remover linha") },
                    { keys: "Alt+→ / Alt+←", label: qsTr("Adicionar / remover coluna") },
                    { keys: "PgUp / PgDown", label: qsTr("Página anterior / próxima") },
                    { keys: "Ctrl+Home / Ctrl+End", label: qsTr("Primeira / última página") },
                    { keys: "F1", label: qsTr("Mostrar estes atalhos") },
                    { keys: "Ctrl+Q / Ctrl+W", label: qsTr("Sair") }
                ]

                delegate: RowLayout {
                    id: shortcutRow
                    required property var modelData
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    Label {
                        Layout.preferredWidth: 190
                        text: shortcutRow.modelData.keys
                        color: shortcutsDialog.palette.highlight
                        font.family: "monospace"
                        font.weight: Font.DemiBold
                    }
                    Label {
                        Layout.fillWidth: true
                        text: shortcutRow.modelData.label
                        color: shortcutsDialog.palette.windowText
                    }
                }
            }
        }
    }

    Connections {
        target: composerPage.controller
        function onErrorOccurred(message) {
            messageDialog.heading = qsTr("Não foi possível concluir")
            messageDialog.message = message
            messageDialog.open()
        }
        function onPrintFinished() {
            messageDialog.heading = qsTr("Impressão")
            messageDialog.message = composerPage.controller.pageCount === 1
                    ? qsTr("A página foi enviada para a impressora.")
                    : qsTr("As %1 páginas foram enviadas para a impressora.")
                      .arg(composerPage.controller.pageCount)
            messageDialog.open()
        }
    }
}
