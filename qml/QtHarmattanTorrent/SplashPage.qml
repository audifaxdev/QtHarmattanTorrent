import com.nokia.meego 1.0
import QtQuick 1.0

Page {
    id: splashPage
    anchors.fill: parent
    anchors{
        top:parent.top
        left:parent.left
        right:parent.right
        bottom:parent.bottom
    }

    PropertyAnimation { id: startSplash; target: bottomSplash; property: "opacity"; to: 1 ; duration: 300}
    PropertyAnimation { id: continueSplash; target: middleSplash; property: "opacity"; to: 1 ; duration: 1500;}
    PropertyAnimation { id: finishSplash; target: topSplash; property: "opacity"; to: 1 ; duration: 1500;}

    function finishLoadSplash(){
        console.log("SplashPage::finishLoadSplash");
        finishSplash.start();
    }
    Connections{
        target: qtorrentApp
        onLoadingFinished:finishLoadSplash()
    }
    function terminateSplash(){
        pageStack.push(Qt.resolvedUrl("PageListTorrent.qml"));
    }
    Component.onCompleted: {startSplash.start();}

    orientationLock: PageOrientation.Automatic

    Rectangle{
        id: splashContainer
        color: "#00000000"
        width:480; height:480;
        z:1
        anchors{
            top:parent.top
            left:parent.left
            right:parent.right
            bottom:parent.bottom
        }
        //anchors.fill: parent
        state: (screen.currentOrientation == Screen.Portrait) ? "portrait" : "landscape"

        states: [
            State {
                name: "landscape"
                PropertyChanges { target: splashContainer; anchors{topMargin:0;  bottomMargin:0;leftMargin:187;rightMargin:187}}
            },
            State {
                name: "portrait"
                PropertyChanges { target: splashContainer; anchors{topMargin:187;  bottomMargin:187;leftMargin:0;rightMargin:0}}
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "anchors.topMargin, anchors.leftMargin, anchors.rightMargin, anchors.bottomMargin"; duration: 500 }
        }

        Image{
            id:bottomSplash
            width:480
            height:480
            z:2
            opacity: 0
            fillMode: Image.PreserveAspectCrop
            smooth: true
            source: "qrc:/images/images/SplashScreenBack.png"
            clip: true
            anchors.fill: parent
            onOpacityChanged: {if(opacity === 100) continueSplash.start();}
        }
        Image{
            id:middleSplash
            width:480
            height:480
            z:3
            opacity: 0
            fillMode: Image.PreserveAspectCrop
            smooth: true
            source: "qrc:/images/images/SplashScreenb.png"
            clip: true
            anchors.fill: bottomSplash
        }
        Image{
            id:topSplash
            width:480
            height:480
            z:4
            opacity: 0
            fillMode: Image.PreserveAspectCrop
            smooth: true
            source: "qrc:/images/images/SplashScreent.png"
            clip: true
            onOpacityChanged: if(opacity === 100) terminateSplash();
            anchors.fill: middleSplash
        }
    }
}
