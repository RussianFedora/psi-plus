HEADERS = screenshot.h \
    server.h \
    editserverdlg.h \
    screenshotoptions.h \
    toolbar.h \
    pixmapwidget.h
SOURCES = screenshotplugin.cpp \
    screenshot.cpp \
    server.cpp \
    editserverdlg.cpp \
    screenshotoptions.cpp \
    toolbar.cpp \
    pixmapwidget.cpp
include(../../psiplugin.pri)
CONFIG += release
QT += network
FORMS += options.ui \
    editserverdlg.ui \
    screenshot.ui \
    screenshotoptions.ui
RESOURCES += screenshotplugin.qrc
