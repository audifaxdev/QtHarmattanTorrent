/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "bencodeparser.h"
#include "connectionmanager.h"
#include "torrentclient.h"
#include "torrentserver.h"
#include "trackerclient.h"

#include <QtCore>
#include "debug.h"

#include <QNetworkRequest>

#define QT_NO_URL_CAST_FROM_STRING

TrackerClient::TrackerClient(TorrentClient *downloader, QObject *parent)
    : QObject(parent), torrentDownloader(downloader)
{
    //dbg("entry");
    length = 0;
    requestInterval = 5 * 60;
    requestIntervalTimer = -1;
    firstTrackerRequest = true;
    lastTrackerRequest = false;
    firstSeeding = true;
    indexTracker = 0;
    m_defTrackerList << "http://bt.e-burg.org:2710";
    m_defTrackerList << "http://retracker.hotplug.ru:2710";
    m_defTrackerList << "http://retracker.kld.ru:2710";
    m_defTrackerList << "http://tracker.yify-torrents.com:80";
    m_defTrackerList << "http://exodus.desync.com:6969";
    m_defTrackerList << "http://tracker.torrentbay.to:6969";
    m_defTrackerList << "http://tracker.anime-miako.to:6969";
    m_defTrackerList << "http://tracker.thepiratebay.org:80";
    m_defTrackerList << "http://tracker.ipv6tracker.org:80";

    //connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpRequestDone(QNetworkReply*)));
}

QByteArray TrackerClient::nextDefTracker(){
    if(indexTracker == m_defTrackerList.count())
        indexTracker = 0;

    return m_defTrackerList.at(indexTracker++);
}

void TrackerClient::start(const MetaInfo &info)
{
    //dbg("entry");
    metaInfo = info;
    QTimer::singleShot(0, this, SLOT(fetchPeerList()));

    if (metaInfo.fileForm() == MetaInfo::SingleFileForm) {
        length = metaInfo.singleFile().length;
    } else {
        QList<MetaInfoMultiFile> files = metaInfo.multiFiles();
        for (int i = 0; i < files.size(); ++i)
            length += files.at(i).length;
    }
}

void TrackerClient::startSeeding()
{
    //dbg("entry");
    firstSeeding = true;
    fetchPeerList();
}

void TrackerClient::stop()
{
    //dbg("entry");
    lastTrackerRequest = true;
    //qnam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);
    fetchPeerList();
}

void TrackerClient::timerEvent(QTimerEvent *event)
{
    //dbg("entry");
    if (event->timerId() == requestIntervalTimer) {
            fetchPeerList();
    } else {
        QObject::timerEvent(event);
    }
}

void TrackerClient::fetchPeerList()
{
    //dbg("entry" << metaInfo.announceUrl());
    // Prepare connection details
    //dbg("announcelist count " << metaInfo.announceList().count());
//    foreach(QString e, metaInfo.announceList()){
//        dbg(e);
//    }
    QString fullUrl = metaInfo.announceUrl();
    QUrl url(fullUrl);
    //dbg(url.toString());
    QString passkey = "?";
    if (fullUrl.contains("?passkey")) {
        passkey = metaInfo.announceUrl().mid(fullUrl.indexOf("?passkey"), -1);
        passkey += '&';
    }

    // Percent encode the hash
    QByteArray infoHash = torrentDownloader->infoHash();
    //dbg(infoHash);
    QString encodedSum = "";
    //QString encodedSum = QUrl::toPercentEncoding(QString(infoHash));
    for (int i = 0; i < infoHash.size(); ++i) {
        encodedSum += '%';
        encodedSum += QString::number(infoHash[i], 16).right(2).rightJustified(2, '0');
        //encodedSum +=  QUrl::toPercentEncoding(QString(infoHash.at(i)));
    }
    //dbg(encodedSum);
    bool seeding = (torrentDownloader->state() == TorrentClient::Seeding);
    QByteArray query = nextDefTracker();
    query += "/anounce";
    query += passkey;
    query += "info_hash=" + encodedSum;
    query += "&peer_id=" + ConnectionManager::instance()->clientId();
    query += "&port=" + QByteArray::number(TorrentServer::instance()->serverPort());
    query += "&compact=1";
    query += "&uploaded=" + QByteArray::number(torrentDownloader->uploadedBytes());

    if (!firstSeeding) {
        query += "&downloaded=0";
        query += "&left=0";
    } else {
        query += "&downloaded=" + QByteArray::number(
            torrentDownloader->downloadedBytes());
        int left = qMax<int>(0, metaInfo.totalSize() - torrentDownloader->downloadedBytes());
        query += "&left=" + QByteArray::number(seeding ? 0 : left);
    }

    if (seeding && firstSeeding) {
        query += "&event=completed";
        firstSeeding = false;
    } else if (firstTrackerRequest) {
        firstTrackerRequest = false;
        query += "&event=started";
    } else if(lastTrackerRequest) {
        query += "&event=stopped";
    }

    if (!trackerId.isEmpty())
        query += "&trackerid=" + trackerId;

    QUrl queryUrl = QUrl::fromUserInput(query);
    if(!queryUrl.isValid()){
        dbg("Unvalid URL " << queryUrl.toString());
        return;
    }
    //dbg(queryUrl.toEncoded());
    QNetworkRequest trackerRequest(queryUrl);

    //dbg("RAW HEADER LIST : ");
    foreach(QByteArray bytes, trackerRequest.rawHeaderList())
    {
        //dbg(bytes);
        //dbg(trackerRequest.rawHeader(bytes));
    }

    QNetworkReply* qnr = qnam.get(trackerRequest);
    if(!qnr) { dbg("EMPTY QNR"); return;}
    connect(qnr, SIGNAL(finished()),SLOT(httpRequestDone()));
    connect(qnr,SIGNAL(downloadProgress( qint64, qint64)),
            SLOT(trackerRequestProgress(qint64, qint64)));
    connect(qnr,SIGNAL(error(QNetworkReply::NetworkError)),SLOT(httpRequestDone()));
}

