import QtQuick 1.1
import com.nokia.meego 1.0

//function getPicStateSrc(){}
PageStackWindow{
    id:rootWindow
    property variant root: rootWindow
    Component.onCompleted: {
        theme.inverted = true;
        console.log("HEIGHT = ");
        console.log(this.width);
    }
    showStatusBar: true
    showToolBar: true
    initialPage: SplashPage{

    }

}
