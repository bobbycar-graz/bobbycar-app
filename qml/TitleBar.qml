import QtQuick 2.15

Rectangle    {
    id: titleBar
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    height: GameSettings.fieldHeight
    color: GameSettings.viewColor

    property var __titles: ["CONNECT", "LIVEDATA", "REMOTECONTROL", "SETTINGS"]
    property int currentIndex: 0

    signal titleClicked(int index)

    Repeater {
        model: 4
        Text {
            width: titleBar.width / 4
            height: titleBar.height
            x: index * width
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            //text: GameSettings.hugeFontSize * 0.4
            //text: currentIndex
            text: __titles[index]
            font.pixelSize: GameSettings.hugeFontSize * 0.3
            color: titleBar.currentIndex === index ? GameSettings.textColor : GameSettings.disabledTextColor

            MouseArea {
                anchors.fill: parent
                onClicked: titleClicked(index)
            }
        }
    }


    Item {
        anchors.bottom: parent.bottom
        width: parent.width / 4
        height: parent.height
        x: currentIndex * width

        BottomLine{}

        Behavior on x { NumberAnimation { duration: 200 } }
    }

}
