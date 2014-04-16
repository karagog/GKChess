#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T10:20:13
#
#-------------------------------------------------

TOP_DIR = ../../../../..

INCLUDEPATH += $$TOP_DIR/include $$TOP_DIR/gutil/include
LIBS += \
    -L$$TOP_DIR/gutil/lib \
    -L$$TOP_DIR/lib \
    -lGUtil \
    -lGKChess

QT       += core

TARGET = test_chess960_starting_positions
TEMPLATE = app


SOURCES += main.cpp

RESOURCES += 
