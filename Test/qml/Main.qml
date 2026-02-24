import QtQuick
import QtLocation
import QtPositioning
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    // 地图视图
    MapView{
        id: map
        anchors.fill: parent

        map.plugin: Plugin{
            id:mapPlugin
            name: "QGroundControl"
        }
        map.center: QtPositioning.coordinate(38.045474, 114.502461)
        map.zoomLevel: 10
        map.minimumZoomLevel: 3
        map.maximumZoomLevel: 18
    }
}

