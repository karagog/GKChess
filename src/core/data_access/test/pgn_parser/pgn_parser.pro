#-------------------------------------------------
#
# Project created by QtCreator 2014-03-01T21:11:23
#
#-------------------------------------------------

TOP_DIR = ../../../../..

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -L$$TOP_DIR/lib \
    -lGUtil \
    -lGKChess_core

QT       += core

QT       -= gui

TARGET = pgn_player
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
