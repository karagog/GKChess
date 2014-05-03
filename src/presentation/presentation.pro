
QT += core gui

TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChessUI

DEFINES += \
    DEBUG \
    GUTIL_CORE_QT_ADAPTERS

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -L$$TOP_DIR/lib \
    -lGUtil \
    -lGUtilQt \
    -lGKChess

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(views/views.pri)
include(models/models.pri)
include(controls/controls.pri)
include(data_access/data_access.pri)

HEADERS += \
    ifactory_pieceicon.h \
    uiglobals.h \
    coloredpieceiconfactory.h

RESOURCES += \
    resources.qrc \
    icons/default/icons_default.qrc

SOURCES += \
    uiglobals.cpp \
    coloredpieceiconfactory.cpp
