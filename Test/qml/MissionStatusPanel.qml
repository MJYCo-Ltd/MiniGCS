import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtPositioning
import MiniGCS

Rectangle {
    id: root

    property int selectedDroneId: -1
    property var selectedEntry: findDrone(selectedDroneId, DroneControl.drones)
    property var vehicle: selectedEntry ? selectedEntry.vehicle : null
    property int plannedPointCount: 0
    property string currentTargetName: qsTr("等待任务开始")

    signal pauseRequested()
    signal returnHomeRequested()

    function findDrone(systemId, drones) {
        for (let index = 0; index < drones.length; ++index) {
            if (Number(drones[index].systemId) === Number(systemId))
                return drones[index]
        }
        return null
    }

    function numberText(value, decimals, suffix) {
        const number = Number(value)
        return isFinite(number) ? number.toFixed(decimals) + suffix : qsTr("--")
    }

    function batteryText() {
        if (!vehicle)
            return qsTr("--")
        const remaining = Number(vehicle.status.batteryRemaining)
        if (!isFinite(remaining) || remaining < 0)
            return qsTr("--")
        const seconds = Number(vehicle.status.batteryTimeRemainingS)
        if (!isFinite(seconds) || seconds <= 0)
            return qsTr("%1%").arg(remaining.toFixed(0))
        return qsTr("%1% · 约 %2 分钟")
                .arg(remaining.toFixed(0))
                .arg(Math.max(1, Math.round(seconds / 60)))
    }

    function flightStateText() {
        if (!vehicle)
            return qsTr("未选择")
        if (!selectedEntry.connected)
            return qsTr("离线")
        if (vehicle.inAir)
            return vehicle.moving ? qsTr("飞行中") : qsTr("空中等待")
        return qsTr("地面待命")
    }

    function distanceHomeText() {
        if (!vehicle || !vehicle.hasGpsPosition ||
            !vehicle.status.isHomePositionOk)
            return qsTr("--")
        const current = QtPositioning.coordinate(
            vehicle.gpsPosition.latitude, vehicle.gpsPosition.longitude)
        const home = QtPositioning.coordinate(
            vehicle.homePosition.latitude, vehicle.homePosition.longitude)
        const distance = current.distanceTo(home)
        return isFinite(distance) ? qsTr("%1 米").arg(distance.toFixed(0))
                                  : qsTr("--")
    }

    function missionTotalCount() {
        if (!vehicle)
            return plannedPointCount
        const total = Number(vehicle.missionTotal)
        return isFinite(total) && total > 0 ? total : plannedPointCount
    }

    function missionDisplayCurrent() {
        if (!vehicle)
            return 0
        const total = missionTotalCount()
        const current = Number(vehicle.missionCurrent)
        if (!isFinite(current) || total <= 0)
            return 0
        const normalized = Math.max(0, Math.min(total, current))
        return vehicle.missionActive && normalized < total
                ? normalized + 1 : normalized
    }

    function missionTargetText() {
        const current = missionDisplayCurrent()
        return vehicle && vehicle.missionActive && current > 0
                ? qsTr("任务点 %1").arg(current) : currentTargetName
    }

    color: "#ffffff"
    border.color: "#d9e0e8"
    implicitHeight: 310

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        text: root.selectedEntry
                              ? root.selectedEntry.name : qsTr("请选择无人机")
                        color: "#17202a"
                        font.bold: true
                    }
                    Label {
                        text: qsTr("正在前往：%1").arg(
                                  root.missionTargetText())
                        color: "#344054"
                    }
                }
                Label {
                    text: "●  " + root.flightStateText()
                    color: root.selectedEntry && root.selectedEntry.connected
                           ? "#16815b" : "#b42318"
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Label { text: qsTr("任务进度"); color: "#344054"; Layout.fillWidth: true }
                Label {
                    text: qsTr("%1 / %2")
                          .arg(root.missionDisplayCurrent())
                          .arg(root.missionTotalCount())
                    color: "#17202a"
                }
            }
            ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: Math.max(1, root.missionTotalCount())
                value: root.missionDisplayCurrent()
            }

            Repeater {
                model: [
                    { "label": qsTr("电量"), "value": root.batteryText(), "green": true },
                    { "label": qsTr("距离家点"), "value": root.distanceHomeText(), "green": false },
                    { "label": qsTr("当前高度"), "value": root.vehicle ? root.numberText(root.vehicle.relativeAltitudeM, 0, qsTr(" 米")) : qsTr("--"), "green": false },
                    { "label": qsTr("预计完成"), "value": root.plannedPointCount > 0 ? qsTr("还需 %1 个任务点").arg(Math.max(0, root.plannedPointCount - (root.vehicle ? root.vehicle.missionCurrent : 0))) : qsTr("--"), "green": false }
                ]

                delegate: ColumnLayout {
                    id: statRow
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 6
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: statRow.modelData.label; color: "#667085"; Layout.fillWidth: true }
                        Label {
                            text: statRow.modelData.value
                            color: statRow.modelData.green ? "#16815b" : "#17202a"
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#e1e7ee"
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: safetyLabel.implicitHeight + 20
                radius: 8
                color: "#fff4df"
                Label {
                    id: safetyLabel
                    anchors.fill: parent
                    anchors.margins: 10
                    text: root.vehicle && root.vehicle.status.isHomePositionOk
                          ? qsTr("安全状态正常：家点已确认，低电时可返回家点")
                          : qsTr("无法安全执行：等待无人机确认家点")
                    color: "#a85b00"
                    wrapMode: Text.Wrap
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            spacing: 8

            Button {
                text: qsTr("暂停任务")
                Layout.fillWidth: true
                Layout.fillHeight: true
                enabled: root.vehicle && root.vehicle.inAir
                onClicked: root.pauseRequested()
            }
            Button {
                text: qsTr("立即返回家点")
                Layout.fillWidth: true
                Layout.fillHeight: true
                enabled: root.selectedEntry && root.selectedEntry.connected
                onClicked: root.returnHomeRequested()

                background: Rectangle {
                    radius: 8
                    color: "transparent"
                    border.width: 1
                    border.color: "#e53935"
                }
                contentItem: Label {
                    text: qsTr("立即返回家点")
                    color: "#d92d20"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
