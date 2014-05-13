
# The pre-build project is designed to prepare the GUtil project
#  for building in a software development environment.

# It has several tasks:

#  1.) Generate top-level GUtil headers (gutil_core.h, gutil_qt.h, etc...)
#  2.) Generate doxygen documentation (and compress it in a .zip)
#  3.) TBD...

TEMPLATE = lib
CONFIG += staticlib

TOP_DIR = ../..

HEADER_CMD = python $$TOP_DIR/gutil/scripts/GenerateHeaders.py
HEADER_PREFIX = gkchess_

# Directory patterns for which we want to ignore all headers
IGNORE_PATHS = Test

# File patterns to ignore
IGNORE_FILES = *_p.h,ui_*.h

HEADERGEN_TARGET_DIRS = core,presentation,plugins


headers.commands = $$HEADER_CMD \
                        --working-dir=.. \
                        --output-dir=../include \
                        --input-dirs=$$HEADERGEN_TARGET_DIRS \
                        --ignore-path=$$IGNORE_PATHS \
                        --output-prefix=$$HEADER_PREFIX \
                        --ignore-patterns=$$IGNORE_FILES

PRE_TARGETDEPS =  headers

QMAKE_EXTRA_TARGETS =  headers
