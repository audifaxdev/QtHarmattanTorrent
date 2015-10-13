import QtQuick 1.0
import com.nokia.meego 1.0

Page{
    anchors.fill: parent
    orientationLock: PageOrientation.Automatic
    id : torrentPageList
    tools: ToolBarLayout {
         id : mainToolBar
         ToolIcon { iconId: "toolbar-add"; onClicked:pageStack.push(Qt.resolvedUrl("NewTorrentDialog.qml"));}
         ToolIcon {
            iconId: "toolbar-mediacontrol-pause";
            visible:false
         }
         ToolIcon {
            iconId: "toolbar-mediacontrol-stop";
            visible:false
         }
         ToolIcon { iconId: "toolbar-view-menu";}
    }
    MainHeaderBar{
        id:mainHeaderBar
        lblText:"Qtorrents"
    }
    //Main Torrent list view
    TorrentListComponent{
        z:0
        anchors.top:mainHeaderBar.bottom
//                    x:{
//                        if(screen.currentOrientation == Screen.Portrait)
//                            return 10;
//                        else
//                            return 10;
//                    }
//                    y:{}
        width:parent.width - 20;
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors{
            leftMargin: 10
            rightMargin:10
        }
    }
}
