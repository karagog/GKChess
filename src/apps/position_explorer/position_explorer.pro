#-------------------------------------------------
#
# Project created by QtCreator 2014-03-11T19:47:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = position_explorer
TEMPLATE = app

TOP_DIR = ../../..

DESTDIR = $$TOP_DIR/bin
DEFINES +=

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -lGUtil


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
