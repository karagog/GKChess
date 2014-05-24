
QT += core gui

TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChessUI

DEFINES += \
    GUTIL_CORE_QT_ADAPTERS

CONFIG(debug, debug|release) {
    #message(Preparing debug build)
    DEFINES += DEBUG
}
else {
    #message(Preparing release build)
}

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -L$$TOP_DIR/lib \
    -lGUtil \
    -lGUtilQt \
    -lGKChess

include(forms/forms.pri)
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
