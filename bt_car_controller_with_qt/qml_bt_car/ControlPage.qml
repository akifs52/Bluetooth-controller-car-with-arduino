import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Flickable {
    id: root

    property var theme
    property string iconFont: theme && theme.fontIcon ? theme.fontIcon : ""
    property var btManager

    readonly property bool hasBtManager: btManager !== null && btManager !== undefined
    readonly property bool connected: hasBtManager ? btManager.isConnected : false
    readonly property int batteryLevel: hasBtManager ? btManager.batteryLevel : 75
    readonly property bool charging: hasBtManager ? btManager.isCharging : false
    property int leftPwm: 100
    property int rightPwm: 100

    interactive: !(leftPwmGauge.dragging || rightPwmGauge.dragging || joystick.dragging)
    contentWidth: width
    contentHeight: contentColumn.implicitHeight + 40
    clip: true

    Column {
        id: contentColumn
        x: 20
        y: 20
        width: root.width - 40
        spacing: 20

        Item {
            id: statusSection
            width: parent.width
            height: statusFlow.implicitHeight > 0 ? statusFlow.implicitHeight : 232
            property real reveal: 0
            opacity: reveal
            transform: Translate { y: (1 - statusSection.reveal) * 16 }

            Flow {
                id: statusFlow
                anchors.fill: parent
                spacing: 16

                Rectangle {
                    width: statusFlow.width >= 720 ? (statusFlow.width - statusFlow.spacing) / 2 : statusFlow.width
                    height: 108
                    radius: 18
                    color: "#662c2c2c"
                    border.color: "#1affffff"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        Column {
                            spacing: 6
                            Layout.alignment: Qt.AlignVCenter
                            Text {
                                text: "SİSTEM DURUMU"
                                font.family: theme.fontHeadline
                                font.pixelSize: 9
                                font.letterSpacing: 2
                                color: theme.textMuted
                            }
                            Text {
                                text: connected ? "HAZIR" : "BEKLEME"
                                font.family: theme.fontHeadline
                                font.pixelSize: 24
                                font.weight: Font.Bold
                                color: theme.textPrimary
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Column {
                            spacing: 6
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                            Row {
                                spacing: 8
                                Text {
                                    text: batteryLevel + "%"
                                    font.family: theme.fontHeadline
                                    font.pixelSize: 22
                                    font.weight: Font.DemiBold
                                    color: theme.primary
                                }

                                Item {
                                    width: 48
                                    height: 18

                                    Rectangle {
                                        id: batteryShell
                                        x: 0
                                        y: 8
                                        width: 40
                                        height: 16
                                        radius: 3
                                        color: "transparent"
                                        border.color: theme.primary
                                        border.width: 1
                                    }

                                    Rectangle {
                                        width: 3
                                        height: 8
                                        radius: 1
                                        x: batteryShell.x + batteryShell.width + 2
                                        y: batteryShell.y + 4
                                        color: theme.primary
                                    }

                                    Rectangle {
                                        id: batteryFill
                                        x: batteryShell.x + 2
                                        y: batteryShell.y + 2
                                        height: batteryShell.height - 4
                                        width: Math.max(2, (batteryShell.width - 4) * Math.max(0, Math.min(100, batteryLevel)) / 100.0)
                                        radius: 2
                                        color: batteryLevel > 60 ? theme.primary : (batteryLevel > 25 ? "#ffd166" : "#ff6b6b")

                                        Behavior on width {
                                            NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
                                        }
                                    }

                                    Text {
                                        visible: charging
                                        anchors.centerIn: batteryShell
                                        text: "bolt"
                                        font.family: iconFont
                                        font.pixelSize: 14
                                        color: "#0e0e0e"
                                    }
                                }
                            }

                            Text {
                                text: "BATARYA DÜZEYİ"
                                font.family: theme.fontHeadline
                                font.pixelSize: 9
                                font.letterSpacing: 2
                                color: theme.textMuted
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }

                Item {
                    width: statusFlow.width >= 720 ? (statusFlow.width - statusFlow.spacing) / 2 : statusFlow.width
                    height: 108

                    Rectangle {
                        anchors.fill: parent
                        radius: 20
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: theme.primary }
                            GradientStop { position: 1.0; color: theme.secondary }
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 1
                        radius: 19
                        color: ma.pressed ? "#ff4d4f" : (ma.containsMouse ? theme.surfaceContainerHigh : theme.surfaceContainer)
                        border.color: ma.pressed ? "#ff4d4f" : (ma.containsMouse ? theme.outline : "transparent")
                        border.width: ma.pressed ? 2 : 0
                        
                        scale: ma.pressed ? 0.98 : (ma.containsMouse ? 1.02 : 1.0)
                        
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }
                        Behavior on scale { NumberAnimation { duration: 100 } }
                    }

                    MouseArea {
                        id: ma
                        anchors.fill: parent
                        hoverEnabled: true
                        onPressed: {
                            // Gauge'a basınca titreşim
                            if (Qt.platform.os === "android") {
                                Qt.vibrate(50) // 50ms titreşim
                            }
                        }
                        onReleased: {
                            // Gauge'dan basınca titreşim
                            if (Qt.platform.os === "android") {
                                Qt.vibrate(30) // 30ms titreşim
                            }
                        }
                        onClicked: {
                            if (hasBtManager) {
                                if (btManager.isConnected) {
                                    // Bağlantı varsa bağlantıyı kes
                                    btManager.disconnect()
                                    console.log("Bluetooth bağlantısı kesildi, tarama başlatılıyor...")
                                }
                                // Yeniden tarama başlat
                                btManager.startDiscovery()
                            }
                        }
                    }

                    Row {
                        anchors.centerIn: parent
                        spacing: 10
                        Text {
                            text: "power_settings_new"
                            font.family: iconFont
                            font.pixelSize: 20
                            color: theme.primary
                        }
                        Text {
                            text: "DENETLEYİCİYİ YENİDEN EŞLE"
                            font.family: theme.fontHeadline
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            font.letterSpacing: 2
                            color: theme.primary
                        }
                    }
                }
            }
        }

        Item {
            id: gaugesSection
            width: parent.width
            height: 200
            property real reveal: 0
            opacity: reveal
            transform: Translate { y: (1 - gaugesSection.reveal) * 16 }

            Row {
                id: gaugesRow
                anchors.fill: parent
                spacing: 16

                Gauge {
                    id: leftPwmGauge
                    width: (gaugesRow.width - gaugesRow.spacing) / 2
                    height: 200
                    accent: "#ff4d4f"
                    label: "Normal Hız"
                    unit: "PWM"
                    minimumValue: 0
                    maximumValue: 255
                    value: root.leftPwm
                    interactive: true
                    fontFamily: theme.fontHeadline
                    labelColor: "#ff4d4f"
                    onValueEdited: {
                        root.leftPwm = value
                        bluetoothManager.sendNormalSpeed(value)
                        console.log("Normal Speed:", value)
                        // Gauge değerini değiştirince titreşim
                        if (Qt.platform.os === "android") {
                            Qt.vibrate(40) // 40ms titreşim
                        }
                    }
                }

                Gauge {
                    id: rightPwmGauge
                    width: (gaugesRow.width - gaugesRow.spacing) / 2
                    height: 200
                    accent: theme.secondary
                    label: "Dönüş Hızı"
                    unit: "PWM"
                    minimumValue: 0
                    maximumValue: 255
                    value: root.rightPwm
                    interactive: true
                    fontFamily: theme.fontHeadline
                    labelColor: theme.secondary
                    onValueEdited: {
                        root.rightPwm = value
                        bluetoothManager.sendTurnSpeed(value)
                        console.log("Turn Speed:", value)
                        // Gauge değerini değiştirince titreşim
                        if (Qt.platform.os === "android") {
                            Qt.vibrate(40) // 40ms titreşim
                        }
                    }
                }
            }
        }

        Item {
            id: joystickSection
            width: parent.width
            height: joystickStack.implicitHeight > 0 ? joystickStack.implicitHeight : 360
            property real reveal: 0
            opacity: reveal
            transform: Translate { y: (1 - joystickSection.reveal) * 16 }

            Column {
                id: joystickStack
                width: parent.width
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                Item {
                    width: parent.width
                    height: 300

                    Rectangle {
                        width: 280
                        height: 280
                        radius: width / 2
                        anchors.centerIn: parent
                        color: theme.primary
                        opacity: 0.08
                    }

                    Joystick {
                        id: joystick
                        anchors.centerIn: parent
                        width: Math.min(parent.width, 260)
                        height: width
                        iconFont: iconFont
                        accent: theme.primary
                        surfaceBright: theme.surfaceBright
                        surface: theme.surfaceContainer
                        trackOuter: theme.surfaceContainer
                        trackInner: "#000000"
                        
                        onMoved: function(x, y) {
                            // Joystick hareketini komuta çevir
                            var direction = "S" // Stop (default)
                            var targetLeftPwm = 0
                            var targetRightPwm = 0
                            
                            // Joystick hareketinde titreşim
                            if (Qt.platform.os === "android") {
                                Qt.vibrate(25) // 25ms titreşim
                            }
                            
                            if (Math.abs(x) > Math.abs(y)) {
                                // Yatay hareket baskın
                                if (x > 0.3) {
                                    direction = "R" // Right
                                    targetRightPwm = Math.abs(x) * 255
                                }
                                else if (x < -0.3) {
                                    direction = "L" // Left
                                    targetLeftPwm = Math.abs(x) * 255
                                }
                            } else {
                                // Dikey hareket baskın - sadece komut gönder, gauge'ları değiştirme
                                if (y > 0.3) {
                                    direction = "B" // Down
                                    // Gaz pedalı - gauge'ları mevcut değerde tut
                                }
                                else if (y < -0.3) {
                                    direction = "F" // Up
                                    // Gaz pedalı - gauge'ları mevcut değerde tut
                                }
                            }
                            
                            if (Math.abs(x) < 0.3 && Math.abs(y) < 0.3) {
                                direction = "S" // Stop
                                // Stop - gauge'ları mevcut değerde tut
                                console.log("Joystick centered - sending STOP command")
                            }
                            
                            // Joystick gauge'ları değiştirmez, sadece komut gönderir
                            // Gauge'lar sadece manuel olarak değiştirilebilir
                            
                            bluetoothManager.sendJoystickCommand(direction)
                            console.log("Joystick command:", direction, "x:", x.toFixed(2), "y:", y.toFixed(2), "leftPWM:", targetLeftPwm.toFixed(0), "rightPWM:", targetRightPwm.toFixed(0))
                        }
                    }
                }

                Row {
                    spacing: 40
                    anchors.horizontalCenter: parent.horizontalCenter

                    Column {
                        spacing: 6
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            text: "YANAL"
                            font.family: theme.fontHeadline
                            font.pixelSize: 9
                            font.letterSpacing: 2
                            color: theme.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            width: 80
                        }
                        Rectangle {
                            width: 52
                            height: 4
                            radius: 2
                            color: theme.surfaceContainer
                            Rectangle {
                                id: lateralBar
                                width: parent.width * (root.leftPwm / 255)
                                height: parent.height
                                radius: parent.radius
                                color: root.leftPwm > 200 ? "#ff4d4f" : theme.primary
                                
                                Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                                Behavior on color { ColorAnimation { duration: 150 } }
                            }
                        }
                    }

                    Column {
                        spacing: 6
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            text: "BOYLAMSAL"
                            font.family: theme.fontHeadline
                            font.pixelSize: 9
                            font.letterSpacing: 2
                            color: theme.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            width: 100
                        }
                        Rectangle {
                            width: 52
                            height: 4
                            radius: 2
                            color: theme.surfaceContainer
                            Rectangle {
                                id: longitudinalBar
                                width: parent.width * (root.rightPwm / 255)
                                height: parent.height
                                radius: parent.radius
                                color: root.rightPwm > 200 ? "#ff4d4f" : theme.primary
                                
                                Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                                Behavior on color { ColorAnimation { duration: 150 } }
                            }
                        }
                    }
                }
            }
        }
    }

    // Hız göstergesi animasyonları
    PropertyAnimation {
        id: leftPwmAnimation
        target: root
        property: "leftPwm"
        duration: 200
        easing.type: Easing.OutQuad
    }
    
    PropertyAnimation {
        id: rightPwmAnimation
        target: root
        property: "rightPwm"
        duration: 200
        easing.type: Easing.OutQuad
    }

    SequentialAnimation {
        running: true
        PropertyAnimation {
            target: statusSection
            property: "reveal"
            from: 0
            to: 1
            duration: 260
            easing.type: Easing.OutCubic
        }
        PauseAnimation { duration: 90 }
        PropertyAnimation {
            target: gaugesSection
            property: "reveal"
            from: 0
            to: 1
            duration: 260
            easing.type: Easing.OutCubic
        }
        PauseAnimation { duration: 90 }
        PropertyAnimation {
            target: joystickSection
            property: "reveal"
            from: 0
            to: 1
            duration: 300
            easing.type: Easing.OutCubic
        }
    }
}
