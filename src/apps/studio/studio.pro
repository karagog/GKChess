#-------------------------------------------------
#
# Project created by QtCreator 2014-03-11T19:47:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = studio
TEMPLATE = app

TOP_DIR = ../../..

DESTDIR = $$TOP_DIR/bin
DEFINES += GUTIL_CORE_QT_ADAPTERS

CONFIG(debug, debug|release) {
    #message(Preparing debug build)
    DEFINES += DEBUG
}
else {
    #message(Preparing release build)
}

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/lib \
    -L$$TOP_DIR/gutil/lib \
    -lGUtil \
    -lGUtilQt \
    -lGKChess \
    -lGKChessUI


SOURCES += main.cpp\
        mainwindow.cpp \
    application.cpp

HEADERS  += mainwindow.h \
    application.h

FORMS    += mainwindow.ui
