import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Effects
import QtQuick.Layouts
import QtLocation
import QtPositioning
import MiniGCS

Window {
    id: root
    width: 1180
    height: 720
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("MiniGCS 无人机控制")

    property int selectedDroneId: -1
    property string selectedGroupName: ""
    property string pendingCommand: ""
    property string pendingTarget: ""
    property bool pendingGroupCommand: false
    property bool mapCenteredOnDrone: false

    function autopilotName(type) {
        if (type === 1)
            return qsTr("PX4")
        if (type === 2)
            return qsTr("ArduPilot")
        return qsTr("未知飞控")
    }

    function vehicleIcon(type) {
        if (type === AutoVehicleType.FixedWing)
            return "images/fixed-wing.svg"
        if (type === AutoVehicleType.Quadrotor
                || type === AutoVehicleType.Coaxial
                || type === AutoVehicleType.Hexarotor
                || type === AutoVehicleType.Octorotor
                || type === AutoVehicleType.Tricopter
                || type === AutoVehicleType.Dodecarotor
                || type === AutoVehicleType.Decarotor
                || type === AutoVehicleType.GenericMultirotor)
            return "images/multirotor.svg"
        if (type === AutoVehicleType.Helicopter)
            return "images/helicopter.svg"
        if (type === AutoVehicleType.VtolTailsitterDuorotor
                || type === AutoVehicleType.VtolTailsitterQuadrotor
                || type === AutoVehicleType.VtolTiltrotor
                || type === AutoVehicleType.VtolFixedrotor
                || type === AutoVehicleType.VtolTailsitter
                || type === AutoVehicleType.VtolTiltwing)
            return "images/vtol.svg"
        if (type === AutoVehicleType.GroundRover)
            return "images/rover.svg"
        if (type === AutoVehicleType.SurfaceBoat)
            return "images/boat.svg"
        if (type === AutoVehicleType.Submarine)
            return "images/submarine.svg"
        if (type === AutoVehicleType.Rocket)
            return "images/rocket.svg"
        return "images/unknown.svg"
    }

    function commandName(command) {
        if (command === "arm")
            return qsTr("解锁")
        if (command === "disarm")
            return qsTr("上锁")
        if (command === "takeoff")
            return qsTr("起飞")
        if (command === "land")
            return qsTr("降落")
        if (command === "returnToLaunch")
            return qsTr("返航")
        if (command === "downloadMission")
            return qsTr("下载任务")
        return command
    }

    function requestCommand(command, groupCommand) {
        pendingCommand = command
        pendingGroupCommand = groupCommand
        pendingTarget = groupCommand
                ? selectedGroupName
                : qsTr("系统 ID %1").arg(selectedDroneId)
        confirmDialog.open()
    }

    function memberSelected(systemId) {
        const configuredGroups = DroneControl.groups
        for (let groupIndex = 0;
             groupIndex < configuredGroups.length; ++groupIndex) {
            const group = configuredGroups[groupIndex]
            if (group.name !== selectedGroupName)
                continue
            const members = group.members
            for (let index = 0; index < members.length; ++index) {
                if (Number(members[index]) === Number(systemId))
                    return true
            }
        }
        return false
    }

    function updateMember(systemId, selected) {
        const current = DroneControl.groupMembers(selectedGroupName)
        const next = []
        for (let index = 0; index < current.length; ++index) {
            if (Number(current[index]) !== Number(systemId))
                next.push(Number(current[index]))
        }
        if (selected)
            next.push(Number(systemId))
        DroneControl.setGroupMembers(selectedGroupName, next)
    }

    MapView {
        id: mapView
        anchors.fill: parent
        anchors.rightMargin: controlPanel.width

        map.plugin: Plugin {
            name: "QGroundControl"
        }
        map.center: QtPositioning.coordinate(38.045474, 114.502461)
        map.zoomLevel: 10
        map.minimumZoomLevel: 3
        map.maximumZoomLevel: 18

        MapItemView {
            parent: mapView.map
            model: DroneControl.drones

            delegate: MapQuickItem {
                id: droneMarker
                required property var modelData
                property var vehicle: modelData.vehicle
                property bool coordinateInitialized: false
                property var displayedCoordinate: QtPositioning.coordinate()
                property bool positionAvailable: vehicle
                    && vehicle.hasGpsPosition
                    && isFinite(vehicle.gpsPosition.latitude)
                    && isFinite(vehicle.gpsPosition.longitude)
                    && vehicle.gpsPosition.latitude >= -90
                    && vehicle.gpsPosition.latitude <= 90
                    && vehicle.gpsPosition.longitude >= -180
                    && vehicle.gpsPosition.longitude <= 180

                function acceptCurrentPosition(forceUpdate) {
                    if (!positionAvailable)
                        return

                    if (!coordinateInitialized || forceUpdate
                            || vehicle.moving) {
                        displayedCoordinate = QtPositioning.coordinate(
                            vehicle.gpsPosition.latitude,
                            vehicle.gpsPosition.longitude,
                            vehicle.gpsPosition.altitude)
                        coordinateInitialized = true
                    }
                }

                coordinate: displayedCoordinate
                visible: positionAvailable
                anchorPoint: Qt.point(sourceItem.width / 2,
                                      sourceItem.height - 2)
                autoFadeIn: false
                z: root.selectedDroneId === modelData.systemId ? 2 : 1

                onCoordinateChanged: {
                    if (positionAvailable && !root.mapCenteredOnDrone) {
                        mapView.map.center = coordinate
                        mapView.map.zoomLevel = 16
                        root.mapCenteredOnDrone = true
                    }
                }

                Component.onCompleted: acceptCurrentPosition(true)

                Connections {
                    target: droneMarker.vehicle

                    function onGpsPositionChanged() {
                        droneMarker.acceptCurrentPosition(false)
                    }

                    function onMotionChanged() {
                        if (droneMarker.vehicle.moving)
                            droneMarker.acceptCurrentPosition(true)
                    }
                }

                sourceItem: Item {
                    width: 110
                    height: 82

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 23
                        width: 38
                        height: 38
                        source: root.vehicleIcon(
                                    droneMarker.vehicle.vehicleType)
                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                        rotation: droneMarker.vehicle.heading
                        transformOrigin: Item.Center
                        scale: root.selectedDroneId === modelData.systemId
                               ? 1.12 : 1.0
                        layer.enabled: root.selectedDroneId
                                       === modelData.systemId
                        layer.effect: MultiEffect {
                            colorization: 1.0
                            colorizationColor: "#dc2626"
                        }
                    }

                    Rectangle {
                        id: droneNameTag
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 0
                        width: Math.min(parent.width,
                                        droneNameLabel.implicitWidth + 10)
                        height: droneNameLabel.implicitHeight + 4
                        radius: 3
                        color: root.selectedDroneId === modelData.systemId
                               ? "#e6b91c1c" : "#cc101828"

                        Label {
                            id: droneNameLabel
                            anchors.centerIn: parent
                            width: droneNameTag.width - 8
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 61
                        text: "\u25bc"
                        color: root.selectedDroneId === modelData.systemId
                               ? "#dc2626" : "#1f2937"
                        font.pixelSize: 18
                        style: Text.Outline
                        styleColor: "white"
                    }

                    TapHandler {
                        onTapped: root.selectedDroneId = modelData.systemId
                    }
                }
            }
        }
    }

    Rectangle {
        id: controlPanel
        width: 410
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
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

                ColumnLayout {
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
                                    onClicked: root.selectedDroneId =
                                                   modelData.systemId
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 4

                                    RowLayout {
                                        Layout.fillWidth: true
                                        RadioButton {
                                            checked: root.selectedDroneId
                                                     === modelData.systemId
                                            onClicked: root.selectedDroneId =
                                                           modelData.systemId
                                        }
                                        Label {
                                            text: modelData.name
                                            font.bold: true
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            text: modelData.vehicle.moving
                                                  ? qsTr("移动 %1 m/s").arg(
                                                      modelData.vehicle
                                                      .groundSpeedMS
                                                      .toFixed(1))
                                                  : qsTr("静止")
                                            color: modelData.vehicle.moving
                                                   ? "#b42318" : "#667085"
                                            font.pixelSize: 11
                                        }
                                        Label {
                                            text: modelData.connected
                                                  ? qsTr("在线") : qsTr("离线")
                                            color: modelData.connected
                                                   ? "#15803d" : "#b42318"
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label {
                                            text: qsTr("ID %1")
                                                .arg(modelData.systemId)
                                            color: "#667085"
                                        }
                                        Label {
                                            text: root.autopilotName(
                                                      modelData.vehicle
                                                      .autopilotType)
                                            color: "#667085"
                                        }
                                        Basic.TextField {
                                            id: droneNameEditor
                                            Layout.fillWidth: true
                                            Layout.minimumHeight: implicitHeight
                                            text: modelData.name
                                            placeholderText: qsTr("无人机别名")
                                            selectByMouse: true
                                        }
                                        Button {
                                            text: qsTr("保存")
                                            onClicked: DroneControl.renameDrone(
                                                modelData.systemId,
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
                            text: qsTr("解锁")
                            Layout.fillWidth: true
                            enabled: root.selectedDroneId >= 0
                            onClicked: root.requestCommand("arm", false)
                        }
                        Button {
                            text: qsTr("上锁")
                            Layout.fillWidth: true
                            enabled: root.selectedDroneId >= 0
                            onClicked: root.requestCommand("disarm", false)
                        }
                        Button {
                            text: qsTr("起飞")
                            Layout.fillWidth: true
                            highlighted: true
                            enabled: root.selectedDroneId >= 0
                            onClicked: root.requestCommand("takeoff", false)
                        }
                        Button {
                            text: qsTr("降落")
                            Layout.fillWidth: true
                            enabled: root.selectedDroneId >= 0
                            onClicked: root.requestCommand("land", false)
                        }
                        Button {
                            text: qsTr("返航")
                            Layout.fillWidth: true
                            enabled: root.selectedDroneId >= 0
                            onClicked: root.requestCommand(
                                           "returnToLaunch", false)
                        }
                        Button {
                            text: qsTr("下载任务")
                            Layout.fillWidth: true
                            enabled: root.selectedDroneId >= 0
                            onClicked: root.requestCommand(
                                           "downloadMission", false)
                        }
                    }
                }

                ColumnLayout {
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Basic.TextField {
                            id: newGroupName
                            Layout.fillWidth: true
                            Layout.minimumHeight: implicitHeight
                            placeholderText: qsTr("新编组名称")
                            selectByMouse: true
                        }
                        Button {
                            text: qsTr("新建")
                            onClicked: {
                                if (DroneControl.addGroup(newGroupName.text)) {
                                    root.selectedGroupName =
                                            newGroupName.text.trim()
                                    newGroupName.clear()
                                }
                            }
                        }
                    }

                    ComboBox {
                        id: groupSelector
                        Layout.fillWidth: true
                        textRole: "name"
                        valueRole: "name"
                        model: DroneControl.groups
                        onCurrentValueChanged: {
                            root.selectedGroupName = currentValue || ""
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: root.selectedGroupName.length > 0
                                  ? qsTr("配置“%1”成员")
                                        .arg(root.selectedGroupName)
                                  : qsTr("请先创建编组")
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        Button {
                            text: qsTr("删除编组")
                            enabled: root.selectedGroupName.length > 0
                            onClicked: {
                                DroneControl.removeGroup(
                                            root.selectedGroupName)
                                root.selectedGroupName = ""
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "white"
                        radius: 6
                        border.color: "#d9dee7"

                        ListView {
                            id: memberList
                            anchors.fill: parent
                            anchors.margins: 6
                            clip: true
                            model: DroneControl.drones

                            delegate: CheckDelegate {
                                required property var modelData
                                width: ListView.view.width
                                text: "%1  (ID %2)  ·  %3"
                                    .arg(modelData.name)
                                    .arg(modelData.systemId)
                                    .arg(modelData.connected
                                         ? qsTr("在线") : qsTr("离线"))
                                checked: root.memberSelected(
                                             modelData.systemId)
                                enabled: root.selectedGroupName.length > 0
                                onToggled: root.updateMember(
                                               modelData.systemId, checked)
                            }
                        }
                    }

                    GridLayout {
                        columns: 2
                        Layout.fillWidth: true
                        Button {
                            text: qsTr("编组解锁")
                            Layout.fillWidth: true
                            enabled: root.selectedGroupName.length > 0
                            onClicked: root.requestCommand("arm", true)
                        }
                        Button {
                            text: qsTr("编组上锁")
                            Layout.fillWidth: true
                            enabled: root.selectedGroupName.length > 0
                            onClicked: root.requestCommand("disarm", true)
                        }
                        Button {
                            text: qsTr("编组起飞")
                            Layout.fillWidth: true
                            highlighted: true
                            enabled: root.selectedGroupName.length > 0
                            onClicked: root.requestCommand("takeoff", true)
                        }
                        Button {
                            text: qsTr("编组降落")
                            Layout.fillWidth: true
                            enabled: root.selectedGroupName.length > 0
                            onClicked: root.requestCommand("land", true)
                        }
                        Button {
                            text: qsTr("编组返航")
                            Layout.fillWidth: true
                            enabled: root.selectedGroupName.length > 0
                            onClicked: root.requestCommand(
                                           "returnToLaunch", true)
                        }
                        Button {
                            text: qsTr("编组下载任务")
                            Layout.fillWidth: true
                            enabled: root.selectedGroupName.length > 0
                            onClicked: root.requestCommand(
                                           "downloadMission", true)
                        }
                    }
                }
            }

            Label {
                id: statusLabel
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: "#344054"
                text: qsTr("控制命令执行前会进行确认")
            }
        }
    }

    Dialog {
        id: confirmDialog
        width: 380
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        modal: true
        title: qsTr("确认控制命令")
        standardButtons: Dialog.Yes | Dialog.No

        Label {
            width: confirmDialog.availableWidth
            wrapMode: Text.Wrap
            text: (root.pendingGroupCommand
                   ? qsTr("编组命令会同时发送给全部在线成员。\n")
                   : "")
                  + qsTr("确定向“%1”发送“%2”命令吗？")
                .arg(root.pendingTarget)
                .arg(root.commandName(root.pendingCommand))
        }

        onAccepted: {
            if (root.pendingGroupCommand) {
                DroneControl.executeGroup(root.selectedGroupName,
                                          root.pendingCommand)
            } else {
                DroneControl.executeSingle(root.selectedDroneId,
                                           root.pendingCommand)
            }
        }
    }

    Connections {
        target: DroneControl

        function onCommandDispatched(command, target, count) {
            statusLabel.text = qsTr("已向“%1”的 %2 架在线无人机发送“%3”")
                .arg(target).arg(count).arg(root.commandName(command))
        }

        function onCommandRejected(reason) {
            statusLabel.text = qsTr("命令未发送：%1").arg(reason)
        }
    }
}
