#include "apimanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>
#include <iostream>

ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
    , networkManager(std::make_unique<QNetworkAccessManager>(this))
    , apiBaseUrl("http://127.0.0.1:8000/api")
{
}

ApiManager::~ApiManager() = default;

QUrl ApiManager::buildUrl(const QString &endpoint) {
    return QUrl(apiBaseUrl + "/" + endpoint);
}

void ApiManager::setVideoDirectory(const QString &directory) {
    videoDirectory = directory;
    // Send to backend
    QJsonObject data;
    data["video_dir"] = directory;
    makePostRequest("set_video_dir", data);
}

void ApiManager::checkStatus() {
    makeGetRequest("status");
}

void ApiManager::loadVideos() {
    makeGetRequest("videos");
}

void ApiManager::loadProgress() {
    makeGetRequest("progress");
}

void ApiManager::playVideo(const QString &videoId) {
    QJsonObject data;
    data["video_id"] = videoId;
    makePostRequest("play", data);
}

void ApiManager::toggleWatched(const QString &videoId) {
    QJsonObject data;
    data["video_id"] = videoId;
    makePostRequest("toggle_watched", data);
}

void ApiManager::setAudioTrack(const QString &videoId, int trackIndex) {
    QJsonObject data;
    data["video_id"] = videoId;
    data["track_index"] = trackIndex;
    makePostRequest("set_track", data);
}

void ApiManager::makeGetRequest(const QString &endpoint) {
    QUrl url = buildUrl(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = networkManager->get(request);
    
    if (endpoint == "videos") {
        connect(reply, &QNetworkReply::finished, this, &ApiManager::onVideosReplyFinished);
    } else if (endpoint == "progress") {
        connect(reply, &QNetworkReply::finished, this, &ApiManager::onProgressReplyFinished);
    }
    
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::errorOccurred),
            this, [this, reply]() {
        emit errorOccurred(reply->errorString());
        std::cerr << "Network error: " << reply->errorString().toStdString() << std::endl;
    });
}

void ApiManager::makePostRequest(const QString &endpoint, const QJsonObject &data) {
    QUrl url = buildUrl(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(data);
    QByteArray postData = doc.toJson();
    
    QNetworkReply *reply = networkManager->post(request, postData);
    
    if (endpoint == "play") {
        connect(reply, &QNetworkReply::finished, this, &ApiManager::onPlayReplyFinished);
    } else if (endpoint == "toggle_watched") {
        connect(reply, &QNetworkReply::finished, this, &ApiManager::onToggleWatchedReplyFinished);
    } else if (endpoint == "set_track") {
        connect(reply, &QNetworkReply::finished, this, &ApiManager::onSetTrackReplyFinished);
    }
    
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::errorOccurred),
            this, [this, reply]() {
        emit errorOccurred(reply->errorString());
    });
}

void ApiManager::onVideosReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QJsonArray videos = obj["videos"].toArray();
        QString videoDir = obj["video_dir"].toString();
        
        std::cout << "📺 Loaded " << videos.size() << " videos from " << videoDir.toStdString() << std::endl;
        
        // Load progress data
        loadProgress();
        
        emit videosLoaded(videos, videoDir);
    }
    
    reply->deleteLater();
}

void ApiManager::onProgressReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        emit progressLoaded(obj);
    }
    
    reply->deleteLater();
}

void ApiManager::onPlayReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Failed to play video");
        reply->deleteLater();
        return;
    }
    
    std::cout << "▶ Video playback started" << std::endl;
    reply->deleteLater();
}

void ApiManager::onToggleWatchedReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Failed to toggle watched status");
        reply->deleteLater();
        return;
    }
    
    // Reload videos to get updated watched status
    loadVideos();
    reply->deleteLater();
}

void ApiManager::onSetTrackReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Failed to set audio track");
        reply->deleteLater();
        return;
    }
    
    std::cout << "🔊 Audio track changed" << std::endl;
    reply->deleteLater();
}
