import QtQuick
import QtQuick.Controls

import QtQuick.Layouts
import QtQuick.Controls.Material
import BeanChatUpdater 1.0
Window
{
    width: 640
    height: 480
    visible: true
    title: qsTr("BeanChatUpdater")
    color:"#05070b"
    Column
    {
        anchors.centerIn: parent
        spacing: 18

        Text
        {
            text: updater.title
            font.pixelSize: 28
            color:"white"
        }

        Text
        {
            text: updater.description
            opacity: 0.7
            color:"white"
        }

        ProgressBar
        {
            width: parent.width
            value: updater.progress
        }

        ScrollView
        {
            id: logScroll
            width: parent.width
            height: 180

            TextArea
            {
                readOnly: true
                text: updater.log
                color: "white"
                wrapMode: TextEdit.Wrap
            }
        }
    }

    Connections
    {
        target:updater
        onStateChanged: function()
        {
            if(updater.state===UpdaterState.WaitForConfirmation)
                confirmToUpdate.open()
            else if(updater.state===UpdaterState.Error)
                failedToUpdatePopup.open()
        }
    }


    Popup
    {
        id: failedToUpdatePopup

        modal: true
        focus: true

        width: 400
        height: 200
        onClosed:
        {
            okButton.clicked()
        }

        anchors.centerIn: Overlay.overlay

        background: Rectangle
        {
            color: "#313338"
            radius: 8
            border.color: "#1e1f22"
        }

        ColumnLayout
        {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text
            {
                text: "Failed to check for update"
                color: "white"
                font.pixelSize: 24
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                elide: Text.ElideRight
            }

            Text
            {
                text: "please check your network or try again later"
                color: "#b5bac1"
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                elide: Text.ElideRight
            }

            Item { Layout.fillHeight: true }

            Button
            {
                id: okButton

                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                onClicked:
                {
                    updater.skipUpdate();
                    failedToUpdatePopup.close()
                }

                background: Rectangle
                {
                    radius: 4

                    color: okButton.down
                           ?  "#4752C4"
                           : "#5865F2"

                    border.width: 1
                    border.color: "#555"
                }

                contentItem: Text
                {
                    text: parent.text
                    color: "white"

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

        }
    }


    Popup
    {
        id: confirmToUpdate

        modal: true
        focus: true

        width: 400
        height: 350

        anchors.centerIn: Overlay.overlay

        background: Rectangle
        {
            color: "#313338"
            radius: 8
            border.color: "#1e1f22"
        }

        ColumnLayout
        {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text
            {
                text: "Confirm To Update"
                color: "white"
                font.pixelSize: 24
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                elide: Text.ElideRight
            }

            Text
            {
                text: "an update is available, please choose update or skip"
                color: "#b5bac1"
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                elide: Text.ElideRight
            }


            Item { Layout.fillHeight: true }


            RowLayout
            {
                Layout.fillWidth: true

                Button
                {
                    id:cancelButton
                    text: "Skip"
                    Layout.fillWidth: true

                    onClicked:
                    {
                        updater.skipUpdate();
                        confirmToUpdate.close()
                    }

                    background: Rectangle
                    {
                        radius: 4

                        color: cancelButton.down
                               ? "#3F4147"
                               : "transparent"

                        border.width: 1
                        border.color: "#555"
                    }

                    contentItem: Text
                    {
                        text: parent.text
                        color: "white"

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button
                {
                    id: acceptButton

                    text: "Update"
                    Layout.fillWidth: true

                    onClicked:
                    {
                        updater.confirmUpdate();
                        confirmToUpdate.close()
                    }

                    background: Rectangle
                    {
                        radius: 4

                        color: acceptButton.down
                               ?  "#4752C4"
                               : "#5865F2"

                        border.width: 1
                        border.color: "#555"
                    }

                    contentItem: Text
                    {
                        text: parent.text
                        color: "white"

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }


}
