import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    property string collectionName: collectionField.text

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        TextField {
            id: collectionField
            placeholderText: "Collection name (e.g., MyBatch01)"
        }

        TextArea {
            id: linksArea
            placeholderText: "Paste links (one per line, or separated by spaces)"
            wrapMode: TextArea.WrapAnywhere
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            onTextChanged:
            {

            }
        }


        RowLayout {
            Button {
                text: "Queue links"
                onClicked: {
                    // let links =convertAllHttpsToHttp(linksArea.text);
                    // linksArea.text=links
                    backend.queueLinks(collectionField.text, linksArea.text)
                }
            }
            Button {
                text: "Start downloads"
                onClicked: backend.start()
            }
            Button {
                text: "Pause all"
                onClicked: backend.pauseAll()
            }
            Button {
                text: "Resume all"
                onClicked: backend.resumeAll()
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: backend.items
            delegate: Rectangle {
                width: list.width
                height: 72
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    Label { text: model.url; elide: Label.ElideRight; Layout.fillWidth: true }
                    ProgressBar {
                        value: model.totalBytes > 0 ? model.bytesReceived / model.totalBytes : 0
                        Layout.preferredWidth: 160
                    }
                    Label { text: model.state }
                    Button {
                        text: "Pause"
                        enabled: model.state === "Downloading"
                        onClicked: backend.pause(model.id)
                    }
                    Button {
                        text: "Resume"
                        enabled: model.state === "Paused"
                        onClicked: backend.resume(model.id)
                    }
                    Button {
                        text: "Cancel"
                        onClicked: backend.cancel(model.id)
                    }
                }
            }
        }
    }
    /**
     * Convert all HTTPS links in a text to HTTP
     * @param {string} text - The input text containing links
     * @returns {string} - The text with all https:// replaced by http://
     */
    function convertAllHttpsToHttp(text) {
      if (typeof text !== "string") return "";
      // Replace every occurrence of https:// with http://
      return text.replace(/https:\/\//g, "http://");
    }
}
