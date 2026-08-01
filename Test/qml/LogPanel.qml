import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property bool expanded: true
    property var businessLogs: []
    property var firmwareLogs: []

    signal clearBusinessRequested()
    signal clearFirmwareRequested()

    implicitHeight: expanded ? 230 : 38
    radius: 7
    color: "#ee111827"
    border.color: "#475467"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 7
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("告警日志 · 业务 %1 · 固件 %2")
                    .arg(root.businessLogs.length)
                    .arg(root.firmwareLogs.length)
                color: "white"
                font.bold: true
                Layout.fillWidth: true
            }
            ToolButton {
                text: qsTr("清空")
                visible: root.expanded
                palette.buttonText: "white"
                onClicked: {
                    if (logTypeTabs.currentIndex === 0)
                        root.clearBusinessRequested()
                    else
                        root.clearFirmwareRequested()
                }
            }
            ToolButton {
                text: root.expanded ? "\u25bc" : "\u25b2"
                palette.buttonText: "white"
                onClicked: root.expanded = !root.expanded
            }
        }

        TabBar {
            id: logTypeTabs
            Layout.fillWidth: true
            visible: root.expanded

            TabButton {
                text: qsTr("业务 (%1)").arg(root.businessLogs.length)
            }
            TabButton {
                text: qsTr("固件 (%1)")
                    .arg(root.firmwareLogs.length)
            }
        }

        StackLayout {
            currentIndex: logTypeTabs.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.expanded

            ListView {
                id: businessLogList
                clip: true
                spacing: 3
                model: root.businessLogs
                onCountChanged: positionViewAtEnd()

                delegate: Label {
                    required property string modelData
                    width: ListView.view.width
                    text: modelData
                    color: "#fecaca"
                    font.family: "Consolas"
                    font.pixelSize: 11
                    wrapMode: Text.WrapAnywhere
                }
            }

            ListView {
                id: firmwareLogList
                clip: true
                spacing: 3
                model: root.firmwareLogs
                onCountChanged: positionViewAtEnd()

                delegate: Label {
                    required property string modelData
                    width: ListView.view.width
                    text: modelData
                    color: "#fde68a"
                    font.family: "Consolas"
                    font.pixelSize: 11
                    wrapMode: Text.WrapAnywhere
                }
            }
        }
    }
}
