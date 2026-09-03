import QtQuick
import QtQuick.Controls

SpinBox {
    id: control

    property real metricValue: 0
    property real fromValue: 0
    property real toValue: 100
    property int decimals: 1
    property int factor: Math.pow(10, decimals)

    from: Math.round(fromValue * factor)
    to: Math.round(toValue * factor)
    value: Math.round(metricValue * factor)
    stepSize: factor / 2
    editable: true

    textFromValue: function(value, locale) {
        return Number(value / factor).toLocaleString(locale, "f", decimals) + " mm"
    }

    valueFromText: function(text, locale) {
        const cleaned = text.replace(/[^0-9,.-]/g, "")
        return Math.round(Number.fromLocaleString(locale, cleaned) * factor)
    }
}
