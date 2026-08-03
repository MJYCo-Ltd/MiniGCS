pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtLocation
import QtPositioning
import MiniGCS

MapView {
    id: root

    property var drones: []
    property int selectedDroneId: -1
    property var waypointModel: null
    property int selectedWaypointIndex: -1
    property var routeCoordinates: []
    property bool routeEditing: false
    property bool centeredOnDrone: false
    property var selectedVehicle: selectedDroneEntry()
    /** 选中时尚无 GPS 时，待该机首次有效定位后再居中 */
    property int pendingLocateDroneId: -1

    signal droneSelected(int systemId)
    signal routeCoordinateRequested(var coordinate)
    signal waypointSelected(int index)

    function selectedDroneEntry() {
        for (let index = 0; index < drones.length; ++index) {
            if (Number(drones[index].systemId) === Number(selectedDroneId))
                return drones[index].vehicle
        }
        return null
    }

    function isValidHome(vehicle) {
        return vehicle
                && vehicle.status.isHomePositionOk
                && isFinite(vehicle.homePosition.latitude)
                && isFinite(vehicle.homePosition.longitude)
                && vehicle.homePosition.latitude >= -90
                && vehicle.homePosition.latitude <= 90
                && vehicle.homePosition.longitude >= -180
                && vehicle.homePosition.longitude <= 180
    }

    function isValidGps(vehicle) {
        return vehicle
                && vehicle.hasGpsPosition
                && isFinite(vehicle.gpsPosition.latitude)
                && isFinite(vehicle.gpsPosition.longitude)
                && vehicle.gpsPosition.latitude >= -90
                && vehicle.gpsPosition.latitude <= 90
                && vehicle.gpsPosition.longitude >= -180
                && vehicle.gpsPosition.longitude <= 180
    }

    /** 将地图居中到指定无人机当前位置；成功返回 true */
    function centerOnDrone(systemId) {
        if (systemId < 0)
            return false
        for (let index = 0; index < drones.length; ++index) {
            const entry = drones[index]
            if (Number(entry.systemId) !== Number(systemId))
                continue
            const vehicle = entry.vehicle
            if (!isValidGps(vehicle))
                return false
            map.center = QtPositioning.coordinate(
                vehicle.gpsPosition.latitude,
                vehicle.gpsPosition.longitude,
                vehicle.gpsPosition.altitude)
            map.zoomLevel = AppConfig.mapVehicleZoom()
            centeredOnDrone = true
            pendingLocateDroneId = -1
            return true
        }
        return false
    }

    function previewRoute() {
        root.map.fitViewportToMapItems()
    }

    onSelectedDroneIdChanged: {
        if (selectedDroneId < 0) {
            pendingLocateDroneId = -1
            return
        }
        if (!centerOnDrone(selectedDroneId))
            pendingLocateDroneId = selectedDroneId
    }

    map.plugin: Plugin {
        name: AppConfig.mapName()
    }
    map.center: QtPositioning.coordinate(
                    AppConfig.mapCenterLatitude(),
                    AppConfig.mapCenterLongitude())
    map.zoomLevel: AppConfig.mapInitialZoom()
    map.minimumZoomLevel: AppConfig.mapMinimumZoom()
    map.maximumZoomLevel: AppConfig.mapMaximumZoom()

    MapPolyline {
        parent: root.map
        line.width: 4
        line.color: "#1769e0"
        path: root.routeCoordinates
        z: 1
    }

    MapQuickItem {
        id: homeMarker
        parent: root.map
        visible: root.isValidHome(root.selectedVehicle)
        coordinate: homeMarker.visible
                    ? QtPositioning.coordinate(
                          root.selectedVehicle.homePosition.latitude,
                          root.selectedVehicle.homePosition.longitude,
                          root.selectedVehicle.homePosition.altitude)
                    : QtPositioning.coordinate()
        anchorPoint: Qt.point(sourceItem.width / 2,
                              sourceItem.height - 3)
        z: 16

        sourceItem: Item {
            width: 86
            height: 58

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 20
                width: 30
                height: 30
                radius: 15
                color: "#16815b"
                border.width: 2
                border.color: "white"

                Label {
                    anchors.centerIn: parent
                    text: qsTr("家")
                    color: "white"
                    font.bold: true
                }
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: homeLabel.implicitWidth + 10
                height: homeLabel.implicitHeight + 4
                radius: 4
                color: "#e616815b"

                Label {
                    id: homeLabel
                    anchors.centerIn: parent
                    text: qsTr("家点")
                    color: "white"
                    font.pixelSize: 11
                }
            }
        }
    }

    MapItemView {
        parent: root.map
        model: root.waypointModel
        z: 20

        delegate: MapQuickItem {
            id: waypointMarker
            required property int index
            required property double latitude
            required property double longitude
            required property double altitude
            required property string title
            required property int action

            coordinate: QtPositioning.coordinate(
                            latitude, longitude, altitude)
            anchorPoint: Qt.point(sourceItem.width / 2, 17)
            z: 20

            sourceItem: Item {
                z: 20
                width: 180
                height: 62

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.selectedWaypointIndex ===
                           waypointMarker.index ? 36 : 30
                    height: width
                    radius: width / 2
                    color: "#1769e0"
                    border.width: root.selectedWaypointIndex ===
                                  waypointMarker.index ? 4 : 2
                    border.color: root.selectedWaypointIndex ===
                                  waypointMarker.index ? "#b7d3fa" : "white"

                    Label {
                        anchors.centerIn: parent
                        text: waypointMarker.index + 1
                        color: "white"
                        font.bold: true
                    }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 40
                    width: Math.min(parent.width,
                                    selectedWaypointLabel.implicitWidth + 12)
                    height: selectedWaypointLabel.implicitHeight + 6
                    radius: 4
                    color: "#eeffffff"
                    border.color: "#d9e0e8"
                    visible: root.selectedWaypointIndex === waypointMarker.index

                    Label {
                        id: selectedWaypointLabel
                        anchors.centerIn: parent
                        text: waypointMarker.title + " · " +
                              DroneControl.missionActionName(
                                  waypointMarker.action)
                        color: "#17202a"
                        font.pixelSize: 11
                    }
                }

                TapHandler {
                    onTapped: root.waypointSelected(waypointMarker.index)
                }
            }
        }
    }

    MapItemView {
        parent: root.map
        model: root.drones

        delegate: MapQuickItem {
            id: droneMarker
            required property var modelData
            property var vehicle: modelData.vehicle
            property bool coordinateInitialized: false
            property var displayedCoordinate: QtPositioning.coordinate()
            property bool positionAvailable: root.isValidGps(vehicle)

            function acceptCurrentPosition(forceUpdate) {
                if (!positionAvailable)
                    return
                if (!coordinateInitialized || forceUpdate || vehicle.moving) {
                    displayedCoordinate = QtPositioning.coordinate(
                        vehicle.gpsPosition.latitude,
                        vehicle.gpsPosition.longitude,
                        vehicle.gpsPosition.altitude)
                    coordinateInitialized = true
                }
                if (Number(root.pendingLocateDroneId) ===
                        Number(modelData.systemId)) {
                    root.centerOnDrone(modelData.systemId)
                }
            }

            coordinate: displayedCoordinate
            visible: positionAvailable
            anchorPoint: Qt.point(sourceItem.width / 2,
                                  sourceItem.height - 2)
            autoFadeIn: false
            z: root.selectedDroneId === modelData.systemId ? 10 : 8

            onCoordinateChanged: {
                if (positionAvailable && !root.centeredOnDrone) {
                    root.map.center = coordinate
                    root.map.zoomLevel = AppConfig.mapVehicleZoom()
                    root.centeredOnDrone = true
                }
            }

            Component.onCompleted: acceptCurrentPosition(true)

            Connections {
                target: droneMarker.vehicle

                function onGpsPositionChanged() {
                    droneMarker.acceptCurrentPosition(false)
                }

                function onMovingChanged() {
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
                    source: DroneControl.vehicleIcon(
                                droneMarker.vehicle.vehicleType)
                    fillMode: Image.PreserveAspectFit
                    mipmap: true
                    rotation: droneMarker.vehicle.attitude.headingDeg
                    transformOrigin: Item.Center
                    scale: root.selectedDroneId ===
                           droneMarker.modelData.systemId ? 1.12 : 1.0
                    layer.enabled: root.selectedDroneId ===
                                   droneMarker.modelData.systemId
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
                    color: root.selectedDroneId ===
                           droneMarker.modelData.systemId
                           ? "#e6b91c1c" : "#cc101828"

                    Label {
                        id: droneNameLabel
                        anchors.centerIn: parent
                        width: droneNameTag.width - 8
                        text: droneMarker.modelData.name
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
                    color: root.selectedDroneId ===
                           droneMarker.modelData.systemId
                           ? "#dc2626" : "#1f2937"
                    font.pixelSize: 18
                    style: Text.Outline
                    styleColor: "white"
                }

                TapHandler {
                    onTapped: root.droneSelected(
                                  droneMarker.modelData.systemId)
                }
            }
        }
    }

    TapHandler {
        enabled: root.routeEditing
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: function(eventPoint) {
            root.routeCoordinateRequested(
                root.map.toCoordinate(eventPoint.position))
        }
    }
}
