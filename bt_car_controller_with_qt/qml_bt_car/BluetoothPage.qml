import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Flickable {
    id: root

    property var theme
    property string iconFont: theme && theme.fontIcon ? theme.fontIcon : ""
    property var btManager

    readonly property bool hasBtManager: btManager !== null && btManager !== undefined
    readonly property bool hasDevicesModel: hasBtManager && btManager.devicesModel !== undefined && btManager.devicesModel !== null
    readonly property bool connected: hasBtManager ? btManager.isConnected : false
    readonly property bool scanning: hasBtManager ? btManager.scanning : false
    readonly property int devicesCount: hasBtManager ? btManager.devicesCount : 0
    readonly property string connectionStatusText: hasBtManager ? btManager.connectionStatus : "Hazır"

    property real scanProgress: scanning ? 0.66 : 1.0
    property string primaryName: (hasBtManager && btManager.primaryDeviceName !== "") ? btManager.primaryDeviceName : "Kinetic cihaz"
    property string primaryAddress: (hasBtManager && btManager.primaryDeviceAddress !== "") ? btManager.primaryDeviceAddress : "--:--:--:--:--:--"
    property int primaryRssi: hasBtManager ? btManager.primaryDeviceRssi : 0

    contentWidth: width
    contentHeight: contentColumn.implicitHeight + 32
    clip: true

    function normalizedRssi(rssiValue) {
        if (rssiValue >= 0) {
            return 0.0
        }
        var minRssi = -100
        var maxRssi = -35
        var normalized = (rssiValue - minRssi) / (maxRssi - minRssi)
        return Math.max(0.0, Math.min(1.0, normalized))
    }

    function statusBadgeText() {
        if (scanning) {
            return "BT: TARAMA"
        }
        if (connectionStatusText === "Bağlanıyor...") {
            return "BT: BAĞLANIYOR"
        }
        if (connected) {
            return "BT: BAĞLI"
        }
        return "BT: HAZIR"
    }

    Column {
        id: contentColumn
        x: 16
        y: 24
        width: root.width - 32
        spacing: 20

        Column {
            spacing: 12

            RowLayout {
                spacing: 12
                width: parent.width

                Column {
                    spacing: 6
                    Layout.alignment: Qt.AlignVCenter
                    Text {
                        text: connectionStatusText
                        font.family: theme.fontHeadline
                        font.pixelSize: 26
                        font.weight: Font.Bold
                        color: theme.textPrimary
                    }
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    height: 34
                    radius: 12
                    color: theme.surfaceContainerHigh
                    border.color: theme.outlineVariant
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 10

                        Item {
                            width: 10
                            height: 10

                            Rectangle {
                                id: ping
                                anchors.centerIn: parent
                                width: 10
                                height: 10
                                radius: 5
                                color: theme.primary
                                opacity: 0.35
                                scale: 1.0
                            }

                            ParallelAnimation {
                                running: true
                                loops: Animation.Infinite
                                NumberAnimation { target: ping; property: "scale"; from: 1.0; to: 2.4; duration: 1200 }
                                NumberAnimation { target: ping; property: "opacity"; from: 0.35; to: 0.0; duration: 1200 }
                            }

                            Rectangle {
                                width: 6
                                height: 6
                                radius: 3
                                anchors.centerIn: parent
                                color: theme.primary
                            }
                        }

                        Text {
                            text: statusBadgeText()
                            font.family: theme.fontHeadline
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            color: theme.primary
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 6
                radius: 3
                color: theme.surfaceVariant
                Rectangle {
                    width: parent.width * scanProgress
                    height: parent.height
                    radius: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: theme.primary }
                        GradientStop { position: 1.0; color: theme.secondary }
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 112
            radius: 18
            color: theme.surfaceContainerHigh
            border.color: theme.outlineVariant
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Rectangle {
                    width: 54
                    height: 54
                    radius: 14
                    color: "#1a00e5ff"
                    border.color: "#2200e5ff"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "directions_car"
                        font.family: iconFont
                        font.pixelSize: 28
                        color: theme.primary
                    }
                }

                Column {
                    spacing: 6
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true
                    Text {
                        text: primaryName
                        font.family: theme.fontHeadline
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: theme.textPrimary
                        elide: Text.ElideRight
                        width: parent.width
                    }
                    Text {
                        text: primaryAddress
                        font.family: theme.fontBody
                        font.pixelSize: 10
                        font.letterSpacing: 2
                        color: theme.textMuted
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }

                Item { Layout.fillWidth: true }

                Column {
                    spacing: 4
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Text {
                        text: primaryRssi < 0 ? (primaryRssi + " dBm") : "-- dBm"
                        font.family: theme.fontHeadline
                        font.pixelSize: 16
                        font.weight: Font.Bold
                        color: theme.primary
                        horizontalAlignment: Text.AlignRight
                    }
                    Text {
                        text: "SİNYAL GÜCÜ"
                        font.family: theme.fontBody
                        font.pixelSize: 9
                        font.letterSpacing: 1.5
                        color: theme.primary
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }

        Column {
            id: deviceListSection
            width: parent.width
            spacing: 12

            Text {
                text: "BULUNAN CİHAZLAR (" + devicesCount + ")"
                font.family: theme.fontHeadline
                font.pixelSize: 11
                font.letterSpacing: 2
                color: theme.primary
            }

            Repeater {
                id: deviceRepeater
                model: hasDevicesModel ? btManager.devicesModel : null

                delegate: Rectangle {
                    required property string name
                    required property string address
                    required property int rssi
                    required property bool paired

                    width: deviceListSection.width
                    height: 128
                    radius: 16
                    color: ma.pressed ? "#4caf50" : (ma.containsMouse ? theme.surfaceContainerHigh : theme.surfaceContainer)
                    border.color: ma.pressed ? "#4caf50" : (ma.containsMouse ? theme.outline : theme.outlineVariant)
                    border.width: ma.pressed ? 2 : 1
                    
                    scale: ma.pressed ? 0.98 : (ma.containsMouse ? 1.02 : 1.0)
                    
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }
                    Behavior on scale { NumberAnimation { duration: 100 } }

                    readonly property string statusText: paired ? "Kayıtlı" : "Kullanılabilir"
                    readonly property string itemIcon: paired ? "bluetooth_connected" : "bluetooth_searching"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            width: parent.width
                            spacing: 8
                            Text {
                                text: itemIcon
                                font.family: iconFont
                                font.pixelSize: 22
                                color: paired ? theme.primary : theme.secondary
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: statusText
                                font.family: theme.fontBody
                                font.pixelSize: 9
                                font.letterSpacing: 1.4
                                color: theme.outline
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        Column {
                            width: parent.width
                            spacing: 4
                            Text {
                                text: name !== "" ? name : address
                                font.family: theme.fontHeadline
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                                color: theme.textPrimary
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: address
                                font.family: theme.fontBody
                                font.pixelSize: 9
                                font.letterSpacing: 1.6
                                color: theme.textMuted
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: rssi < 0 ? (rssi + " dBm") : "-- dBm"
                                font.family: theme.fontBody
                                font.pixelSize: 9
                                color: theme.outline
                            }
                            Rectangle {
                                width: parent.width
                                height: 4
                                radius: 2
                                color: theme.surfaceVariant

                                Rectangle {
                                    width: parent.width * root.normalizedRssi(rssi)
                                    height: parent.height
                                    radius: parent.radius
                                    color: root.normalizedRssi(rssi) > 0.66
                                           ? theme.primary
                                           : (root.normalizedRssi(rssi) > 0.33 ? theme.secondary : "#ff8a65")
                                }
                            }
                        }
                    }

                    MouseArea {
                        id: ma
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            if (hasBtManager && address !== "") {
                                btManager.connectToDevice(address)
                            }
                        }
                    }
                }
            }

            Text {
                visible: !scanning && devicesCount === 0
                text: "Cihaz bulunamadı. Yenile ile tekrar tarayın."
                font.family: theme.fontBody
                font.pixelSize: 12
                color: theme.textMuted
            }
        }

        Rectangle {
            id: quickMatchCard
            width: parent.width
            height: 132
            radius: 18
            color: "#1a1a1a66"
            border.color: "#22ffffff"
            border.width: 1
            clip: true

            Rectangle {
                width: 120
                height: 120
                radius: 60
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: -40
                anchors.rightMargin: -40
                color: theme.primary
                opacity: 0.08
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: theme.surfaceContainer
                    border.color: "#2200e5ff"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "radar"
                        font.family: iconFont
                        font.pixelSize: 28
                        color: theme.primary
                    }
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "HIZLI EŞLEŞME"
                        font.family: theme.fontHeadline
                        font.pixelSize: 11
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                        color: theme.primary
                    }
                    Text {
                        text: "Yakındaki Kinetic uyumlu cihazlar otomatik olarak listenin en başında görünür. Bağlanmak için cihaza dokunun."
                        font.family: theme.fontBody
                        font.pixelSize: 11
                        color: theme.textMuted
                        wrapMode: Text.Wrap
                        width: quickMatchCard.width - 120
                    }
                }
            }
        }
    }
}
