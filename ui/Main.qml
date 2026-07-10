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
    color:"#313338"
    Column
    {
        width: parent.width/1.50
        height: parent.height
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

        Text
        {
            text: "Log:"
            opacity: 0.7
            color:"white"
        }
        ScrollView
        {
            id: logScroll
            width: parent.width
            height: 180

            ScrollBar.vertical: ScrollBar {
                   width: 8

                   policy: ScrollBar.AsNeeded

                   contentItem: Rectangle {
                       implicitWidth: 8
                       radius: width / 2
                       color: parent.pressed
                                ? "#7289DA"
                                : parent.hovered
                                  ? "#5B6EAE"
                                  : "#4E5D94"
                   }

                   background: Rectangle {
                       radius: width / 2
                       color: "#1E1F22"
                       opacity: 0.5
                   }
               }

            TextArea
            {
                readOnly: true
                text: updater.log
                color: "white"
                wrapMode: TextEdit.Wrap
            }
        }



        Button
        {
            id:closeButton
            text: "Close"
            visible: false
            anchors.horizontalCenter: parent.horizontalCenter

            onClicked:
            {
                updater.closeApplication()
            }

            background: Rectangle
            {
                radius: 4

                color: closeButton.down
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
                text: "Failed to update"
                color: "white"
                font.pixelSize: 24
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                elide: Text.ElideRight
            }

            Text
            {
                text: "please check your network or try again later\n read log for more information."
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
                    closeButton.visible=true
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

        property string updateSize;
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
                text: "Update size " +confirmToUpdate.updateSize
                color: "#b5bac1"
                font.pixelSize: 20
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: parent.width
                elide: Text.ElideRight
            }

            Text
            {
                text: "Update is available, please choose update or cancel"
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
                    text: "Cancel"
                    Layout.fillWidth: true

                    onClicked:
                    {
                        updater.cancelUpdate();
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



    Connections
    {
        target:updater
        onNothingToDo: function()
        {
            closeButton.visible=true
        }

        onConfirmUpdateOrCancel: function(sizeToDownload)
        {
            confirmToUpdate.updateSize=sizeToDownload
            confirmToUpdate.open()
        }

        onUpdatedCanceled: function()
        {
            closeButton.visible=true
        }

        onShowCloseButton: function()
        {
            failedToUpdatePopup.open() //show a popup say error check logs.
        }

        onUpdatedSuccessfully: function()
        {
            closeButton.visible=true
        }
    }


}
