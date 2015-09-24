#include "main_window.h"
#include "ui_main_window.h"

#include <QFileDialog>
#include <QMediaMetaData>
#include <QMessageBox>
#include <QSaveFile>
#include <QTime>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{    
    // Initialize the playlist:
    playlist = new QMediaPlaylist();
    playlist->setPlaybackMode(QMediaPlaylist::Sequential);

    // Initialize the media player:
    player = new QMediaPlayer(this);
    player->setPlaylist(playlist);

    // Initialize the playlist model:
    playlistModel = new PlaylistModel();
    playlistModel->setPlaylist(playlist);

    // Subscribe to media player events:
    connect(player, SIGNAL(mutedChanged(bool)), SLOT(muted(bool)));
    connect(player, SIGNAL(volumeChanged(int)), SLOT(volumeChanged(int)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(stateChanged(QMediaPlayer::State)));
    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(progressChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), SLOT(displayErrorMessage()));

    // Subscribe to playlist events:
    connect(playlist, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));

    // Initialize the user interface.
    ui->setupUi(this);

    // Set button icons via Qt standard icons:
    ui->play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->previous->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->next->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->mute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    // Set the user interface model.
    ui->tracks->setModel(playlistModel);
}

MainWindow::~MainWindow()
{
    delete playlist;
    delete player;
    delete playlistModel;
    delete ui;
}

void MainWindow::muted(bool muted)
{
    if (muted) {
        ui->volume->setValue(0);
        ui->volumeDisplay->display(0);
    } else {
        ui->volume->setValue(player->volume());
        ui->volumeDisplay->display(player->volume());
    }

    // Update the mute button icon.
    ui->mute->setIcon(muted ? style()->standardIcon(QStyle::SP_MediaVolumeMuted)
                            : style()->standardIcon(QStyle::SP_MediaVolume));
}

void MainWindow::volumeChanged(int volume)
{
    ui->volume->setValue(player->volume());
    ui->volumeDisplay->display(player->volume());
}

void MainWindow::stateChanged(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        // Update the user interface:
        ui->play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        ui->duration->setEnabled(true);
        break;

    case QMediaPlayer::PausedState:
        // Update the user interface:
        ui->play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        ui->duration->setEnabled(true);
        break;

    case QMediaPlayer::StoppedState:
        // Update the user interface:
        ui->play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        ui->duration->setEnabled(false);

        // Reset the duration display:
        ui->currentDuration->setText("--:--");
        ui->maxDuration->setText("--:--");
        break;
    }
}

void MainWindow::durationChanged(qint64 duration)
{
    ui->duration->setMaximum(duration / 1000);
}

void MainWindow::progressChanged(qint64 progress)
{
    // Update the duration slider if the user is not currently changing it's value.
    if (!ui->duration->isSliderDown()) {
        ui->duration->setValue(progress / 1000);
    }

    if (progress) {
        //
        qint64 duration = player->duration() / 1000;
        qint64 current = progress / 1000;

        //
        QTime currentTime((current/3600)%60, (current/60)%60, current%60, (current*1000)%1000);
        QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);

        QString format = duration < 3600 ? "mm:ss" : "hh:mm:ss";

        ui->currentDuration->setText(currentTime.toString(format));
        ui->maxDuration->setText(totalTime.toString(format));
    }
}

void MainWindow::metaDataChanged()
{
    if (player->state() == QMediaPlayer::PlayingState ||
        player->state() == QMediaPlayer::PausedState)
    {
        // Update the current window title with meta data from the current track.
        setWindowTitle(QString("Media Player: %1 - %2")
                       .arg(player->metaData(QMediaMetaData::AlbumArtist).toString())
                       .arg(player->metaData(QMediaMetaData::Title).toString()));
    } else {
        setWindowTitle("Media Player");
    }
}

void MainWindow::playlistPositionChanged(int position)
{
    ui->tracks->setCurrentIndex(playlistModel->index(position, 0));
}

void MainWindow::statusChanged(QMediaPlayer::MediaStatus status)
{
    // TODO: Implement me.
}

void MainWindow::displayErrorMessage()
{
    // TODO: Implement me.
}

void MainWindow::on_tracks_activated(const QModelIndex& index)
{
    if (index.isValid()) {
        playlist->setCurrentIndex(index.row());
    }
}

void MainWindow::on_resetPlaylist_clicked()
{
    // Stop the player if it is currently playing or paused.
    if (player->state() == QMediaPlayer::PlayingState ||
        player->state() == QMediaPlayer::PausedState)
    {
        player->stop();
    }

    // Clear the playlist.
    playlist->clear();
}

