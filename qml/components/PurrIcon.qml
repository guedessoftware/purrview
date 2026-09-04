import QtQuick

Item {
    id: root

    property string name: ""
    property color color: UiTheme.textPrimary
    property real strokeWidth: 1.8

    implicitWidth: 18
    implicitHeight: 18

    onNameChanged: iconCanvas.requestPaint()
    onColorChanged: iconCanvas.requestPaint()
    onStrokeWidthChanged: iconCanvas.requestPaint()
    onWidthChanged: iconCanvas.requestPaint()
    onHeightChanged: iconCanvas.requestPaint()

    Canvas {
        id: iconCanvas

        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            ctx.setTransform(1, 0, 0, 1, 0, 0)
            ctx.clearRect(0, 0, width, height)
            ctx.save()
            ctx.scale(width / 24, height / 24)
            ctx.strokeStyle = root.color
            ctx.fillStyle = root.color
            ctx.lineWidth = root.strokeWidth
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            function line(x1, y1, x2, y2) {
                ctx.beginPath()
                ctx.moveTo(x1, y1)
                ctx.lineTo(x2, y2)
                ctx.stroke()
            }

            function circle(x, y, radius, fill) {
                ctx.beginPath()
                ctx.arc(x, y, radius, 0, Math.PI * 2)
                if (fill)
                    ctx.fill()
                else
                    ctx.stroke()
            }

            switch (root.name) {
            case "plus":
                line(12, 5, 12, 19)
                line(5, 12, 19, 12)
                break
            case "minus":
                line(5, 12, 19, 12)
                break
            case "close":
                line(6, 6, 18, 18)
                line(18, 6, 6, 18)
                break
            case "check":
                ctx.beginPath()
                ctx.moveTo(5, 12.5)
                ctx.lineTo(10, 17)
                ctx.lineTo(19, 7)
                ctx.stroke()
                break
            case "chevron-left":
                ctx.beginPath()
                ctx.moveTo(15, 5)
                ctx.lineTo(8, 12)
                ctx.lineTo(15, 19)
                ctx.stroke()
                break
            case "chevron-right":
                ctx.beginPath()
                ctx.moveTo(9, 5)
                ctx.lineTo(16, 12)
                ctx.lineTo(9, 19)
                ctx.stroke()
                break
            case "fullscreen":
                line(4, 9, 4, 4)
                line(4, 4, 9, 4)
                line(15, 4, 20, 4)
                line(20, 4, 20, 9)
                line(20, 15, 20, 20)
                line(20, 20, 15, 20)
                line(9, 20, 4, 20)
                line(4, 20, 4, 15)
                break
            case "warning":
                ctx.beginPath()
                ctx.moveTo(12, 3)
                ctx.lineTo(22, 20)
                ctx.lineTo(2, 20)
                ctx.closePath()
                ctx.stroke()
                line(12, 8, 12, 14)
                circle(12, 17, 1, true)
                break
            case "printer":
                ctx.strokeRect(7, 3, 10, 6)
                ctx.strokeRect(4, 8, 16, 9)
                ctx.fillRect(4, 13, 3, 4)
                ctx.fillRect(17, 13, 3, 4)
                ctx.fillRect(16.5, 10.5, 1.5, 1.5)
                ctx.fillStyle = "transparent"
                ctx.strokeRect(7, 13, 10, 8)
                line(9, 16, 15, 16)
                line(9, 18.5, 15, 18.5)
                break
            case "filmstrip":
                ctx.strokeRect(3, 4, 18, 16)
                line(7, 4, 7, 20)
                line(17, 4, 17, 20)
                line(3, 8, 7, 8)
                line(17, 8, 21, 8)
                line(3, 16, 7, 16)
                line(17, 16, 21, 16)
                break
            case "rotate-left":
            case "rotate-right": {
                const right = root.name === "rotate-right"
                ctx.beginPath()
                ctx.arc(12, 13, 7, right ? Math.PI * 1.12 : Math.PI * 1.88,
                        right ? Math.PI * 0.02 : Math.PI * 0.98, right)
                ctx.stroke()
                ctx.beginPath()
                if (right) {
                    ctx.moveTo(17.5, 4.5)
                    ctx.lineTo(20.5, 8)
                    ctx.lineTo(16, 8.5)
                } else {
                    ctx.moveTo(6.5, 4.5)
                    ctx.lineTo(3.5, 8)
                    ctx.lineTo(8, 8.5)
                }
                ctx.closePath()
                ctx.fill()
                break
            }
            case "info":
                circle(12, 12, 9, false)
                circle(12, 8, 1, true)
                line(12, 11, 12, 17)
                break
            case "pin":
                ctx.beginPath()
                ctx.moveTo(8, 4)
                ctx.lineTo(16, 4)
                ctx.lineTo(15, 9)
                ctx.lineTo(18, 12)
                ctx.lineTo(13, 12)
                ctx.lineTo(12, 20)
                ctx.lineTo(11, 12)
                ctx.lineTo(6, 12)
                ctx.lineTo(9, 9)
                ctx.closePath()
                ctx.stroke()
                break
            case "more":
                circle(6, 12, 1.6, true)
                circle(12, 12, 1.6, true)
                circle(18, 12, 1.6, true)
                break
            case "add-image":
                ctx.strokeRect(3, 5, 15, 15)
                circle(8, 10, 1.5, false)
                ctx.beginPath()
                ctx.moveTo(5.5, 17)
                ctx.lineTo(9.5, 13)
                ctx.lineTo(12, 15.5)
                ctx.lineTo(14, 13.5)
                ctx.lineTo(18, 17.5)
                ctx.stroke()
                circle(18.5, 6, 4, true)
                ctx.strokeStyle = UiTheme.brightText
                ctx.lineWidth = 1.5
                line(18.5, 4, 18.5, 8)
                line(16.5, 6, 20.5, 6)
                break
            case "trash":
                line(5, 7, 19, 7)
                line(9, 4, 15, 4)
                ctx.strokeRect(7, 7, 10, 13)
                line(10.5, 10, 10.5, 17)
                line(13.5, 10, 13.5, 17)
                break
            case "duplicate":
                ctx.strokeRect(7, 7, 13, 13)
                ctx.beginPath()
                ctx.moveTo(4, 16)
                ctx.lineTo(4, 4)
                ctx.lineTo(16, 4)
                ctx.stroke()
                break
            case "eye":
                ctx.beginPath()
                ctx.moveTo(3, 12)
                ctx.bezierCurveTo(7, 5, 17, 5, 21, 12)
                ctx.bezierCurveTo(17, 19, 7, 19, 3, 12)
                ctx.stroke()
                circle(12, 12, 3, false)
                break
            case "page":
                ctx.beginPath()
                ctx.moveTo(6, 3)
                ctx.lineTo(14, 3)
                ctx.lineTo(19, 8)
                ctx.lineTo(19, 21)
                ctx.lineTo(6, 21)
                ctx.closePath()
                ctx.stroke()
                line(14, 3, 14, 8)
                line(14, 8, 19, 8)
                line(9, 12, 16, 12)
                line(9, 16, 16, 16)
                break
            case "paper":
                ctx.strokeRect(5, 3, 14, 18)
                line(8, 7, 16, 7)
                line(8, 11, 16, 11)
                break
            case "orientation":
                ctx.strokeRect(4, 5, 16, 14)
                line(8, 3, 8, 5)
                line(16, 3, 16, 5)
                break
            case "grid":
                ctx.strokeRect(3, 3, 7, 7)
                ctx.strokeRect(14, 3, 7, 7)
                ctx.strokeRect(3, 14, 7, 7)
                ctx.strokeRect(14, 14, 7, 7)
                break
            case "image":
                ctx.strokeRect(3, 4, 18, 16)
                circle(8, 9, 1.5, false)
                ctx.beginPath()
                ctx.moveTo(5, 17)
                ctx.lineTo(10, 12)
                ctx.lineTo(13, 15)
                ctx.lineTo(16, 12)
                ctx.lineTo(20, 17)
                ctx.stroke()
                break
            case "margins":
                ctx.strokeRect(3, 3, 18, 18)
                ctx.setLineDash([2, 2])
                ctx.strokeRect(7, 7, 10, 10)
                ctx.setLineDash([])
                break
            case "spacing":
                ctx.strokeRect(3, 5, 7, 14)
                ctx.strokeRect(14, 5, 7, 14)
                line(11.5, 8, 11.5, 16)
                break
            }

            ctx.restore()
        }
    }
}
