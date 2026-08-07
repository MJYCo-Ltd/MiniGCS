import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import MiniGCS

Rectangle {
    id: root

    color: "#f4f7fb"

    function loadConfiguration() {
        mapProvider.editText = AppConfig.mapName()
        centerLatitude.text = String(AppConfig.mapCenterLatitude())
        centerLongitude.text = String(AppConfig.mapCenterLongitude())
        initialZoom.text = String(AppConfig.mapInitialZoom())
        vehicleZoom.text = String(AppConfig.mapVehicleZoom())
        minimumZoom.text = String(AppConfig.mapMinimumZoom())
        maximumZoom.text = String(AppConfig.mapMaximumZoom())
        defaultAltitude.text = String(AppConfig.missionDefaultAltitude())
        minimumAltitude.text = String(AppConfig.missionMinimumAltitude())
        maximumAltitude.text = String(AppConfig.missionMaximumAltitude())
    }

    function saveConfiguration() {
        const config = {
            "mapName": mapProvider.editText.trim(),
            "centerLatitude": Number(centerLatitude.text),
            "centerLongitude": Number(centerLongitude.text),
            "initialZoom": Number(initialZoom.text),
            "vehicleZoom": Number(vehicleZoom.text),
            "minimumZoom": Number(minimumZoom.text),
            "maximumZoom": Number(maximumZoom.text),
            "defaultAltitude": Number(defaultAltitude.text),
            "minimumAltitude": Number(minimumAltitude.text),
            "maximumAltitude": Number(maximumAltitude.text)
        }
        const saved = AppConfig.setMapConfiguration(config)
        resultLabel.text = saved
                ? qsTr("地图配置已保存并重新加载")
                : qsTr("配置无效：请检查坐标、缩放范围和高度范围")
        resultLabel.color = saved ? "#027a48" : "#b42318"
    }

    Component.onCompleted: loadConfiguration()

    Connections {
        target: AppConfig
        function onMapConfigurationChanged() {
            root.loadConfiguration()
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: Math.min(parent.width - 40, 900)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            Item { Layout.preferredHeight: 4 }

            Label {
                text: qsTr("地图配置")
                font.pixelSize: 24
                font.bold: true
                color: "#1d2939"
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("配置地图插件、默认中心点、缩放范围和航线高度。保存后中央地图会自动重载。")
                wrapMode: Text.Wrap
                color: "#667085"
            }

            Basic.Frame {
                Layout.fillWidth: true
                background: Rectangle { color: "white"; radius: 8; border.color: "#d0d5dd" }

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 18
                    rowSpacing: 12

                    Label { text: qsTr("地图插件"); font.bold: true }
                    AppComboBox {
                        id: mapProvider
                        Layout.fillWidth: true
                        editable: true
                        model: ["QGroundControl", "osm"]
                    }

                    Label { text: qsTr("默认中心纬度"); font.bold: true }
                    TextField {
                        id: centerLatitude
                        Layout.fillWidth: true
                        placeholderText: "-90 … 90"
                        validator: DoubleValidator { bottom: -90; top: 90; decimals: 8 }
                    }

                    Label { text: qsTr("默认中心经度"); font.bold: true }
                    TextField {
                        id: centerLongitude
                        Layout.fillWidth: true
                        placeholderText: "-180 … 180"
                        validator: DoubleValidator { bottom: -180; top: 180; decimals: 8 }
                    }
                }
            }

            Basic.Frame {
                Layout.fillWidth: true
                background: Rectangle { color: "white"; radius: 8; border.color: "#d0d5dd" }

                GridLayout {
                    anchors.fill: parent
                    columns: 4
                    columnSpacing: 12
                    rowSpacing: 10

                    Label { text: qsTr("初始缩放"); font.bold: true }
                    TextField { id: initialZoom; Layout.fillWidth: true; validator: DoubleValidator { bottom: 0; top: 30 } }
                    Label { text: qsTr("跟随无人机缩放"); font.bold: true }
                    TextField { id: vehicleZoom; Layout.fillWidth: true; validator: DoubleValidator { bottom: 0; top: 30 } }
                    Label { text: qsTr("最小缩放"); font.bold: true }
                    TextField { id: minimumZoom; Layout.fillWidth: true; validator: DoubleValidator { bottom: 0; top: 30 } }
                    Label { text: qsTr("最大缩放"); font.bold: true }
                    TextField { id: maximumZoom; Layout.fillWidth: true; validator: DoubleValidator { bottom: 0; top: 30 } }
                }
            }

            Basic.Frame {
                Layout.fillWidth: true
                background: Rectangle { color: "white"; radius: 8; border.color: "#d0d5dd" }

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 18
                    rowSpacing: 10

                    Label { text: qsTr("默认航点高度"); font.bold: true }
                    TextField { id: defaultAltitude; Layout.fillWidth: true; validator: DoubleValidator {} }
                    Label { text: qsTr("最小航点高度"); font.bold: true }
                    TextField { id: minimumAltitude; Layout.fillWidth: true; validator: DoubleValidator {} }
                    Label { text: qsTr("最大航点高度"); font.bold: true }
                    TextField { id: maximumAltitude; Layout.fillWidth: true; validator: DoubleValidator {} }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Button { text: qsTr("重新读取"); onClicked: root.loadConfiguration() }
                Item { Layout.fillWidth: true }
                Label { id: resultLabel; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; wrapMode: Text.Wrap }
                Button { text: qsTr("保存并应用"); highlighted: true; onClicked: root.saveConfiguration() }
            }

            Item { Layout.preferredHeight: 16 }
        }
    }
}
