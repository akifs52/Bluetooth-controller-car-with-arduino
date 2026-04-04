import QtQuick 2.15

Item {
    id: root

    property int minimumValue: 0
    property int maximumValue: 255
    property int value: 128
    readonly property real progress: maximumValue > minimumValue
                                     ? (value - minimumValue) / (maximumValue - minimumValue)
                                     : 0.0

    property string unit: "PWM"
    property string label: "Sürüş PWM"
    property color accent: "#81ecff"
    property color trackColor: "#262626"
    property color textColor: "#ffffff"
    property color subTextColor: "#adaaaa"
    property color cardColor: "#662c2c2c"
    property color borderColor: "#1affffff"
    property color labelColor: accent
    property string fontFamily: ""
    property bool interactive: true
    property bool dragging: false

    signal valueEdited(int value)

    implicitWidth: 180
    implicitHeight: 200

    function clampedValue(v) {
        return Math.max(minimumValue, Math.min(maximumValue, v))
    }

    function updateFromCanvas(px, py) {
        var centerX = gaugeCanvas.width / 2
        var centerY = gaugeCanvas.height / 2
        var dx = px - centerX
        var dy = py - centerY
        var distance = Math.sqrt(dx * dx + dy * dy)

        // Merkez dokunuşlarında yanlış sıçramayı engelle.
        if (distance < gaugeCanvas.width * 0.22) {
            return
        }

        var angle = Math.atan2(dy, dx) + Math.PI / 2
        if (angle < 0) {
            angle += Math.PI * 2
        }

        var normalized = angle / (Math.PI * 2)
        var next = minimumValue + Math.round(normalized * (maximumValue - minimumValue))
        next = clampedValue(next)

        if (next !== value) {
            value = next
            valueEdited(next)
        }
    }

    onValueChanged: {
        var clamped = clampedValue(value)
        if (clamped !== value) {
            value = clamped
            return
        }
        gaugeCanvas.requestPaint()
    }

    Rectangle {
        anchors.fill: parent
        radius: 18
        color: cardColor
        border.color: borderColor
        border.width: 1
    }

    Rectangle {
        anchors.fill: parent
        radius: 18
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#10181b" }
            GradientStop { position: 1.0; color: "#00000000" }
        }
        opacity: 0.5
    }

    Canvas {
        id: gaugeCanvas
        width: 120
        height: 120
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 16
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var centerX = width / 2
            var centerY = height / 2
            var radius = Math.min(width, height) / 2 - 8
            var startAngle = -Math.PI / 2
            var normalized = Math.max(0, Math.min(1, root.progress))
            var endAngle = startAngle + Math.PI * 2 * normalized

            ctx.lineWidth = 5
            ctx.strokeStyle = root.trackColor
            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, 0, Math.PI * 2)
            ctx.stroke()

            ctx.lineWidth = 6
            ctx.lineCap = "round"
            ctx.strokeStyle = root.accent
            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, startAngle, endAngle)
            ctx.stroke()

            ctx.lineWidth = 12
            ctx.globalAlpha = 0.18
            ctx.strokeStyle = root.accent
            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, startAngle, endAngle)
            ctx.stroke()
            ctx.globalAlpha = 1.0

            var knobX = centerX + Math.cos(endAngle) * radius
            var knobY = centerY + Math.sin(endAngle) * radius

            ctx.fillStyle = root.accent
            ctx.beginPath()
            ctx.arc(knobX, knobY, 5, 0, Math.PI * 2)
            ctx.fill()
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        MouseArea {
            anchors.fill: parent
            enabled: root.interactive
            preventStealing: true
            onPressed: function(mouse) {
                root.dragging = true
                root.updateFromCanvas(mouse.x, mouse.y)
            }
            onPositionChanged: function(mouse) {
                if (pressed) {
                    root.updateFromCanvas(mouse.x, mouse.y)
                }
            }
            onReleased: root.dragging = false
            onCanceled: root.dragging = false
        }
    }

    Column {
        anchors.centerIn: gaugeCanvas
        anchors.horizontalCenterOffset: 4
        spacing: 2

        Text {
            text: value
            font.family: root.fontFamily
            font.pixelSize: 30
            font.weight: Font.Bold
            color: root.textColor
            width: 72
            horizontalAlignment: Text.AlignHCenter
        }
        Text {
            text: unit
            font.family: root.fontFamily
            font.pixelSize: 10
            font.letterSpacing: 2
            color: root.subTextColor
            width: 72
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Text {
        text: label
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: gaugeCanvas.bottom
        anchors.topMargin: 10
        font.family: root.fontFamily
        font.pixelSize: 10
        font.letterSpacing: 2
        font.weight: Font.DemiBold
        color: root.labelColor
    }

    onAccentChanged: gaugeCanvas.requestPaint()
    onTrackColorChanged: gaugeCanvas.requestPaint()
}
