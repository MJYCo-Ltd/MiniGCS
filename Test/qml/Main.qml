import QtQuick
import QtLocation
import QtPositioning
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    // 地图视图
    Map {
        id:map
        anchors.fill: parent
        plugin: Plugin {
            name: "QGroundControl"   // 使用 OpenStreetMap 插件
            PluginParameter {
                name: "TiandiTuKey"
                value: "cbc71550f33685acbd0bff46a661e63d"
            }
        }

        // 设置初始中心点
        center: QtPositioning.coordinate(38.045474, 114.502461) // 北京
        zoomLevel: 8
        activeMapType:supportedMapTypes[0]
        minimumZoomLevel: 1
        maximumZoomLevel:23

        function clampZoom(delta){
            map.zoomLevel = Math.max(map.minimumZoomLevel, Math.min(map.maximumZoomLevel, map.zoomLevel + delta))
        }

        // 地图控制
        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                                 map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
                             }
            onScaleChanged: (delta) => {
                                map.clampZoom(Math.log2(delta))
                                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
                            }
            onRotationChanged: (delta) => {
                                   map.bearing -= delta
                                   map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
                               }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        WheelHandler {
            id: wheel
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            rotationScale: 1/120
            onWheel: (event) => {
                         map.clampZoom(event.angleDelta.y * rotationScale)
                         event.accepted = true
                     }
        }

        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => {
                                      // 允许拖动地图（锁定飞机时，地图会自动跟随飞机回中）
                                      map.pan(-delta.x, -delta.y)
                                  }
        }
    }
}

