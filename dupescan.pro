TARGET = MXLOT_TSW
include(app.pri)
QT += xml
SOURCES += dupescandialog.cpp \
    xmlwriter.cpp
HEADERS += dupescandialog.h \
    ikeyvaluewriter.h \
    xmlwriter.h
DEFINES += DUPE_CHECK
equals(QT_MAJOR_VERSION, 4): LIBS += -Lqjson-backport -lqjson-backport
