import QtQuick
import QtQuick.Controls

Dialog {
    id: root

    property string commandLabel: ""
    property string targetName: ""
    property bool groupCommand: false
    property string dialogTitle: qsTr("确认操作")
    property string detailText: ""

    signal commandConfirmed()

    width: 380
    modal: true
    title: root.dialogTitle
    standardButtons: Dialog.Yes | Dialog.No

    Label {
        width: root.availableWidth
        wrapMode: Text.Wrap
        text: (root.detailText.length > 0
               ? root.detailText + "\n\n" : "")
              + (root.groupCommand
               ? qsTr("编组命令会同时发送给全部在线成员。\n")
               : "")
              + qsTr("确定让“%1”执行“%2”吗？")
                .arg(root.targetName)
                .arg(root.commandLabel)
    }

    onAccepted: commandConfirmed()
}
