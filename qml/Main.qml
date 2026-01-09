import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 1080
    height: 1920

    Column {
        anchors.centerIn: parent
        spacing: 40

        Button {
            text: "拍照并生成报纸"
            onClicked: backend.capture()
        }

        Image {
            width: 600
            height: 800
            // source: "file:///data/output/final.jpg"
            fillMode: Image.PreserveAspectFit
        }
    }
}
