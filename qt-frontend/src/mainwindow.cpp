#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTimer>
#include <QClipboard>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QScreen>
#include <QScrollBar>
#include <QProgressBar>
#include <QComboBox>
#include <QSpacerItem>
#include <QFont>
#include <QProcessEnvironment>
#include <QSizePolicy>
#include <QThread>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , apiManager(std::make_unique<ApiManager>())
    , videoLibrary(std::make_unique<VideoLibrary>())
    , currentDir("")
    , isCleaningUp(false)
{
    setWindowTitle("CINEBASE");
    setWindowIcon(QIcon::fromTheme("video-player"));
    
    // Get screen dimensions
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = std::min(1200, screenGeometry.width());
    int height = std::min(800, screenGeometry.height());
    resize(width, height);
    setMinimumSize(800, 600);
    
    // Center window
    move((screenGeometry.width() - width) / 2,
         (screenGeometry.height() - height) / 2);
    
    setupUI();
    applyStyles();
    
    // Connect API signals
    connect(apiManager.get(), &ApiManager::videosLoaded, this, &MainWindow::onVideosLoaded);
    connect(apiManager.get(), &ApiManager::errorOccurred, this, &MainWindow::onApiError);
    connect(apiManager.get(), &ApiManager::progressLoaded, this, &MainWindow::loadProgress);
    
    // Load initial data
    apiManager->checkStatus();
    requestVideos();
    
    // Start progress refresh timer
    timerIdProgress = startTimer(refreshInterval);
    
    std::cout << "🎬 CINEBASE Started (Qt C++)" << std::endl;
}

MainWindow::~MainWindow() {
    cleanupResources();
}

void MainWindow::setupUI() {
    // Main container with splitter
    centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setStyleSheet("QSplitter::handle { background-color: #1a1a20; }");
    
    createSidebar();
    createMainPanel();
    
    splitter->addWidget(sidebarWidget);
    splitter->addWidget(mainPanelWidget);
    splitter->setSizes({520, 680});
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    
    mainLayout->addWidget(splitter);
    setCentralWidget(centralWidget);
}

void MainWindow::createSidebar() {
    sidebarWidget = new QWidget();
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(15, 15, 15, 15);
    sidebarLayout->setSpacing(15);
    
    // Logo
    logoLabel = new QLabel("◆ CINEBASE");
    QFont logoFont("Segoe UI");
    logoFont.setPointSize(16);
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    logoLabel->setStyleSheet("color: #a892ff; padding: 10px;");
    sidebarLayout->addWidget(logoLabel);
    
    // Folder selection button
    folderButton = new QPushButton("📂 Media Root");
    folderButton->setMinimumHeight(40);
    QFont btnFont("Segoe UI");
    btnFont.setPointSize(10);
    folderButton->setFont(btnFont);
    connect(folderButton, &QPushButton::clicked, this, &MainWindow::onSelectFolder);
    sidebarLayout->addWidget(folderButton);
    
    // Current directory info
    currentDirLabel = new QLabel();
    currentDirLabel->setWordWrap(true);
    currentDirLabel->setStyleSheet("color: #888; font-size: 10px;");
    sidebarLayout->addWidget(currentDirLabel);
    
    // Search input
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Search series or file name...");
    searchInput->setMinimumHeight(35);
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    sidebarLayout->addWidget(searchInput);
    
    // Library header
    QLabel *libraryHeader = new QLabel("LIBRARY");
    libraryHeader->setStyleSheet("color: #888; font-weight: bold; font-size: 11px;");
    sidebarLayout->addWidget(libraryHeader);

    libraryStatusLabel = new QLabel("Choose a media folder to load your library.");
    libraryStatusLabel->setWordWrap(true);
    libraryStatusLabel->setStyleSheet("color: #c9c4dc; font-size: 12px;");
    sidebarLayout->addWidget(libraryStatusLabel);

    libraryLoadingBar = new QProgressBar();
    libraryLoadingBar->setRange(0, 0);
    libraryLoadingBar->setTextVisible(false);
    libraryLoadingBar->setFixedHeight(8);
    libraryLoadingBar->setVisible(false);
    libraryLoadingBar->setStyleSheet(
        "QProgressBar { border: 1px solid #3a3a40; background-color: #1a1a20; border-radius: 4px; }"
        "QProgressBar::chunk { background-color: #a892ff; border-radius: 4px; }"
    );
    sidebarLayout->addWidget(libraryLoadingBar);

    // Compact one-line video selector list.
    videoListWidget = new QListWidget();
    videoListWidget->setSpacing(1);
    videoListWidget->setUniformItemSizes(true);
    videoListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    videoListWidget->setTextElideMode(Qt::ElideRight);
    videoListWidget->setEnabled(false);
    videoListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(videoListWidget, &QListWidget::itemSelectionChanged, this, [this]() {
        if (videoListWidget->currentItem()) {
            onVideoSelected(videoListWidget->currentItem());
        }
    });
    sidebarLayout->addWidget(videoListWidget, 1);
    sidebarWidget->setStyleSheet("QWidget { background-color: #0B0B0F; }");
}

