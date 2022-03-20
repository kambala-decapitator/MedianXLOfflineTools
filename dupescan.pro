TARGET = MXLOT_TSW
include(app.pri)
QT += xml
SOURCES += src/dupescandialog.cpp \
    src/jsonwriter.cpp \
    src/xmlwriter.cpp
HEADERS += src/dupescandialog.h \
    src/ikeyvaluewriter.h \
    src/jsonwriter.h \
    src/xmlwriter.h
DEFINES += DUPE_CHECK
equals(QT_MAJOR_VERSION, 4): LIBS += -Lqjson-backport -lqjson-backport
