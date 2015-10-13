import QtQuick 1.0
import Qt.labs.folderlistmodel 1.0
import com.nokia.meego 1.0


Page {
    property string  path: ""
    Component.onCompleted:{
        console.log("Confirm component completed ");
        console.log(path);
        qtorrentApp.metaInfoForTorrent(path);
        if(miniInfo.totalSize() == 0) pageStack.pop();

        torrentFilepath.text = miniInfo.torrentFile();
    }
    anchors.fill: parent
    orientationLock: PageOrientation.Automatic
    id : torrentFileList
    tools: ToolBarLayout {
         id : mainToolBar
         ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop(); }
         ToolIcon { id:goToValidation; iconId:  "toolbar-done"; visible:false; onClicked: ;}
    }
    MainHeaderBar{
        id: headerBar
        lblText:"Add a new torrent :"
    }
    Rectangle{
        id: formValidate
        height: parent.height
        anchors.top: headerBar.bottom
        width:parent.width - 20;
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors{
            leftMargin: 10
            rightMargin:10
        }
        Text{
            id:torrentFilepath
        }
        Text{
            id:torrentAnnounce
        }
        Text{
            id:torrentComment
        }
        Text{
            id:torrentCreator
        }
        Text{
            id:torrentSize
        }
        ListView{
            id:newTorrentFiles

        }
    }
}
