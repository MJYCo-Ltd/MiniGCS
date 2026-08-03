import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import MiniGCS

ColumnLayout {
    id: root

    property int selectedDroneId: -1
    signal commandRequested(int command)
    signal statusRequested()
    /** 每次点击列表项时发出（含重复选中同一架），用于地图定位 */
    signal locateRequested(int systemId)

    function selectDrone(systemId) {
        const alreadySelected = Number(selectedDroneId) === Number(systemId)
        selectedDroneId = systemId
        if (alreadySelected)
            locateRequested(systemId)
    }

    spacing: 10

    Label {
        text: qsTr("选择无人机")
        font.bold: true
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "white"
        radius: 6
        border.color: "#d9dee7"

        ListView {
            id: droneList
            anchors.fill: parent
            anchors.margins: 6
            clip: true
            spacing: 4
            model: DroneControl.drones

            delegate: Rectangle {
                id: droneDelegate
                required property var modelData
                width: ListView.view.width
                height: 104
                radius: 5
                color: root.selectedDroneId === modelData.systemId
                       ? "#e8f1ff" : "#ffffff"
                border.color: root.selectedDroneId === modelData.systemId
                              ? "#3b82f6" : "#e5e7eb"

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.selectDrone(
                                   droneDelegate.modelData.systemId)
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        RadioButton {
                            checked: root.selectedDroneId ===
                                     droneDelegate.modelData.systemId
                            onClicked: root.selectDrone(
                                           droneDelegate.modelData.systemId)
                        }
                        Label {
                            text: droneDelegate.modelData.name
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        Label {
                            text: droneDelegate.modelData.vehicle.moving
                                  ? qsTr("移动 %1 m/s").arg(
                                      droneDelegate.modelData.vehicle
                                      .velocity.groundSpeedMS.toFixed(1))
                                  : qsTr("静止")
                            color: droneDelegate.modelData.vehicle.moving
                                   ? "#b42318" : "#667085"
                            font.pixelSize: 11
                        }
                        Label {
                            text: droneDelegate.modelData.connected
                                  ? qsTr("在线") : qsTr("离线")
                            color: droneDelegate.modelData.connected
                                   ? "#15803d" : "#b42318"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: qsTr("ID %1").arg(
                                      droneDelegate.modelData.systemId)
                            color: "#667085"
                        }
                        Label {
                            text: droneDelegate.modelData.vehicle.autopilotName
                            color: "#667085"
                        }
                        Basic.TextField {
                            id: droneNameEditor
                            Layout.fillWidth: true
                            Layout.minimumHeight: implicitHeight
                            text: droneDelegate.modelData.name
                            placeholderText: qsTr("无人机别名")
                            selectByMouse: true
                        }
                        Button {
                            text: qsTr("保存")
                            onClicked: DroneControl.renameDrone(
                                droneDelegate.modelData.systemId,
                                droneNameEditor.text)
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: droneList.count === 0
                text: qsTr("等待发现无人机…")
                color: "#667085"
            }
        }
    }

    GridLayout {
        columns: 2
        Layout.fillWidth: true

        Button {
            text: DroneControl.commandName(DroneControl.armCommand)
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(DroneControl.armCommand)
        }
        Button {
            text: DroneControl.commandName(DroneControl.disarmCommand)
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(DroneControl.disarmCommand)
        }
        Button {
            text: DroneControl.commandName(DroneControl.takeoffCommand)
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(DroneControl.takeoffCommand)
        }
        Button {
            text: DroneControl.commandName(DroneControl.landCommand)
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(DroneControl.landCommand)
        }
        Button {
            text: DroneControl.commandName(
                      DroneControl.returnToLaunchCommand)
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(
                           DroneControl.returnToLaunchCommand)
        }
        Button {
            text: DroneControl.commandName(
                      DroneControl.downloadMissionCommand)
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(
                           DroneControl.downloadMissionCommand)
        }
        Button {
            text: DroneControl.commandName(
                      DroneControl.startMissionCommand)
            Layout.columnSpan: 2
            Layout.fillWidth: true
            highlighted: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.commandRequested(
                           DroneControl.startMissionCommand)
        }
        Button {
            text: qsTr("查看详细状态")
            Layout.columnSpan: 2
            Layout.fillWidth: true
            enabled: root.selectedDroneId >= 0
            onClicked: root.statusRequested()
        }
    }
}
