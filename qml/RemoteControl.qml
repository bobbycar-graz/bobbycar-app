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

        property real maxSpeed: 100
        property real remoteControlFrontLeft: maxSpeed * handler.relativeY * (handler.relativeX < 0 ? 1 + handler.relativeX : 1)
        property real remoteControlFrontRight: maxSpeed * handler.relativeY * (handler.relativeX > 0 ? 1 - handler.relativeX : 1)
        property real remoteControlBackLeft: 0
        property real remoteControlBackRight: 0

        onRemoteControlFrontLeftChanged: deviceHandler.remoteControlFrontLeft = remoteControlFrontLeft
        onRemoteControlFrontRightChanged: deviceHandler.remoteControlFrontRight = remoteControlFrontRight
        onRemoteControlBackLeftChanged: deviceHandler.remoteControlBackLeft = remoteControlBackLeft
        onRemoteControlBackRightChanged: deviceHandler.remoteControlBackRight = remoteControlBackRight

        Rectangle {
            parent: container
            color: "white"
            anchors.centerIn: parent
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: 20
            height: width
            radius: width / 2
        }

        Text {
            anchors.top: parent.top
            minimumPixelSize: 10
            font.pixelSize: GameSettings.mediumFontSize
            color: GameSettings.textColor
            text: {
                return "x:" + handler.relativeX.toFixed(1) + " y:" + handler.relativeY.toFixed(1) + "\n" +
                        "fl:" + Math.round(container.remoteControlFrontLeft) + " fr:" + Math.round(container.remoteControlFrontRight) + "\n" +
                        "bl:" + Math.round(container.remoteControlBackLeft) + " br:" + Math.round(container.remoteControlBackRight);
            }
        }

        PointHandler {
            id: handler
            //acceptedDevices: PointerDevice.TouchScreen
            property real clampedX: handler.active ? Math.min(Math.max(handler.point.position.x, 0), container.width) : (container.width / 2)
            property real clampedY: handler.active ? Math.min(Math.max(handler.point.position.y, 0), container.height) : (container.height / 2)
            property real relativeX: ((clampedX / container.width) - 0.5) * 2
            property real relativeY: ((clampedY / container.height) - 0.5) * -2

            target: Rectangle {
                parent: container
                color: "red"
                visible: handler.active
                x: handler.clampedX - width / 2
                y: handler.clampedY - height / 2
                width: 50
                height: width
                radius: width / 2
            }

            onActiveChanged: deviceHandler.remoteControlActive = handler.active
        }
    }
}
