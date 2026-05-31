#include "videolibrary.h"
#include <QJsonArray>
#include <QJsonObject>

VideoLibrary::VideoLibrary() = default;

void VideoLibrary::updateVideos(const QJsonArray &newVideos) {
    videos = newVideos;
}

void VideoLibrary::updateProgress(const QJsonObject &progress) {
    progressMap.clear();
    tracksMap.clear();
    watchedList.clear();

    // Load progress data
    QJsonObject progressObj = progress["progress"].toObject();
    for (auto it = progressObj.begin(); it != progressObj.end(); ++it) {
        progressMap[it.key()] = it.value().toDouble();
    }

    // Load tracks data
    QJsonObject tracksObj = progress["tracks"].toObject();
    for (auto it = tracksObj.begin(); it != tracksObj.end(); ++it) {
        tracksMap[it.key()] = it.value().toInt();
    }

    // Load watched list
    QJsonArray watchedArray = progress["watched"].toArray();
    for (const auto &item : watchedArray) {
        watchedList.append(item.toString());
    }
}

QJsonArray VideoLibrary::getVideos() const {
    return videos;
}

QMap<QString, double> VideoLibrary::getProgress() const {
    return progressMap;
}

QMap<QString, int> VideoLibrary::getTracks() const {
    return tracksMap;
}

QStringList VideoLibrary::getWatched() const {
    return watchedList;
}

double VideoLibrary::getVideoProgress(const QString &videoId) const {
    return progressMap.value(videoId, 0.0);
}

bool VideoLibrary::isVideoWatched(const QString &videoId) const {
    return watchedList.contains(videoId);
}

int VideoLibrary::getVideoTrack(const QString &videoId) const {
    return tracksMap.value(videoId, 1);
}
