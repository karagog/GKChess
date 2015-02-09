#-------------------------------------------------
#
# Project created by QtCreator 2014-05-18T10:26:27
#
#-------------------------------------------------

QT       -= gui

TARGET = uciEnginePlugin
TEMPLATE = lib
CONFIG += plugin

TOP_DIR = ../../../..

DESTDIR = $$TOP_DIR/bin
QMAKE_CXXFLAGS += -std=c++11

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
    -lGKChess

SOURCES += uci_client.cpp

HEADERS += uci_client.h
