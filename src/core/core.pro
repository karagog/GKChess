
#QT          -= gui
TEMPLATE    = lib

TOP_DIR = ../..

DESTDIR = $$TOP_DIR/lib
TARGET = GKChess

CONFIG(debug, debug|release) {
    #message(Preparing debug build)
    DEFINES += DEBUG
}
else {
    #message(Preparing release build)
}

DEFINES += GUTIL_CORE_QT_ADAPTERS
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/lib \
    -L$$TOP_DIR/gutil/lib \
    -lGUtil \
    -lGUtilQt

include(managers/managers.pri)
include(business_objects/business_objects.pri)
include(data_access/data_access.pri)
include(utils/utils.pri)

HEADERS += \
    common.h
