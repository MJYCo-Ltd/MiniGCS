import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import MiniGCS

ColumnLayout {
    id: root

    property string selectedGroupName: ""
    property bool showCommands: true
    signal commandRequested(int command)

    spacing: 10

    function memberSelected(systemId) {
        const configuredGroups = DroneControl.groups
        for (let groupIndex = 0;
             groupIndex < configuredGroups.length; ++groupIndex) {
            const group = configuredGroups[groupIndex]
            if (group.name !== selectedGroupName)
                continue
            const members = group.members
            for (let index = 0; index < members.length; ++index) {
                if (Number(members[index]) === Number(systemId))
                    return true
            }
        }
        return false
    }

    function updateMember(systemId, selected) {
        const current = DroneControl.groupMembers(selectedGroupName)
        const next = []
        for (let index = 0; index < current.length; ++index) {
            if (Number(current[index]) !== Number(systemId))
                next.push(Number(current[index]))
        }
        if (selected)
            next.push(Number(systemId))
        DroneControl.setGroupMembers(selectedGroupName, next)
    }

    RowLayout {
        Layout.fillWidth: true
        Basic.TextField {
            id: newGroupName
            Layout.fillWidth: true
            Layout.minimumHeight: implicitHeight
            placeholderText: qsTr("新编组名称")
            selectByMouse: true
        }
        Button {
            text: qsTr("新建")
            onClicked: {
                if (DroneControl.addGroup(newGroupName.text)) {
                    root.selectedGroupName = newGroupName.text.trim()
                    newGroupName.clear()
                }
            }
        }
    }

    AppComboBox {
        id: groupSelector
        Layout.fillWidth: true
        textRole: "name"
        valueRole: "name"
        model: DroneControl.groups
        onCurrentValueChanged: {
            root.selectedGroupName = currentValue || ""
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Label {
            text: root.selectedGroupName.length > 0
                  ? qsTr("配置“%1”成员").arg(root.selectedGroupName)
                  : qsTr("请先创建编组")
            font.bold: true
            Layout.fillWidth: true
        }
        Button {
            text: qsTr("删除编组")
            enabled: root.selectedGroupName.length > 0
            onClicked: {
                DroneControl.removeGroup(root.selectedGroupName)
                root.selectedGroupName = ""
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "white"
        radius: 6
        border.color: "#d9dee7"

        ListView {
            id: memberList
            anchors.fill: parent
            anchors.margins: 6
            clip: true
            model: DroneControl.drones

            delegate: CheckDelegate {
                id: memberDelegate
                required property var modelData
                width: ListView.view.width
                text: "%1  (ID %2)  ·  %3"
                    .arg(modelData.name)
                    .arg(modelData.systemId)
                    .arg(modelData.connected
                         ? qsTr("在线") : qsTr("离线"))
                checked: root.memberSelected(modelData.systemId)
                enabled: root.selectedGroupName.length > 0
                onToggled: root.updateMember(
                               modelData.systemId, checked)
            }
        }
    }

    GridLayout {
        visible: root.showCommands
        columns: 2
        Layout.fillWidth: true

        Button {
            text: qsTr("编组%1").arg(
                      DroneControl.commandName(DroneControl.armCommand))
            Layout.fillWidth: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(DroneControl.armCommand)
        }
        Button {
            text: qsTr("编组%1").arg(
                      DroneControl.commandName(DroneControl.disarmCommand))
            Layout.fillWidth: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(DroneControl.disarmCommand)
        }
        Button {
            text: qsTr("编组%1").arg(
                      DroneControl.commandName(DroneControl.takeoffCommand))
            Layout.fillWidth: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(DroneControl.takeoffCommand)
        }
        Button {
            text: qsTr("编组%1").arg(
                      DroneControl.commandName(DroneControl.landCommand))
            Layout.fillWidth: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(DroneControl.landCommand)
        }
        Button {
            text: qsTr("编组%1").arg(DroneControl.commandName(
                      DroneControl.returnToLaunchCommand))
            Layout.fillWidth: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(
                           DroneControl.returnToLaunchCommand)
        }
        Button {
            text: qsTr("编组%1").arg(DroneControl.commandName(
                      DroneControl.downloadMissionCommand))
            Layout.fillWidth: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(
                           DroneControl.downloadMissionCommand)
        }
        Button {
            text: qsTr("编组%1").arg(DroneControl.commandName(
                      DroneControl.startMissionCommand))
            Layout.columnSpan: 2
            Layout.fillWidth: true
            highlighted: true
            enabled: root.selectedGroupName.length > 0
            onClicked: root.commandRequested(
                           DroneControl.startMissionCommand)
        }
    }
}
