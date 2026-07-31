import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var vehicle: null
    property var waypointModel: null
    property string droneName: ""
    signal closeRequested()

    color: "#f4f7fb"

    function numberText(value, decimals, suffix) {
        const number = Number(value)
        return isFinite(number)
                ? number.toFixed(decimals) + (suffix || "") : qsTr("--")
    }

    function stateText(value) {
        return value ? qsTr("是") : qsTr("否")
    }

    function dateText(value) {
        if (!value || !value.toLocaleString)
            return qsTr("--")
        const text = value.toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm:ss")
        return text.length > 0 ? text : qsTr("--")
    }

    component Metric: ColumnLayout {
        id: metric
        property string label: ""
        property string value: "--"
        property color valueColor: "#111827"

        width: 168
        spacing: 3

        Label {
            Layout.fillWidth: true
            text: metric.label
            color: "#667085"
            font.pixelSize: 12
            elide: Text.ElideRight
        }
        Label {
            Layout.fillWidth: true
            text: metric.value
            color: metric.valueColor
            font.pixelSize: 15
            font.bold: true
            wrapMode: Text.Wrap
        }
    }

    component InfoSection: Rectangle {
        id: infoSection
        property string title: ""
        default property alias metrics: metricFlow.data

        radius: 10
        color: "white"
        border.color: "#e4e7ec"
        implicitHeight: sectionColumn.implicitHeight + 28

        ColumnLayout {
            id: sectionColumn
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12

            Label {
                text: infoSection.title
                color: "#1d2939"
                font.pixelSize: 17
                font.bold: true
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#eef0f3"
            }
            Flow {
                id: metricFlow
                Layout.fillWidth: true
                spacing: 12
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            color: "#172b4d"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 22
                anchors.rightMargin: 16
                spacing: 14

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3
                    Label {
                        text: root.droneName.length > 0
                              ? root.droneName : qsTr("无人机状态")
                        color: "white"
                        font.pixelSize: 23
                        font.bold: true
                    }
                    Label {
                        text: root.vehicle
                              ? qsTr("系统 ID %1 · %2")
                                  .arg(root.vehicle.systemId)
                                  .arg(root.vehicle.connected
                                       ? qsTr("在线") : qsTr("离线"))
                              : qsTr("请选择无人机")
                        color: "#c7d7ef"
                    }
                }
                Button {
                    text: qsTr("关闭")
                    onClicked: root.closeRequested()
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            Flow {
                id: cards
                width: parent.width
                padding: 16
                spacing: 14

                InfoSection {
                    title: qsTr("设备概览")
                    width: cards.width > 960
                           ? (cards.width - cards.leftPadding
                              - cards.rightPadding - cards.spacing) / 2
                           : cards.width - cards.leftPadding - cards.rightPadding

                    Metric { label: qsTr("载具类型"); value: root.vehicle ? root.vehicle.vehicleName : "--" }
                    Metric { label: qsTr("自动驾驶仪"); value: root.vehicle ? root.vehicle.autopilotName : "--" }
                    Metric { label: qsTr("固件版本"); value: root.vehicle && root.vehicle.firmwareVersion.length > 0 ? root.vehicle.firmwareVersion : "--" }
                    Metric { label: qsTr("软件版本"); value: root.vehicle && root.vehicle.softwareVersion.length > 0 ? root.vehicle.softwareVersion : "--" }
                    Metric { label: qsTr("连接状态"); value: root.vehicle ? (root.vehicle.connected ? qsTr("在线") : qsTr("离线")) : "--"; valueColor: root.vehicle && root.vehicle.connected ? "#027a48" : "#b42318" }
                    Metric { label: qsTr("最后连接"); value: root.vehicle ? root.dateText(root.vehicle.lastConnectedTime) : "--" }
                    Metric { label: qsTr("最后断开"); value: root.vehicle ? root.dateText(root.vehicle.lastDisconnectedTime) : "--" }
                }

                InfoSection {
                    title: qsTr("飞行状态与姿态")
                    width: cards.width > 960
                           ? (cards.width - cards.leftPadding
                              - cards.rightPadding - cards.spacing) / 2
                           : cards.width - cards.leftPadding - cards.rightPadding

                    Metric { label: qsTr("解锁"); value: root.vehicle ? root.stateText(root.vehicle.armed) : "--"; valueColor: root.vehicle && root.vehicle.armed ? "#b54708" : "#027a48" }
                    Metric { label: qsTr("空中"); value: root.vehicle ? root.stateText(root.vehicle.inAir) : "--" }
                    Metric { label: qsTr("运动中"); value: root.vehicle ? root.stateText(root.vehicle.moving) : "--" }
                    Metric { label: qsTr("飞行模式"); value: root.vehicle ? root.vehicle.flightModeName : "--" }
                    Metric { label: qsTr("着陆状态"); value: root.vehicle ? root.vehicle.landedStateName : "--" }
                    Metric { label: qsTr("航向"); value: root.vehicle ? root.numberText(root.vehicle.heading, 1, "°") : "--" }
                    Metric { label: qsTr("横滚"); value: root.vehicle ? root.numberText(root.vehicle.rollDeg, 1, "°") : "--" }
                    Metric { label: qsTr("俯仰"); value: root.vehicle ? root.numberText(root.vehicle.pitchDeg, 1, "°") : "--" }
                    Metric { label: qsTr("偏航"); value: root.vehicle ? root.numberText(root.vehicle.yawDeg, 1, "°") : "--" }
                    Metric { label: qsTr("水平速度"); value: root.vehicle ? root.numberText(root.vehicle.groundSpeedMS, 2, " m/s") : "--" }
                    Metric { label: qsTr("垂直速度"); value: root.vehicle ? root.numberText(root.vehicle.verticalSpeedMS, 2, " m/s") : "--" }
                    Metric { label: qsTr("N/E/D 速度"); value: root.vehicle ? root.numberText(root.vehicle.velocityNorthMS, 1, " / ") + root.numberText(root.vehicle.velocityEastMS, 1, " / ") + root.numberText(root.vehicle.velocityDownMS, 1, " m/s") : "--" }
                }

                InfoSection {
                    title: qsTr("GPS 与位置")
                    width: cards.width > 960
                           ? (cards.width - cards.leftPadding
                              - cards.rightPadding - cards.spacing) / 2
                           : cards.width - cards.leftPadding - cards.rightPadding

                    Metric { label: qsTr("定位方式"); value: root.vehicle ? root.vehicle.status.gpsStatus : "--" }
                    Metric { label: qsTr("卫星数"); value: root.vehicle ? String(root.vehicle.status.gpsCount) : "--" }
                    Metric { label: qsTr("经度"); value: root.vehicle && root.vehicle.hasGpsPosition ? root.numberText(root.vehicle.gpsPosition.longitude, 7, "°") : "--" }
                    Metric { label: qsTr("纬度"); value: root.vehicle && root.vehicle.hasGpsPosition ? root.numberText(root.vehicle.gpsPosition.latitude, 7, "°") : "--" }
                    Metric { label: qsTr("海拔高度"); value: root.vehicle && root.vehicle.hasGpsPosition ? root.numberText(root.vehicle.gpsPosition.altitude, 2, " m") : "--" }
                    Metric { label: qsTr("相对高度"); value: root.vehicle ? root.numberText(root.vehicle.relativeAltitudeM, 2, " m") : "--" }
                    Metric { label: qsTr("HDOP / VDOP"); value: root.vehicle ? root.numberText(root.vehicle.gpsHdop, 2, " / ") + root.numberText(root.vehicle.gpsVdop, 2, "") : "--" }
                    Metric { label: qsTr("GPS 地速 / 航迹角"); value: root.vehicle ? root.numberText(root.vehicle.gpsVelocityMS, 2, " m/s · ") + root.numberText(root.vehicle.gpsCourseDeg, 1, "°") : "--" }
                    Metric { label: qsTr("水平 / 垂直误差"); value: root.vehicle ? root.numberText(root.vehicle.gpsHorizontalUncertaintyM, 2, " / ") + root.numberText(root.vehicle.gpsVerticalUncertaintyM, 2, " m") : "--" }
                    Metric { label: qsTr("速度 / 航向误差"); value: root.vehicle ? root.numberText(root.vehicle.gpsVelocityUncertaintyMS, 2, " m/s · ") + root.numberText(root.vehicle.gpsHeadingUncertaintyDeg, 1, "°") : "--" }
                    Metric { label: qsTr("Home 经度 / 纬度"); value: root.vehicle ? root.numberText(root.vehicle.homePosition.longitude, 6, " / ") + root.numberText(root.vehicle.homePosition.latitude, 6, "°") : "--" }
                    Metric { label: qsTr("Home 高度"); value: root.vehicle ? root.numberText(root.vehicle.homePosition.altitude, 2, " m") : "--" }
                    Metric { label: qsTr("NED 位置"); value: root.vehicle ? root.numberText(root.vehicle.nedPosition.north, 1, " / ") + root.numberText(root.vehicle.nedPosition.east, 1, " / ") + root.numberText(root.vehicle.nedPosition.down, 1, " m") : "--" }
                }

                InfoSection {
                    title: qsTr("电池、健康与遥控")
                    width: cards.width > 960
                           ? (cards.width - cards.leftPadding
                              - cards.rightPadding - cards.spacing) / 2
                           : cards.width - cards.leftPadding - cards.rightPadding

                    Metric { label: qsTr("剩余电量"); value: root.vehicle ? root.numberText(root.vehicle.status.batteryRemaining, 1, "%") : "--" }
                    Metric { label: qsTr("电压 / 电流"); value: root.vehicle ? root.numberText(root.vehicle.status.batteryVoltage, 2, " V · ") + root.numberText(root.vehicle.status.batteryCurrentA, 2, " A") : "--" }
                    Metric { label: qsTr("电池温度"); value: root.vehicle ? root.numberText(root.vehicle.status.batteryTemperatureC, 1, " °C") : "--" }
                    Metric { label: qsTr("已消耗容量"); value: root.vehicle ? root.numberText(root.vehicle.status.batteryConsumedAh, 3, " Ah") : "--" }
                    Metric { label: qsTr("预计剩余时间"); value: root.vehicle ? root.numberText(root.vehicle.status.batteryTimeRemainingS, 0, " s") : "--" }
                    Metric { label: qsTr("电池 ID / 用途"); value: root.vehicle ? String(root.vehicle.status.batteryId) + " · " + root.vehicle.status.batteryFunction : "--" }
                    Metric { label: qsTr("允许解锁"); value: root.vehicle ? root.stateText(root.vehicle.status.isArmable) : "--" }
                    Metric { label: qsTr("陀螺仪校准"); value: root.vehicle ? root.stateText(root.vehicle.status.isGyrometerCalibrationOk) : "--" }
                    Metric { label: qsTr("加速度计校准"); value: root.vehicle ? root.stateText(root.vehicle.status.isAccelerometerCalibrationOk) : "--" }
                    Metric { label: qsTr("磁力计校准"); value: root.vehicle ? root.stateText(root.vehicle.status.isMagnetometerCalibrationOk) : "--" }
                    Metric { label: qsTr("本地 / 全局位置"); value: root.vehicle ? root.stateText(root.vehicle.status.isLocalPositionOk) + " / " + root.stateText(root.vehicle.status.isGlobalPositionOk) : "--" }
                    Metric { label: qsTr("Home 有效"); value: root.vehicle ? root.stateText(root.vehicle.status.isHomePositionOk) : "--" }
                    Metric { label: qsTr("遥控器"); value: root.vehicle ? (root.stateText(root.vehicle.status.rcIsAvailable) + " · " + root.numberText(root.vehicle.status.rcSignalStrengthPercent, 0, "%")) : "--" }
                }

                InfoSection {
                    title: qsTr("固定翼指标")
                    width: cards.width > 960
                           ? (cards.width - cards.leftPadding
                              - cards.rightPadding - cards.spacing) / 2
                           : cards.width - cards.leftPadding - cards.rightPadding

                    Metric { label: qsTr("空速"); value: root.vehicle ? root.numberText(root.vehicle.fixedwing.airspeedMS, 2, " m/s") : "--" }
                    Metric { label: qsTr("地速"); value: root.vehicle ? root.numberText(root.vehicle.fixedwing.groundspeedMS, 2, " m/s") : "--" }
                    Metric { label: qsTr("油门"); value: root.vehicle ? root.numberText(root.vehicle.fixedwing.throttlePercentage, 1, "%") : "--" }
                    Metric { label: qsTr("爬升率"); value: root.vehicle ? root.numberText(root.vehicle.fixedwing.climbRateMS, 2, " m/s") : "--" }
                    Metric { label: qsTr("固定翼航向"); value: root.vehicle ? root.numberText(root.vehicle.fixedwing.headingDeg, 1, "°") : "--" }
                    Metric { label: qsTr("固定翼高度"); value: root.vehicle ? root.numberText(root.vehicle.fixedwing.absoluteAltitudeM, 2, " m") : "--" }
                }

                Rectangle {
                    width: cards.width > 960
                           ? (cards.width - cards.leftPadding
                              - cards.rightPadding - cards.spacing) / 2
                           : cards.width - cards.leftPadding - cards.rightPadding
                    height: Math.max(210, routeColumn.implicitHeight + 28)
                    radius: 10
                    color: "white"
                    border.color: "#e4e7ec"

                    ColumnLayout {
                        id: routeColumn
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("当前航线"); font.pixelSize: 17; font.bold: true; color: "#1d2939" }
                            Item { Layout.fillWidth: true }
                            Label { text: qsTr("%1 个航点").arg(root.waypointModel ? root.waypointModel.count : 0); color: "#475467" }
                        }
                        Label {
                            Layout.fillWidth: true
                            text: root.vehicle && root.vehicle.airLineDownloading
                                  ? qsTr("正在下载航线…")
                                  : root.vehicle && root.vehicle.airLineUploading
                                    ? qsTr("正在上传航线…")
                                    : qsTr("显示航线编辑器中当前选中无人机的航点")
                            color: "#667085"
                            wrapMode: Text.Wrap
                        }
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: 125
                            clip: true
                            model: root.waypointModel
                            spacing: 4
                            delegate: Rectangle {
                                id: waypointRow
                                required property int index
                                required property double latitude
                                required property double longitude
                                required property double altitude
                                width: ListView.view.width
                                height: 34
                                radius: 4
                                color: index % 2 === 0 ? "#f8fafc" : "white"
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    Label { text: qsTr("航点 %1").arg(waypointRow.index + 1); font.bold: true; color: "#344054" }
                                    Label { Layout.fillWidth: true; text: waypointRow.latitude.toFixed(6) + ", " + waypointRow.longitude.toFixed(6); color: "#475467" }
                                    Label { text: waypointRow.altitude.toFixed(1) + " m"; color: "#475467" }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
