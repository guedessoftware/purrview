pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root

    required property var controller

    clip: true
    contentWidth: availableWidth
    padding: 1

    background: Rectangle {
        color: UiTheme.panelSurface
        border.color: UiTheme.brandBorder
        border.width: 1
        radius: UiTheme.radiusLarge
    }

    component PurrChoiceButton: Button {
        id: choiceButton

        implicitHeight: 34
        font.weight: Font.DemiBold
        background: Rectangle {
            radius: 7
            color: choiceButton.checked || choiceButton.highlighted
                   ? Qt.rgba(UiTheme.brandPurple.r, UiTheme.brandPurple.g,
                             UiTheme.brandPurple.b, 0.24)
                   : Qt.rgba(root.palette.button.r, root.palette.button.g,
                             root.palette.button.b, 0.72)
            border.color: choiceButton.checked || choiceButton.highlighted
                          ? UiTheme.brandPink : root.palette.mid
            border.width: choiceButton.checked || choiceButton.highlighted ? 2 : 1
        }
        contentItem: Text {
            text: choiceButton.text
            color: choiceButton.enabled ? root.palette.buttonText
                                        : root.palette.placeholderText
            font: choiceButton.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: 0

        IconLabel {
            Layout.fillWidth: true
            Layout.leftMargin: 22
            Layout.rightMargin: 22
            Layout.topMargin: 22
            Layout.bottomMargin: 22
            iconName: "page"
            iconSize: 19
            text: qsTr("Configuração da página")
            color: root.palette.windowText
            font.pixelSize: 18
            font.weight: Font.Bold
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: root.palette.mid
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 22
            spacing: 10

            IconLabel {
                iconName: "paper"
                text: qsTr("Tamanho do papel")
                color: UiTheme.brandPurple
                font.weight: Font.DemiBold
            }

            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("A4 — 210 × 297 mm"), qsTr("A3 — 297 × 420 mm"), qsTr("A5 — 148 × 210 mm"), qsTr("Carta — 215,9 × 279,4 mm"), qsTr("Ofício / Legal — 215,9 × 355,6 mm"), qsTr("Foto — 100 × 150 mm")]
                currentIndex: root.controller.paperSize
                onActivated: root.controller.paperSize = currentIndex
                Accessible.name: qsTr("Tamanho do papel")
            }

            Item {
                Layout.preferredHeight: 4
            }

            IconLabel {
                iconName: "orientation"
                text: qsTr("Orientação")
                color: UiTheme.brandPurple
                font.weight: Font.DemiBold
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("Retrato")
                    checkable: true
                    checked: !root.controller.landscape
                    onClicked: root.controller.landscape = false
                }

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("Paisagem")
                    checkable: true
                    checked: root.controller.landscape
                    onClicked: root.controller.landscape = true
                }

            }

            Item {
                Layout.preferredHeight: 4
            }

            IconLabel {
                iconName: "grid"
                text: qsTr("Modelos de grade")
                color: UiTheme.brandCyan
                font.weight: Font.DemiBold
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 3
                columnSpacing: 6
                rowSpacing: 6

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("1 × 1")
                    highlighted: root.controller.rows === 1 && root.controller.columns === 1
                    onClicked: root.controller.setGridPreset(1, 1)
                }

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("1 × 2")
                    highlighted: root.controller.rows === 1 && root.controller.columns === 2
                    onClicked: root.controller.setGridPreset(1, 2)
                }

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("2 × 2")
                    highlighted: root.controller.rows === 2 && root.controller.columns === 2
                    onClicked: root.controller.setGridPreset(2, 2)
                }

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("2 × 3")
                    highlighted: root.controller.rows === 2 && root.controller.columns === 3
                    onClicked: root.controller.setGridPreset(2, 3)
                }

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("3 × 3")
                    highlighted: root.controller.rows === 3 && root.controller.columns === 3
                    onClicked: root.controller.setGridPreset(3, 3)
                }

                PurrChoiceButton {
                    Layout.fillWidth: true
                    text: qsTr("3 × 4")
                    highlighted: root.controller.rows === 3 && root.controller.columns === 4
                    onClicked: root.controller.setGridPreset(3, 4)
                }

            }

            Item {
                Layout.preferredHeight: 4
            }

            IconLabel {
                iconName: "grid"
                text: qsTr("Grade")
                color: UiTheme.brandCyan
                font.weight: Font.DemiBold
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 6

                Label {
                    text: qsTr("Linhas")
                    color: root.palette.placeholderText
                }

                Label {
                    text: qsTr("Colunas")
                    color: root.palette.placeholderText
                }

                SpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 20
                    value: root.controller.rows
                    editable: true
                    onValueModified: root.controller.rows = value
                }

                SpinBox {
                    Layout.fillWidth: true
                    from: 1
                    to: 20
                    value: root.controller.columns
                    editable: true
                    onValueModified: root.controller.columns = value
                }

            }

            Item {
                Layout.preferredHeight: 4
            }

            IconLabel {
                iconName: "image"
                text: qsTr("Encaixe das imagens")
                color: UiTheme.brandPink
                font.weight: Font.DemiBold
            }

            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("Ajustar (Fit)"), qsTr("Preencher (Fill)"), qsTr("Esticar")]
                currentIndex: root.controller.placementMode
                onActivated: root.controller.placementMode = currentIndex
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: root.palette.mid
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 22
            spacing: 8

            IconLabel {
                iconName: "margins"
                text: qsTr("Margens")
                color: UiTheme.brandCyan
                font.weight: Font.DemiBold
                Layout.bottomMargin: 2
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 6

                Label {
                    text: qsTr("Superior")
                    color: root.palette.placeholderText
                }

                Label {
                    text: qsTr("Direita")
                    color: root.palette.placeholderText
                }

                MetricSpinBox {
                    Layout.fillWidth: true
                    metricValue: root.controller.marginTop
                    toValue: 80
                    onValueModified: root.controller.marginTop = value / factor
                }

                MetricSpinBox {
                    Layout.fillWidth: true
                    metricValue: root.controller.marginRight
                    toValue: 80
                    onValueModified: root.controller.marginRight = value / factor
                }

                Label {
                    text: qsTr("Inferior")
                    color: root.palette.placeholderText
                }

                Label {
                    text: qsTr("Esquerda")
                    color: root.palette.placeholderText
                }

                MetricSpinBox {
                    Layout.fillWidth: true
                    metricValue: root.controller.marginBottom
                    toValue: 80
                    onValueModified: root.controller.marginBottom = value / factor
                }

                MetricSpinBox {
                    Layout.fillWidth: true
                    metricValue: root.controller.marginLeft
                    toValue: 80
                    onValueModified: root.controller.marginLeft = value / factor
                }

            }

            Item {
                Layout.preferredHeight: 6
            }

            IconLabel {
                iconName: "spacing"
                text: qsTr("Espaçamento")
                color: UiTheme.brandPink
                font.weight: Font.DemiBold
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10
                rowSpacing: 6

                Label {
                    text: qsTr("Horizontal")
                    color: root.palette.placeholderText
                }

                Label {
                    text: qsTr("Vertical")
                    color: root.palette.placeholderText
                }

                MetricSpinBox {
                    Layout.fillWidth: true
                    metricValue: root.controller.horizontalSpacing
                    toValue: 40
                    onValueModified: root.controller.horizontalSpacing = value / factor
                }

                MetricSpinBox {
                    Layout.fillWidth: true
                    metricValue: root.controller.verticalSpacing
                    toValue: 40
                    onValueModified: root.controller.verticalSpacing = value / factor
                }

            }

        }

        Item {
            Layout.fillHeight: true
            implicitHeight: 18
        }

    }

}
