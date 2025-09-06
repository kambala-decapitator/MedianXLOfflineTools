TARGET = MedianXLOfflineTools
include(app.pri)

SOURCES += qtsingleapplication/qtsingleapplication.cpp \
           qtsingleapplication/qtlockedfile.cpp \
           qtsingleapplication/qtlocalpeer.cpp

HEADERS += qtsingleapplication/QtSingleApplication \
           qtsingleapplication/qtsingleapplication.h \
           qtsingleapplication/qtlockedfile.h \
           qtsingleapplication/qtlocalpeer.h

win32: SOURCES += qtsingleapplication/qtlockedfile_win.cpp
else:  SOURCES += qtsingleapplication/qtlockedfile_unix.cpp

!macx: DEFINES += HAS_QTSINGLEAPPLICATION=1
