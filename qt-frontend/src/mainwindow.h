#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include "apimanager.h"
#include "videolibrary.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private slots:
    void onSelectFolder();
    void onVideosLoaded(const QJsonArray &videos, const QString &videoDir);
    void onLibraryUpdated();
    void onVideoSelected(QListWidgetItem *item);
    void onPlayVideo();
    void onToggleWatched();
    void onTrackChanged(int index);
    void onSearchTextChanged(const QString &text);
    void updateProgress();
    void onApiError(const QString &error);
    void cleanupResources();

private:
    void setupUI();
    void applyStyles();
    void createSidebar();
    void createMainPanel();
    void requestVideos();
    void setLibraryLoading(bool loading);
    void updateVideoList(const QJsonArray &videos);
    void displayVideoDetails(const QJsonObject &video);
    void loadProgress();
    QString formatTime(double seconds);
    double getProgressPercent(const QString &videoId);
    void updateWatchedStatus();

    // UI Components
    QWidget *centralWidget;
    
    // Sidebar
    QWidget *sidebarWidget;
    QLabel *logoLabel;
    QPushButton *folderButton;
    QLabel *currentDirLabel;
    QLabel *libraryStatusLabel;
    QProgressBar *libraryLoadingBar;
    QLineEdit *searchInput;
    QListWidget *videoListWidget;
    
    // Main Panel
    QWidget *mainPanelWidget;
    QLabel *videoTitleLabel;
    QLabel *videoFilenameLabel;
    QLabel *videoDurationLabel;
    QLabel *progressLabel;
    QWidget *progressBarWidget;
    QPushButton *playButton;
    QPushButton *watchedButton;
    QWidget *tracksWidget;
    
    // Data
    std::unique_ptr<ApiManager> apiManager;
    std::unique_ptr<VideoLibrary> videoLibrary;
    QJsonArray currentVideos;
    QJsonObject selectedVideo;
    QMap<QString, double> progressMap;
    QMap<QString, int> tracksMap;
    QStringList watchedList;
    QString currentDir;
    
    // State
    bool isCleaningUp = false;
    int refreshInterval = 2000; // 2 seconds
    int timerIdProgress = 0;
};
