
TEMPLATE = subdirs

# Each library may depend on the ones listed above it, but necessarily don't
#   depend on libraries below it.
SUBDIRS += \
    pre_build \
    core \
    presentation \
    apps \
    doc

CONFIG += ordered

