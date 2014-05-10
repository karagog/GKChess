TEMPLATE    = lib

TOP_DIR = ../../..

DESTDIR = $$TOP_DIR/lib
TARGET = pg_utils

#DEFINES += DEBUG

HEADERS += \
    board.h \
    book.h \
    hash.h \
    move.h \
    pg_types.h

SOURCES += \
    board.cpp \
    book.cpp \
    hash.cpp \
    move.cpp

OTHER_FILES += \
    README
