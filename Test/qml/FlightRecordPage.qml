pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning
import MiniGCS

Rectangle {
    id: root

    color: "#f6f8fb"
    property var records: DroneControl.flightRecords
    property var selectedRecord: null
    property var trackPath: []
    property var missionPath: []

    function recordId(record) {
        return record ? String(record.id) : ""
    }

    function formatTime(value) {
        const date = new Date(value)
        if (isNaN(date.getTime()))
            return qsTr("时间未知")
        return Qt.formatDateTime(date, "yyyy-MM-dd hh:mm")
    }

    function durationText(record) {
        if (!record)
            return "--"
        const start = new Date(record.startedAt)
        const end = new Date(record.completedAt)
        const seconds = Math.max(0, Math.round(
            (end.getTime() - start.getTime()) / 1000))
        if (!isFinite(seconds))
            return "--"
        const minutes = Math.floor(seconds / 60)
        return minutes > 0 ? qsTr("%1 分 %2 秒").arg(minutes).arg(seconds % 60)
                           : qsTr("%1 秒").arg(seconds)
    }

    function missionSummary(record) {
        if (!record || !record.missionPoints
                || record.missionPoints.length === 0) {
            return qsTr("未保存任务点说明")
        }
        const values = []
        for (let index = 0; index < record.missionPoints.length; ++index) {
            const point = record.missionPoints[index]
            const title = point.title ? String(point.title)
                                      : qsTr("任务点 %1").arg(index + 1)
            values.push(qsTr("%1（%2）").arg(title).arg(
                DroneControl.missionActionName(Number(point.action))))
        }
        return values.join(qsTr(" → "))
    }

    function coordinates(values) {
        const result = []
        if (!values)
            return result
        for (let index = 0; index < values.length; ++index) {
            const value = values[index]
            const latitude = Number(value.latitude)
            const longitude = Number(value.longitude)
            const altitude = Number(value.altitude)
            if (isFinite(latitude) && isFinite(longitude)) {
                result.push(QtPositioning.coordinate(
                    latitude, longitude, isFinite(altitude) ? altitude : 0))
            }
        }
        return result
    }

    function selectRecord(record) {
        selectedRecord = record
        trackPath = coordinates(record ? record.track : [])
        missionPath = coordinates(record ? record.missionPoints : [])
        Qt.callLater(function() {
            if (trackPath.length > 0 || missionPath.length > 0)
                recordMap.map.fitViewportToMapItems()
        })
    }

    onRecordsChanged: {
        if (!records || records.length === 0) {
            selectRecord(null)
            return
        }
        const selectedId = recordId(selectedRecord)
        for (let index = 0; index < records.length; ++index) {
            if (recordId(records[index]) === selectedId) {
                selectRecord(records[index])
                return
            }
        }
        selectRecord(records[0])
    }

    Component.onCompleted: {
        if (records && records.length > 0)
            selectRecord(records[0])
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Rectangle {
            SplitView.preferredWidth: 350
            SplitView.minimumWidth: 300
            color: "white"
            border.color: "#d9e0e8"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("成功任务")
                        font.pixelSize: 20
                        font.bold: true
                        color: "#17202a"
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: qsTr("%1 条").arg(root.records.length)
                        color: "#667085"
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("按无人机保存实际飞行轨迹和已完成任务")
                    wrapMode: Text.Wrap
                    color: "#667085"
                }

                ListView {
                    id: recordList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    clip: true
                    model: root.records

                    delegate: Rectangle {
                        id: recordDelegate
                        required property var modelData
                        width: recordList.width
                        height: 112
                        radius: 10
                        color: root.recordId(root.selectedRecord)
                               === root.recordId(modelData)
                               ? "#eaf2ff" : "#f8fafc"
                        border.color: root.recordId(root.selectedRecord)
                                      === root.recordId(modelData)
                                      ? "#1769e0" : "#d9e0e8"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 4
                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("%1 · ID %2")
                                          .arg(recordDelegate.modelData.droneName)
                                          .arg(recordDelegate.modelData.systemId)
                                    font.bold: true
                                    color: "#17202a"
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: qsTr("任务成功")
                                    color: "#16815b"
                                }
                            }
                            Label {
                                text: root.formatTime(
                                    recordDelegate.modelData.startedAt)
                                color: "#667085"
                            }
                            Label {
                                text: qsTr("%1 个任务点 · %2 个轨迹点")
                                      .arg(recordDelegate.modelData.missionPoints.length)
                                      .arg(recordDelegate.modelData.track.length)
                                color: "#475467"
                            }
                        }

                        TapHandler {
                            onTapped: root.selectRecord(
                                recordDelegate.modelData)
                        }
                    }

                    Label {
                        anchors.centerIn: parent
                        visible: recordList.count === 0
                        text: qsTr("暂无成功任务\n任务完成后会自动保存在这里")
                        horizontalAlignment: Text.AlignHCenter
                        color: "#98a2b3"
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("清空飞行记录")
                    enabled: root.records.length > 0
                    onClicked: clearConfirm.open()
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 460
            color: "#e9eef5"

            MapView {
                id: recordMap
                anchors.fill: parent
                map.plugin: Plugin { name: AppConfig.mapName() }
                map.center: QtPositioning.coordinate(
                    AppConfig.mapCenterLatitude(), AppConfig.mapCenterLongitude())
                map.zoomLevel: AppConfig.mapInitialZoom()
                map.minimumZoomLevel: AppConfig.mapMinimumZoom()
                map.maximumZoomLevel: AppConfig.mapMaximumZoom()

                MapPolyline {
                    parent: recordMap.map
                    line.width: 3
                    line.color: "#8ba4c7"
                    path: root.missionPath
                }
                MapPolyline {
                    parent: recordMap.map
                    line.width: 5
                    line.color: "#16815b"
                    path: root.trackPath
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 16
                height: detailColumn.implicitHeight + 28
                radius: 10
                visible: root.selectedRecord !== null
                color: "#f8ffffff"
                border.color: "#d9e0e8"

                ColumnLayout {
                    id: detailColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 24
                        Label {
                            text: qsTr("无人机：%1").arg(
                                root.selectedRecord
                                ? root.selectedRecord.droneName : "--")
                            font.bold: true
                        }
                        Label {
                            text: qsTr("任务点：%1").arg(
                                root.selectedRecord
                                ? root.selectedRecord.missionPoints.length : 0)
                        }
                        Label {
                            text: qsTr("飞行时长：%1").arg(
                                root.durationText(root.selectedRecord))
                        }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: qsTr("绿色：实际轨迹")
                            color: "#16815b"
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("任务：%1").arg(
                            root.missionSummary(root.selectedRecord))
                        elide: Text.ElideRight
                        color: "#475467"
                    }
                }
            }
        }
    }

    Dialog {
        id: clearConfirm
        anchors.centerIn: parent
        modal: true
        title: qsTr("清空飞行记录")
        standardButtons: Dialog.Yes | Dialog.No
        Label {
            text: qsTr("确定删除全部已保存的成功任务和飞行轨迹吗？")
        }
        onAccepted: DroneControl.clearFlightRecords()
    }
}
