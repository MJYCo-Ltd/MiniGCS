pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MiniGCS

ApplicationWindow {
    id: root

    width: 1440
    height: 900
    minimumWidth: 1024
    minimumHeight: 700
    visible: true
    title: qsTr("MiniGCS 无人机控制")

    property int pendingCommand: -1
    property bool pendingGroupCommand: false
    property var pendingWaypoints: []

    function selectedDroneEntry() {
        const drones = DroneControl.drones
        for (let index = 0; index < drones.length; ++index) {
            if (Number(drones[index].systemId) ===
                    Number(controlPanel.selectedDroneId))
                return drones[index]
        }
        return null
    }

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
            MenuItem {
                text: qsTr("无人机详细状态")
                enabled: controlPanel.selectedDroneId >= 0
                onTriggered: {
                    controlPanelMenuItem.checked = true
                    rightPanelTabs.currentIndex = 1
                }
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
        Menu {
            title: qsTr("配置")

            MenuItem {
                text: qsTr("链路配置")
                onTriggered: {
                    settingsTabs.currentIndex = 0
                    settingsDialog.open()
                }
            }
            MenuItem {
                text: qsTr("地图配置")
                onTriggered: {
                    settingsTabs.currentIndex = 1
                    settingsDialog.open()
                }
            }
        }
    }

    SplitView {
        id: horizontalSplit
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Rectangle {
            implicitWidth: 5
            implicitHeight: 5
            color: SplitHandle.pressed ? "#3b82f6"
                  : SplitHandle.hovered ? "#93c5fd" : "#d0d5dd"
        }

        Rectangle {
            id: routeDock
            visible: routeEditorMenuItem.checked
            SplitView.preferredWidth: 360
            SplitView.minimumWidth: 240
            SplitView.maximumWidth: 520
            color: "#f5f7fa"
            border.color: "#d0d5dd"

            RouteEditor {
                id: routeEditor
                anchors.fill: parent
                radius: 0
                color: "#f8fafc"
                selectedDroneId: controlPanel.selectedDroneId
                selectedGroupName: controlPanel.selectedGroupName

                onUploadRequested: function(groupCommand, waypoints) {
                    root.requestCommand(DroneControl.uploadMissionCommand,
                                        groupCommand, waypoints)
                }
            }
        }

        SplitView {
            id: centerSplit
            SplitView.fillWidth: true
            SplitView.minimumWidth: 350
            orientation: Qt.Vertical

            handle: Rectangle {
                implicitWidth: 5
                implicitHeight: 5
                color: SplitHandle.pressed ? "#3b82f6"
                      : SplitHandle.hovered ? "#93c5fd" : "#d0d5dd"
            }

            Rectangle {
                SplitView.fillHeight: true
                SplitView.minimumHeight: 260
                color: "#e9eef5"

                Loader {
                    id: mapLoader
                    anchors.fill: parent
                    sourceComponent: mapComponent
                }

                Component {
                    id: mapComponent
                    DroneMap {
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
                }

                Connections {
                    target: AppConfig
                    function onMapConfigurationChanged() {
                        mapLoader.active = false
                        Qt.callLater(function() {
                            mapLoader.active = true
                        })
                    }
                }
            }

            LogPanel {
                id: logPanel
                visible: logPanelMenuItem.checked
                SplitView.preferredHeight: expanded ? 230 : 38
                SplitView.minimumHeight: expanded ? 120 : 38
                SplitView.maximumHeight: expanded ? 520 : 38
                radius: 0
                businessLogs: DroneControl.businessLogs
                firmwareLogs: DroneControl.firmwareLogs

                onClearBusinessRequested: DroneControl.clearBusinessLogs()
                onClearFirmwareRequested: DroneControl.clearFirmwareLogs()
            }
        }

        Rectangle {
            id: rightDock
            visible: controlPanelMenuItem.checked
            SplitView.preferredWidth: 460
            SplitView.minimumWidth: 320
            SplitView.maximumWidth: 720
            color: "#f5f7fa"
            border.color: "#d0d5dd"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                TabBar {
                    id: rightPanelTabs
                    Layout.fillWidth: true
                    TabButton { text: qsTr("控制") }
                    TabButton { text: qsTr("状态") }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: rightPanelTabs.currentIndex

                    DroneControlPanel {
                        id: controlPanel

                        onCommandRequested: function(command, groupCommand) {
                            root.requestCommand(command, groupCommand, [])
                        }
                        onStatusRequested: rightPanelTabs.currentIndex = 1
                    }

                    DroneStatusPage {
                        vehicle: {
                            const entry = root.selectedDroneEntry()
                            return entry ? entry.vehicle : null
                        }
                        droneName: {
                            const entry = root.selectedDroneEntry()
                            return entry ? entry.name : ""
                        }
                        waypointModel: routeEditor.waypointModel
                        onCloseRequested: rightPanelTabs.currentIndex = 0
                    }
                }
            }
        }
    }

    Dialog {
        id: settingsDialog
        modal: true
        width: Math.min(root.width - 80, 1050)
        height: Math.min(root.height - 80, 720)
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        padding: 0
        title: settingsTabs.currentIndex === 0
               ? qsTr("链路配置") : qsTr("地图配置")
        standardButtons: Dialog.Close

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            TabBar {
                id: settingsTabs
                Layout.fillWidth: true
                TabButton { text: qsTr("链路配置") }
                TabButton { text: qsTr("地图配置") }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: settingsTabs.currentIndex
                LinkConfigPage {}
                MapConfigPage {}
            }
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

        function onCommandResult(systemId, command, success, reason) {
            controlPanel.statusText = success
                    ? qsTr("无人机 %1 已确认“%2”")
                        .arg(systemId).arg(DroneControl.commandName(command))
                    : qsTr("无人机 %1 执行“%2”失败：%3")
                        .arg(systemId)
                        .arg(DroneControl.commandName(command))
                        .arg(reason)
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