void MainWindow::on_loadPlaylist_clicked()
{
    // Request the filename from which the current playlist should be loaded.
    QString filename = QFileDialog::getOpenFileName(
                this,
                QString("Save Playlist As..."),
                QDir::homePath(),
                QString("Simple Playlist Files (*.spf);;All Files and Folders (*.*)"));

    // If the filename is empty, the operation was cancelled by the user.
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        // Validate the simple playlist format header.
        QString header = in.readLine();
        if (header != "Simple Playlist Format 1.0") {
            QMessageBox messageBox;
            messageBox.setText("The selected file is not a valid SPF file.");
            messageBox.setIcon(QMessageBox::Critical);
            messageBox.exec();
            return;
        }

        // Load all filenames from the playlist file.
        QStringList filenames;
        while (!in.atEnd()) {
            filenames.push_back(in.readLine());
        }

        // Append the loaded files to the playlist.
        appendPlaylist(filenames);
    } else {
        QMessageBox messageBox;
        messageBox.setText("An error occurred while loading the playlist.");
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
    }
}

void MainWindow::on_savePlaylist_clicked()
{
    // Request the filename to which the current playlist should be written.
    QString filename = QFileDialog::getSaveFileName(
                this,
                QString("Save Playlist As..."),
                QDir::homePath(),
                QString("Simple Playlist Files (*.spf);;All Files and Folders (*.*)"));

    // If the filename is empty, the operation was cancelled by the user.
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // Write the SPF header.
        out << "Simple Playlist Format 1.0\n";

        for (int i = 0; i < playlist->mediaCount(); ++i) {
            // Get media information:
            QMediaContent content = playlist->media(i);
            QUrl location = content.canonicalUrl();
            QFileInfo info(location.path());
            QString path = info.filePath();

            // Write the current content path to the file.
            out << path.right(path.length() - 1) << "\n";
        }

        // Notify the user that his or her playlist has been successfully saved.
        QMessageBox messageBox;
        messageBox.setText("Playlist successfully saved.");
        messageBox.setIcon(QMessageBox::Information);
        messageBox.exec();
    } else {
        QMessageBox messageBox;
        messageBox.setText("An error occurred while saving the playlist.");
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.exec();
    }
}

void MainWindow::on_addTracks_clicked()
{
    // Request a list of files from the user.
    QStringList filenames = QFileDialog::getOpenFileNames(
                this,
                QString("Add Tracks"),
                QDir::homePath(),
                QString("Audio Files (*.mp3 *.m3u);;All Files and Folders (*.*)"));

    // Add all selected files to the playlist.
    appendPlaylist(filenames);
}

void MainWindow::appendPlaylist(const QStringList& filenames) {
    // Load the selected audio files.
    foreach (const QString& filename, filenames) {
        QFileInfo info(filename);

        if (info.exists()) {
            QUrl url = QUrl::fromLocalFile(info.absoluteFilePath());

            if (info.suffix().toLower() == QString("m3u")) {
                playlist->load(url);
            } else {
                playlist->addMedia(url);
            }
        } else {
            QUrl url(filename);

            if (url.isValid()) {
                playlist->addMedia(url);
            }
        }
    }
}

void MainWindow::on_removeTracks_clicked()
{
    // Remove the track at the currently selected index.
    QModelIndexList selection = ui->tracks->selectionModel()->selectedIndexes();
    playlist->removeMedia(selection.first().row());
}

void MainWindow::on_duration_sliderMoved(int position)
{
    // Update the player position.
    player->setPosition(position * 1000);
}

void MainWindow::on_play_clicked()
{
    if (player->state() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        player->play();
    }
}

void MainWindow::on_stop_clicked()
{
    if (player->state() == QMediaPlayer::PlayingState) {
        ui->play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        player->stop();

        // Reset the window title.
        setWindowTitle("Media Player");
    }
}

void MainWindow::on_previous_clicked()
{
    playlist->previous();
}

void MainWindow::on_next_clicked()
{
    playlist->next();
}

void MainWindow::on_mute_clicked()
{
    if (player->isMuted())
        player->setMuted(false);
    else
        player->setMuted(true);
}

void MainWindow::on_volume_valueChanged(int volume)
{
    if (volume == 0) {
        player->setMuted(true);
    } else {
        player->setMuted(false);
        player->setVolume(volume);
    }
}
