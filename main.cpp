#include <QtGui/QApplication>

#include "qmlapplicationviewer.h"
#include "maintorrentmanager.h"
#include <QDeclarativeContext>

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));
    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());

    //Launch Torrent manager instance
    MainTorrentManager manager;

    //Sharing data to js
    QDeclarativeContext *ctxt = viewer.data()->rootContext();
    ctxt->setContextProperty("qtorrentApp", &manager);
    ctxt->setContextProperty("miniInfo", &manager.miniInfo);
    ctxt->setContextProperty("qtorrentModel", &manager.jobs);

    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer->setMainQmlFile(QLatin1String("qml/QtHarmattanTorrent/main.qml"));
    viewer->showExpanded();

    return app->exec();
}
