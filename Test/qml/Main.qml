import QtQuick
import QtQuick.Controls
import MiniGCS

ApplicationWindow {
    id: root

    width: 1180
    height: 720
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("MiniGCS 无人机控制")

    property int pendingCommand: -1
    property bool pendingGroupCommand: false
    property var pendingWaypoints: []

    function requestCommand(command, groupCommand, waypoints) {
        pendingCommand = command
        pendingGroupCommand = groupCommand
        pendingWaypoints = waypoints || []
        confirmDialog.open()
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("视图")

            MenuItem {
                id: controlPanelMenuItem
                text: qsTr("无人机控制面板")
                checkable: true
                checked: true
            }
            MenuItem {
                id: routeEditorMenuItem
                text: qsTr("航线编辑")
                checkable: true
                checked: true
            }
            MenuItem {
                id: logPanelMenuItem
                text: qsTr("告警日志")
                checkable: true
                checked: true
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("显示全部")
                onTriggered: {
                    controlPanelMenuItem.checked = true
                    routeEditorMenuItem.checked = true
                    logPanelMenuItem.checked = true
                }
            }
            MenuItem {
                text: qsTr("隐藏全部")
                onTriggered: {
                    controlPanelMenuItem.checked = false
                    routeEditorMenuItem.checked = false
                    logPanelMenuItem.checked = false
                }
            }
        }
    }

    DroneMap {
        anchors.fill: parent
        anchors.rightMargin: controlPanel.visible ? controlPanel.width : 0
        drones: DroneControl.drones
        selectedDroneId: controlPanel.selectedDroneId
        waypointModel: routeEditor.waypointModel
        routeCoordinates: routeEditor.routeCoordinates
        routeEditing: routeEditor.editing

        onDroneSelected: function(systemId) {
            controlPanel.selectedDroneId = systemId
        }
        onRouteCoordinateRequested: function(coordinate) {
            routeEditor.addCoordinate(coordinate)
        }
    }

    RouteEditor {
        id: routeEditor
        visible: routeEditorMenuItem.checked
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 12
        z: 10
        selectedDroneId: controlPanel.selectedDroneId
        selectedGroupName: controlPanel.selectedGroupName

        onUploadRequested: function(groupCommand, waypoints) {
            root.requestCommand(DroneControl.uploadMissionCommand,
                                groupCommand, waypoints)
        }
    }

    LogPanel {
        id: logPanel
        visible: logPanelMenuItem.checked
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 12
        anchors.rightMargin: controlPanel.visible
                             ? controlPanel.width + 12 : 12
        anchors.bottomMargin: 12
        z: 9
        businessLogs: DroneControl.businessLogs
        firmwareLogs: DroneControl.firmwareLogs

        onClearBusinessRequested: DroneControl.clearBusinessLogs()
        onClearFirmwareRequested: DroneControl.clearFirmwareLogs()
    }

    DroneControlPanel {
        id: controlPanel
        visible: controlPanelMenuItem.checked
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onCommandRequested: function(command, groupCommand) {
            root.requestCommand(command, groupCommand, [])
        }
    }

    CommandConfirmDialog {
        id: confirmDialog
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        commandLabel: DroneControl.commandName(root.pendingCommand)
        targetName: root.pendingGroupCommand
                    ? controlPanel.selectedGroupName
                    : qsTr("系统 ID %1").arg(controlPanel.selectedDroneId)
        groupCommand: root.pendingGroupCommand

        onCommandConfirmed: {
            if (root.pendingCommand ===
                    DroneControl.uploadMissionCommand) {
                if (root.pendingGroupCommand) {
                    DroneControl.uploadMissionGroup(
                                controlPanel.selectedGroupName,
                                root.pendingWaypoints)
                } else {
                    DroneControl.uploadMissionSingle(
                                controlPanel.selectedDroneId,
                                root.pendingWaypoints)
                }
            } else if (root.pendingGroupCommand) {
                DroneControl.executeGroup(
                            controlPanel.selectedGroupName,
                            root.pendingCommand)
            } else {
                DroneControl.executeSingle(
                            controlPanel.selectedDroneId,
                            root.pendingCommand)
            }
        }
    }

    Connections {
        target: DroneControl

        function onCommandDispatched(command, target, count) {
            controlPanel.statusText =
                qsTr("已向“%1”的 %2 架在线无人机发送“%3”")
                    .arg(target).arg(count)
                    .arg(DroneControl.commandName(command))
        }

        function onCommandRejected(reason) {
            controlPanel.statusText = qsTr("命令未发送：%1").arg(reason)
        }

        function onMissionDownloaded(systemId, waypoints) {
            if (Number(systemId) ===
                    Number(controlPanel.selectedDroneId)) {
                routeEditor.loadMissionWaypoints(waypoints)
                controlPanel.statusText =
                    qsTr("已加载无人机 %1 的 %2 个航点")
                        .arg(systemId).arg(waypoints.length)
            }
        }

        function onMissionUploadResult(systemId, success, reason) {
            controlPanel.statusText = success
                    ? qsTr("无人机 %1 航线上传成功").arg(systemId)
                    : qsTr("无人机 %1 航线上传失败：%2")
                        .arg(systemId).arg(reason)
        }
    }
}
