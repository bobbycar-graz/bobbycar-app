import QtQuick 2.15
import QtQuick.Controls 2.15

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

        property real remoteControlFrontLeft: (handler.relativeX * frontLeftRightSpinbox.value) + (handler.relativeY * frontUpDownSpinbox.value)
        property real remoteControlFrontRight: (-handler.relativeX * frontLeftRightSpinbox.value) + (handler.relativeY * frontUpDownSpinbox.value)
        property real remoteControlBackLeft: (handler.relativeX * backLeftRightSpinbox.value) + (handler.relativeY * backUpDownSpinbox.value)
        property real remoteControlBackRight: (-handler.relativeX * backLeftRightSpinbox.value) + (handler.relativeY * backUpDownSpinbox.value)

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

    Grid {
        anchors.top: container.bottom
        anchors.left: container.left
        anchors.right: container.right
        columns: 2

        SpinBox {
            id: frontLeftRightSpinbox
            value: 100
            editable: true
            from: 0
            to: 1000
        }

        SpinBox {
            id: frontUpDownSpinbox
            value: 100
            editable: true
            from: 0
            to: 1000
        }

        SpinBox {
            id: backLeftRightSpinbox
            value: 100
            editable: true
            from: 0
            to: 1000
        }

        SpinBox {
            id: backUpDownSpinbox
            value: 100
            editable: true
            from: 0
            to: 1000
        }
    }
}
