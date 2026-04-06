import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: window
    width: 430
    height: 932
    visible: true
    title: "Kinetic Drive"
    color: theme.background

    property int activeTab: 0

    readonly property var btBackend: bluetoothManager
    readonly property bool hasBtManager: btBackend !== null
    readonly property bool connected: hasBtManager ? btBackend.isConnected : false
    readonly property int batteryLevel: hasBtManager ? btBackend.batteryLevel : 75
    readonly property bool charging: hasBtManager ? btBackend.isCharging : false
    readonly property var gamepadBackend: gamepadManager

    onActiveTabChanged: {
        if (activeTab === 1 && bluetoothManager) {
            Qt.callLater(function() {
                bluetoothManager.startDiscovery()
            })
        }
    }

    FontLoader {
        id: interVariable
        source: "qrc:/qml_bt_car/fonts/Inter-Variable.ttf"
    }

    FontLoader {
        id: materialIcons
        source: "qrc:/qml_bt_car/fonts/MaterialIcons-Regular.ttf"
    }

    QtObject {
        id: theme
        property color background: "#0e0e0e"
        property color surface: "#0e0e0e"
        property color surfaceContainer: "#1a1a1a"
        property color surfaceContainerLow: "#131313"
        property color surfaceContainerHigh: "#20201f"
        property color surfaceContainerLowest: "#000000"
        property color surfaceVariant: "#262626"
        property color surfaceBright: "#2c2c2c"
        property color primary: "#81ecff"
        property color secondary: "#10d5ff"
        property color textPrimary: "#ffffff"
        property color textMuted: "#adaaaa"
        property color outline: "#767575"
        property color outlineVariant: "#484847"
        property color accentGlow: "#00e5ff"

        property string fontHeadline: interVariable.name !== "" ? interVariable.name : "Segoe UI"
        property string fontBody: interVariable.name !== "" ? interVariable.name : "Segoe UI"
        property string fontIcon: materialIcons.name !== "" ? materialIcons.name : "Segoe UI Symbol"
    }

    Rectangle {
        anchors.fill: parent
        color: theme.background
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0e0e0e" }
            GradientStop { position: 0.45; color: "#0c1113" }
            GradientStop { position: 1.0; color: "#0b0b0b" }
        }
        opacity: 0.9
    }

    Rectangle {
        width: 320
        height: 320
        radius: width / 2
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: -80
        anchors.topMargin: -100
        color: theme.primary
        opacity: 0.08
    }

    Rectangle {
        width: 360
        height: 360
        radius: width / 2
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: -140
        anchors.bottomMargin: -160
        color: theme.secondary
        opacity: 0.06
    }

    Rectangle {
        id: topBar
        height: 72
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#cc0e0e0e"
        border.color: "#22ffffff"
        border.width: 1

        Row {
            spacing: 12
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: backMa.containsMouse ? "#1a1a1a" : "#0e0e0e"
                border.color: backMa.pressed ? theme.accentGlow : (backMa.containsMouse ? "#44ffffff" : "#22ffffff")
                border.width: backMa.pressed ? 2 : 1
                
                scale: backMa.pressed ? 0.9 : (backMa.containsMouse ? 1.1 : 1.0)
                
                Behavior on color { ColorAnimation { duration: 150 } }
                Behavior on border.color { ColorAnimation { duration: 150 } }
                Behavior on scale { NumberAnimation { duration: 100 } }

                Text {
                    anchors.centerIn: parent
                    text: "arrow_back"
                    font.family: theme.fontIcon
                    font.pixelSize: 22
                    color: backMa.pressed ? "#ffffff" : theme.accentGlow
                    
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
                
                MouseArea {
                    id: backMa
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (activeTab === 0) {
                            // Sürüş ekranındaysa uygulamadan çık
                            Qt.quit()
                        } else {
                            // Diğer ekranlardaysa bir önceki ekrana dön
                            activeTab = 0
                        }
                    }
                }
            }

            Text {
                text: ""
                font.family: theme.fontHeadline
                font.pixelSize: 12
                font.weight: Font.DemiBold
                font.letterSpacing: 3
                color: theme.accentGlow
            }
        }

        RowLayout {
            spacing: 8
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            height: 36

            Rectangle {
                width: 96
                height: 28
                radius: height / 2
                color: theme.surfaceContainer
                border.color: "#1affffff"
                border.width: 1
                Layout.alignment: Qt.AlignVCenter

                Row {
                    spacing: 4
                    anchors.centerIn: parent

                    Text {
                        text: "bluetooth_searching"
                        font.family: theme.fontIcon
                        font.pixelSize: 14
                        color: theme.primary
                    }

                    Text {
                        text: connected ? "BT: BAĞLI" : "BT: DEĞİL"
                        font.family: theme.fontHeadline
                        font.pixelSize: 8
                        font.letterSpacing: 1.4
                        color: theme.textPrimary
                    }
                }
            }

            Rectangle {
                width: 76
                height: 28
                radius: height / 2
                color: theme.surfaceContainer
                border.color: "#1affffff"
                border.width: 1
                Layout.alignment: Qt.AlignVCenter

                Row {
                    anchors.centerIn: parent
                    spacing: 6

                    Text {
                        //text: batteryLevel + "%"
                        font.family: theme.fontHeadline
                        font.pixelSize: 9
                        font.weight: Font.DemiBold
                        color: theme.primary
                    }

                    Item {
                        width: 20
                        height: 12

                        Rectangle {
                            id: topBarBatteryShell
                            x: 0
                            y: 1
                            width: 16
                            height: 10
                            radius: 2
                            color: "transparent"
                            border.color: theme.primary
                            border.width: 1
                        }

                        Rectangle {
                            width: 2
                            height: 5
                            x: topBarBatteryShell.x + topBarBatteryShell.width + 1
                            y: topBarBatteryShell.y + 3
                            radius: 1
                            color: theme.primary
                        }

                        Rectangle {
                            x: topBarBatteryShell.x + 1
                            y: topBarBatteryShell.y + 1
                            height: topBarBatteryShell.height - 2
                            width: Math.max(1, (topBarBatteryShell.width - 2) * Math.max(0, Math.min(100, batteryLevel)) / 100.0)
                            radius: 1
                            color: batteryLevel > 60 ? theme.primary : (batteryLevel > 25 ? "#ffd166" : "#ff6b6b")
                        }

                        Text {
                            visible: charging
                            anchors.centerIn: topBarBatteryShell
                            text: "bolt"
                            font.family: theme.fontIcon
                            font.pixelSize: 10
                            color: "#0e0e0e"
                        }
                    }
                }
            }

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: "#0e0e0e"
                border.color: "#22ffffff"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "settings"
                    font.family: theme.fontIcon
                    font.pixelSize: 20
                    color: theme.accentGlow
                }
            }
        }
    }

    StackLayout {
        id: pages
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomBar.top
        currentIndex: activeTab

        ControlPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: theme
            iconFont: theme.fontIcon
            btManager: btBackend
            gamepadManager: gamepadBackend
        }

        BluetoothPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: theme
            iconFont: theme.fontIcon
            btManager: btBackend
        }

    RGBPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            theme: theme
            iconFont: theme.fontIcon
            bluetoothManager: btBackend
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                anchors.centerIn: parent
                text: "PROFİL"
                font.family: theme.fontHeadline
                font.pixelSize: 24
                font.weight: Font.DemiBold
                color: theme.textMuted
            }
        }
    }

    Rectangle {
        id: bottomBar
        height: 92
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#cc0e0e0e"
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0e0e0e" }
            GradientStop { position: 1.0; color: "#091215" }
        }
        border.color: "#1affffff"
        border.width: 1

        Row {
            id: navRow
            width: parent.width - 40
            anchors.centerIn: parent
            spacing: 10

            Repeater {
                model: ListModel {
                    ListElement { icon: "directions_car"; label: "SÜRÜŞ" }
                    ListElement { icon: "bluetooth_searching"; label: "BAĞLANTI" }
                    ListElement { icon: "lightbulb"; label: "RGB" }
                    ListElement { icon: "account_circle"; label: "PROFİL" }
                }

                delegate: Item {
                    id: navItem
                    width: (navRow.width - navRow.spacing * 3) / 4
                    height: 64
                    property bool active: index === activeTab

                    MouseArea {
                        id: navMa
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: activeTab = index
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 18
                        color: active ? "#1a00e5ff" : (navMa.containsMouse ? "#1a767575" : "transparent")
                        border.color: active ? "#2200e5ff" : (navMa.containsMouse ? "#33767575" : "transparent")
                        border.width: 1
                        
                        scale: navMa.pressed ? 0.95 : (navMa.containsMouse ? 1.05 : 1.0)
                        
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }
                        Behavior on scale { NumberAnimation { duration: 100 } }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 4

                        Text {
                            text: icon
                            font.family: theme.fontIcon
                            font.pixelSize: 22
                            color: active ? theme.accentGlow : "#666666"
                        }

                        Text {
                            text: label
                            font.family: theme.fontHeadline
                            font.pixelSize: 9
                            font.letterSpacing: 1.6
                            font.weight: Font.DemiBold
                            color: active ? theme.accentGlow : "#666666"
                        }
                    }
                }
            }
        }
    }
}
