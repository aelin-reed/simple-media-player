#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#include "playlist_model.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void muted(bool muted);
    void volumeChanged(int volume);
    void stateChanged(QMediaPlayer::State state);
    void durationChanged(qint64 duration);
    void progressChanged(qint64 progress);
    void metaDataChanged();
    void playlistPositionChanged(int position);
    void statusChanged(QMediaPlayer::MediaStatus status);
    void displayErrorMessage();

    void on_tracks_activated(const QModelIndex &index);
    void on_resetPlaylist_clicked();
    void on_loadPlaylist_clicked();
    void on_savePlaylist_clicked();
    void on_addTracks_clicked();
    void on_removeTracks_clicked();
    void on_duration_sliderMoved(int position);
    void on_play_clicked();
    void on_stop_clicked();
    void on_previous_clicked();
    void on_next_clicked();
    void on_mute_clicked();
    void on_volume_valueChanged(int volume);

private:
    void appendPlaylist(const QStringList& filenames);

    Ui::MainWindow *ui;
    QMediaPlayer* player;
    QMediaPlaylist* playlist;
    PlaylistModel* playlistModel;
};

#endif // MAIN_WINDOW_H
