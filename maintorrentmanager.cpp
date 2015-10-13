#include <QApplication>
#include "maintorrentmanager.h"
#include "ratecontroller.h"
#include "Job.h"
#include "debug.h"
#include <QSettings>
#include <QDir>

static int rateFromValue(int value)
{
    int rate = 0;
    if (value >= 0 && value < 250) {
        rate = 1 + int(value * 0.124);
    } else if (value < 500) {
        rate = 32 + int((value - 250) * 0.384);
    } else if (value < 750) {
        rate = 128 + int((value - 500) * 1.536);
    } else {
        rate = 512 + int((value - 750) * 6.1445);
    }
    return rate;
}

MainTorrentManager::MainTorrentManager(QObject *parent) :
    QObject(parent),
    saveChanges(false),
    lastDirectory("/Users/demat-store/Desktop/")
    //lastDirectory("/home/user/MyDocs/Downloads/")
{
    connect(qApp,SIGNAL(aboutToQuit()),SLOT(closingRequest()));
    QMetaObject::invokeMethod(this, "loadSettings");
}
MiniMetaInfo* MainTorrentManager::metaInfoForTorrent(QString torrentFile){

    miniInfo.clear();
    MetaInfo metaInfo;
    if (torrentFile.isEmpty()) {
        return &miniInfo;
    }

    if (!torrentFile.isEmpty())
        lastDirectory = QFileInfo(torrentFile).absolutePath();

    QFile torrent(torrentFile);
    if (!torrent.open(QFile::ReadOnly) || !metaInfo.parse(torrent.readAll())) {
        miniInfo.clear();
        return &miniInfo;
    }
    miniInfo.setMetaInfo(metaInfo, torrentFile);
    return &miniInfo;

}

QObjectListModel* MainTorrentManager::getJobs(){
    return &jobs;
}

void MainTorrentManager::loadSettings()
{
    // Load base settings (last working directory, upload/download limits).
    QSettings settings("QtHarmattanTorrent", "Torrent");
    lastDirectory = settings.value("LastDirectory").toString();
    if (lastDirectory.isEmpty())
        lastDirectory = "/home/user/MyDocs/Downloads/";
    int up = settings.value("UploadLimit").toInt();
    int down = settings.value("DownloadLimit").toInt();
    uploadLimit = up;
    downloadLimit = down;

    // Resume all previous downloads.
    int size = settings.beginReadArray("Torrents");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QByteArray resumeState = settings.value("resumeState").toByteArray();
        QString fileName = settings.value("sourceFileName").toString();
        QString dest = settings.value("destinationFolder").toString();

        if (addTorrent(fileName, dest, resumeState)) {
            Job* lastJob = qobject_cast<Job*>(jobs.at(jobs.count() - 1));
            if(!lastJob) return;
            TorrentClient *client = lastJob->client;
            client->setDownloadedBytes(settings.value("downloadedBytes").toLongLong());
            client->setUploadedBytes(settings.value("uploadedBytes").toLongLong());
        }
    }
    emit loadingFinished();
    //In cas C++ is too fast, qml would not be able to catch the first signal so, do it again in 3 secs
    QTimer::singleShot(3000,this,SIGNAL(loadingFinished()));
}

void MainTorrentManager::saveSettings()
{
    if (!saveChanges)
      return;
    saveChanges = false;

    // Prepare and reset the settings
    QSettings settings("QtHarmattanTorrent", "Torrent");
    settings.clear();

    settings.setValue("LastDirectory", lastDirectory);
    settings.setValue("UploadLimit", uploadLimit);
    settings.setValue("DownloadLimit", downloadLimit);

    Job* job = 0;
    // Store data on all known torrents
    settings.beginWriteArray("Torrents");
    for (int i = 0; i < jobs.size(); ++i) {
        job = qobject_cast<Job*>(jobs.at(i));
        if(!job) return;
        settings.setArrayIndex(i);
        settings.setValue("sourceFileName", job->torrentFileName());
        settings.setValue("destinationFolder", job->destinationDirectory());
        settings.setValue("uploadedBytes", job->client->uploadedBytes());
        settings.setValue("downloadedBytes", job->client->downloadedBytes());
        settings.setValue("resumeState", job->client->dumpedState());
    }
    settings.endArray();
    settings.sync();
}

