import QtQuick 2.15

GamePage {
    id: remoteControlPage

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: container.top
        anchors.bottomMargin: GameSettings.fieldMargin

        font.pixelSize: GameSettings.hugeFontSize
        color: GameSettings.textColor
        text: qsTr("REMOTE CONTROL")
    }

    Rectangle {
        id: container
        anchors.centerIn: parent
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(remoteControlPage.width, remoteControlPage.height-GameSettings.fieldHeight*4) - 2*GameSettings.fieldMargin
        height: width
        radius: GameSettings.buttonRadius
        color: GameSettings.viewColor

        PointHandler {
            id: handler
            //acceptedDevices: PointerDevice.TouchScreen
            target: Rectangle {
                parent: container
                color: "red"
                visible: handler.active
                x: handler.point.position.x - width / 2
                y: handler.point.position.y - height / 2
                width: 20
                height: width
                radius: width / 2
            }
        }
    }
}
