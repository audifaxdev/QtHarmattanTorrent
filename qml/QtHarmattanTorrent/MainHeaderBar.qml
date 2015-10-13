import QtQuick 1.0
import com.nokia.meego 1.0

Rectangle {

    property string  lblText
        id: headerBar
        width: parent.width
        color: "black"
        opacity:1
        z:1
        height: {return UiConstants.HeaderDefaultHeightPortrait;}

        anchors {
            top:parent.top
            left:parent.left
            right:parent.right
        }
        //Title bar label
        Label{
            id: headerLbl
            text: lblText
            font: UiConstants.HeaderFont
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            width:parent.width
            anchors {
                top:parent.top
                left:parent.left
                right:parent.right
                topMargin: {
                    if(screen.currentOrientation == Screen.Portrait)
                        return UiConstants.HeaderDefaultTopSpacingPortrait;
                    else
                        return UiConstants.HeaderDefaultTopSpacingLandscape
                }
                bottomMargin:{
                    if(screen.currentOrientation == Screen.Portrait)
                        return UiConstants.HeaderDefaultBottomSpacingPortrait;
                    else
                        return UiConstants.HeaderDefaultBottomSpacingLandscape
                }
            }
        }
        //bottom border of Title bar
        Rectangle{
            id:headerBorder
            height:1
            color:"grey"

            width:parent.width - 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors{
                leftMargin: 10
                rightMargin:10
            }

        }
    }
