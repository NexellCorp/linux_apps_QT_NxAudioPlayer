#-------------------------------------------------
#
# Project created by QtCreator 2016-09-27T09:40:24
#
#-------------------------------------------------

QT += core gui

QT += network \
      xml \
      multimedia \
      multimediawidgets \
      widgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxAudioPlayer
TEMPLATE = app


SOURCES += main.cpp \
        mainwindow.cpp \
        NX_CFileList.cpp \
        playlistwindow.cpp \
        eventsender.cpp \
        SockUtils.cpp \
        nx_ipc_cmd_receiver.cpp


INCLUDEPATH += $$PWD/../libid3/include
LIBS        += $$PWD/../libid3/lib/libid3.a
LIBS        += $$PWD/../libid3/lib/libz.a


HEADERS  += mainwindow.h \
			NX_CFileList.h \
			playlistwindow.h \
            nx_ipc_cmd_receiver.h \
            eventsender.h \
            SockUtils.h

FORMS    += mainwindow.ui \
			playlistwindow.ui

DISTFILES += \
    default.jpeg

RESOURCES += \
	resources.qrc
