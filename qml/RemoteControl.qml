import QtQuick 2.15
import QtQuick.Controls 2.15

GamePage {
    id: remoteControlPage

    errorMessage: deviceHandler.error
    infoMessage: deviceHandler.info

    property real avgSpeed: (deviceHandler.frontLeftSpeed + deviceHandler.frontRightSpeed + deviceHandler.backLeftSpeed + deviceHandler.backRightSpeed) / 4
    property real avgVoltage: (deviceHandler.frontVoltage + deviceHandler.backVoltage) / 2
    property real totalCurrent: deviceHandler.frontLeftDcLink + deviceHandler.frontRightDcLink + deviceHandler.backLeftDcLink + deviceHandler.backRightDcLink
    property real totalPower: totalCurrent * avgVoltage

    Column {
        anchors.centerIn: parent
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(remoteControlPage.width, remoteControlPage.height-GameSettings.fieldHeight*4) - 2*GameSettings.fieldMargin

        Column {
            width: parent.width

            Text {
                font.pixelSize: GameSettings.hugeFontSize
                color: GameSettings.textColor
                text: Number(avgSpeed).toLocaleString(Qt.locale()) + 'km/h'
            }

            Text {
                font.pixelSize: GameSettings.hugeFontSize
                color: GameSettings.textColor
                text: Number(totalCurrent).toLocaleString(Qt.locale()) + 'A'
            }

            Text {
                font.pixelSize: GameSettings.hugeFontSize
                color: GameSettings.textColor
                text: Number(totalPower>1000?(totalPower/1000):totalPower).toLocaleString(Qt.locale()) + (totalPower > 1000 ? "kW" : "W")
            }

            Text {
                font.pixelSize: GameSettings.hugeFontSize
                color: GameSettings.textColor
                text: Number(avgVoltage).toLocaleString(Qt.locale()) + 'V'
            }
        }

        Rectangle {
            id: container
            width: parent.width
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
            width: parent.width
            columns: 2
            horizontalItemAlignment: Grid.AlignHCenter

            SpinBox {
                id: frontLeftRightSpinbox
                value: 100
                editable: true
                from: -1000
                to: 1000
            }

            SpinBox {
                id: frontUpDownSpinbox
                value: 75
                editable: true
                from: -1000
                to: 1000
            }

            SpinBox {
                id: backLeftRightSpinbox
                value: 0
                editable: true
                from: -1000
                to: 1000
            }

            SpinBox {
                id: backUpDownSpinbox
                value: 100
                editable: true
                from: -1000
                to: 1000
            }
        }
    }
}
