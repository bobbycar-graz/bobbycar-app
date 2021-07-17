import QtQuick 2.5

GamePage {

    Column {
        anchors.centerIn: parent
        width: parent.width

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: GameSettings.hugeFontSize
            color: GameSettings.textColor
            text: qsTr("RESULT")
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: GameSettings.giganticFontSize*3
            color: GameSettings.textColor
            text: (deviceHandler.maxSpeed - deviceHandler.minSpeed).toFixed(0)
        }

        Item {
            height: GameSettings.fieldHeight
            width: 1
        }

        StatsLabel {
            title: qsTr("MIN")
            value: deviceHandler.minSpeed.toFixed(0)
        }

        StatsLabel {
            title: qsTr("MAX")
            value: deviceHandler.maxSpeed.toFixed(0)
        }

        StatsLabel {
            title: qsTr("AVG")
            value: deviceHandler.avgSpeed.toFixed(1)
        }


        StatsLabel {
            title: qsTr("DISTANCE")
            value: deviceHandler.distance.toFixed(3)
        }
    }
}
