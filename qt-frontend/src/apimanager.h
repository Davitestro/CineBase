#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

class ApiManager : public QObject {
    Q_OBJECT

public:
    explicit ApiManager(QObject *parent = nullptr);
    ~ApiManager();

    void loadVideos();
    void playVideo(const QString &videoId);
    void toggleWatched(const QString &videoId);
    void setAudioTrack(const QString &videoId, int trackIndex);
    void loadProgress();
    void setVideoDirectory(const QString &directory);
    void checkStatus();

signals:
    void videosLoaded(const QJsonArray &videos, const QString &videoDir);
    void progressLoaded(const QJsonObject &progress);
    void errorOccurred(const QString &error);
    void videoPlayed(const QString &videoId);
    void watchedToggled(const QString &videoId);
    void trackChanged(const QString &videoId);

private slots:
    void onVideosReplyFinished();
    void onProgressReplyFinished();
    void onPlayReplyFinished();
    void onToggleWatchedReplyFinished();
    void onSetTrackReplyFinished();

private:
    QUrl buildUrl(const QString &endpoint);
    void makePostRequest(const QString &endpoint, const QJsonObject &data);
    void makeGetRequest(const QString &endpoint);

    std::unique_ptr<QNetworkAccessManager> networkManager;
    QString apiBaseUrl;
    QString videoDirectory;
};
