CONFIG += release
TARGET = stopspamplugin
include(../../psiplugin.pri)
SOURCES += stopspamplugin.cpp \
    view.cpp \
    model.cpp \
    viewer.cpp \
    typeaheadfind.cpp
HEADERS += view.h \
    model.h \
    viewer.h \
    typeaheadfind.h
