import QtQuick 1.0
import Qt.labs.folderlistmodel 1.0
import com.nokia.meego 1.0

ListView {
    id:list
    property string pathView
    property variant filterView
    property variant myListView : list
    signal selectedPath(string filepath)
    FolderListModel {
        id: folderModel
        folder: pathView
        nameFilters: filterView
        showDirs:false
        showDotAndDotDot:false
    }
    highlight: Component{
        Rectangle{
            height:45
            width: parent.width
            color:"silver"
        }
    }
    delegate: Item{
        id: fileDelegate
        height:45
        width: parent.width
        MouseArea{
            anchors.fill: parent
            onClicked: {
                myListView.currentIndex = index;
                myListView.selectedPath(filename.text);
            }
        }
        Rectangle{
            width: parent.width
            anchors{
                top:parent.top
                right:parent.right
                left:parent.left
                bottom:parent.bottom
            }
            color: "#00000000"
       }
       Text {
            id: filename;
            height:45;
            width:parent.width;
            color:"grey"
            font.pixelSize: 35
            text:{ return fileName;}
       }
    }
    model: folderModel
    focus: true
}
