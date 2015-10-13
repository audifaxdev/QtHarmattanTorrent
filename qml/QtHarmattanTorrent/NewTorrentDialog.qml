import QtQuick 1.0
import Qt.labs.folderlistmodel 1.0
import com.nokia.meego 1.0

Page {
    property string  fileSelected: ""
    function checkTorrentPath(path){
        console.log("checkTorrentPath - " + path);
        if(path !== ""){
            goToValidation.visible = true;
            fileSelected = "/home/user/MyDocs/Downloads/" + path ;
            console.log("fileSelected :" + path);
        }
    }
    anchors.fill: parent
    orientationLock: PageOrientation.Automatic
    id : torrentFileList
    tools: ToolBarLayout {
         id : mainToolBar
         ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop(); }
         ToolIcon {
             id:goToValidation;
             iconId:  "toolbar-next";
             visible:false;
             onClicked: pageStack.push(Qt.resolvedUrl("PageConfirmTorrent.qml"),
                                       {path: fileSelected});
         }
    }
    MainHeaderBar{
        id: headerBar
        lblText:"Select torrent file to add :"
    }
    FileView{
        id: torrentSelection
        pathView: "/home/user/MyDocs/Downloads/"
        filterView:  ["*.torrent"]
        onSelectedPath: checkTorrentPath(filepath)
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
    }
}
