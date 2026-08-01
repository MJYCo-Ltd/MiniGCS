import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MiniGCS

Rectangle {
    id: root

    property alias selectedDroneId: singleControl.selectedDroneId
    property alias selectedGroupName: groupControl.selectedGroupName
    property string statusText: qsTr("控制命令执行前会进行确认")

    signal commandRequested(int command, bool groupCommand)
    signal statusRequested()
    signal locateRequested(int systemId)

    function setStatus(text) {
        statusText = text
    }

    implicitWidth: 410
    color: "#f5f7fa"
    border.color: "#d9dee7"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: qsTr("无人机配置与控制")
            font.pixelSize: 22
            font.bold: true
            color: "#1f2937"
        }

        Label {
            text: qsTr("已发现 %1 架 · 编组 %2 个")
                .arg(DroneControl.drones.length)
                .arg(DroneControl.groups.length)
            color: "#667085"
        }

        TabBar {
            id: modeTabs
            Layout.fillWidth: true
            TabButton { text: qsTr("单机控制") }
            TabButton { text: qsTr("编组控制") }
        }

        StackLayout {
            currentIndex: modeTabs.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            SingleDroneControl {
                id: singleControl
                onCommandRequested: function(command) {
                    root.commandRequested(command, false)
                }
                onStatusRequested: root.statusRequested()
                onLocateRequested: function(systemId) {
                    root.locateRequested(systemId)
                }
            }

            GroupDroneControl {
                id: groupControl
                onCommandRequested: function(command) {
                    root.commandRequested(command, true)
                }
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: "#344054"
            text: root.statusText
        }
    }
}
