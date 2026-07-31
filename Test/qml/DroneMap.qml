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
    property var routeCoordinates: []
    property bool routeEditing: false
    property bool centeredOnDrone: false

    signal droneSelected(int systemId)
    signal routeCoordinateRequested(var coordinate)

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
        line.color: "#dc2626"
        path: root.routeCoordinates
        z: 1
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

            coordinate: QtPositioning.coordinate(
                            latitude, longitude, altitude)
            anchorPoint: Qt.point(sourceItem.width / 2,
                                  sourceItem.height / 2)
            z: 20

            sourceItem: Rectangle {
                z: 20
                width: 28
                height: 28
                radius: 14
                color: "#dc2626"
                border.width: 2
                border.color: "white"

                Label {
                    z: 21
                    anchors.centerIn: parent
                    text: waypointMarker.index + 1
                    color: "white"
                    font.bold: true
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
                if (!coordinateInitialized || forceUpdate || vehicle.moving) {
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
                    source: DroneControl.vehicleIcon(
                                droneMarker.vehicle.vehicleType)
                    fillMode: Image.PreserveAspectFit
                    mipmap: true
                    rotation: droneMarker.vehicle.heading
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
