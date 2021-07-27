import QtQuick 2.15
import "."

Item {
    id: root
    anchors.fill: parent

    property bool appIsReady: false
    property bool splashIsReady: false

    property bool ready: appIsReady && splashIsReady
    onReadyChanged: if (ready) readyToGo();

    signal readyToGo()

    function appReady()
    {
        appIsReady = true
    }

    function errorInLoadingApp()
    {
        Qt.quit()
    }

    Image {
        id: bobbycar
        anchors.centerIn: parent
        width: Math.min(parent.height, parent.width)*0.6
        height: GameSettings.heightForWidth(width, sourceSize)
        source: "images/logo.png"

        SequentialAnimation{
            id: bobbycarAnim
            running: true
            loops: Animation.Infinite
            alwaysRunToEnd: true
            PropertyAnimation { target: bobbycar; property: "scale"; to: 1.2; duration: 500; easing.type: Easing.InQuad }
            PropertyAnimation { target: bobbycar; property: "scale"; to: 1.0; duration: 500; easing.type: Easing.OutQuad }
        }
    }

    Timer {
        id: splashTimer
        interval: 1000
        onTriggered: splashIsReady = true
    }

    Component.onCompleted: splashTimer.start()
}
