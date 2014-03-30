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
    -lGUtilQt \
    -lGKChess \
    -lGKChessUI

QT       += core gui webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_boardview
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