bool MainTorrentManager::addTorrent(const QString &fileName, const QString &destinationFolder,
                            const QByteArray &resumeState)
{
    //
    Job* job = 0;
    for(int i = 0; i < jobs.count();i++){
        job = qobject_cast<Job*>(jobs.at(i));
        if(!job) return false;
        if (job->torrentFileName() == fileName && job->destinationDirectory() == destinationFolder) {
            dbg("Already Downloading");
            return false;
        }
    }
    job = 0;
    // Create a new torrent client and attempt to parse the torrent data.
    TorrentClient *client = new TorrentClient(this);
    if (!client->setTorrent(fileName)) {
        dbg("Torrent cannot be opened" << fileName);
        delete client;
        return false;
    }
    client->setDestinationFolder(destinationFolder);
    client->setDumpedState(resumeState);

    // Setup the client connections.
    connect(client, SIGNAL(stateChanged(TorrentClient::State)), this, SLOT(updateState(TorrentClient::State)));
    connect(client, SIGNAL(peerInfoUpdated()), this, SLOT(updatePeerInfo()));
    connect(client, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
    connect(client, SIGNAL(downloadRateUpdated(int)), this, SLOT(updateDownloadRate(int)));
    connect(client, SIGNAL(uploadRateUpdated(int)), this, SLOT(updateUploadRate(int)));
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    connect(client, SIGNAL(error(TorrentClient::Error)), this, SLOT(torrentError(TorrentClient::Error)));

    // Add the client to the list of downloading jobs.
    job = new Job(&jobs);
    job->setClient(client);
    job->setTorrentFileName(fileName);
    job->setDestinationDirectory(destinationFolder);
    jobs.append(job);

    QString baseFileName = QFileInfo(fileName).fileName();
    if (baseFileName.toLower().endsWith(".torrent"))
        baseFileName.remove(baseFileName.size() - 8);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
    client->start();
    dbg("Starting torrent");
    return true;
}

const TorrentClient *MainTorrentManager::clientForRow(int row) const
{

    // Return the client at the given row.
    Job *job = qobject_cast<Job*>(jobs.at(row));
    if(!job) return 0;
    return job->client;
}

int MainTorrentManager::rowOfClient(TorrentClient *client) const
{

    // Return the row that displays this client's status, or -1 if the
    // client is not known.
    int row = 0;
    Job* job = 0;
    for(int i = 0; i < jobs.count();i++){
        job = qobject_cast<Job*>(jobs.at(i));
        if(!job) return -1;
        if (job->client == client)
            return row;
        ++row;
    }
    return -1;
}

bool MainTorrentManager::addTorrent(QString fileName)
{

    if (fileName.isEmpty())
        return false;

    // Add the torrent to our list of downloads
    addTorrent(fileName, lastDirectory);
    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(1000, this, SLOT(saveSettings()));
    }
    return true;
}

void MainTorrentManager::pauseTorrent(int row)
{
    Job* job = qobject_cast<Job*>(jobs.at(row));
    if(!job) return;
    TorrentClient *client = job->client;
    client->setPaused(client->state() != TorrentClient::Paused);
}

void MainTorrentManager::removeTorrent(int row)
{

    Job* job = qobject_cast<Job*>(jobs.at(row));
    if(!job) return;
    TorrentClient *client = job->client;

    // Stop the client.
    client->disconnect();
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    client->stop();

    jobs.removeAt(row);

    saveChanges = true;
    saveSettings();
}

void MainTorrentManager::torrentStopped()
{
    // Schedule the client for deletion.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    dbg("entry" << client->metaInfo().name());
    client->deleteLater();
}

void MainTorrentManager::torrentError(TorrentClient::Error)
{

    // Delete the client.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    Job* job = qobject_cast<Job*>(jobs.at(row));
    if(!job) return;
    QString fileName = job->torrentFileName();
    jobs.removeAt(row);

    // Display the warning.
    dbg( "Error occured : " << client->errorString() );

    client->deleteLater();
}


void MainTorrentManager::updateState(TorrentClient::State)
{

    // Update the state string whenever the client's state changes.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    dbg(client->stateString());

}

void MainTorrentManager::updatePeerInfo()
{

    // Update the number of connected, visited, seed and leecher peers.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    dbg("Peer : " << client->connectedPeerCount());
    dbg("Seed : " << client->seedCount());
}

void MainTorrentManager::updateProgress(int percent)
{
    dbg("entry" << percent);
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    // NOTIFY percent
}

void MainTorrentManager::updateDownloadRate(int bytesPerSecond)
{
    dbg("entry" << bytesPerSecond);
    // Update the download rate.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
//    QString num;
//    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainTorrentManager::updateUploadRate(int bytesPerSecond)
{
    // Update the upload rate.
    dbg("entry" << bytesPerSecond);
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
//    QString num;
//    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainTorrentManager::setUploadLimit(int value)
{
    dbg("entry" << value);
    int rate = rateFromValue(value);
    //uploadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
    RateController::instance()->setUploadLimit(rate * 1024);
}

void MainTorrentManager::setDownloadLimit(int value)
{
    dbg("entry" << value);
    int rate = rateFromValue(value);
    //downloadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
    RateController::instance()->setDownloadLimit(rate * 1024);
}


void MainTorrentManager::closingRequest(){

    if (jobs.isEmpty())
        return;

    // Save upload / download numbers.
    saveSettings();
    saveChanges = false;

    // Stop all clients, remove the rows from the view and wait for
    // them to signal that they have stopped.
    Job* job = 0;
    for(int i = 0; i < jobs.count();i++){
        job = qobject_cast<Job*>(jobs.at(i));
        if(!job) return;
        TorrentClient *client = job->client;
        client->disconnect();
        connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
        client->stop(); 
    }
}
