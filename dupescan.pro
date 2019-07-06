TARGET = MXLOT_TSW
include(app.pri)
QT += xml
SOURCES += dupescandialog.cpp
HEADERS += dupescandialog.h
DEFINES += DUPE_CHECK
equals(QT_MAJOR_VERSION, 4): LIBS += -Lqjson-backport -lqjson-backport
