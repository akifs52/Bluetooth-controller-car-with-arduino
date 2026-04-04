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
                font.family: theme.fontHeadline
                font.pixelSize: 20
                font.bold: true
                color: theme.primary
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
                color: theme.surfaceContainer
                radius: 12
                border.color: theme.primary
                border.width: 1
                
                Text {
                    id: customText
                    text: "CUSTOM"
                    anchors.centerIn: parent
                    font.family: theme.fontHeadline
                    font.pixelSize: 12
                    font.bold: true
                    color: theme.primary
                }
                
                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        if (Qt.platform.os === "android") {
                            Qt.vibrate(25)
                        }
                        // Custom tab'ı aktif et
                        customTab.color = theme.surfaceContainer
                        customTab.border.color = theme.primary
                        customText.color = theme.primary
                        
                        randomTab.color = "transparent"
                        randomTab.border.color = theme.outlineVariant
                        randomText.color = theme.onSurfaceVariant
                        
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
                border.color: theme.onSurfaceVariant
                border.width: 1
                
                Text {
                    id: randomText
                    text: "RANDOM"
                    anchors.centerIn: parent
                    font.family: theme.fontHeadline
                    font.pixelSize: 12
                    font.bold: true
                    color: theme.onSurfaceVariant
                }
                
                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        if (Qt.platform.os === "android") {
                            Qt.vibrate(25)
                        }
                        // Random tab'ı aktif et
                        randomTab.color = theme.surfaceContainer
                        randomTab.border.color = theme.primary
                        randomText.color = theme.primary
                        
                        customTab.color = "transparent"
                        customTab.border.color = theme.outlineVariant
                        customText.color = theme.onSurfaceVariant
                        
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
                        GradientStop { position: 0.14; color: "#FF00FF" }
                        GradientStop { position: 0.28; color: "#0000FF" }
                        GradientStop { position: 0.42; color: "#00FFFF" }
                        GradientStop { position: 0.56; color: "#00FF00" }
                        GradientStop { position: 0.70; color: "#FFFF00" }
                        GradientStop { position: 0.84; color: "#FF8800" }
                        GradientStop { position: 1.0; color: "#FF0000" }
                    }
                    
                    // Center Display
                    Rectangle {
                        width: 192
                        height: 192
                        radius: 96
                        anchors.centerIn: parent
                        color: theme.surfaceContainerLow
                        
                        Column {
                            anchors.centerIn: parent
                            spacing: 8
                            
                            Text {
                                text: currentColor
                                font.family: theme.fontHeadline
                                font.pixelSize: 32
                                font.bold: true
                                color: theme.primary
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            
                            Text {
                                text: "ACTIVE COLOR"
                                font.family: theme.fontLabel
                                font.pixelSize: 10
                                color: theme.onSurfaceVariant
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
                        border.color: theme.surfaceContainer
                        border.width: 4
                        
                        // Başlangıç pozisyonu: renkli daire kenarında (12 o'clock)
                        Component.onCompleted: {
                            var wheelRadius = Math.min(parent.width, parent.height) / 2 - 6
                            x = parent.width / 2 - width / 2
                            y = 6
                        }
                        
                        property bool dragging: false
                        
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
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(30)
                                }
                                // Tıklandığında knob'u renkli daire içinde tut
                                forceKnobToCircle()
                                updateColorFromPosition()
                            }
                            
                            onPositionChanged: {
                                if (pressed) {
                                    updateColorFromPosition()
                                }
                            }
                            
                            onReleased: {
                                pickerKnob.dragging = false
                                if (Qt.platform.os === "android") {
                                    Qt.vibrate(20)
                                }
                            }
                            
                            function forceKnobToCircle() {
                                // Knob'u her zaman renkli daire kenarında tut
                                var wheelWidth = parent.parent.width
                                var wheelHeight = parent.parent.height
                                var wheelRadius = Math.min(wheelWidth, wheelHeight) / 2 - 6
                                var centerX = wheelWidth / 2
                                var centerY = wheelHeight / 2
                                
                                // Mevcut pozisyon
                                var knobX = pickerKnob.x + pickerKnob.width / 2
                                var knobY = pickerKnob.y + pickerKnob.height / 2
                                
                                // Merkeze göre pozisyon
                                var deltaX = knobX - centerX
                                var deltaY = knobY - centerY
                                var distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY)
                                
                                // Knob'u daire kenarına zorla
                                if (distance > 0) {
                                    var normalizedX = deltaX / distance
                                    var normalizedY = deltaY / distance
                                    pickerKnob.x = centerX + normalizedX * wheelRadius - pickerKnob.width / 2
                                    pickerKnob.y = centerY + normalizedY * wheelRadius - pickerKnob.height / 2
                                }
                            }
                            
                            function updateColorFromPosition() {
                                // Tekerlek boyutları
                                var wheelWidth = parent.parent.width
                                var wheelHeight = parent.parent.height
                                var wheelRadius = Math.min(wheelWidth, wheelHeight) / 2 - 6
                                
                                // Merkez noktası
                                var centerX = wheelWidth / 2
                                var centerY = wheelHeight / 2
                                
                                // Knob'un mevcut pozisyonunu al
                                var knobX = pickerKnob.x + pickerKnob.width / 2
                                var knobY = pickerKnob.y + pickerKnob.height / 2
                                
                                // Merkeze göre pozisyon
                                var deltaX = knobX - centerX
                                var deltaY = knobY - centerY
                                
                                // Açıyı hesapla (0-2π)
                                var angle = Math.atan2(deltaY, deltaX)
                                if (angle < 0) angle = angle + 2 * Math.PI
                                
                                // Açıyı 0-1 arasına normalize et (gradient pozisyonu)
                                var gradientPosition = angle / (2 * Math.PI)
                                
                                // Gradient'den renk oku
                                var color = getColorFromGradient(gradientPosition)
                                currentColor = color
                                
                                // Knob'u renkli daire içinde tut (360° hareket için)
                                var distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY)
                                
                                // Knob'u her zaman daire kenarında tut (renkli gradient üzerinde)
                                if (distance !== wheelRadius) {
                                    // Knob'u tam daire kenarına getir
                                    var normalizedX = deltaX / distance
                                    var normalizedY = deltaY / distance
                                    pickerKnob.x = centerX + normalizedX * wheelRadius - pickerKnob.width / 2
                                    pickerKnob.y = centerY + normalizedY * wheelRadius - pickerKnob.height / 2
                                }
                                
                                // Debug: pozisyon ve açı bilgisi
                                console.log("Knob pozisyon: X=" + pickerKnob.x.toFixed(1) + ", Y=" + pickerKnob.y.toFixed(1) + 
                                           ", Açı=" + (angle * 180 / Math.PI).toFixed(1) + "°, Renk=" + color)
                            }
                            
                            function getColorFromGradient(position) {
                                // Gradient renkleri (aynı sıralama)
                                var colors = [
                                    { pos: 0.0,   color: "#FF0000" },  // Kırmızı
                                    { pos: 0.14,  color: "#FF00FF" },  // Magenta
                                    { pos: 0.28,  color: "#0000FF" },  // Mavi
                                    { pos: 0.42,  color: "#00FFFF" },  // Cyan
                                    { pos: 0.56,  color: "#00FF00" },  // Yeşil
                                    { pos: 0.70,  color: "#FFFF00" },  // Sarı
                                    { pos: 0.84,  color: "#FF8800" },  // Turuncu
                                    { pos: 1.0,   color: "#FF0000" }   // Kırmızı (döngü)
                                ]
                                
                                // Doğru renk aralığını bul
                                for (var i = 0; i < colors.length - 1; i++) {
                                    if (position >= colors[i].pos && position <= colors[i + 1].pos) {
                                        // İki renk arasında interpolate et
                                        var range = colors[i + 1].pos - colors[i].pos
                                        var localPos = (position - colors[i].pos) / range
                                        return interpolateColor(colors[i].color, colors[i + 1].color, localPos)
                                    }
                                }
                                
                                // Son renk (döngü için)
                                return colors[colors.length - 1].color
                            }
                            
                            function interpolateColor(color1, color2, factor) {
                                // Hex renkleri RGB'ye çevir
                                var c1 = hexToRgb(color1)
                                var c2 = hexToRgb(color2)
                                
                                // RGB değerleri interpolate et
                                var r = Math.round(c1.r + (c2.r - c1.r) * factor)
                                var g = Math.round(c1.g + (c2.g - c1.g) * factor)
                                var b = Math.round(c1.b + (c2.b - c1.b) * factor)
                                
                                // RGB'yi hex'e çevir
                                return rgbToHex(r, g, b)
                            }
                            
                            function hexToRgb(hex) {
                                var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex)
                                return result ? {
                                    r: parseInt(result[1], 16),
                                    g: parseInt(result[2], 16),
                                    b: parseInt(result[3], 16)
                                } : null
                            }
                            
                            function rgbToHex(r, g, b) {
                                var toHex = function(c) {
                                    var hex = c.toString(16)
                                    return hex.length === 1 ? "0" + hex : hex
                                }
                                return "#" + toHex(r) + toHex(g) + toHex(b)
                            }
                            
                            function hslToRgb(h, s, l) {
                                var r, g, b
                                
                                if (s === 0) {
                                    r = g = b = l // gri
                                } else {
                                    var hue2rgb = function(p, q, t) {
                                        if (t < 0) t += 1
                                        if (t > 1) t -= 1
                                        if (t < 1/6) return p + (q - p) * 6 * t
                                        if (t < 1/2) return q
                                        if (t < 2/3) return p + (q - p) * (2/3 - t) * 6
                                        return p
                                    }
                                    
                                    var q = l < 0.5 ? l * (1 + s) : l + s - l * s
                                    var p = 2 * l - q
                                    r = hue2rgb(p, q, h + 1/3)
                                    g = hue2rgb(p, q, h)
                                    b = hue2rgb(p, q, h - 1/3)
                                }
                                
                                // RGB'yi hex'e dönüştür
                                var toHex = function(c) {
                                    var hex = Math.round(c * 255).toString(16)
                                    return hex.length === 1 ? "0" + hex : hex
                                }
                                
                                return "#" + toHex(r) + toHex(g) + toHex(b)
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
                        color: theme.surfaceContainerHighest
                        border.color: theme.outlineVariant
                        border.width: 2
                        
                        Text {
                            text: iconFont + "add"
                            font.family: iconFont
                            font.pixelSize: 24
                            color: theme.onSurfaceVariant
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
