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
    title: qsTr("MiniGCS 任务控制")

    // Windows 系统 palette 下 ComboBox/Menu 高亮项常为白字+浅底，导致选中项不可见。
    palette.highlight: "#dbeafe"
    palette.highlightedText: "#101828"
    palette.windowText: "#101828"
    palette.text: "#101828"
    palette.buttonText: "#101828"

    property int selectedDroneId: -1
    property string selectedGroupName: ""
    property int pendingCommand: -1
    property bool pendingGroupCommand: false
    property var pendingWaypoints: []
    property string pendingActionLabel: ""
    property string pendingDetailText: ""
    property bool pendingStartAfterUpload: false

    function selectedDroneEntry() {
        const drones = DroneControl.drones
        for (let index = 0; index < drones.length; ++index) {
            if (Number(drones[index].systemId) === Number(selectedDroneId))
                return drones[index]
        }
        return null
    }

    function selectedDroneName() {
        const entry = selectedDroneEntry()
        return entry ? entry.name : qsTr("所选无人机")
    }

    function selectFirstDroneIfNeeded() {
        if (selectedDroneId >= 0 || DroneControl.drones.length === 0)
            return
        selectedDroneId = Number(DroneControl.drones[0].systemId)
    }

    function centerOnDrone(systemId) {
        if (mapLoader.item)
            mapLoader.item.centerOnDrone(systemId)
    }

    function setUserStatus(text) {
        advancedControlPanel.setStatus(text)
    }

    function requestCommand(command, groupCommand, waypoints,
                            actionLabel, detailText, startAfterUpload) {
        pendingCommand = command
        pendingGroupCommand = groupCommand
        pendingWaypoints = waypoints || []
        pendingActionLabel = actionLabel || ""
        pendingDetailText = detailText || ""
        pendingStartAfterUpload = Boolean(startAfterUpload)
        confirmDialog.open()
    }

    function requestStartTask(waypoints) {
        const entry = selectedDroneEntry()
        if (!entry || !entry.connected) {
            setUserStatus(qsTr("无法开始：请选择在线无人机"))
            return
        }
        if (!entry.vehicle.status.isHomePositionOk) {
            setUserStatus(qsTr("无法开始：无人机尚未确认家点"))
            return
        }
        requestCommand(
            DroneControl.uploadMissionCommand,
            false,
            waypoints,
            qsTr("开始任务"),
            qsTr("系统会先把当前任务路线保存到无人机，然后开始执行。"),
            true)
    }

    Component.onCompleted: selectFirstDroneIfNeeded()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 62
            color: "#ffffff"
            border.color: "#d9e0e8"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 18
                anchors.rightMargin: 12
                spacing: 12

                Rectangle {
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    radius: 9
                    color: "#1769e0"
                    Label {
                        anchors.centerIn: parent
                        text: qsTr("航")
                        color: "white"
                        font.bold: true
                    }
                }

                Label {
                    text: qsTr("MiniGCS")
                    color: "#17202a"
                    font.pixelSize: 17
                    font.bold: true
                }

                Button {
                    text: qsTr("任务规划")
                    highlighted: true
                }
                Button {
                    text: qsTr("飞行记录")
                    flat: true
                    onClicked: flightRecordDialog.open()
                }
                Button {
                    text: qsTr("设备")
                    flat: true
                    onClicked: {
                        deviceManagementPanel.selectedDroneId =
                                root.selectedDroneId
                        deviceDialog.open()
                    }
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: DroneControl.drones.length > 0
                          ? qsTr("●  %1 架在线").arg(
                                DroneControl.drones.length)
                          : qsTr("●  等待设备")
                    color: DroneControl.drones.length > 0
                           ? "#16815b" : "#b54708"
                }

                Button {
                    text: qsTr("⚙")
                    flat: true
                    onClicked: settingsDialog.open()
                }
            }
        }

        SplitView {
            id: horizontalSplit
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            handle: Rectangle {
                implicitWidth: 4
                implicitHeight: 4
                color: SplitHandle.pressed ? "#1769e0"
                      : SplitHandle.hovered ? "#9bc0f6" : "#d9e0e8"
            }

            RouteEditor {
                id: routeEditor
                SplitView.preferredWidth: 330
                SplitView.minimumWidth: 285
                SplitView.maximumWidth: 440
            }

            Rectangle {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 360
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
                        selectedDroneId: root.selectedDroneId
                        waypointModel: routeEditor.waypointModel
                        selectedWaypointIndex: routeEditor.selectedWaypointIndex
                        routeCoordinates: routeEditor.routeCoordinates
                        routeEditing: routeEditor.editing

                        onDroneSelected: function(systemId) {
                            root.selectedDroneId = systemId
                            root.centerOnDrone(systemId)
                        }
                        onRouteCoordinateRequested: function(coordinate) {
                            routeEditor.addCoordinate(coordinate)
                        }
                        onWaypointSelected: function(index) {
                            routeEditor.selectedWaypointIndex = index
                        }
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.margins: 14
                    width: mapHint.implicitWidth + 22
                    height: mapHint.implicitHeight + 14
                    radius: 8
                    color: "#eaf2ff"
                    border.color: "#9bc0f6"
                    visible: routeEditor.editing

                    Label {
                        id: mapHint
                        anchors.centerIn: parent
                        text: qsTr("点击地图添加任务点")
                        color: "#175bb7"
                    }
                }

                Row {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: 14
                    spacing: 8

                    Button {
                        text: qsTr("⌖  定位无人机")
                        onClicked: root.centerOnDrone(root.selectedDroneId)
                    }
                    Button {
                        text: qsTr("▱  地图")
                        onClicked: settingsDialog.open()
                    }
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 14
                    width: 250
                    implicitHeight: homeCardColumn.implicitHeight + 24
                    radius: 10
                    color: "#f8ffffff"
                    border.color: "#d9e0e8"

                    ColumnLayout {
                        id: homeCardColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 5
                        Label {
                            text: qsTr("⌂  家点：起飞位置")
                            color: "#17202a"
                            font.bold: true
                        }
                        Label {
                            Layout.fillWidth: true
                            text: qsTr("任务结束或电量不足时回到这里")
                            color: "#667085"
                            wrapMode: Text.Wrap
                        }
                        Button {
                            text: qsTr("在地图上重新设置")
                            flat: true
                            onClicked: root.setUserStatus(
                                qsTr("家点由无人机在起飞时确认；请在高级控制中完成飞控家点设置。"))
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

        }

        MissionStatusPanel {
            id: missionStatus
            Layout.fillWidth: true
            Layout.preferredHeight: 310
            selectedDroneId: root.selectedDroneId
            plannedPointCount: routeEditor.waypointModel.count
            currentTargetName: routeEditor.currentTaskName

            onPauseRequested: root.requestCommand(
                DroneControl.pauseMissionCommand,
                false, [], qsTr("暂停任务"),
                qsTr("无人机将在当前位置暂停任务并保持等待。"), false)
            onReturnHomeRequested: root.requestCommand(
                DroneControl.returnToLaunchCommand,
                false, [], qsTr("立即返回家点"),
                qsTr("无人机将中止当前任务并返回已确认的家点。"), false)
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 66
            color: "#ffffff"
            border.color: "#d9e0e8"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        text: routeEditor.waypointModel.count > 0
                              ? qsTr("任务路线已就绪")
                              : qsTr("等待添加任务点")
                        color: "#17202a"
                        font.bold: true
                    }
                    Label {
                        text: routeEditor.waypointModel.count > 0
                              ? qsTr("%1 个任务点 · %2")
                                  .arg(routeEditor.waypointModel.count)
                                  .arg(routeEditor.returnHomeAfterMission
                                       ? qsTr("结束后返回家点")
                                       : qsTr("结束后留在最后任务点"))
                              : qsTr("在左侧打开添加模式，然后点击地图")
                        color: "#667085"
                    }
                }

                Button {
                    text: qsTr("预览路线")
                    enabled: routeEditor.waypointModel.count > 0
                             && root.selectedDroneId >= 0
                    onClicked: {
                        if (mapLoader.item)
                            mapLoader.item.previewRoute()
                    }
                }
                Button {
                    text: qsTr("开始任务")
                    highlighted: true
                    enabled: routeEditor.waypointModel.count > 0
                             && root.selectedDroneId >= 0
                             && root.selectedDroneEntry()
                             && root.selectedDroneEntry().connected
                    onClicked: root.requestStartTask(
                        routeEditor.missionWaypoints())
                }
            }
        }

    }

    Dialog {
        id: flightRecordDialog
        modal: true
        width: Math.min(root.width - 60, 1180)
        height: Math.min(root.height - 60, 760)
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        padding: 0
        title: qsTr("飞行记录")
        standardButtons: Dialog.Close

        FlightRecordPage {
            anchors.fill: parent
        }
    }

    Dialog {
        id: deviceDialog
        modal: true
        width: Math.min(root.width - 80, 920)
        height: Math.min(root.height - 80, 690)
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        padding: 0
        title: qsTr("无人机管理")
        standardButtons: Dialog.Close

        DroneControlPanel {
            id: deviceManagementPanel
            anchors.fill: parent
            managementMode: true
            selectedDroneId: root.selectedDroneId
            statusText: qsTr("可查看无人机、修改名称并管理编组")

            onSelectedDroneIdChanged: {
                if (selectedDroneId >= 0)
                    root.selectedDroneId = selectedDroneId
            }
            onSelectedGroupNameChanged:
                root.selectedGroupName = selectedGroupName
            onStatusRequested: {
                advancedControlPanel.selectedDroneId = root.selectedDroneId
                advancedTabs.currentIndex = 1
                deviceDialog.close()
                advancedDialog.open()
            }
            onLocateRequested: function(systemId) {
                root.centerOnDrone(systemId)
                deviceDialog.close()
            }
        }
    }

    Dialog {
        id: advancedDialog
        modal: true
        width: Math.min(root.width - 60, 1120)
        height: Math.min(root.height - 60, 780)
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        padding: 0
        title: qsTr("设备与高级控制")
        standardButtons: Dialog.Close

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            TabBar {
                id: advancedTabs
                Layout.fillWidth: true
                TabButton { text: qsTr("高级控制") }
                TabButton { text: qsTr("详细状态") }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: advancedTabs.currentIndex

                DroneControlPanel {
                    id: advancedControlPanel
                    selectedDroneId: root.selectedDroneId

                    onSelectedDroneIdChanged: {
                        if (selectedDroneId >= 0)
                            root.selectedDroneId = selectedDroneId
                    }
                    onSelectedGroupNameChanged:
                        root.selectedGroupName = selectedGroupName
                    onCommandRequested: function(command, groupCommand) {
                        root.requestCommand(
                            command, groupCommand, [],
                            DroneControl.commandName(command), "", false)
                    }
                    onStatusRequested: advancedTabs.currentIndex = 1
                    onLocateRequested: function(systemId) {
                        root.centerOnDrone(systemId)
                    }
                }

                DroneStatusPage {
                    vehicle: {
                        const entry = root.selectedDroneEntry()
                        return entry ? entry.vehicle : null
                    }
                    droneName: root.selectedDroneName()
                    waypointModel: routeEditor.waypointModel
                    onCloseRequested: advancedDialog.close()
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
        title: qsTr("设置")
        standardButtons: Dialog.Close

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            TabBar {
                id: settingsTabs
                Layout.fillWidth: true
                TabButton { text: qsTr("连接") }
                TabButton { text: qsTr("地图与任务") }
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
        commandLabel: root.pendingActionLabel.length > 0
                      ? root.pendingActionLabel
                      : DroneControl.commandName(root.pendingCommand)
        detailText: root.pendingDetailText
        targetName: root.pendingGroupCommand
                    ? root.selectedGroupName : root.selectedDroneName()
        groupCommand: root.pendingGroupCommand

        onRejected: root.pendingStartAfterUpload = false

        onCommandConfirmed: {
            if (root.pendingCommand === DroneControl.uploadMissionCommand) {
                if (root.pendingGroupCommand) {
                    DroneControl.uploadMissionGroup(
                                root.selectedGroupName,
                                root.pendingWaypoints,
                                routeEditor.returnHomeAfterMission)
                } else {
                    DroneControl.uploadMissionSingle(
                                root.selectedDroneId,
                                root.pendingWaypoints,
                                routeEditor.returnHomeAfterMission)
                }
            } else if (root.pendingGroupCommand) {
                DroneControl.executeGroup(
                            root.selectedGroupName,
                            root.pendingCommand)
            } else {
                DroneControl.executeSingle(
                            root.selectedDroneId,
                            root.pendingCommand)
            }
        }
    }

    Connections {
        target: DroneControl

        function onDronesChanged() {
            root.selectFirstDroneIfNeeded()
        }

        function onCommandDispatched(command, target, count) {
            root.setUserStatus(
                qsTr("已让“%1”的 %2 架在线无人机执行“%3”")
                    .arg(target).arg(count)
                    .arg(DroneControl.commandName(command)))
        }

        function onCommandRejected(reason) {
            root.pendingStartAfterUpload = false
            root.setUserStatus(qsTr("操作未执行：%1").arg(reason))
        }

        function onCommandResult(systemId, command, success, reason) {
            root.setUserStatus(success
                    ? qsTr("无人机 %1 已完成“%2”")
                        .arg(systemId).arg(DroneControl.commandName(command))
                    : qsTr("无人机 %1 执行“%2”失败：%3")
                        .arg(systemId)
                        .arg(DroneControl.commandName(command))
                        .arg(reason))
        }

        function onMissionDownloaded(systemId, waypoints) {
            if (Number(systemId) === Number(root.selectedDroneId)) {
                routeEditor.loadMissionWaypoints(waypoints)
                root.setUserStatus(
                    qsTr("已读取 %1 个任务点").arg(waypoints.length))
            }
        }

        function onMissionUploadResult(systemId, success, reason) {
            const shouldStart = root.pendingStartAfterUpload
                    && Number(systemId) === Number(root.selectedDroneId)
            root.pendingStartAfterUpload = false
            if (!success) {
                root.setUserStatus(
                    qsTr("任务保存失败：%1").arg(reason))
                return
            }
            if (shouldStart) {
                root.setUserStatus(qsTr("任务已保存，正在开始执行…"))
                DroneControl.executeSingle(
                            systemId, DroneControl.startMissionCommand)
            } else {
                root.setUserStatus(qsTr("任务已保存到无人机 %1").arg(systemId))
            }
        }
    }
}
