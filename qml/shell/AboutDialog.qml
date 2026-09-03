import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    required property var aboutInfo

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(600, parent ? parent.width - 40 : 600)
    modal: true
    dim: true
    focus: true
    title: qsTr("Sobre o PurrView")
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0

    function openExternal(url) {
        if (url && url.length > 0)
            Qt.openUrlExternally(url)
    }

    Overlay.modal: Rectangle {
        color: Qt.rgba(0.025, 0.028, 0.055, 0.72)
    }

    background: Rectangle {
        radius: UiTheme.radiusLarge
        border.width: 1
        border.color: Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                              UiTheme.brandPurple.b, 0.48)
        gradient: Gradient {
            GradientStop { position: 0; color: "#1b1d30" }
            GradientStop { position: 1; color: "#121420" }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            height: 3
            radius: 2
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0; color: UiTheme.brandPurple }
                GradientStop { position: 0.5; color: UiTheme.brandPink }
                GradientStop { position: 1; color: UiTheme.brandCoral }
            }
        }
    }

    header: Rectangle {
        implicitHeight: 54
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: UiTheme.spacingXl
            anchors.rightMargin: UiTheme.spacingMd
            anchors.topMargin: 3
            spacing: UiTheme.spacingSm

            Label {
                Layout.fillWidth: true
                text: qsTr("Sobre o PurrView")
                color: UiTheme.textPrimary
                font.pixelSize: 17
                font.weight: Font.DemiBold
            }

            ToolButton {
                id: closeButton

                Layout.preferredWidth: 34
                Layout.preferredHeight: 34
                text: "×"
                Accessible.name: qsTr("Fechar")
                onClicked: root.close()

                contentItem: Label {
                    text: closeButton.text
                    color: closeButton.hovered ? UiTheme.textPrimary : UiTheme.textSecondary
                    font.pixelSize: 22
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: width / 2
                    color: closeButton.down ? Qt.rgba(1, 1, 1, 0.14)
                                            : closeButton.hovered ? Qt.rgba(1, 1, 1, 0.08)
                                                                  : "transparent"
                }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: UiTheme.borderSubtle
        }
    }

    footer: Item {
        implicitHeight: 0
    }

    contentItem: Item {
        implicitHeight: aboutContent.implicitHeight + UiTheme.spacingXl * 2

        ColumnLayout {
            id: aboutContent

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: UiTheme.spacingXl
            spacing: 18

            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                Rectangle {
                    Layout.preferredWidth: 98
                    Layout.preferredHeight: 98
                    radius: 49
                    color: Qt.rgba(1, 1, 1, 0.045)
                    border.color: Qt.rgba(1, 1, 1, 0.10)

                    Image {
                        anchors.fill: parent
                        anchors.margins: 5
                        source: "qrc:/qt/qml/Impage/assets/purrview.svg"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                        Accessible.name: qsTr("Logotipo do PurrView")
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Label {
                        text: "PurrView"
                        color: UiTheme.textPrimary
                        font.family: "serif"
                        font.pixelSize: 32
                        font.bold: true
                        font.italic: true
                    }

                    Rectangle {
                        implicitWidth: versionLabel.implicitWidth + 18
                        implicitHeight: 26
                        radius: 13
                        color: Qt.rgba(UiTheme.brandCoral.r, UiTheme.brandCoral.g,
                                       UiTheme.brandCoral.b, 0.13)
                        border.color: Qt.rgba(UiTheme.brandCoral.r, UiTheme.brandCoral.g,
                                              UiTheme.brandCoral.b, 0.32)

                        Label {
                            id: versionLabel
                            anchors.centerIn: parent
                            text: qsTr("Versão %1").arg(root.aboutInfo.applicationVersion)
                            color: UiTheme.brandCoral
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Visualize imagens e componha páginas prontas para impressão.")
                        color: UiTheme.textSecondary
                        font.pixelSize: 13
                        lineHeight: 1.15
                        wrapMode: Text.Wrap
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: detailsLayout.implicitHeight + 36
                radius: UiTheme.radiusMedium
                color: Qt.rgba(1, 1, 1, 0.045)
                border.color: UiTheme.borderSubtle

                GridLayout {
                    id: detailsLayout
                    anchors.fill: parent
                    anchors.margins: 18
                    columns: 2
                    columnSpacing: 22
                    rowSpacing: 11

                    Label { text: qsTr("Licença"); color: UiTheme.textMuted; font.pixelSize: 13 }
                    Label { text: "GNU GPLv3"; color: UiTheme.textPrimary; font.pixelSize: 13; font.weight: Font.DemiBold; Layout.fillWidth: true }

                    Label { text: qsTr("Sistema"); color: UiTheme.textMuted; font.pixelSize: 13 }
                    Label {
                        Layout.fillWidth: true
                        text: "%1 · %2".arg(root.aboutInfo.operatingSystem)
                                          .arg(root.aboutInfo.architecture)
                        color: UiTheme.textPrimary
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }

                    Label { text: qsTr("Tecnologia"); color: UiTheme.textMuted; font.pixelSize: 13 }
                    Label {
                        text: "Qt %1".arg(root.aboutInfo.qtVersion)
                        color: UiTheme.textPrimary
                        font.pixelSize: 13
                    }
                }
            }

            Button {
                id: projectButton

                Layout.fillWidth: true
                Layout.preferredHeight: 44
                text: qsTr("Abrir projeto no GitHub  ↗")
                font.pixelSize: 14
                font.weight: Font.DemiBold
                onClicked: root.openExternal(root.aboutInfo.projectUrl)

                contentItem: Label {
                    text: projectButton.text
                    color: "white"
                    font: projectButton.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: UiTheme.radiusSmall
                    opacity: projectButton.enabled ? (projectButton.down ? 0.82 : 1) : 0.4
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0; color: UiTheme.brandPurple }
                        GradientStop { position: 0.54; color: UiTheme.brandPink }
                        GradientStop { position: 1; color: UiTheme.brandCoral }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: "© 2026 PurrView contributors  ·  " + qsTr("Software livre sob a GNU GPLv3")
                color: UiTheme.textMuted
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                font.pixelSize: 12
            }
        }
    }
}
