import QtQuick 2.0

Item {
  property string spacing: "      "
    property string combined: text + spacing
    property string display: combined.substring(step) + combined.substring(0, step)
    property int step: 0

    Timer {
      interval: 200
      running: true
      repeat: true
      onTriggered: parent.step = (parent.step + 1) % parent.combined.length
    }

    Text {
      text: parent.display
    }
  }
