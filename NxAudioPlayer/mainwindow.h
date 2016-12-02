#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QMessageBox>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QTime>
#include <QMouseEvent>
#include <QImage>
#include <QImageReader>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "NX_CFileList.h"

#define VOLUME_MIN  0
#define VOLUME_MAX  100

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	//
	//	Mouse Event for Progress Bar
	//
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	//
	void UpdateAlbumInfo();

private:
    qint64 m_duration;
    int    m_volValue;
    int    m_fileIndex;
    int    m_curFileListIdx;

    QMediaPlayer *m_pPlayer;
    NX_CFileList m_fileList;
	QGraphicsScene m_Scene;
	QGraphicsPixmapItem m_AlbumArt;

	//	Progress Bar
	bool	m_bSeekReady;
    bool    m_bVoumeCtrlReady;

private:
    void updateDurationInfo(qint64 currentInfo);
    //  Update Progress Bar
    void updateProgressBar(QMouseEvent *event, bool bReleased);
    void updateVolumeBar(QMouseEvent *event, bool bReleased);

private slots:
	//
	//	QMediaPlayer's Slots
	//
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void statusChanged(QMediaPlayer::MediaStatus status);

	//
	//	Button Events
	//
	//	player control
    void on_prevButton_released();
    void on_playButton_released();
    void on_pauseButton_released();
    void on_nextButton_released();
    void on_stopButton_released();

	//	Playlist Button & Close Button
	void on_playListButton_released();
	void on_closeButton_released();

	void on_playListButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
