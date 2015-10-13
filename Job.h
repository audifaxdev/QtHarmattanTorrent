#ifndef JOB_H
#define JOB_H
#include <QAbstractListModel>
#include "torrentclient.h"
#include "maintorrentmanager.h"
#include "metainfo.h"

#define itemDbg(x) qDebug() << torrentFileName() << __PRETTY_FUNCTION__ << x

class Job : public QObject
{
    Q_OBJECT
    friend class MainTorrentManager;

    Q_PROPERTY(QString m_torrentFileName READ torrentFileName)
    Q_PROPERTY(QString m_destinationDirectory READ destinationDirectory)

    Q_PROPERTY(qint64 m_totalSize READ totalSize)
    Q_PROPERTY(qint64 m_totalDownloaded READ totalDownloaded)
    Q_PROPERTY(int m_percentProgress READ percentProgress NOTIFY percentProgressChanged)
    Q_PROPERTY(int m_currentUpSpeed READ currentUpSpeed NOTIFY currentUpSpeedChanged)
    Q_PROPERTY(int m_currentDownSpeed READ currentDownSpeed NOTIFY currentDownSpeedChanged)
    Q_PROPERTY(QString m_currentState READ currentState NOTIFY currentStateChanged)

public:
    explicit Job(QObject* parent = 0){}
    QString torrentFileName(){return m_torrentFileName;}
    QString destinationDirectory(){return m_destinationDirectory;}

    qint64  totalSize(){return m_totalSize;}
    qint64  totalDownloaded(){return m_totalDownloaded;}
    int  currentUpSpeed(){return m_currentUpSpeed;}
    int  currentDownSpeed(){return m_currentDownSpeed;}
    QString currentState(){return m_currentState;}
    int percentProgress(){return m_percentProgress;}

public slots:
    void progressChanged(int percent){
        itemDbg(percent);
        m_percentProgress = percent;
    }
    void upRateChanged(int r){
        itemDbg(r);
        m_currentUpSpeed = r;
    }
    void downRateChanged(int r){
        itemDbg(r);
        m_currentDownSpeed = r;
    }
    void stateChanged(TorrentClient::State state){
        Q_UNUSED(state)
        itemDbg(client->stateString());
        m_currentState = client->stateString();
        emit currentStateChanged(m_currentState);
    }

signals:
    void percentProgressChanged(int);
    void currentUpSpeedChanged(int);
    void currentDownSpeedChanged(int);
    void currentStateChanged(QString);

/*enum State {
    Idle,
    Paused,
    Stopping,
    Preparing,
    Searching,
    Connecting,
    WarmingUp,
    Downloading,
    Endgame,
    Seeding
};*/
protected:
    void setClient(TorrentClient* _client){
        client = _client;
        m_totalSize = client->metaInfo().totalSize();
        m_totalDownloaded = client->downloadedBytes();
        m_currentState = client->stateString();
        m_percentProgress = client->progress();
        itemDbg(m_totalDownloaded << "/" << m_totalSize);
        itemDbg( m_currentState);
        itemDbg( m_percentProgress);
        connect(client,SIGNAL(progressUpdated(int)),SIGNAL(percentProgressChanged(int)));
        connect(client,SIGNAL(progressUpdated(int)),SLOT(progressChanged(int)));

        connect(client,SIGNAL(stateChanged(TorrentClient::State)),SLOT(stateChanged(TorrentClient::State)));
        //connect(client,SIGNAL(stateChanged(TorrentClient::State)),SLOT(currentStateChanged(TorrentClient::State)));

        connect(client,SIGNAL(uploadRateUpdated(int)),SLOT(upRateChanged(int)));
        connect(client,SIGNAL(uploadRateUpdated(int)),SIGNAL(currentUpSpeedChanged(int)));

        connect(client,SIGNAL(downloadRateUpdated(int)),SLOT(downRateChanged(int)));
        connect(client,SIGNAL(downloadRateUpdated(int)),SIGNAL(currentDownSpeedChanged(int)));


    }
    void setTorrentFileName(QString _torrentFileName){
        QStringList xpld = _torrentFileName.split("/");
        xpld.size() > 1 ? m_torrentFileName = xpld.at(xpld.size() - 1) : m_torrentFileName = _torrentFileName;
        itemDbg(m_torrentFileName);
    }
    void setDestinationDirectory(QString _destinationDirectory){
        m_destinationDirectory = _destinationDirectory;
        itemDbg(m_destinationDirectory);
    }

    TorrentClient *client;
    QString m_torrentFileName;
    QString m_destinationDirectory;

private:
    qint64  m_totalSize;
    qint64  m_totalDownloaded;
    int  m_currentUpSpeed;
    int  m_currentDownSpeed;
    QString m_currentState;
    qint64 m_percentProgress;
};
#endif // JOB_H
