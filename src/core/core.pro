#-------------------------------------------------
#
# Project created by QtCreator 2014-02-16T09:24:24
#
#-------------------------------------------------

QT          -= gui
TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChess_core

DEFINES +=

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -lGUtil

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(business_objects/business_objects.pri)
include(business_logic/business_logic.pri)

HEADERS += \
    globals.h
