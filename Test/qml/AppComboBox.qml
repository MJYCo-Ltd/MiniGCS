import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic as Basic

// 高亮项强制深色字；文案走 ComboBox.textAt，避免自定义 model 解析踩坑。
Basic.ComboBox {
    id: control

    palette.highlight: "#dbeafe"
    palette.highlightedText: "#101828"
    palette.text: "#101828"
    palette.windowText: "#101828"
    palette.buttonText: "#101828"

    delegate: ItemDelegate {
        id: itemDelegate
        required property int index

        width: control.width
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        text: control.textAt(index)

        contentItem: Text {
            text: itemDelegate.text
            color: "#101828"
            font: control.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: 8
            rightPadding: 8
        }

        background: Rectangle {
            implicitHeight: 36
            color: itemDelegate.highlighted || itemDelegate.hovered
                   ? "#dbeafe" : "transparent"
            radius: 4
        }
    }
}
