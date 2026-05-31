#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

class VideoLibrary {
public:
    VideoLibrary();

    void updateVideos(const QJsonArray &videos);
    void updateProgress(const QJsonObject &progress);

    QJsonArray getVideos() const;
    QMap<QString, double> getProgress() const;
    QMap<QString, int> getTracks() const;
    QStringList getWatched() const;

    double getVideoProgress(const QString &videoId) const;
    bool isVideoWatched(const QString &videoId) const;
    int getVideoTrack(const QString &videoId) const;

private:
    QJsonArray videos;
    QMap<QString, double> progressMap;
    QMap<QString, int> tracksMap;
    QStringList watchedList;
};
