
TEMPLATE = lib
CONFIG += staticlib

TOP_DIR = ../..

ZIP_CMD = zip
DOCS_COMMAND = doxygen
DOCS_PATH = $$TOP_DIR/doc
DOCS_DIR = html
DOCS_ZIP_TARGET = GKChess_docs.zip

docs_clean.commands =
exists($$DOCS_PATH/$$DOCS_DIR){
    docs_clean.commands = rm $$DOCS_PATH/$$DOCS_DIR -rf;
}
exists($$DOCS_PATH/$$DOCS_ZIP_TARGET){
    docs_clean.commands += rm $$DOCS_PATH/$$DOCS_ZIP_TARGET;
}

docs_gen.commands = $$DOCS_COMMAND
docs_zip.commands = cd $$DOCS_PATH; zip $$DOCS_ZIP_TARGET -r $$DOCS_DIR;


PRE_TARGETDEPS += docs_clean
PRE_TARGETDEPS += docs_gen
PRE_TARGETDEPS += docs_zip

QMAKE_EXTRA_TARGETS += docs_clean
QMAKE_EXTRA_TARGETS += docs_gen
QMAKE_EXTRA_TARGETS += docs_zip

OTHER_FILES += Doxyfile
