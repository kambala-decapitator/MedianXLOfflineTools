TARGET = MXLOT_TSW
include(app.pri)
QT += xml
SOURCES += dupescandialog.cpp \
    jsonwriter.cpp \
    xmlwriter.cpp
HEADERS += dupescandialog.h \
    ikeyvaluewriter.h \
    jsonwriter.h \
    xmlwriter.h
DEFINES += DUPE_CHECK
equals(QT_MAJOR_VERSION, 4): LIBS += -Lqjson-backport -lqjson-backport
