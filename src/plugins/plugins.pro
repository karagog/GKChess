
TEMPLATE = subdirs

# Each library may depend on the ones listed above it, but necessarily don't
#   depend on libraries below it.
SUBDIRS += \
    pg_utils

CONFIG += ordered

