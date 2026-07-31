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
    property alias waypointModel: routeModel
    property var routeCoordinates: []

    signal uploadRequested(bool groupCommand, var waypoints)

    width: 360
    height: 326
    radius: 8
    color: "#eeffffff"
    border.color: "#cbd5e1"

    ListModel {
        id: routeModel
    }

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
        let altitude = Number(defaultAltitudeField.text)
        if (!isFinite(altitude))
            altitude = AppConfig.missionDefaultAltitude()
        routeModel.append({
            "latitude": coordinate.latitude,
            "longitude": coordinate.longitude,
            "altitude": altitude
        })
        refreshRoutePath()
    }

    function removeLastWaypoint() {
        if (routeModel.count > 0) {
            routeModel.remove(routeModel.count - 1)
            refreshRoutePath()
        }
    }

    function clearRoute() {
        routeModel.clear()
        refreshRoutePath()
    }

    function missionWaypoints() {
        const waypoints = []
        for (let index = 0; index < routeModel.count; ++index) {
            const waypoint = routeModel.get(index)
            waypoints.push({
                "latitude": Number(waypoint.latitude),
                "longitude": Number(waypoint.longitude),
                "altitude": Number(waypoint.altitude)
            })
        }
        return waypoints
    }

    function loadMissionWaypoints(waypoints) {
        routeModel.clear()
        for (let index = 0; index < waypoints.length; ++index) {
            const waypoint = waypoints[index]
            routeModel.append({
                "latitude": Number(waypoint.latitude),
                "longitude": Number(waypoint.longitude),
                "altitude": Number(waypoint.altitude)
            })
        }
        refreshRoutePath()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 7

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("航线编辑")
                font.pixelSize: 17
                font.bold: true
                Layout.fillWidth: true
            }
            Switch {
                text: root.editing ? qsTr("编辑中") : qsTr("浏览")
                checked: root.editing
                onToggled: root.editing = checked
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.editing
                  ? qsTr("点击地图添加航点，拖动地图前请关闭编辑")
                  : qsTr("开启编辑后可在地图上依次添加航点")
            color: "#667085"
            wrapMode: Text.Wrap
            font.pixelSize: 11
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("默认高度") }
            Basic.TextField {
                id: defaultAltitudeField
                Layout.preferredWidth: 72
                Layout.preferredHeight: 34
                text: String(AppConfig.missionDefaultAltitude())
                validator: DoubleValidator {
                    bottom: AppConfig.missionMinimumAltitude()
                    top: AppConfig.missionMaximumAltitude()
                    decimals: 1
                }
                horizontalAlignment: Text.AlignRight
            }
            Label { text: qsTr("米") }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("撤销")
                enabled: routeModel.count > 0
                onClicked: root.removeLastWaypoint()
            }
            Button {
                text: qsTr("清空")
                enabled: routeModel.count > 0
                onClicked: root.clearRoute()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 5
            border.color: "#e5e7eb"

            ListView {
                id: routeList
                anchors.fill: parent
                anchors.margins: 4
                clip: true
                spacing: 2
                model: routeModel

                delegate: RowLayout {
                    id: waypointRow
                    required property int index
                    required property double latitude
                    required property double longitude
                    required property double altitude
                    width: ListView.view.width
                    height: 36

                    Label {
                        text: waypointRow.index + 1
                        font.bold: true
                        color: "#dc2626"
                        Layout.preferredWidth: 20
                    }
                    Label {
                        text: waypointRow.latitude.toFixed(6) + ", "
                              + waypointRow.longitude.toFixed(6)
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        font.pixelSize: 11
                    }
                    Basic.TextField {
                        Layout.preferredWidth: 62
                        Layout.preferredHeight: 30
                        text: waypointRow.altitude.toFixed(1)
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
                                    waypointRow.index, "altitude", value)
                                root.refreshRoutePath()
                            }
                        }
                    }
                    Label {
                        text: qsTr("m")
                        font.pixelSize: 11
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: routeList.count === 0
                    text: qsTr("尚未添加航点")
                    color: "#98a2b3"
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: qsTr("上传到单机")
                Layout.fillWidth: true
                enabled: routeModel.count > 0
                         && root.selectedDroneId >= 0
                onClicked: root.uploadRequested(
                               false, root.missionWaypoints())
            }
            Button {
                text: qsTr("上传到编组")
                Layout.fillWidth: true
                highlighted: true
                enabled: routeModel.count > 0
                         && root.selectedGroupName.length > 0
                onClicked: root.uploadRequested(
                               true, root.missionWaypoints())
            }
        }
    }
}
