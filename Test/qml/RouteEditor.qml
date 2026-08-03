pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import QtPositioning
import MiniGCS

Rectangle {
    id: root

    property bool editing: false
    property int selectedDroneId: -1
    property string selectedGroupName: ""
    property string selectedTargetName: ""
    property bool returnHomeAfterMission: true
    property int selectedWaypointIndex: -1
    property var actionChoices: DroneControl.missionActions()
    property alias waypointModel: routeModel
    property var routeCoordinates: []
    readonly property string currentTaskName: {
        if (routeModel.count === 0)
            return qsTr("等待任务开始")
        const index = Math.max(0, Math.min(
            routeModel.count - 1, selectedWaypointIndex))
        return routeModel.get(index).title
    }

    implicitWidth: 330
    color: "#ffffff"
    border.color: "#d9e0e8"

    ListModel { id: routeModel }

    function refreshRoutePath() {
        const coordinates = []
        for (let index = 0; index < routeModel.count; ++index) {
            const waypoint = routeModel.get(index)
            coordinates.push(QtPositioning.coordinate(
                                 waypoint.latitude,
                                 waypoint.longitude,
                                 waypoint.altitude))
        }
        routeCoordinates = coordinates
    }

    function addCoordinate(coordinate) {
        if (!coordinate || !coordinate.isValid)
            return
        const index = routeModel.count
        routeModel.append({
            "title": qsTr("任务点 %1").arg(index + 1),
            "latitude": coordinate.latitude,
            "longitude": coordinate.longitude,
            "altitude": AppConfig.missionDefaultAltitude(),
            "action": 0,
            "actionDurationS": 10.0,
            "speedMS": 0.0,
            "flyThrough": true
        })
        selectedWaypointIndex = index
        editing = false
        refreshRoutePath()
    }

    function removeWaypoint(index) {
        if (index < 0 || index >= routeModel.count)
            return
        routeModel.remove(index)
        selectedWaypointIndex = routeModel.count === 0
                ? -1 : Math.min(index, routeModel.count - 1)
        refreshRoutePath()
    }

    function clearRoute() {
        routeModel.clear()
        selectedWaypointIndex = -1
        refreshRoutePath()
    }

    function missionWaypoints() {
        const waypoints = []
        for (let index = 0; index < routeModel.count; ++index) {
            const waypoint = routeModel.get(index)
            waypoints.push({
                "title": waypoint.title,
                "latitude": Number(waypoint.latitude),
                "longitude": Number(waypoint.longitude),
                "altitude": Number(waypoint.altitude),
                "action": Number(waypoint.action),
                "actionDurationS": Number(waypoint.actionDurationS),
                "speedMS": Number(waypoint.speedMS),
                "flyThrough": Boolean(waypoint.flyThrough)
            })
        }
        return waypoints
    }

    function loadMissionWaypoints(waypoints) {
        routeModel.clear()
        for (let index = 0; index < waypoints.length; ++index) {
            const waypoint = waypoints[index]
            routeModel.append({
                "title": qsTr("任务点 %1").arg(index + 1),
                "latitude": Number(waypoint.latitude),
                "longitude": Number(waypoint.longitude),
                "altitude": Number(waypoint.altitude),
                "action": Number(waypoint.action || 0),
                "actionDurationS": Number(waypoint.actionDurationS || 10),
                "speedMS": Number(waypoint.speedMS || 0),
                "flyThrough": Boolean(waypoint.flyThrough)
            })
        }
        selectedWaypointIndex = routeModel.count > 0 ? 0 : -1
        refreshRoutePath()
    }

    function actionIndex(action) {
        for (let index = 0; index < actionChoices.length; ++index) {
            if (Number(actionChoices[index].value) === Number(action))
                return index
        }
        return 0
    }

    function actionNeedsDuration(action) {
        const index = actionIndex(action)
        return index >= 0 && Boolean(actionChoices[index].needsDuration)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("这次要怎么飞")
                color: "#17202a"
                font.bold: true
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("%1 个点").arg(routeModel.count)
                color: "#17202a"
            }
        }

        ListView {
            id: routeList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 180
            clip: true
            spacing: 10
            model: routeModel

            delegate: Item {
                id: waypointDelegate
                required property int index
                required property string title
                required property double latitude
                required property double longitude
                required property double altitude
                required property int action
                required property double actionDurationS
                required property double speedMS
                required property bool flyThrough
                width: ListView.view.width
                height: root.actionNeedsDuration(action) ? 142 : 114

                Rectangle {
                    x: 13
                    y: 0
                    width: 1
                    height: parent.height + 10
                    color: "#d9e0e8"
                    visible: waypointDelegate.index < routeModel.count - 1
                }

                Rectangle {
                    x: 1
                    y: 12
                    width: 25
                    height: 25
                    radius: 13
                    color: root.selectedWaypointIndex === waypointDelegate.index
                           ? "#1769e0" : "#f7f9fc"
                    border.color: root.selectedWaypointIndex === waypointDelegate.index
                                  ? "#1769e0" : "#d9e0e8"
                    Label {
                        anchors.centerIn: parent
                        text: waypointDelegate.index + 1
                        color: root.selectedWaypointIndex === waypointDelegate.index
                               ? "white" : "#667085"
                    }
                }

                Rectangle {
                    x: 32
                    width: parent.width - 32
                    height: parent.height
                    radius: 9
                    color: root.selectedWaypointIndex === waypointDelegate.index
                           ? "#eaf2ff" : "#ffffff"
                    border.color: root.selectedWaypointIndex === waypointDelegate.index
                                  ? "#1769e0" : "#d9e0e8"

                    TapHandler {
                        onTapped: root.selectedWaypointIndex =
                                      waypointDelegate.index
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: "⌾"; color: "#17202a" }
                            Basic.TextField {
                                Layout.fillWidth: true
                                text: waypointDelegate.title
                                font.bold: true
                                background: Item {}
                                onEditingFinished: routeModel.setProperty(
                                    waypointDelegate.index, "title",
                                    text.trim().length > 0
                                    ? text.trim()
                                    : qsTr("任务点 %1").arg(
                                          waypointDelegate.index + 1))
                            }
                            Button {
                                text: "×"
                                flat: true
                                onClicked: root.removeWaypoint(
                                    waypointDelegate.index)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: qsTr("飞行高度"); color: "#667085" }
                            Basic.TextField {
                                Layout.preferredWidth: 58
                                Layout.preferredHeight: 28
                                text: waypointDelegate.altitude.toFixed(0)
                                validator: DoubleValidator {
                                    bottom: AppConfig.missionMinimumAltitude()
                                    top: AppConfig.missionMaximumAltitude()
                                    decimals: 1
                                }
                                horizontalAlignment: Text.AlignRight
                                onEditingFinished: {
                                    const value = Number(text)
                                    if (isFinite(value)) {
                                        routeModel.setProperty(
                                            waypointDelegate.index,
                                            "altitude", value)
                                        root.refreshRoutePath()
                                    }
                                }
                            }
                            Label { text: qsTr("米"); color: "#667085" }
                            Item { Layout.fillWidth: true }
                            Label {
                                visible: root.actionNeedsDuration(
                                             waypointDelegate.action)
                                text: qsTr("停留 %1 秒").arg(
                                    waypointDelegate.actionDurationS.toFixed(0))
                                color: "#667085"
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Basic.ComboBox {
                                id: actionSelector
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                textRole: "name"
                                valueRole: "value"
                                model: root.actionChoices
                                currentIndex: root.actionIndex(
                                    waypointDelegate.action)
                                onActivated: function(index) {
                                    const value = Number(valueAt(index))
                                    routeModel.setProperty(
                                        waypointDelegate.index,
                                        "action", value)
                                    routeModel.setProperty(
                                        waypointDelegate.index,
                                        "flyThrough", value === 0)
                                    if (value === 4)
                                        root.returnHomeAfterMission = false
                                    else if (waypointDelegate.index ===
                                             routeModel.count - 1)
                                        root.returnHomeAfterMission = true
                                }
                                background: Rectangle {
                                    radius: 15
                                    color: "#e8f7f1"
                                    border.color: "#c9eadc"
                                }
                                contentItem: Label {
                                    text: actionSelector.displayText
                                    color: "#16815b"
                                    leftPadding: 10
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            visible: root.actionNeedsDuration(
                                         waypointDelegate.action)
                            Label { text: qsTr("持续时间"); color: "#667085" }
                            Basic.TextField {
                                Layout.preferredWidth: 60
                                Layout.preferredHeight: 28
                                text: waypointDelegate.actionDurationS.toFixed(0)
                                validator: IntValidator { bottom: 1; top: 3600 }
                                horizontalAlignment: Text.AlignRight
                                onEditingFinished: {
                                    const value = Number(text)
                                    if (isFinite(value) && value > 0)
                                        routeModel.setProperty(
                                            waypointDelegate.index,
                                            "actionDurationS", value)
                                }
                            }
                            Label { text: qsTr("秒"); color: "#667085" }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: routeList.count === 0
                width: parent.width - 20
                text: qsTr("还没有任务点\n点击下方按钮后在地图上选择位置")
                color: "#98a2b3"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
        }

        Button {
            Layout.fillWidth: true
            text: root.editing
                  ? qsTr("请在地图上点击新任务点")
                  : qsTr("＋  在地图上添加任务点")
            highlighted: root.editing
            onClicked: root.editing = !root.editing
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#e1e7ee"
        }

        Label {
            text: qsTr("意外情况怎么处理")
            color: "#17202a"
            font.bold: true
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("▣  电量不足\n     自动回到家点并降落")
            color: "#344054"
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("♒  失去联系\n     等待 10 秒，然后返回家点")
            color: "#344054"
            wrapMode: Text.Wrap
        }

    }
}
