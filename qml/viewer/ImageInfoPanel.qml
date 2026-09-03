import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    required property var controller
    required property var themePalette

    color: UiTheme.panelSurfaceStrong
    border.width: 1
    border.color: UiTheme.brandBorder
    clip: true
    Accessible.role: Accessible.Pane
    Accessible.name: qsTr("Informações da imagem")

    Rectangle {
        id: header

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 60
        color: Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                       UiTheme.brandPurple.b, 0.06)

        Label {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 20
            text: qsTr("Informações")
            color: UiTheme.textPrimary
            font.pixelSize: 16
            font.weight: Font.Bold
        }

        BusyIndicator {
            anchors.right: closeButton.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 8
            width: 24
            height: 24
            running: visible
            visible: root.controller.metadata.loading
            Accessible.name: qsTr("Carregando metadados")
        }

        ToolButton {
            id: closeButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            width: 38
            height: 38
            text: "×"
            Accessible.name: qsTr("Fechar informações")
            onClicked: root.controller.toggleInfoPanel()

            contentItem: Label {
                text: closeButton.text
                color: UiTheme.textPrimary
                font.pixelSize: 22
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: UiTheme.radiusSmall
                color: closeButton.down ? Qt.rgba(1, 1, 1, 0.18) : (closeButton.hovered ? UiTheme.floatingSurfaceHover : "transparent")
            }

        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Qt.rgba(1, 1, 1, 0.1)
        }

        Rectangle {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 20
            width: 54
            height: 2
            radius: 1
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0; color: UiTheme.brandPurple }
                GradientStop { position: 1; color: UiTheme.brandPink }
            }
        }

    }

    ScrollView {
        id: infoScroll

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        Column {
            width: infoScroll.availableWidth
            padding: 20
            spacing: 20

            MetadataSection {
                width: parent.width - parent.leftPadding - parent.rightPadding
                title: qsTr("Arquivo")

                MetadataRow {
                    labelText: qsTr("Nome")
                    valueText: root.controller.metadata.fileName
                }

                MetadataRow {
                    labelText: qsTr("Formato")
                    valueText: root.controller.metadata.format
                }

                MetadataRow {
                    labelText: qsTr("Tamanho")
                    valueText: root.controller.metadata.fileSizeText
                }

                MetadataRow {
                    labelText: root.controller.metadata.dateLabel
                    valueText: root.controller.metadata.dateText
                }

                MetadataRow {
                    labelText: qsTr("Localização do arquivo")
                    valueText: root.controller.metadata.absolutePath
                }

                Button {
                    id: openFolderButton

                    width: 112
                    height: 36
                    text: qsTr("Abrir pasta")
                    enabled: root.controller.metadata.absolutePath.length > 0
                    Accessible.name: qsTr("Abrir pasta da imagem")
                    onClicked: root.controller.openCurrentFolder()

                    contentItem: Label {
                        text: openFolderButton.text
                        color: openFolderButton.enabled ? UiTheme.textPrimary : UiTheme.disabled
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        radius: UiTheme.radiusSmall
                        color: openFolderButton.down
                               ? Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                                         UiTheme.brandPurple.b, 0.30)
                               : (openFolderButton.hovered
                                  ? Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                                            UiTheme.brandPurple.b, 0.20)
                                  : Qt.rgba(1, 1, 1, 0.06))
                        border.width: 1
                        border.color: openFolderButton.hovered
                                      ? UiTheme.brandPink : UiTheme.brandBorder
                    }
                }

            }

            MetadataSection {
                width: parent.width - parent.leftPadding - parent.rightPadding
                title: qsTr("Imagem")
                visible: root.controller.metadata.hasImageSection

                MetadataRow {
                    labelText: qsTr("Dimensões")
                    valueText: root.controller.metadata.dimensionsText
                }

                MetadataRow {
                    labelText: qsTr("Resolução")
                    valueText: root.controller.metadata.megapixelsText
                }

                MetadataRow {
                    labelText: qsTr("Orientação")
                    valueText: root.controller.metadata.orientationText
                }

                MetadataRow {
                    labelText: qsTr("Perfil de cor")
                    valueText: root.controller.metadata.colorProfileText
                }

            }

            MetadataSection {
                width: parent.width - parent.leftPadding - parent.rightPadding
                title: qsTr("Câmera")
                visible: root.controller.metadata.hasCameraSection

                MetadataRow {
                    labelText: qsTr("Câmera")
                    valueText: root.controller.metadata.cameraName
                }

                MetadataRow {
                    labelText: qsTr("Lente")
                    valueText: root.controller.metadata.lens
                }

                MetadataRow {
                    labelText: qsTr("Abertura")
                    valueText: root.controller.metadata.apertureText
                }

                MetadataRow {
                    labelText: qsTr("Exposição")
                    valueText: root.controller.metadata.exposureText
                }

                MetadataRow {
                    labelText: qsTr("ISO")
                    valueText: root.controller.metadata.isoText
                }

                MetadataRow {
                    labelText: qsTr("Distância focal")
                    valueText: root.controller.metadata.focalLengthText
                }

                MetadataRow {
                    labelText: qsTr("Equivalente em 35 mm")
                    valueText: root.controller.metadata.focalLength35mmText
                }

                MetadataRow {
                    labelText: qsTr("Programa")
                    valueText: root.controller.metadata.exposureProgram
                }

                MetadataRow {
                    labelText: qsTr("Medição")
                    valueText: root.controller.metadata.meteringMode
                }

                MetadataRow {
                    labelText: qsTr("Flash")
                    valueText: root.controller.metadata.flash
                }

                MetadataRow {
                    labelText: qsTr("Balanço de branco")
                    valueText: root.controller.metadata.whiteBalance
                }

            }

            MetadataSection {
                width: parent.width - parent.leftPadding - parent.rightPadding
                title: qsTr("Localização")
                visible: root.controller.metadata.hasGpsSection

                MetadataRow {
                    labelText: qsTr("Latitude")
                    valueText: root.controller.metadata.latitudeText
                }

                MetadataRow {
                    labelText: qsTr("Longitude")
                    valueText: root.controller.metadata.longitudeText
                }

                MetadataRow {
                    labelText: qsTr("Altitude")
                    valueText: root.controller.metadata.altitudeText
                }

                Label {
                    width: parent.width
                    text: qsTr("Coordenadas lidas somente do arquivo local.")
                    color: UiTheme.textMuted
                    font.pixelSize: 10
                    wrapMode: Text.Wrap
                }

            }

            Label {
                width: parent.width - parent.leftPadding - parent.rightPadding
                visible: text.length > 0
                text: root.controller.metadata.warning
                color: UiTheme.warning
                font.pixelSize: 11
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
            }

            Item {
                width: 1
                height: 8
            }

        }

    }

}
