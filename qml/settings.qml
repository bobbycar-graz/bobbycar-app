import QtQuick 2.15
import QtQuick.Controls 2.15
import Shared 1.0

GamePage {
    id: livedatePage

    errorMessage: deviceHandler.error
    infoMessage: deviceHandler.info

    function close()
    {
        deviceHandler.disconnectService();
        app.prevPage();
    }

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        anchors.topMargin: GameSettings.fieldMargin + messageHeight
        anchors.bottomMargin: GameSettings.fieldMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - GameSettings.fieldMargin*2
        color: GameSettings.viewColor
        radius: GameSettings.buttonRadius


        Text {
            id: title
            width: parent.width
            height: GameSettings.fieldHeight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: GameSettings.textColor
            font.pixelSize: GameSettings.mediumFontSize
            text: qsTr("WIFIS")

            BottomLine {
                height: 1;
                width: parent.width
                color: "#898989"
            }
        }


        ListView {
            id: wifis
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: title.bottom
            //model: deviceFinder.devices // Needs to be changed
            clip: true

            delegate: Rectangle {
                id: box
                height:GameSettings.fieldHeight * 1.2
                width: parent.width
                color: index % 2 === 0 ? GameSettings.delegate1Color : GameSettings.delegate2Color

                MouseArea {
                anchors.fill: parent
                    onClicked: {
                        //Handle editor
                    }
                }

                Text {
                    id: device
                    font.pixelSize: GameSettings.smallFontSize
                    text: modelData.ssid
                    anchors.top: parent.top
                    anchors.topMargin: parent.height * 0.1
                    anchors.leftMargin: parent.height * 0.1
                    anchors.left: parent.left
                    color: GameSettings.textColor
                }
/*
                Text {
                    id: deviceAddress
                    font.pixelSize: GameSettings.smallFontSize
                    text: modelData.deviceAddress
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.1
                    anchors.rightMargin: parent.height * 0.1
                    anchors.right: parent.right
                    color: Qt.darker(GameSettings.textColor)
                }*/
            }
        }
    }
}
