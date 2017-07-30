include($$PWD/qtcec.pri)

HEADERS += qtcecagent.h
SOURCES += main.cpp \
    qtcecagent.cpp

INCLUDEPATH += .

TARGET = qtcecplugin

PLUGIN_TYPE = generic
PLUGIN_EXTENDS = -
PLUGIN_CLASS_NAME = QtCECPlugin
load(qt_plugin)

DESTDIR=$${TOPLEVEL}/out

OTHER_FILES += \
    configure.json
