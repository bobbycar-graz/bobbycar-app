import QtQuick 2.15
import QtQuick.Controls 2.15

GamePage {
    id: livedatePage

    errorMessage: deviceHandler.error
    infoMessage: deviceHandler.info

    property real avgSpeed: (deviceHandler.frontLeftSpeed + deviceHandler.frontRightSpeed + deviceHandler.backLeftSpeed + deviceHandler.backRightSpeed) / 4
    property real avgVoltage: (deviceHandler.frontVoltage + deviceHandler.backVoltage) / 2
    property real totalCurrent: deviceHandler.frontLeftDcLink + deviceHandler.frontRightDcLink + deviceHandler.backLeftDcLink + deviceHandler.backRightDcLink
    property real totalPower: totalCurrent * avgVoltage

    function close()
    {
        deviceHandler.disconnectService();
        app.prevPage();
    }

    Rectangle {
        id: container
        anchors.centerIn: parent
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(livedatePage.width, livedatePage.height-GameSettings.fieldHeight*4) - 2*GameSettings.fieldMargin
        height: livedatePage.height-GameSettings.fieldHeight*4
        radius: GameSettings.buttonRadius
        color: GameSettings.viewColor

        Flickable {
            id: flickable

            anchors.fill: parent
            //spacing: GameSettings.buttonRadius

            contentWidth: contentColumn.width
            contentHeight: contentColumn.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            Column {
                id: contentColumn

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: GameSettings.hugeFontSize * 2
                    color: GameSettings.textColor
                    text: Number(avgSpeed).toLocaleString(Qt.locale()) + 'km/h'
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: GameSettings.hugeFontSize * 2
                    color: GameSettings.textColor
                    text: Number(totalCurrent).toLocaleString(Qt.locale()) + 'A'
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: GameSettings.hugeFontSize * 2
                    color: GameSettings.textColor
                    text: Number(totalPower>1000?(totalPower/1000):totalPower).toLocaleString(Qt.locale()) + (totalPower > 1000 ? "kW" : "W")
                }

                Text {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                    text: "Front: " + Number(deviceHandler.frontVoltage).toLocaleString(Qt.locale()) + "V / " + Number(deviceHandler.frontTemperature).toLocaleString(Qt.locale()) + "°C"
                    color: GameSettings.textColor
                    minimumPixelSize: 10
                    font.pixelSize: GameSettings.mediumFontSize
                }

                Grid {
                    columns: 2
                    spacing: 10

                    LivedataMotor {
                        width: (container.width/2)-10;

                        error: deviceHandler.frontLeftError
                        speed: deviceHandler.frontLeftSpeed
                        dcLink: deviceHandler.frontLeftDcLink
                        voltage: deviceHandler.frontVoltage
                    }

                    LivedataMotor {
                        width: (container.width/2)-10;

                        error: deviceHandler.frontRightError
                        speed: deviceHandler.frontRightSpeed
                        dcLink: deviceHandler.frontRightDcLink
                        voltage: deviceHandler.frontVoltage
                    }

                    LivedataMotor {
                        width: (container.width/2)-10;

                        error: deviceHandler.backLeftError
                        speed: deviceHandler.backLeftSpeed
                        dcLink: deviceHandler.backLeftDcLink
                        voltage: deviceHandler.backVoltage
                    }

                    LivedataMotor {
                        width: (container.width/2)-10;

                        error: deviceHandler.backRightError
                        speed: deviceHandler.backRightSpeed
                        dcLink: deviceHandler.backRightDcLink
                        voltage: deviceHandler.backVoltage
                    }
                }

                Text {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                    text: "Back: " + Number(deviceHandler.backVoltage).toLocaleString(Qt.locale()) + "V / " + Number(deviceHandler.backTemperature).toLocaleString(Qt.locale()) + "°C"
                    //visible: deviceHandler.alive
                    color: GameSettings.textColor
                    minimumPixelSize: 10
                    font.pixelSize: GameSettings.mediumFontSize
                }

                Row {
                    Label {
                        text: 'iMotMax:'
                        color: GameSettings.textColor
                        minimumPixelSize: 10
                        font.pixelSize: GameSettings.mediumFontSize
                    }

                    SpinBox {
                        value: 50
                    }
                }

                Row {
                    Label {
                        text: 'iDcMax:'
                        color: GameSettings.textColor
                        minimumPixelSize: 10
                        font.pixelSize: GameSettings.mediumFontSize
                    }

                    SpinBox {
                        value: 50
                    }
                }
            }
        }
    }

    GameButton {
        id: startButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: GameSettings.fieldMargin
        width: container.width
        height: GameSettings.fieldHeight
        radius: GameSettings.buttonRadius

        onClicked: app.showPage("RemoteControl.qml")

        Text {
            anchors.centerIn: parent
            font.pixelSize: GameSettings.tinyFontSize
            text: qsTr("REMOTE")
            color: startButton.enabled ? GameSettings.textColor : GameSettings.disabledTextColor
        }
    }
}
