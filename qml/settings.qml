import QtQuick 2.15
import QtQuick.Controls 2.15

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
        id: container
        anchors.centerIn: parent
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(livedatePage.width, livedatePage.height-GameSettings.fieldHeight*4) - 2*GameSettings.fieldMargin
        height: livedatePage.height-GameSettings.fieldHeight*4
        radius: GameSettings.buttonRadius
        color: GameSettings.viewColor

        Text {
            id: test
            text: qsTr("text")
            color: "white"
        }
    }
}