void TrackerClient::trackerRequestProgress(qint64 bytesReceived, qint64 bytesTotal){
    //dbg("bytesReceived : " << bytesReceived);
    //dbg("bytesTotal : " << bytesTotal);
}


void TrackerClient::httpRequestDone()
{
    //dbg("entry");
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) { dbg("EMPTY QNR"); return;}
    if(reply->error() != QNetworkReply::NoError){
        //dbg("error: " << reply->errorString());
        emit connectionError(reply->error());
        return;
    }
    QByteArray response = reply->readAll();
    //dbg(response);
    if (lastTrackerRequest) {
        emit stopped();
        return;
    }

//    if (error) {
//        emit connectionError(http.error());
//        return;
//    }

    reply->deleteLater();
    BencodeParser parser;
    if (!parser.parse(response)) {
        qWarning("Error parsing bencode response from tracker: %s",
                 qPrintable(parser.errorString()));
        //http.abort();
        return;
    }

    QMap<QByteArray, QVariant> dict = parser.dictionary();

    if (dict.contains("failure reason")) {
        // no other items are present
        emit failure(QString::fromUtf8(dict.value("failure reason").toByteArray()));
        return;
    }

    if (dict.contains("warning message")) {
        // continue processing
        emit warning(QString::fromUtf8(dict.value("warning message").toByteArray()));
    }

    if (dict.contains("tracker id")) {
        // store it
        trackerId = dict.value("tracker id").toByteArray();
    }

    if (dict.contains("interval")) {
        // Mandatory item
        if (requestIntervalTimer != -1)
            killTimer(requestIntervalTimer);
        requestIntervalTimer = startTimer(dict.value("interval").toInt() * 1000);
    }

    if (dict.contains("peers")) {
        // store it
        peers.clear();
        QVariant peerEntry = dict.value("peers");
        if (peerEntry.type() == QVariant::List) {
            QList<QVariant> peerTmp = peerEntry.toList();
            for (int i = 0; i < peerTmp.size(); ++i) {
                TorrentPeer tmp;
                QMap<QByteArray, QVariant> peer = qvariant_cast<QMap<QByteArray, QVariant> >(peerTmp.at(i));
                tmp.id = QString::fromUtf8(peer.value("peer id").toByteArray());
                tmp.address.setAddress(QString::fromUtf8(peer.value("ip").toByteArray()));
                tmp.port = peer.value("port").toInt();
                peers << tmp;
            }
        } else {
            QByteArray peerTmp = peerEntry.toByteArray();
            for (int i = 0; i < peerTmp.size(); i += 6) {
                TorrentPeer tmp;
                uchar *data = (uchar *)peerTmp.constData() + i;
                tmp.port = (int(data[4]) << 8) + data[5];
                uint ipAddress = 0;
                ipAddress += uint(data[0]) << 24;
                ipAddress += uint(data[1]) << 16;
                ipAddress += uint(data[2]) << 8;
                ipAddress += uint(data[3]);
                tmp.address.setAddress(ipAddress);
                peers << tmp;
            }
        }
        emit peerListUpdated(peers);
    }
}
