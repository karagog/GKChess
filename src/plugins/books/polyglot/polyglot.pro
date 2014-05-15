#-------------------------------------------------
#
# Project created by QtCreator 2014-05-13T17:22:09
#
#-------------------------------------------------

QT       += core

TARGET = polyglotReaderPlugin
TEMPLATE = lib
CONFIG += plugin

TOP_DIR = ../../../..

DESTDIR = $$TOP_DIR/bin

CONFIG(debug, debug|release) {
    #message(Preparing debug build)
    DEFINES += DEBUG
}
else {
    #message(Preparing release build)
}

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/src/third_party $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/lib \
    -L$$TOP_DIR/gutil/lib \
    -lGUtil \
    -lpg_utils

SOURCES += \
    polyglotreader.cpp

HEADERS += \
    polyglotreader.h
