#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QThread>
#include <QTextCodec>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "playlistwindow.h"
#include "nx_ipc_cmd_receiver.h"
#include "eventsender.h"

#include "../libid3/include/id3/tag.h"
#include "../libid3/include/id3/field.h"
#include "../libid3/include/id3/misc_support.h"

static const char *NX_AUDIO_EXTENSION[] = {
	".mp3", ".ogg", ".flac", ".wma", ".aac", ".wav", ".mp4a"
};


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_duration(0)
	, m_volValue(100)
	, m_fileIndex(0)
	, m_curFileListIdx (0)
	, m_pPlayer(NULL)
	, m_bSeekReady(false)
	, m_bVoumeCtrlReady (false)
	, ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	m_fileList.MakeFileList("/run/media",(const char **)&NX_AUDIO_EXTENSION, sizeof(NX_AUDIO_EXTENSION) / sizeof(NX_AUDIO_EXTENSION[0]) );

	qDebug("******** m_fileList.m_totalIndex %d", m_fileList.GetSize());

#if 0
	for(int32_t i = 0;i<m_fileList.GetSize();i++)
	{
		qDebug() << "******** list " << i << m_fileList.GetList(i);
	}
#endif

	//	Application Name Style
	ui->appNameLabel->setStyleSheet("QLabel {"
									"color : yellow;"
									"outline-color : black;"
//									"outline-offset : black;"
									"}");


	QFont font = ui->appNameLabel->font();
	font.setBold(true);
	ui->appNameLabel->setFont(font);

	QGraphicsDropShadowEffect *dse = new QGraphicsDropShadowEffect();
	dse->setBlurRadius(10);
//	ui->appNameLabel->setGraphicsEffect(dse);

	QGraphicsColorizeEffect *ce = new QGraphicsColorizeEffect();
	ce->setEnabled(true);
	ce->setColor(QColor(0,0,0,128));
	ui->appNameLabel->setGraphicsEffect(ce);

	//	Initialize Player
	m_pPlayer = new QMediaPlayer();

	//	Connect Solt Functions
	connect(m_pPlayer, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
	connect(m_pPlayer, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
	connect(m_pPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), SLOT(statusChanged(QMediaPlayer::MediaStatus)));
	connect(m_pPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(stateChanged(QMediaPlayer::State)));

	//	Initialize Volume Control & Player's Volume
	m_pPlayer->setVolume(m_volValue);
	ui->volumeProgressBar->setValue(m_volValue);
	ui->volumelabel->setText(QString("Vol: %1").arg(m_volValue));

	if( m_fileList.GetSize() > 0 )
	{
		m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
		m_curFileListIdx = m_fileIndex;
		m_pPlayer->play();
		UpdateAlbumInfo();
	}

	//	Connect Update Window Event
	connect( &m_EventSender, SIGNAL(UpdateWindowEvent(int)), SLOT(UpdateWindowEvent(int)) );
	connect( &m_EventSender, SIGNAL(ExtCmdProcedure(char*)), SLOT(ExtCmdProcedure(char*)) );

	//	Registration(Initialize) External Command Callback Function
	StartCommandProc();
	RegCommandCallback( this, ExtCmdCallback );
}

MainWindow::~MainWindow()
{
	StopCommandProc();
	delete ui;
}
////////////////////////////////////////////////////////////////////
//
//      Update Player Progress Bar & Volume Progress Bar
//
////////////////////////////////////////////////////////////////////
void MainWindow::updateProgressBar(QMouseEvent *event, bool bReleased)
{
	int x = event->x();
	int y = event->y();

	//  Progress Bar Update
	QPoint pos = ui->progressBar->pos();
	int width = ui->progressBar->width();
	int height = ui->progressBar->height();
	int minX = pos.rx();
	int maxX = pos.rx() + width;
	int minY = pos.ry();
	int maxY = pos.ry() + height;

	if( bReleased )
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			//	 Do Seek
			if( m_bSeekReady )
			{
				if( QMediaPlayer::StoppedState !=m_pPlayer->state() )
				{
					double ratio = (double)(x-minX)/(double)width;
					qint64 position = ratio * m_pPlayer->duration();
					m_pPlayer->setPosition( position );
					qDebug("Position = %lld", position);
				}
				qDebug("Do Seek !!!");
			}
		}
		m_bSeekReady = false;
	}
	else
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			m_bSeekReady = true;
			qDebug("Ready to Seek\n");
		}
		else
		{
			m_bSeekReady = false;
		}
	}
}

