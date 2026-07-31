pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MiniGCS

Rectangle {
    id: root

    property int selectedIndex: -1
    property var portNames: []
    property var baudRates: []

    color: "#f4f7fb"

    ListModel { id: linkModel }

    function pick(config, camel, pascal, fallback) {
        if (config[camel] !== undefined && config[camel] !== null)
            return config[camel]
        if (config[pascal] !== undefined && config[pascal] !== null)
            return config[pascal]
        return fallback
    }

    function normalized(config) {
        // 兼容 C++/INI 侧偶发传入的 PascalCase 键；空 hostName 表示绑定 0.0.0.0
        return {
            "type": String(pick(config, "type", "Type", "Serial")),
            "name": String(pick(config, "name", "Name", "")),
            "portName": String(pick(config, "portName", "PortName", "")),
            "baudRate": Number(pick(config, "baudRate", "BaudRate", 115200)),
            "hostName": String(pick(config, "hostName", "HostName", "127.0.0.1")),
            "port": Number(pick(config, "port", "Port", 14550))
        }
    }

    function reloadConfiguration() {
        linkModel.clear()
        const configs = AppConfig.linkConfigList()
        for (let index = 0; index < configs.length; ++index)
            linkModel.append(normalized(configs[index]))
        selectedIndex = linkModel.count > 0 ? 0 : -1
        linkList.currentIndex = selectedIndex
        loadEditor(selectedIndex)
        resultLabel.text = qsTr("已读取 %1 条链路配置").arg(linkModel.count)
        resultLabel.color = "#475467"
    }

    function refreshSerialOptions() {
        portNames = AppConfig.refreshPortName()
        baudRates = AppConfig.standardBaudRates()
    }

    function typeIndex(type) {
        for (let index = 0; index < typeModel.count; ++index) {
            if (typeModel.get(index).value === type)
                return index
        }
        return 0
    }

    function isServerType(type) {
        return type === "TcpServer" || type === "UdpServer"
    }

    function isClientType(type) {
        return type === "TcpClient" || type === "UdpClient"
    }

    function loadEditor(index) {
        if (index < 0 || index >= linkModel.count) {
            nameField.text = ""
            return
        }
        const config = linkModel.get(index)
        typeCombo.currentIndex = typeIndex(config.type)
        nameField.text = config.name
        portCombo.editText = config.portName
        baudCombo.editText = String(config.baudRate)
        hostField.text = config.hostName
        networkPort.value = config.port
    }

    function editorConfiguration() {
        return normalized({
            "type": typeCombo.currentValue,
            "name": nameField.text.trim(),
            "portName": portCombo.editText.trim(),
            "baudRate": Number(baudCombo.editText),
            "hostName": hostField.text.trim(),
            "port": networkPort.value
        })
    }

    function storageConfiguration(index) {
        const config = linkModel.get(index)
        return {
            "type": String(config.type),
            "name": String(config.name),
            "portName": String(config.portName),
            "baudRate": Number(config.baudRate),
            "hostName": String(config.hostName),
            "port": Number(config.port)
        }
    }

    function valid(config) {
        if (config.type === "Serial")
            return config.portName.length > 0 && config.baudRate > 0
        if (config.port < 1 || config.port > 65535)
            return false
        if (root.isServerType(config.type))
            return true
        return root.isClientType(config.type) && config.hostName.length > 0
    }

    function updateCurrent() {
        if (selectedIndex < 0 || selectedIndex >= linkModel.count)
            return true
        const config = editorConfiguration()
        if (!valid(config)) {
            resultLabel.text = qsTr("当前链路参数无效")
            resultLabel.color = "#b42318"
            return false
        }
        linkModel.set(selectedIndex, config)
        return true
    }

    function addConfiguration() {
        if (!updateCurrent())
            return
        const config = normalized({
            "type": "Serial",
            "name": qsTr("链路 %1").arg(linkModel.count + 1),
            "portName": portNames.length > 0 ? portNames[0] : "",
            "baudRate": 115200
        })
        linkModel.append(config)
        selectedIndex = linkModel.count - 1
        linkList.currentIndex = selectedIndex
        loadEditor(selectedIndex)
    }

    function removeConfiguration() {
        if (selectedIndex < 0)
            return
        linkModel.remove(selectedIndex)
        selectedIndex = Math.min(selectedIndex, linkModel.count - 1)
        linkList.currentIndex = selectedIndex
        loadEditor(selectedIndex)
    }

    function persist(applyNow) {
        if (!updateCurrent())
            return
        while (AppConfig.linkCount() > 0)
            AppConfig.removeLinkConfigAt(AppConfig.linkCount() - 1)
        for (let index = 0; index < linkModel.count; ++index)
            AppConfig.appendLinkConfig(storageConfiguration(index))
        AppConfig.saveLinkConfigs()

        if (!applyNow) {
            resultLabel.text = qsTr("链路配置已保存，下次启动时生效")
            resultLabel.color = "#027a48"
            return
        }
        const applied = DroneControl.applyConfiguredLinks()
        resultLabel.text = applied
                ? qsTr("链路配置已保存并重新连接")
                : qsTr("链路配置已保存，但应用失败，请检查日志")
        resultLabel.color = applied ? "#027a48" : "#b42318"
    }

    Component.onCompleted: {
        refreshSerialOptions()
        reloadConfiguration()
    }

    ListModel {
        id: typeModel
        ListElement { text: qsTr("串口"); value: "Serial" }
        ListElement { text: qsTr("TCP 服务端"); value: "TcpServer" }
        ListElement { text: qsTr("TCP 客户端"); value: "TcpClient" }
        ListElement { text: qsTr("UDP 服务端"); value: "UdpServer" }
        ListElement { text: qsTr("UDP 客户端"); value: "UdpClient" }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label { text: qsTr("链路配置"); font.pixelSize: 24; font.bold: true; color: "#1d2939" }
        Label {
            Layout.fillWidth: true
            text: qsTr("配置串口、TCP 和 UDP 链路。“保存并重连”会断开当前链路后按新配置重新建立连接。")
            wrapMode: Text.Wrap
            color: "#667085"
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            Frame {
                SplitView.preferredWidth: 330
                SplitView.minimumWidth: 250
                background: Rectangle { color: "white"; border.color: "#d0d5dd" }

                ColumnLayout {
                    anchors.fill: parent
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("已配置链路 (%1)").arg(linkModel.count); font.bold: true; Layout.fillWidth: true }
                        ToolButton { text: qsTr("+"); onClicked: root.addConfiguration() }
                        ToolButton { text: qsTr("−"); enabled: root.selectedIndex >= 0; onClicked: root.removeConfiguration() }
                    }
                    ListView {
                        id: linkList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 4
                        model: linkModel
                        onCurrentIndexChanged: {
                            if (currentIndex === root.selectedIndex)
                                return
                            if (!root.updateCurrent()) {
                                currentIndex = root.selectedIndex
                                return
                            }
                            root.selectedIndex = currentIndex
                            root.loadEditor(currentIndex)
                        }
                        delegate: Rectangle {
                            id: linkRow
                            required property int index
                            required property string type
                            required property string name
                            required property string portName
                            required property int baudRate
                            required property string hostName
                            required property int port
                            width: ListView.view.width
                            height: 64
                            radius: 5
                            color: ListView.isCurrentItem ? "#e8f1ff" : "#f8fafc"
                            border.color: ListView.isCurrentItem ? "#3b82f6" : "#e4e7ec"
                            MouseArea { anchors.fill: parent; onClicked: linkList.currentIndex = linkRow.index }
                            Column {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 4
                                Label { text: linkRow.name.length > 0 ? linkRow.name : linkRow.type; font.bold: true; color: "#1d2939" }
                                Label {
                                    text: linkRow.type === "Serial"
                                          ? linkRow.portName + " · " + linkRow.baudRate
                                          : ((linkRow.hostName.length > 0
                                              ? linkRow.hostName
                                              : "0.0.0.0") + ":" + linkRow.port)
                                    color: "#667085"
                                }
                            }
                        }
                    }
                }
            }

            Frame {
                SplitView.fillWidth: true
                SplitView.minimumWidth: 360
                background: Rectangle { color: "white"; border.color: "#d0d5dd" }

                GridLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    columns: 2
                    columnSpacing: 16
                    rowSpacing: 12
                    enabled: root.selectedIndex >= 0

                    Label { text: qsTr("名称"); font.bold: true }
                    TextField { id: nameField; Layout.fillWidth: true; placeholderText: qsTr("例如：飞控串口") }
                    Label { text: qsTr("链路类型"); font.bold: true }
                    ComboBox { id: typeCombo; Layout.fillWidth: true; model: typeModel; textRole: "text"; valueRole: "value" }

                    Label { visible: typeCombo.currentValue === "Serial"; text: qsTr("串口"); font.bold: true }
                    RowLayout {
                        visible: typeCombo.currentValue === "Serial"
                        Layout.fillWidth: true
                        ComboBox { id: portCombo; Layout.fillWidth: true; editable: true; model: root.portNames }
                        Button { text: qsTr("刷新"); onClicked: root.refreshSerialOptions() }
                    }
                    Label { visible: typeCombo.currentValue === "Serial"; text: qsTr("波特率"); font.bold: true }
                    ComboBox { id: baudCombo; visible: typeCombo.currentValue === "Serial"; Layout.fillWidth: true; editable: true; model: root.baudRates }

                    Label {
                        visible: typeCombo.currentValue !== "Serial"
                        text: root.isServerType(typeCombo.currentValue)
                              ? qsTr("绑定地址") : qsTr("主机地址")
                        font.bold: true
                    }
                    TextField {
                        id: hostField
                        visible: typeCombo.currentValue !== "Serial"
                        Layout.fillWidth: true
                        placeholderText: root.isServerType(typeCombo.currentValue)
                                         ? "0.0.0.0" : "127.0.0.1"
                    }
                    Label { visible: typeCombo.currentValue !== "Serial"; text: qsTr("端口"); font.bold: true }
                    SpinBox { id: networkPort; visible: typeCombo.currentValue !== "Serial"; Layout.fillWidth: true; from: 1; to: 65535; value: 14550; editable: true }

                    Item { Layout.columnSpan: 2; Layout.fillHeight: true }
                    Item { Layout.fillWidth: true }
                    Button { text: qsTr("更新当前链路"); enabled: root.selectedIndex >= 0; onClicked: root.updateCurrent() }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button { text: qsTr("重新读取"); onClicked: root.reloadConfiguration() }
            Item { Layout.fillWidth: true }
            Label { id: resultLabel; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; wrapMode: Text.Wrap }
            Button { text: qsTr("仅保存"); onClicked: root.persist(false) }
            Button { text: qsTr("保存并重连"); highlighted: true; onClicked: root.persist(true) }
        }
    }
}
