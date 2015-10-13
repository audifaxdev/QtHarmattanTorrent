import QtQuick 1.0
import com.nokia.meego 1.0

ListView{
    id: torrentList
    property variant rootView: torrentList
    model: qtorrentModel
    spacing:4
    focus: true
    //highlightFollowsCurrentItem: false
    function roundVal(val){
            var dec = 1;
            var result = Math.round(val*Math.pow(10,dec))/Math.pow(10,dec);
            return result;
    }
    function formatSpeed(currentXSpeed){
        if(currentXSpeed >= 1000)
            return (roundVal(currentXSpeed / 1000)) + " MB/s";
        else
            return currentXSpeed + " KB/s";
    }
    function getPicStatePath(currentState,percentProgress){
        console.log("getPicStatePath(" + currentState + "," + percentProgress + ")");
        switch(currentState){
        case "downloading":
            return "qrc:/images/images/downloading66.png";
        case "stopped":

            if(percentProgress === 100)
                return "qrc:/images/images/stopped66.png";
            else
                return "qrc:/images/images/pause66.png";

        case "seeding":
            return "qrc:/images/images/up66.png";
        }
    }
    function formatName(name){
        if(name.length === undefined) return "";
        if(screen.currentOrientation == Screen.Portrait){
            if(name.length > 29)
                return name.substring(0,29) + "...";
            else
                return name;
        }else{
            if(name.length > 66)
                return name.substring(0,66) + "...";
            else
                return name;
        }
    }
    function torrentPercentage(totalDownloaded,totalSize){
        return (totalDownloaded * 100 / totalSize ) | 0;
    }
    highlight: Component {
         id: highlight
         Rectangle {
             opacity:0.33
             width: parent.width; height: 66
             color: "gold"; radius: 5
             //y: list.currentItem.y;
             //Behavior on y { SpringAnimation { spring: 2; damping: 0.1 } }
         }
     }
    delegate: Item {
        MouseArea{
            anchors.fill: parent
            onClicked: rootView.currentIndex = index
        }
        id: torrentItem
        height: 66
        width: parent.width

        Rectangle{
            id: bottomItemBorder
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            //anchors.leftMargin: 10
            width: parent.width
            height: 1
            color:"grey"
            opacity:0.75
        }
        Rectangle{
            color: "#00000000"
            id: leftContainer
            height:66
            width:66
            anchors.left: torrentItem.left

            Image{
                id: picState
                width: 66; height: 66
                fillMode: Image.PreserveAspectFit
                smooth:true
                anchors{
                    bottom:parent.bottom
                    right:parent.right
                }
                source:{rootView.getPicStatePath(torrentItem.currentState, torrentItem.percentProgress)}
            }
        }
        Rectangle{
            color: "#00000000"
            id: rightContainer
            height:66
            anchors.right: torrentItem.right
            anchors.left:leftContainer.right
            anchors.top:torrentItem.top
            //color: "lightsteelblue"
            Rectangle{
                color: "#00000000"

                id: namelblContainer
                anchors.top:parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height:33
                Label{
                    id: labelName
                    anchors.margins: UiConstants.DefaultMargin
                    y:5
                    text:{rootView.formatName(torrentItem.torrentFileName);}
                }
            }
            Rectangle{
                color: "#00000000"
                //color:"yellow"
                id:detailBlock
                anchors.top:namelblContainer.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height:33
                Rectangle{
                    color: "#00000000"
                    //border.color: "yellow"
                    //border.width: 1
                    id:upDetail
                    height:parent.height
                    width:100
                    anchors.top:parent.top
                    anchors.left: detailBlock.left
                    Image{

                        id:upImgthumb
                        width: 33; height: 33
                        fillMode: Image.PreserveAspectFit
                        smooth:true
                        anchors{
                            top:parent.top
                            left:parent.left
                        }
                        source:"qrc:/images/images/up66.png"
                    }
                    Text{
                        text: { rootView.formatSpeed(torrentItem.currentUpSpeed);}
                        color: "grey"
                        font.bold:false
                        font.pixelSize: 16

                        horizontalAlignment: "AlignHCenter"
                        verticalAlignment: "AlignVCenter"
                        anchors{
                            left:upImgthumb.right
                            bottom:parent.bottom
                            top:parent.top
                        }
                    }
                }
                Rectangle{
                    color: "#00000000"
                    id:downDetail
                    height:parent.height
                    width:100
                    anchors.top:parent.top
                    anchors.left: upDetail.right
                    Image{
                        id:downImgthumb
                        width: 33; height: 33
                        fillMode: Image.PreserveAspectFit
                        smooth:true
                        anchors{
                            top:parent.top
                            left:parent.left
                        }
                        source:"qrc:/images/images/down66.png"
                    }
                    Text{
                        text: { rootView.formatSpeed(torrentItem.currentDownSpeed);}
                        color: "grey"
                        font.bold:false
                        font.pixelSize: 16

                        horizontalAlignment: "AlignHCenter"
                        verticalAlignment: "AlignVCenter"
                        anchors{
                            left:downImgthumb.right
                            bottom:parent.bottom
                            top:parent.top
                        }
                    }
                }

                Rectangle{
                    color: "#00000000"
                    //border.color: "red"
                    //border.width: 1
                    id:progressBarContainer
                    height:parent.height
                    anchors.top:parent.top
                    anchors.left: downDetail.right
                    anchors.right: parent.right
                    ProgressBar{
                        id: torrentProgressBar
                        anchors.left: parent.left
                        anchors.right: parent.right
                        indeterminate: false
                        maximumValue: 100
                        minimumValue: 0
                        value: torrentItem.percentProgress
                    }
                    Text{
                        id:percText
                        color:"white"
                        font.pixelSize: 12
                        horizontalAlignment: "AlignHCenter"
                        verticalAlignment: "AlignVCenter"
                        text:{return torrentItem.percentProgress + "%";}
                        anchors{
                            left:parent.left
                            right:parent.right
                            bottom:parent.bottom
                            top:torrentProgressBar.bottom
                        }
                    }
                }
            }
        }
     }
}
