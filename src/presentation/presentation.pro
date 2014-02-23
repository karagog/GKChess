
QT += core gui

TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChess_presentation

DEFINES +=

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    #-lGUtil

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(views/views.pri)
include(models/models.pri)
