#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T19:24:55
#
#-------------------------------------------------

TARGET = MedianXLOfflineTools
TEMPLATE = app
VERSION = 0.3


SOURCES += main.cpp \
           medianxlofflinetools.cpp \
           resurrectpenaltydialog.cpp \
           qd2charrenamer.cpp \
           enums.cpp \
           itemdatabase.cpp \
           propertiesviewerwidget.cpp \
           itemsviewerdialog.cpp \
           itemstoragetablemodel.cpp \
           itemstoragetableview.cpp \
           itemspropertiessplitter.cpp \
           helpers.cpp \
           finditemsdialog.cpp \
           reversebitwriter.cpp \
           reversebitreader.cpp \
           itemparser.cpp \
           propertiesdisplaymanager.cpp \
           findresultswidget.cpp \
           skillplandialog.cpp \
           application.cpp

HEADERS += medianxlofflinetools.h \
           resurrectpenaltydialog.h \
           qd2charrenamer.h \
           enums.h \
           colors.hpp \
           itemdatabase.h \
           structs.h \
           propertiesviewerwidget.h \
           itemsviewerdialog.h \
           itemstoragetablemodel.h \
           itemstoragetableview.h \
           itemspropertiessplitter.h \
           helpers.h \
           finditemsdialog.h \
           languagemanager.hpp \
           reversebitwriter.h \
           reversebitreader.h \
           itemparser.h \
           resourcepathmanager.hpp \
           propertiesdisplaymanager.h \
           findresultswidget.h \
           characterinfo.hpp \
           skillplandialog.h \
           application.h

FORMS += medianxlofflinetools.ui \
         resurrectpenaltydialog.ui \
         qd2charrenamer.ui \
         propertiesviewerwidget.ui \
         finditemsdialog.ui \
         skillplandialog.ui

RESOURCES += resources/medianxlofflinetools.qrc

TRANSLATIONS += resources/translations/medianxlofflinetools_ru.ts \
                resources/translations/medianxlofflinetools.ts

OTHER_FILES += TODO.txt


# QMAKE_CXXFLAGS += -std=c++0x


win32 {
    SOURCES += medianxlofflinetools_win.cpp

    LIBS += -lshell32 \ # SHChangeNotify()
        -lole32         # CoCreateInstance()
    
    RC_FILE = resources/win/medianxlofflinetools.rc
}
macx {
    SOURCES += medianxlofflinetools_mac.cpp

    OBJECTIVE_SOURCES += application_mac.mm

    LIBS += -framework ApplicationServices \
            -framework AppKit

    ICON = resources/mac/icon.icns
    QMAKE_INFO_PLIST = resources/mac/Info.plist
    
    OTHER_FILES += resources/mac/Info.plist

    # release build is intended to be compiled on 10.6 (or even earlier) for PPC support
    CONFIG(release, debug|release) {
        message(release build)
        QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
        CONFIG += x86 ppc
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
    }
}
