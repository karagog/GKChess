#-------------------------------------------------
#
# Project created by QtCreator 2014-05-01T20:12:01
#
#-------------------------------------------------

TOP_DIR = ../../../../..

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -L$$TOP_DIR/lib \
    -lGUtil \
    -lGKChess

QT       += core

QT       -= gui

TARGET = uci_client
#CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
