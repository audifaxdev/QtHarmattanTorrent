#ifndef MAINTORRENTMANAGER_H
#define MAINTORRENTMANAGER_H

#include <QObject>
#include "qtimer.h"
#include "torrentclient.h"
#include "qobjectlistmodel.h"
#include "metainfo.h"

class MainTorrentManager;

class MiniMetaInfo : public QObject
{
    Q_OBJECT
    friend class MainTorrentManager;
public:
    explicit MiniMetaInfo(QObject *parent = 0){}
    Q_INVOKABLE QString torrentFile(){return m_torrentFile ;}
    Q_INVOKABLE QString announceUrl(){return m_announceUrl ;}
    Q_INVOKABLE QString comment(){return m_comment ;}
    Q_INVOKABLE QString creator(){return m_creator ;}
    Q_INVOKABLE quint64 totalSize(){return m_totalSize ;}
    Q_INVOKABLE bool singleFile(){return m_singleFile ;}
    Q_INVOKABLE QStringList files(){return m_files ;}

protected:
    void setMetaInfo(MetaInfo& metaInfo, QString torrentFile){
        clear();
        m_announceUrl = metaInfo.announceUrl();
        m_torrentFile = torrentFile;
        m_comment = metaInfo.comment();
        m_creator = metaInfo.createdBy();
        m_totalSize = metaInfo.totalSize();
        bool m_singleFile =  (metaInfo.fileForm() == MetaInfo::SingleFileForm);
        if(metaInfo.fileForm() == MetaInfo::SingleFileForm){
            m_files << metaInfo.singleFile().name;
        }else{
            foreach (MetaInfoMultiFile file, metaInfo.multiFiles()) {
                m_files << metaInfo.name();
            }
        }
    }
    void clear(){
        m_torrentFile = "";
        m_announceUrl = "";
        m_comment = "";
        m_creator = "";
        m_totalSize = 0;
        bool m_singleFile = true;
        m_files.clear();
    }
private:
    QString m_torrentFile;
    QString m_announceUrl;
    QString m_comment;
    QString m_creator;
    quint64 m_totalSize;
    bool m_singleFile;
    QStringList m_files;
};

class MainTorrentManager : public QObject
{
    Q_OBJECT

public:
    explicit MainTorrentManager(QObject *parent = 0);
    const TorrentClient *clientForRow(int row) const;
    bool addTorrent(QString);
    Q_INVOKABLE QObjectListModel* getJobs();
    Q_INVOKABLE MiniMetaInfo* metaInfoForTorrent(QString);
    MiniMetaInfo miniInfo;
    QObjectListModel jobs;

Q_SIGNAL void loadingFinished();

public slots:
    void closingRequest();

private slots:
    void loadSettings();
    void saveSettings();
    void removeTorrent(int);
    void pauseTorrent(int);
    void torrentStopped();
    void torrentError(TorrentClient::Error error);

    void updateState(TorrentClient::State state);
    void updatePeerInfo();
    void updateProgress(int percent);
    void updateDownloadRate(int bytesPerSecond);
    void updateUploadRate(int bytesPerSecond);

    void setUploadLimit(int bytes);
    void setDownloadLimit(int bytes);

private:
    int rowOfClient(TorrentClient *client) const;
    bool addTorrent(const QString &fileName, const QString &destinationFolder,
                    const QByteArray &resumeState = QByteArray());

    int uploadLimit;
    int downloadLimit;
    bool saveChanges;
    QString lastDirectory;

};

#endif // MAINTORRENTMANAGER_H