void MainWindow::updateVolumeBar(QMouseEvent *event, bool bReleased)
{
	int x = event->x();
	int y = event->y();

	//  Progress Bar Update
	QPoint pos = ui->volumeProgressBar->pos();
	int width = ui->volumeProgressBar->width();
	int height = ui->volumeProgressBar->height();
	int minX = pos.rx();
	int maxX = pos.rx() + width;
	int minY = pos.ry();
	int maxY = pos.ry() + height;

	if( bReleased )
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			//	 Change Volume
			if( m_bVoumeCtrlReady )
			{
				if( QMediaPlayer::StoppedState !=m_pPlayer->state() )
				{
					double ratio = (double)(maxY-y)/(double)height;
					qint64 position = ratio * VOLUME_MAX;
					m_volValue = position;
					ui->volumeProgressBar->setValue(m_volValue);
					ui->volumelabel->setText(QString("%1").arg(m_volValue));
					m_pPlayer->setVolume(m_volValue);
					qDebug("Volume Value = %lld", position);
				}
				qDebug("Change Volume !!!");
			}
		}
		m_bVoumeCtrlReady = false;
	}
	else
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			m_bVoumeCtrlReady = true;
			qDebug("Ready to Volume Control\n");
		}
		else
		{
			m_bVoumeCtrlReady = false;
		}
	}
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
	updateProgressBar(event, false);
	updateVolumeBar(event, false);
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	updateProgressBar(event, true);
	updateVolumeBar(event, true);
}


void MainWindow::durationChanged(qint64 duration)
{
	m_duration = duration/1000;
	//	ProgressBar
	ui->progressBar->setMaximum(m_duration / 1000);
	ui->progressBar->setRange(0, duration / 1000);
}

void MainWindow::positionChanged(qint64 position)
{
	//	Prgoress Bar
	ui->progressBar->setValue(position / 1000);
	updateDurationInfo(position / 1000);
}

void MainWindow::statusChanged(QMediaPlayer::MediaStatus status)
{
	// handle status message
	switch (status)
	{
	case QMediaPlayer::LoadedMedia:
		UpdateAlbumInfo();
	case QMediaPlayer::UnknownMediaStatus:
	case QMediaPlayer::NoMedia:
	case QMediaPlayer::BufferingMedia:
	case QMediaPlayer::BufferedMedia:
	//        setStatusInfo(QString());
		break;
	case QMediaPlayer::LoadingMedia:
	//        setStatusInfo(tr("Loading..."));
		break;
	case QMediaPlayer::StalledMedia:
	//        setStatusInfo(tr("Media Stalled"));
		break;
	case QMediaPlayer::EndOfMedia:
		ui->progressBar->setValue(0);
		updateDurationInfo(0);
		if(m_pPlayer)
		{
			if( (m_fileList.GetSize()-1 == m_fileIndex) || ( 1 == m_fileList.GetSize() ) )
			{
				m_fileIndex = 0;
				m_curFileListIdx = m_fileIndex;
			}
			else
			{
				m_fileIndex++;
				m_curFileListIdx = m_fileIndex;
			}
			m_pPlayer->stop();
			m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
			m_pPlayer->play();
			UpdateAlbumInfo();
		}
	//        QApplication::alert(this);
		break;
	case QMediaPlayer::InvalidMedia:
		printf("******** InvalidMedia !!!\n");
		break;
	}
}

void MainWindow::stateChanged(QMediaPlayer::State state)
{
	switch ( state )
	{
		case QMediaPlayer::StoppedState:
			qDebug("Player State Stopped\n");
			break;
		case QMediaPlayer::PlayingState:
			qDebug("Player State Playing\n");
			break;
		case QMediaPlayer::PausedState:
			qDebug("Player State Paused\n");
			break;
	}
}

void MainWindow::updateDurationInfo(qint64 currentInfo)
{
	QString tStr;
	if (currentInfo || m_duration)
	{
		QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
		QTime totalTime((m_duration/3600)%60, (m_duration/60)%60, m_duration%60, (m_duration*1000)%1000);
		QString format = "mm:ss";
		if (m_duration > 3600)
		{
			format = "hh:mm:ss";
		}
		tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
	}
	ui->durationlabel->setText(tStr);
}


//
//
//		Player Control Button Events
//
//
void MainWindow::on_prevButton_released()
{
	if(m_pPlayer)
	{
		if( (0 == m_fileIndex) || (1 == m_fileList.GetSize()) )
		{
			m_fileIndex = m_fileList.GetSize()-1;
            m_curFileListIdx = m_fileIndex;
		}
		else
		{
			m_fileIndex--;
            m_curFileListIdx = m_fileIndex;
		}
		m_pPlayer->stop();
		m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
		m_pPlayer->play();
		UpdateAlbumInfo();
    }
}

