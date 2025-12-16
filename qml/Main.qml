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
        }

        // 设置初始中心点
        center: QtPositioning.coordinate(39.9042, 116.4074) // 北京
        zoomLevel: 12
        activeMapType:supportedMapTypes[6];
        // 地图控制
        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                                 map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
                             }
            onScaleChanged: (delta) => {
                                map.zoomLevel += Math.log2(delta)
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
            property: "zoomLevel"
        }

        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => {
                                      // 允许拖动地图（锁定飞机时，地图会自动跟随飞机回中）
                                      map.pan(-delta.x, -delta.y)
                                  }
        }
        Component.onCompleted: {
            console.log("Supported map types:")
            for (var i = 0; i < supportedMapTypes.length; ++i) {
                var t = supportedMapTypes[i]
                console.log(
                            i,
                            t.name,
                            t.description,
                            t.style
                            )
            }
        }
    }
}
