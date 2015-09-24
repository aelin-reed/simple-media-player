#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T00:07:25
#
#-------------------------------------------------

QT += core \
      gui \
      network \
      xml \
      multimedia \
      multimediawidgets \
      widgets


TARGET = MediaPlayer
TEMPLATE = app


SOURCES += main.cpp\
        main_window.cpp \
    playlist_model.cpp

HEADERS  += main_window.h \
    playlist_model.h

FORMS    += main_window.ui