void MainWindow::on_playButton_released()
{
	if(m_pPlayer)
	{
		if(QMediaPlayer::PausedState == m_pPlayer->state())
		{
			m_pPlayer->play();
		}
		else if(QMediaPlayer::StoppedState == m_pPlayer->state())
		{
			m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
			m_pPlayer->play();
		}
    }
}


void MainWindow::on_pauseButton_released()
{
	if(m_pPlayer)
	{
		m_pPlayer->pause();
	}
}

void MainWindow::on_nextButton_released()
{
	if(m_pPlayer)
	{
		if( (m_fileList.GetSize()-1 == m_fileIndex) || ( 1 == m_fileList.GetSize() ) )
		{
			m_fileIndex = 0;
			m_curFileListIdx = m_fileIndex;
		}
		else
		{
			m_fileIndex++;
			m_curFileListIdx = m_fileIndex;
		}
		m_pPlayer->stop();
		m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
		m_pPlayer->play();
		UpdateAlbumInfo();
	}
}


void MainWindow::on_stopButton_released()
{
	if(m_pPlayer)
	{
		m_pPlayer->stop();
	}
}


//
//		Play List Button
//
void MainWindow::on_playListButton_released()
{
	PlayListWindow *pPlayList = new PlayListWindow();
	pPlayList->setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
	pPlayList->setList(&m_fileList);
	pPlayList->setCurrentIndex(m_curFileListIdx);
	if( pPlayList->exec() )
	{
		qDebug("OK ~~~~~~");
		m_curFileListIdx = pPlayList->getCurrentIndex();
		m_fileIndex = m_curFileListIdx;
		on_stopButton_released();
		on_playButton_released();
	}
	else
	{
		qDebug("Cancel !!!!!");
	}
	delete pPlayList;
}

//
//		Windows Close
//
void MainWindow::on_closeButton_released()
{
	this->close();
}

void MainWindow::on_playListButton_clicked()
{

}

//
//	Update Album Information using ID3Tag
//

void MainWindow::UpdateAlbumInfo()
{
	QTextCodec * codec = QTextCodec::codecForName("eucKR");
	QString fileName = m_fileList.GetList(m_fileIndex);
#if 1	//	ID3TAG
	size_t num;
	char *str;
	ID3_Tag id3Tag;

	//	Get Album General Information From ID3V1 Parser
	id3Tag.Clear();
	id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ID3V1 | ID3TT_LYRICS3V2 | ID3TT_MUSICMATCH);
	//id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ID3V1);

	//	Album
	str = ID3_GetAlbum( &id3Tag );
	ui->labelAlbum->setText("Album : "  + codec->toUnicode(str) );
	delete []str;

	//	Artist
	str = ID3_GetArtist( &id3Tag );
	ui->labelArtist->setText("Artist : " + codec->toUnicode(str) );
	delete []str;

	//	Title
	str = ID3_GetTitle( &id3Tag );
	ui->labelTitle->setText("Title : "  + codec->toUnicode(str) );
	delete []str;

	//	Track Number
	num = ID3_GetTrackNum( &id3Tag );
	ui->labelTrackNumber->setText("Artist : " + codec->toUnicode(QString::number(num).toStdString().c_str()));
	//	Genre : ID3 Version 1 Information
	num = ID3_GetGenreNum( &id3Tag );
	if( 0 < num && num < ID3_NR_OF_V1_GENRES )
	{
		ui->labelGenre->setText("Genre : "  + codec->toUnicode(ID3_v1_genre_description[num]) );
	}
	else
	{
		ui->labelGenre->setText("Genre : unknown");
	}

	//	Get Album Cover from ID3V2 Parser
	id3Tag.Clear();
	id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ALL);

	if( ID3_HasPicture( &id3Tag ) )
	{
		qDebug() << "Has Picture!!!!" << endl;
		ID3_GetPictureData(&id3Tag, "./temp.jpg");
		QPixmap pix("./temp.jpg");
		QPixmap scaledPix = pix.scaledToHeight(ui->albumArtView->height());
		m_AlbumArt.setPixmap(pix.scaledToHeight(ui->albumArtView->height()));
		ui->albumArtView->setScene(&m_Scene);
		m_Scene.addItem(&m_AlbumArt);
	}
	else
	{
		QPixmap pix(":/default.jpg");
		QPixmap scaledPix = pix.scaledToHeight(ui->albumArtView->height());
		m_AlbumArt.setPixmap(pix.scaledToHeight(ui->albumArtView->height()));
		ui->albumArtView->setScene(&m_Scene);
		m_Scene.addItem(&m_AlbumArt);
	}
