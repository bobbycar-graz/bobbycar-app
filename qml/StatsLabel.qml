import QtQuick 2.5
import "."

Item {
    height: GameSettings.fieldHeight
    width: parent.width

    property alias title: leftText.text
    property alias value: rightText.text

    Text {
        id: leftText
        anchors.left: parent.left
        height: parent.height
        width: parent.width * 0.45
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: GameSettings.mediumFontSize
        color: GameSettings.textColor
    }

    Text {
        id: rightText
        anchors.right: parent.right
        height: parent.height
        width: parent.width * 0.45
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: GameSettings.mediumFontSize
        color: GameSettings.textColor
    }
}
