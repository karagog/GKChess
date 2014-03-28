
QT += core gui

TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChess_presentation

DEFINES += GUTIL_CORE_QT_ADAPTERS

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -L$$TOP_DIR/lib \
    -lGUtil \
    -lGUtilQt \
    -lGKChess_core

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(views/views.pri)
include(models/models.pri)

HEADERS += \
    ipieceiconfactory.h
