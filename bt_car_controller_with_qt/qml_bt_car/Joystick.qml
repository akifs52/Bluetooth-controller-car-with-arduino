import QtQuick 2.15

Item {
    id: root
    property color trackOuter: "#1a1a1a"
    property color trackInner: "#000000"
    property color surface: "#1a1a1a"
    property color surfaceBright: "#2c2c2c"
    property color accent: "#81ecff"
    property string iconFont: ""
    property real xAxis: 0
    property real yAxis: 0
    property bool dragging: false
    signal moved(real x, real y)

    implicitWidth: 260
    implicitHeight: 260

    readonly property real knobSize: Math.max(96, width * 0.5)
    readonly property real travelRadius: (width - knobSize) / 2 - 6

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: trackOuter
        border.color: "#1affffff"
        border.width: 1
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: (width - 8) / 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: trackOuter }
            GradientStop { position: 1.0; color: trackInner }
        }
    }

    Item {
        id: knob
        width: knobSize
        height: knobSize
        x: (root.width - width) / 2 + root.xAxis * root.travelRadius
        y: (root.height - height) / 2 + root.yAxis * root.travelRadius

        Behavior on x {
            NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
        }
        Behavior on y {
            NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
        }

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: surfaceBright }
                GradientStop { position: 1.0; color: surface }
            }
            border.color: "#1affffff"
            border.width: 1
        }

        Rectangle {
            width: parent.width * 0.72
            height: width
            radius: width / 2
            anchors.centerIn: parent
            color: surface
            border.color: "#1affffff"
            border.width: 1
        }

        Rectangle {
            width: 10
            height: 10
            radius: 5
            anchors.centerIn: parent
            color: accent
        }
    }

    MouseArea {
        anchors.fill: parent
        preventStealing: true
        onPressed: function(mouse) {
            root.dragging = true
            updateKnob(mouse.x, mouse.y)
        }
        onPositionChanged: function(mouse) {
            if (pressed) {
                updateKnob(mouse.x, mouse.y)
            }
        }
        onReleased: {
            root.dragging = false
            resetKnob()
        }
        onCanceled: {
            root.dragging = false
            resetKnob()
        }
    }

    function updateKnob(px, py) {
        var dx = px - root.width / 2
        var dy = py - root.height / 2
        var distance = Math.sqrt(dx * dx + dy * dy)
        var maxDistance = root.travelRadius
        if (distance > maxDistance && distance > 0) {
            dx = dx / distance * maxDistance
            dy = dy / distance * maxDistance
        }
        root.xAxis = maxDistance > 0 ? dx / maxDistance : 0
        root.yAxis = maxDistance > 0 ? dy / maxDistance : 0
        root.moved(root.xAxis, root.yAxis)
    }

    function resetKnob() {
        root.xAxis = 0
        root.yAxis = 0
        root.moved(root.xAxis, root.yAxis)
    }
}