void MainWindow::createMainPanel() {
    mainPanelWidget = new QWidget();
    QVBoxLayout *panelLayout = new QVBoxLayout(mainPanelWidget);
    panelLayout->setContentsMargins(30, 30, 30, 30);
    panelLayout->setSpacing(20);
    
    // Video title
    videoTitleLabel = new QLabel("Select a video...");
    QFont titleFont("Segoe UI");
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    videoTitleLabel->setFont(titleFont);
    videoTitleLabel->setStyleSheet("color: #ffffff;");
    videoTitleLabel->setWordWrap(true);
    panelLayout->addWidget(videoTitleLabel);
    
    // Video filename
    videoFilenameLabel = new QLabel("");
    videoFilenameLabel->setStyleSheet("color: #888; font-size: 12px;");
    panelLayout->addWidget(videoFilenameLabel);
    
    // Progress information
    QHBoxLayout *progressLayout = new QHBoxLayout();
    
    progressLabel = new QLabel("Not watched");
    progressLabel->setStyleSheet("color: #a892ff; font-weight: bold;");
    progressLayout->addWidget(progressLabel);
    
    // Progress bar
    progressBarWidget = new QWidget();
    QHBoxLayout *barLayout = new QHBoxLayout(progressBarWidget);
    barLayout->setContentsMargins(0, 0, 0, 0);
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setStyleSheet(
        "QProgressBar { border: 1px solid #3a3a40; background-color: #1a1a20; border-radius: 4px; height: 8px; }"
        "QProgressBar::chunk { background-color: #a892ff; border-radius: 4px; }"
    );
    progressBar->setObjectName("progressBar");
    barLayout->addWidget(progressBar);
    
    videoDurationLabel = new QLabel("00:00");
    videoDurationLabel->setStyleSheet("color: #888; font-size: 11px; min-width: 50px; text-align: right;");
    barLayout->addWidget(videoDurationLabel);
    
    panelLayout->addLayout(progressLayout);
    
    // Buttons layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    playButton = new QPushButton("▶ Play");
    playButton->setMinimumHeight(45);
    playButton->setMinimumWidth(150);
    QFont playFont("Segoe UI");
    playFont.setPointSize(11);
    playFont.setBold(true);
    playButton->setFont(playFont);
    playButton->setStyleSheet(
        "QPushButton { background-color: #a892ff; color: white; border: none; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #9477dd; }"
        "QPushButton:pressed { background-color: #7d5fc0; }"
    );
    connect(playButton, &QPushButton::clicked, this, &MainWindow::onPlayVideo);
    buttonLayout->addWidget(playButton);
    
    watchedButton = new QPushButton("☐ Mark as Watched");
    watchedButton->setMinimumHeight(45);
    watchedButton->setMinimumWidth(180);
    watchedButton->setFont(playFont);
    watchedButton->setStyleSheet(
        "QPushButton { background-color: #3a3a40; color: #ccc; border: 1px solid #555; border-radius: 6px; }"
        "QPushButton:hover { background-color: #454549; }"
        "QPushButton:pressed { background-color: #2a2a30; }"
    );
    connect(watchedButton, &QPushButton::clicked, this, &MainWindow::onToggleWatched);
    buttonLayout->addWidget(watchedButton);
    
    buttonLayout->addStretch();
    panelLayout->addLayout(buttonLayout);
    
    // Audio tracks section
    QLabel *tracksLabel = new QLabel("Audio Tracks");
    tracksLabel->setStyleSheet("color: #888; font-weight: bold; font-size: 11px;");
    panelLayout->addWidget(tracksLabel);
    
    tracksWidget = new QWidget();
    QVBoxLayout *tracksLayout = new QVBoxLayout(tracksWidget);
    tracksLayout->setContentsMargins(0, 0, 0, 0);
    tracksLayout->setSpacing(8);
    tracksWidget->setLayout(tracksLayout);
    panelLayout->addWidget(tracksWidget);
    
    panelLayout->addStretch();
    mainPanelWidget->setStyleSheet("QWidget { background-color: #0B0B0F; }");
}