#endif

}


//
//	Event Sender's Slot
//

void MainWindow::UpdateWindowEvent(int32_t id)
{
	if( -1 == id )	//	Update All Informations
	{
		//	Update Volume
		ui->volumeProgressBar->setValue(m_volValue);
		ui->volumelabel->setText(QString("%1").arg(m_volValue));
		//	Update Album Info
		UpdateAlbumInfo();
	}
}

//
//	External Command Procedure
//
void MainWindow::ExtCmdCallback( void *pArg, char *cmd )
{
	MainWindow *pObj = (MainWindow *)pArg;
	if( pObj )
	{
		pObj->m_EventSender.RunExtCmd( cmd );
	}
}

void MainWindow::ExtCmdProcedure( char *cmd )
{
	if( 0 )
	{
	}
	else if( !strncasecmp(cmd, "Music Play", 10) || !strncasecmp(cmd, "play_music", 10) )
	{
		qDebug() << "MusicPlayer: Music Play!!!!" << endl;
		if(m_pPlayer)
		{
			m_pPlayer->play();
		}
	}
	else if( !strncasecmp(cmd, "Music Pause", 11) || !strncasecmp(cmd, "stop_music", 10) )
	{
		qDebug() << "MusicPlayer: Music Pause!!!!" << endl;
		if(m_pPlayer)
		{
			m_pPlayer->pause();
			m_pPlayer->pause();
			m_pPlayer->pause();
		}
	}
	else if( !strncasecmp(cmd, "Music Resume", 12) )
	{
		qDebug() << "MusicPlayer: Music Resume!!!!" << endl;
		if(m_pPlayer)
		{
			m_pPlayer->play();
		}
	}
	else if( !strncasecmp(cmd, "Volume Up", 9) || !strncasecmp(cmd, "volume_up", 9) )
	{
		int32_t prevVolume = m_volValue;
		if( m_pPlayer )
		{
			m_volValue += 20;
			if( m_volValue > 100 )
				m_volValue = 100;
			m_pPlayer->setVolume(m_volValue);
		}
		m_EventSender.UpdateWindow( -1 );
		qDebug() << "MusicPlayer: Volume Up (" << prevVolume << "-->" << m_volValue << ")" << endl;
	}
	else if( !strncasecmp(cmd, "Volume Down", 11) || !strncasecmp(cmd, "volume_down", 11) )
	{
		int32_t prevVolume = m_volValue;
		if( m_pPlayer )
		{
			m_volValue -= 20;
			if( m_volValue < 0 )
				m_volValue = 0;
			m_pPlayer->setVolume(m_volValue);
			m_EventSender.UpdateWindow( -1 );
		}
		qDebug() << "MusicPlayer: Volume Down (" << prevVolume << "-->" << m_volValue << ")" << endl;
	}
	else if( !strncasecmp(cmd, "Music Next", 10) || !strncasecmp(cmd, "next_song", 9) )
//	else if( !strncasecmp(cmd, "LED On", 6) )
	{
		if(m_pPlayer)
		{
			if( (m_fileList.GetSize()-1 == m_fileIndex) || ( 1 == m_fileList.GetSize() ) )
			{
				m_fileIndex = 0;
				m_curFileListIdx = m_fileIndex;
			}
			else
			{
				m_fileIndex++;
				m_curFileListIdx = m_fileIndex;
			}
			m_pPlayer->stop();
			m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
			m_pPlayer->play();
			m_EventSender.UpdateWindow( -1 );
		}
		qDebug() << "MusicPlayer: Music Next!!!!" << "( " << m_fileIndex << " )" << endl;
	}
	else if( !strncasecmp(cmd, "Music Previous", 14) || !strncasecmp(cmd, "previous_song", 13) )
//	else if( !strncasecmp(cmd, "LED Off", 7) )
	{
		if(m_pPlayer)
		{
			if( (0 == m_fileIndex) || (1 == m_fileList.GetSize()) )
			{
				m_fileIndex = m_fileList.GetSize()-1;
				m_curFileListIdx = m_fileIndex;
			}
			else
			{
				m_fileIndex--;
				m_curFileListIdx = m_fileIndex;
			}
			m_pPlayer->stop();
			m_pPlayer->setMedia(QUrl::fromLocalFile(m_fileList.GetList(m_fileIndex)));
			m_pPlayer->play();
			m_EventSender.UpdateWindow( -1 );
		}
		qDebug() << "MusicPlayer: Music Previous!!!!"<< "( " << m_fileIndex << " )" << endl;
	}
	else
	{
		qDebug() << "Unknwon Command :" << cmd << endl;
	}
}
