TEMPLATE    = lib

TOP_DIR = ../../..

DESTDIR = $$TOP_DIR/lib
TARGET = pg_utils

#DEFINES += DEBUG

DEFINES += DLL_EXPORT

unix{
QMAKE_CFLAGS += -fvisibility=hidden
}

HEADERS += \
    board.h \
    book.h \
    hash.h \
    move.h \
    lookup.h \
    file.h \
    pg_utils.h

SOURCES += \
    lookup.c \
    file.c \
    hash.c \
    board.c \
    book.c \
    move.c

OTHER_FILES += \
    README \
    CHANGES