void MainWindow::applyStyles() {
    QString styleSheet = R"(
        QMainWindow { background-color: #0B0B0F; }
        QWidget { background-color: #0B0B0F; color: #ffffff; }
        
        QPushButton {
            background-color: #3a3a40;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 6px;
            padding: 8px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #454549;
        }
        
        QPushButton:pressed {
            background-color: #2a2a30;
        }
        
        QLineEdit {
            background-color: #1a1a20;
            color: #ffffff;
            border: 1px solid #3a3a40;
            border-radius: 6px;
            padding: 8px;
            selection-background-color: #a892ff;
        }
        
        QLineEdit:focus {
            border: 1px solid #a892ff;
        }
        
        QListWidget {
            background-color: #0B0B0F;
            border: none;
            outline: none;
        }
        
        QListWidget::item {
            padding: 10px 12px;
            border-radius: 0px;
            margin: 0px;
            border-bottom: 1px solid #16161c;
        }
        
        QListWidget::item:hover {
            background-color: #1a1a20;
        }
        
        QListWidget::item:selected {
            background-color: #a892ff;
            color: #ffffff;
        }
        
        QScrollBar:vertical {
            background-color: #0B0B0F;
            width: 12px;
            border: none;
        }
        
        QScrollBar::handle:vertical {
            background-color: #3a3a40;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #4a4a50;
        }
        
        QLabel { color: #ffffff; }
        
        QComboBox {
            background-color: #1a1a20;
            color: #ffffff;
            border: 1px solid #3a3a40;
            border-radius: 4px;
            padding: 5px;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox:focus {
            border: 1px solid #a892ff;
        }
    )";
    
    qApp->setStyle("Fusion");
    qApp->setStyleSheet(styleSheet);
}

void MainWindow::onSelectFolder() {
    QString folder = QFileDialog::getExistingDirectory(
        this,
        "Select Media Folder",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folder.isEmpty()) {
        currentDir = folder;
        currentDirLabel->setText("CURRENT\n" + folder.split("/").last());
        searchInput->clear();
        apiManager->setVideoDirectory(folder);
        
        // Reload videos after a short delay to let backend update
        QTimer::singleShot(500, this, [this]() {
            requestVideos();
        });
    }
}

void MainWindow::onVideosLoaded(const QJsonArray &videos, const QString &videoDir) {
    if (!currentDir.isEmpty() && videoDir != currentDir) {
        std::cout << "Ignoring stale library response from " << videoDir.toStdString() << std::endl;
        return;
    }

    setLibraryLoading(false);
    currentDir = videoDir;
    if (!videoDir.isEmpty()) {
        currentDirLabel->setText("CURRENT\n" + QDir(videoDir).dirName());
    }

    currentVideos = videos;
    updateVideoList(videos);
    libraryStatusLabel->setText(QString("Loaded %1 videos.").arg(videos.size()));

    if (selectedVideo.isEmpty() && videoListWidget->count() > 0) {
        videoListWidget->setCurrentRow(0);
    }
}

void MainWindow::requestVideos() {
    setLibraryLoading(true);
    apiManager->loadVideos();
}

void MainWindow::setLibraryLoading(bool loading) {
    if (libraryLoadingBar) {
        libraryLoadingBar->setVisible(loading);
    }

    if (videoListWidget) {
        videoListWidget->setEnabled(!loading && videoListWidget->count() > 0);
    }

    if (libraryStatusLabel) {
        libraryStatusLabel->setText(loading ? "Loading media library..." : "Library loaded.");
    }
}

void MainWindow::updateVideoList(const QJsonArray &videos) {
    videoListWidget->clear();
    
    QString searchText = searchInput->text().toLower();
    QString selectedVideoId = selectedVideo["id"].toString();
    int selectedListIndex = -1;
    
    for (const auto &videoRef : videos) {
        QJsonObject video = videoRef.toObject();
        QString title = video["title"].toString();
        QString filename = video["filename"].toString();
        QString videoId = video["id"].toString();
        
        if (searchText.isEmpty() || 
            title.toLower().contains(searchText) || 
            filename.toLower().contains(searchText)) {
            
            QListWidgetItem *item = new QListWidgetItem();
            QString displayText = title;
            bool watched = watchedList.contains(videoId);
            
            // Add progress info if exists
            if (progressMap.contains(videoId) && progressMap[videoId] > 3) {
                double progress = progressMap[videoId];
                displayText += QString(" (%1)").arg(formatTime(progress));
            }

            if (watched) {
                displayText = QString("✓  %1").arg(displayText);
            }
            
            item->setText(displayText);
            item->setData(Qt::UserRole, videoId);
            item->setSizeHint(QSize(0, 42));
            
            // Mark watched items with different style
            if (watched) {
                item->setBackground(QColor("#12121a"));
                item->setForeground(QColor("#66cc66"));
            }
            
            videoListWidget->addItem(item);

            if (!selectedVideoId.isEmpty() && videoId == selectedVideoId) {
                selectedListIndex = videoListWidget->count() - 1;
            }
        }
    }

    if (videoListWidget->count() == 0) {
        videoListWidget->setEnabled(false);
        selectedVideo = QJsonObject();
        videoTitleLabel->setText("No videos found");
        videoFilenameLabel->clear();
    } else {
        videoListWidget->setEnabled(true);
        videoListWidget->setCurrentRow(selectedListIndex >= 0 ? selectedListIndex : 0);
    }
}

void MainWindow::onVideoSelected(QListWidgetItem *item) {
    QString videoId = item->data(Qt::UserRole).toString();
    
    for (const auto &videoRef : currentVideos) {
        QJsonObject video = videoRef.toObject();
        if (video["id"].toString() == videoId) {
            selectedVideo = video;
            displayVideoDetails(video);
            return;
        }
    }
}

void MainWindow::displayVideoDetails(const QJsonObject &video) {
    QString videoId = video["id"].toString();
    QString title = video["title"].toString();
    QString filename = video["filename"].toString();
    double duration = video["duration"].toDouble();
    QJsonArray tracks = video["tracks"].toArray();
    
    videoTitleLabel->setText(title);
    videoFilenameLabel->setText(filename);
    videoDurationLabel->setText(formatTime(duration));
    
    // Update progress bar
    QProgressBar *progressBar = findChild<QProgressBar*>("progressBar");
    if (progressBar) {
        int progressPercent = static_cast<int>(getProgressPercent(videoId));
        progressBar->setValue(progressPercent);
    }
    
    // Update progress label
    if (watchedList.contains(videoId)) {
        progressLabel->setText("✓ Watched");
        progressLabel->setStyleSheet("color: #66cc66; font-weight: bold;");
        watchedButton->setText("↺ Mark Unwatched");
    } else if (progressMap.contains(videoId) && progressMap[videoId] > 3) {
        double progress = progressMap[videoId];
        progressLabel->setText(QString("Resume: %1").arg(formatTime(progress)));
        progressLabel->setStyleSheet("color: #a892ff; font-weight: bold;");
        watchedButton->setText("✓ Mark Watched");
    } else {
        progressLabel->setText("Not watched");
        progressLabel->setStyleSheet("color: #888; font-weight: bold;");
        watchedButton->setText("✓ Mark Watched");
    }
    
    // Clear and recreate tracks
    QLayout *tracksLayout = tracksWidget->layout();
    while (QLayoutItem *item = tracksLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    
    if (tracks.isEmpty()) {
        QLabel *noTracksLabel = new QLabel("No audio tracks found");
        noTracksLabel->setStyleSheet("color: #888;");
        tracksLayout->addWidget(noTracksLabel);
    } else {
        for (int i = 0; i < tracks.size(); ++i) {
            QJsonObject track = tracks[i].toObject();
            QString label = track["label"].toString();
            int index = track["index"].toInt();
            
            QComboBox *comboBox = new QComboBox();
            comboBox->addItem(label);
            
            int savedTrack = tracksMap.value(videoId, 1);
            if (index == savedTrack) {
                comboBox->setStyleSheet("background-color: #2a5a2a; color: #66cc66;");
            }
            
            connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                    this, [this, videoId, index]() {
                onTrackChanged(index);
            });
            
            tracksLayout->addWidget(comboBox);
        }
    }
    
    tracksLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

void MainWindow::onPlayVideo() {
    if (selectedVideo.isEmpty()) {
        QMessageBox::warning(this, "No Video Selected", "Please select a video to play.");
        return;
    }
    
    apiManager->playVideo(selectedVideo["id"].toString());
    std::cout << "▶ Playing video..." << std::endl;
}

void MainWindow::onToggleWatched() {
    if (selectedVideo.isEmpty()) return;
    
    QString videoId = selectedVideo["id"].toString();
    if (watchedList.contains(videoId)) {
        watchedList.removeAll(videoId);
    } else {
        watchedList.append(videoId);
    }

    updateVideoList(currentVideos);
    displayVideoDetails(selectedVideo);
    apiManager->toggleWatched(videoId);
}

void MainWindow::onTrackChanged(int trackIndex) {
    if (selectedVideo.isEmpty()) return;
    
    QString videoId = selectedVideo["id"].toString();
    apiManager->setAudioTrack(videoId, trackIndex);
}

void MainWindow::onSearchTextChanged(const QString &text) {
    updateVideoList(currentVideos);
}

void MainWindow::onLibraryUpdated() {
    requestVideos();
}

void MainWindow::updateProgress() {
    apiManager->loadProgress();
}

void MainWindow::loadProgress(const QJsonObject &progress) {
    videoLibrary->updateProgress(progress);
    progressMap = videoLibrary->getProgress();
    tracksMap = videoLibrary->getTracks();
    watchedList = videoLibrary->getWatched();

    updateVideoList(currentVideos);
    if (!selectedVideo.isEmpty()) {
        displayVideoDetails(selectedVideo);
    }
}

void MainWindow::onApiError(const QString &error) {
    setLibraryLoading(false);
    if (libraryStatusLabel) {
        libraryStatusLabel->setText("Could not load the media library.");
    }
    std::cerr << "API Error: " << error.toStdString() << std::endl;
}

QString MainWindow::formatTime(double seconds) {
    if (seconds <= 0) return "00:00";
    
    int hours = static_cast<int>(seconds / 3600);
    int minutes = static_cast<int>((static_cast<int>(seconds) % 3600) / 60);
    int secs = static_cast<int>(seconds) % 60;
    
    if (hours > 0) {
        return QString::asprintf("%d:%02d:%02d", hours, minutes, secs);
    }
    return QString::asprintf("%d:%02d", minutes, secs);
}

double MainWindow::getProgressPercent(const QString &videoId) {
    double progress = progressMap.value(videoId, 0.0);
    double duration = 0.0;
    
    for (const auto &videoRef : currentVideos) {
        QJsonObject video = videoRef.toObject();
        if (video["id"].toString() == videoId) {
            duration = video["duration"].toDouble();
            break;
        }
    }
    
    if (duration <= 0) return 0.0;
    return (progress / duration) * 100.0;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    cleanupResources();
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (event->timerId() == timerIdProgress) {
        updateProgress();
    }
    QMainWindow::timerEvent(event);
}

void MainWindow::cleanupResources() {
    if (isCleaningUp) return;
    isCleaningUp = true;
    
    if (timerIdProgress != 0) {
        killTimer(timerIdProgress);
        timerIdProgress = 0;
    }
    
    std::cout << "🛑 CINEBASE Closing - Cleaning up resources..." << std::endl;

    // Let mpv quit normally first so --save-position-on-quit can write resume data.
    QProcess::execute("pkill", QStringList() << "-TERM" << "-f" << "mpv");
    QThread::msleep(1800);

    // If a player ignored TERM, finish cleanup so closing the UI does not leave it running.
    if (QProcess::execute("pgrep", QStringList() << "-f" << "mpv") == 0) {
        QProcess::execute("pkill", QStringList() << "-KILL" << "-f" << "mpv");
    }

    apiManager->shutdownBackendSync(1000);
    
    std::cout << "[Clean] Resources cleaned successfully" << std::endl;
}
