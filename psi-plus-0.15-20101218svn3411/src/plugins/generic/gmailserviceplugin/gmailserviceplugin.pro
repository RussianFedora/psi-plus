CONFIG += release
include(../../psiplugin.pri)
SOURCES += gmailserviceplugin.cpp \
    accountsettings.cpp \
    common.cpp \
    actionslist.cpp
FORMS += options.ui
HEADERS += gmailserviceplugin.h \
    accountsettings.h \
    common.h \
    actionslist.h
RESOURCES += gmailserviceplugin.qrc
