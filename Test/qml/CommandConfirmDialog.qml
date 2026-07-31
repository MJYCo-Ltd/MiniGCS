import QtQuick
import QtQuick.Controls

Dialog {
    id: root

    property string commandLabel: ""
    property string targetName: ""
    property bool groupCommand: false

    signal commandConfirmed()

    width: 380
    modal: true
    title: qsTr("确认控制命令")
    standardButtons: Dialog.Yes | Dialog.No

    Label {
        width: root.availableWidth
        wrapMode: Text.Wrap
        text: (root.groupCommand
               ? qsTr("编组命令会同时发送给全部在线成员。\n")
               : "")
              + qsTr("确定向“%1”发送“%2”命令吗？")
                .arg(root.targetName)
                .arg(root.commandLabel)
    }

    onAccepted: commandConfirmed()
}
