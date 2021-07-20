import QtQuick 2.15

Rectangle {
    color: GameSettings.delegate1Color
    height: width*0.8

    property int error;
    property real speed;
    property real dcLink;
    property real voltage;
    property real power: voltage * dcLink;

    Column {
        Text {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: ["OK", "HALL miss", "HALL short", "MOTOR"][error] + " (" + error + ")";
            color: error == 0 ? "green" : "red"
            minimumPixelSize: 10
            font.pixelSize: GameSettings.mediumFontSize
        }

        Text {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: Number(speed).toLocaleString(Qt.locale()) + "km/h"
            color: GameSettings.textColor
            minimumPixelSize: 10
            font.pixelSize: GameSettings.mediumFontSize
        }

        Text {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: Number(dcLink).toLocaleString(Qt.locale()) + "A"
            color: GameSettings.textColor
            minimumPixelSize: 10
            font.pixelSize: GameSettings.mediumFontSize
        }

        Text {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: Number(power>1000?(power/1000):power).toLocaleString(Qt.locale()) + (power > 1000 ? "kW" : "W")
            color: GameSettings.textColor
            minimumPixelSize: 10
            font.pixelSize: GameSettings.mediumFontSize
        }
    }
}
