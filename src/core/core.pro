
#QT          -= gui
TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChess

DEFINES += DEBUG

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -lGUtil

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(business_objects/business_objects.pri)
include(utils/utils.pri)

HEADERS += \
    globals.h
