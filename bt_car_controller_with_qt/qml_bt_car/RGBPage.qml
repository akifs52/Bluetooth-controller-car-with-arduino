import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Flickable {
    id: root
    
    property var theme
    property string iconFont: theme && theme.fontIcon ? theme.fontIcon : ""
    property var bluetoothManager
    
    readonly property bool hasBtManager: bluetoothManager !== null && bluetoothManager !== undefined
    
    interactive: true
    contentWidth: width
    contentHeight: contentColumn.implicitHeight + 40
    clip: true
    
    // RGB State
    property string currentColor: "#00E3FD"
    property real brightness: 0.85
    property real saturation: 1.0
    property bool powerOn: true
    
    Column {
        id: contentColumn
        x: 20
        y: 20
        width: root.width - 40
        spacing: 40
        
        // Header
        Row {
            width: parent.width
            spacing: 16
            
            Text {
                text: "RGB CONTROL"
                font.family: theme && theme.fontHeadline ? theme.fontHeadline : "Arial"
                font.pixelSize: 20
                font.bold: true
                color: theme && theme.primary ? theme.primary : "#00E3FD"
                anchors.verticalCenter: parent.verticalCenter
            }
            
            Item { width: 40; height: 40 }
        }
        
        // Selection Tabs
        Row {
            width: parent.width
            spacing: 4
            padding: 6
            
            Rectangle {
                id: customTab
                width: (parent.width - 8) / 2
                height: 48
                color: theme && theme.surfaceContainer ? theme.surfaceContainer : "#1A1A1A"
                radius: 12
                border.color: theme && theme.primary ? theme.primary : "#00E3FD"
                border.width: 1
                
                Behavior on color {
                    ColorAnimation { duration: 200; easing.type: Easing.OutQuad }
                }
                
                Behavior on border.color {
                    ColorAnimation { duration: 200; easing.type: Easing.OutQuad }
                }
                
                Text {
                    id: customText
                    text: "CUSTOM"
                    anchors.centerIn: parent
                    font.family: theme && theme.fontHeadline ? theme.fontHeadline : "Arial"
                    font.pixelSize: 12
                    font.bold: true
                    color: theme && theme.primary ? theme.primary : "#00E3FD"
                    
                    Behavior on color {
                        ColorAnimation { duration: 200; easing.type: Easing.OutQuad }
                    }
                }
                
                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        try {
                            if (Qt.platform.os === "android") {
                                Qt.vibrate(25)
                            }
                        } catch(e) {
                            console.log("Vibration error: " + e)
                        }
                        // Custom tab'ı aktif et
                        customTab.color = theme.surfaceContainer
                        customTab.border.color = theme.primary
                        customText.color = theme.primary
                        
                        randomTab.color = "transparent"
                        randomTab.border.color = theme.outlineVariant
                        randomText.color = theme.onSurface
                        
                        // RGB tekerleğini aktif et
                        rgbWheelSection.opacity = 1.0
                        rgbWheelSection.enabled = true
                    }
                }
            }
            
            Rectangle {
                id: randomTab
                width: (parent.width - 8) / 2
                height: 48
                color: "transparent"
                radius: 12
                border.color: theme && theme.outlineVariant ? theme.outlineVariant : "#444444"
                border.width: 1
                
                Behavior on color {
                    ColorAnimation { duration: 200; easing.type: Easing.OutQuad }
                }
                
                Behavior on border.color {
                    ColorAnimation { duration: 200; easing.type: Easing.OutQuad }
                }
                
                Text {
                    id: randomText
                    text: "RANDOM"
                    anchors.centerIn: parent
                    font.family: theme && theme.fontHeadline ? theme.fontHeadline : "Arial"
                    font.pixelSize: 12
                    font.bold: true
                    color: theme && theme.onSurface ? theme.onSurface : "#FFFFFF"
                    
                    Behavior on color {
                        ColorAnimation { duration: 200; easing.type: Easing.OutQuad }
                    }
                }
                
                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        try {
                            if (Qt.platform.os === "android") {
                                Qt.vibrate(25)
                            }
                        } catch(e) {
                            console.log("Vibration error: " + e)
                        }
                        // Random tab'ı aktif et
                        randomTab.color = theme.surfaceContainer
                        randomTab.border.color = theme.primary
                        randomText.color = theme.primary
                        
                        customTab.color = "transparent"
                        customTab.border.color = theme.outlineVariant
                        customText.color = theme.onSurface
                        
                        // RGB tekerleğini devre dışı et
                        rgbWheelSection.opacity = 0.3
                        rgbWheelSection.enabled = false
                    }
                }
            }
        }
        
        // RGB Wheel Section
        Column {
            id: rgbWheelSection
            width: parent.width
            spacing: 20
            
            Behavior on opacity {
                NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
            }
            
            Rectangle {
                width: 288
                height: 288
                radius: 144
                anchors.horizontalCenter: parent.horizontalCenter
                color: theme.surfaceContainerHigh
                border.color: theme.primary + "30"
                border.width: 2
                
                // Color Wheel
                Rectangle {
                    width: parent.width - 12
                    height: parent.height - 12
                    radius: parent.radius - 6
                    anchors.centerIn: parent
                    
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#FF0000" }
                        GradientStop { position: 0.17; color: "#FFFF00" }
                        GradientStop { position: 0.33; color: "#00FF00" }
                        GradientStop { position: 0.50; color: "#00FFFF" }
                        GradientStop { position: 0.67; color: "#0000FF" }
                        GradientStop { position: 0.83; color: "#FF00FF" }
                        GradientStop { position: 1.0; color: "#FF0000" }
                    }
                    
                    // Center Display
                    Rectangle {
                        width: 192
                        height: 192
                        radius: 96
                        anchors.centerIn: parent
                        color: theme && theme.surfaceContainerLow ? theme.surfaceContainerLow : "#0A0A0A"
                        
                        Column {
                            anchors.centerIn: parent
                            spacing: 8
                            
                            Text {
                                text: currentColor
                                font.family: theme && theme.fontHeadline ? theme.fontHeadline : "Arial"
                                font.pixelSize: 32
                                font.bold: true
                                color: theme && theme.primary ? theme.primary : "#00E3FD"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            
                            Text {
                                text: "ACTIVE COLOR"
                                font.family: theme && theme.fontLabel ? theme.fontLabel : "Arial"
                                font.pixelSize: 10
                                color: theme && theme.onSurfaceVariant ? theme.onSurfaceVariant : "#888888"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                    
                    // Picker Knob
                    Rectangle {
                        id: pickerKnob
                        width: 32
                        height: 32
                        radius: 16
                        color: "white"
                        border.color: theme ? theme.surfaceContainer : "#333333"
                        border.width: 4
                        
                        property bool dragging: false
                        
                        // Fonksiyonları buraya taşı - dışarıdan erişilebilir olsun
                        function forceKnobToCircle() {
                            var wheelWidth = parent.parent.width
                            var wheelHeight = parent.parent.height
                            var wheelRadius = Math.min(wheelWidth, wheelHeight) / 2 - 6
                            var centerX = wheelWidth / 2
                            var centerY = wheelHeight / 2
                            
                            var knobX = pickerKnob.x + pickerKnob.width / 2
                            var knobY = pickerKnob.y + pickerKnob.height / 2
                            
                            var deltaX = knobX - centerX
                            var deltaY = knobY - centerY
                            var distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY)
                            
                            if (distance > 0) {
                                var normalizedX = deltaX / distance
                                var normalizedY = deltaY / distance
                                pickerKnob.x = centerX + normalizedX * wheelRadius - pickerKnob.width / 2
                                pickerKnob.y = centerY + normalizedY * wheelRadius - pickerKnob.height / 2
                            }
                        }
                        
                        function updateColorFromPosition() {
                            var wheelWidth = parent.parent.width
                            var wheelHeight = parent.parent.height
                            var wheelRadius = Math.min(wheelWidth, wheelHeight) / 2 - 6
                            var centerX = wheelWidth / 2
                            var centerY = wheelHeight / 2
                            
                            var knobX = pickerKnob.x + pickerKnob.width / 2
                            var knobY = pickerKnob.y + pickerKnob.height / 2
                            
                            var deltaX = knobX - centerX
                            var deltaY = knobY - centerY
                            var angle = Math.atan2(deltaY, deltaX)
                            if (angle < 0) angle = angle + 2 * Math.PI
                            
                            var gradientPosition = angle / (2 * Math.PI)
                            var color = getColorFromGradient(gradientPosition)
                            currentColor = color
                            
                            var distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY)
                            if (distance !== wheelRadius) {
                                var normalizedX = deltaX / distance
                                var normalizedY = deltaY / distance
                                pickerKnob.x = centerX + normalizedX * wheelRadius - pickerKnob.width / 2
                                pickerKnob.y = centerY + normalizedY * wheelRadius - pickerKnob.height / 2
                            }
                            
                            console.log("Knob pozisyon: X=" + pickerKnob.x.toFixed(1) + ", Y=" + pickerKnob.y.toFixed(1) + 
                                       ", Açı=" + (angle * 180 / Math.PI).toFixed(1) + "°, Renk=" + color)
                        }
                        
                        function getColorFromGradient(position) {
                            var colors = [
                                { pos: 0.0,   color: "#FF0000" },
                                { pos: 0.17,  color: "#FFFF00" },
                                { pos: 0.33,  color: "#00FF00" },
                                { pos: 0.50,  color: "#00FFFF" },
                                { pos: 0.67,  color: "#0000FF" },
                                { pos: 0.83,  color: "#FF00FF" },
                                { pos: 1.0,   color: "#FF0000" }
                            ]
                            
                            for (var i = 0; i < colors.length - 1; i++) {
                                if (position >= colors[i].pos && position <= colors[i + 1].pos) {
                                    var range = colors[i + 1].pos - colors[i].pos
                                    var localPos = (position - colors[i].pos) / range
                                    return interpolateColor(colors[i].color, colors[i + 1].color, localPos)
                                }
                            }
                            return colors[colors.length - 1].color
                        }
                        
                        function interpolateColor(color1, color2, factor) {
                            var c1 = hexToRgb(color1)
                            var c2 = hexToRgb(color2)
                            var r = Math.round(c1.r + (c2.r - c1.r) * factor)
                            var g = Math.round(c1.g + (c2.g - c1.g) * factor)
                            var b = Math.round(c1.b + (c2.b - c1.b) * factor)
                            return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1).toUpperCase()
                        }
                        
                        function hexToRgb(hex) {
                            var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex)
                            return result ? {
                                r: parseInt(result[1], 16),
                                g: parseInt(result[2], 16),
                                b: parseInt(result[3], 16)
                            } : { r: 0, g: 0, b: 0 }
                        }
                        
                        // Başlangıç pozisyonu
                        Component.onCompleted: {
                            var wheelRadius = Math.min(parent.width, parent.height) / 2 - 6
                            x = parent.width / 2 - width / 2
                            y = 6
                            forceKnobToCircle()
                        }
                        
                        MouseArea {
                            id: pickerMouseArea
                            anchors.fill: parent
                            drag.target: pickerKnob
                            drag.axis: Drag.XAndYAxis
                            drag.minimumX: -1000
                            drag.minimumY: -1000
                            drag.maximumX: 1000
                            drag.maximumY: 1000
                            
                            onPressed: {
                                pickerKnob.dragging = true
                                try {
                                    if (Qt.platform.os === "android") {
                                        Qt.vibrate(30)
                                    }
                                } catch(e) {
                                    console.log("Vibration error: " + e)
                                }
                                pickerKnob.forceKnobToCircle()
                                pickerKnob.updateColorFromPosition()
                            }
                            
                            onPositionChanged: {
                                if (pressed) {
                                    pickerKnob.updateColorFromPosition()
                                }
                            }
                            
                            onReleased: {
                                pickerKnob.dragging = false
                                try {
                                    if (Qt.platform.os === "android") {
                                        Qt.vibrate(20)
                                    }
                                } catch(e) {
                                    console.log("Vibration error: " + e)
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Sliders Group
        
        // Quick Presets
        Column {
            width: parent.width
            spacing: 16
            
            Text {
                text: "Quick Presets"
                font.family: theme.fontHeadline
                font.pixelSize: 10
                font.bold: true
                color: theme.onSurfaceVariant
            }
            
            Rectangle {
                width: parent.width
                height: 72
                radius: 16
                color: theme.surfaceContainer + "10"
                border.color: theme.primary + "20"
                border.width: 1
                
                Row {
                    anchors.centerIn: parent
                    spacing: 16
                    
                    // Preset Colors
                    Rectangle {
                        width: 48
                        height: 48
                        radius: 24
                        color: "#FF0000"
                        border.color: theme.surfaceContainer
                        border.width: 4
                        
                        MouseArea {
                            anchors.fill: parent
                            onPressed: {
                                currentColor = "#FF0000"
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(25)
                                }
                            }
                        }
                    }
                    
                    Rectangle {
                        width: 48
                        height: 48
                        radius: 24
                        color: "#00FF00"
                        border.color: theme.surfaceContainer
                        border.width: 4
                        
                        MouseArea {
                            anchors.fill: parent
                            onPressed: {
                                currentColor = "#00FF00"
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(25)
                                }
                            }
                        }
                    }
                    
                    Rectangle {
                        width: 48
                        height: 48
                        radius: 24
                        color: "#0000FF"
                        border.color: theme.surfaceContainer
                        border.width: 4
                        
                        MouseArea {
                            anchors.fill: parent
                            onPressed: {
                                currentColor = "#0000FF"
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(25)
                                }
                            }
                        }
                    }
                    
                    Rectangle {
                        width: 48
                        height: 48
                        radius: 24
                        color: "#FFFF00"
                        border.color: theme.surfaceContainer
                        border.width: 4
                        
                        MouseArea {
                            anchors.fill: parent
                            onPressed: {
                                currentColor = "#FFFF00"
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(25)
                                }
                            }
                        }
                    }
                    
                    Rectangle {
                        width: 48
                        height: 48
                        radius: 24
                        color: "#FF00FF"
                        border.color: theme.surfaceContainer
                        border.width: 4
                        
                        MouseArea {
                            anchors.fill: parent
                            onPressed: {
                                currentColor = "#FF00FF"
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(25)
                                }
                            }
                        }
                    }
                    
                    // Add Button
                    Rectangle {
                        width: 56
                        height: 56
                        radius: 28
                        color: theme && theme.surfaceContainerHighest ? theme.surfaceContainerHighest : "#222222"
                        border.color: theme && theme.outlineVariant ? theme.outlineVariant : "#444444"
                        border.width: 2
                        
                        Text {
                            text: iconFont + "add"
                            font.family: iconFont
                            font.pixelSize: 24
                            color: theme && theme.onSurfaceVariant ? theme.onSurfaceVariant : "#888888"
                            anchors.centerIn: parent
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onPressed: {
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(25)
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Random Mode (Disabled)
    }
}
