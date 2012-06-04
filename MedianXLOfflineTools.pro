#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T19:24:55
#
#-------------------------------------------------

TARGET = MedianXLOfflineTools
TEMPLATE = app
VERSION = 0.2.2
QT += network


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
           application.cpp \
           qtsingleapplication/qtsingleapplication.cpp \
           qtsingleapplication/qtlockedfile.cpp \
           qtsingleapplication/qtlocalpeer.cpp \
           experienceindicatorgroupbox.cpp

HEADERS += medianxlofflinetools.h \
           resurrectpenaltydialog.h \
           qd2charrenamer.h \
           enums.h \
           colorsmanager.hpp \
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
           application.h \
           qtsingleapplication/qtsingleapplication.h \
           qtsingleapplication/qtlockedfile.h \
           qtsingleapplication/qtlocalpeer.h \
           fileassociationmanager.h \
           experienceindicatorgroupbox.h

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
    SOURCES += medianxlofflinetools_win.cpp \
               qtsingleapplication/qtlockedfile_win.cpp \
               fileassociationmanager_win.cpp
               
    HEADERS += windowsincludes.h

    LIBS += -lshell32 \ # SHChangeNotify()
            -lole32 \   # CoCreateInstance() and other COM shit
            -luser32    # AllowSetForegroundWindow()
    
    RC_FILE = resources/win/medianxlofflinetools.rc
}
macx {
    SOURCES += fileassociationmanager_mac.cpp

    OBJECTIVE_SOURCES += application_mac.mm \
                         messagecheckbox_mac.mm

    OBJECTIVE_HEADERS += messagecheckbox.h

    LIBS += -framework ApplicationServices \ # LSGetApplicationForInfo() 
            -framework AppKit                # NSWindow calls to disable Lion window resoration and NSAlert

    ICON = resources/mac/icon.icns
    QMAKE_INFO_PLIST = resources/mac/Info.plist
    
    OTHER_FILES += resources/mac/Info.plist

    # for Xcode 4.3+
    MAC_SDK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
    # for earlier versions
    if (!exists($$MAC_SDK)) {
        MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
    }

    # release build is intended to be compiled on 10.6 (or even earlier) for PPC support
    CONFIG(release, debug|release) {
        message(release build)
        MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
        CONFIG += x86 ppc
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
    }

    if (!exists($$MAC_SDK)) {
        error(The selected Mac OS X SDK does not exist at $$MAC_SDK!)
    }
    QMAKE_MAC_SDK = $$MAC_SDK
}
unix {
    SOURCES += qtsingleapplication/qtlockedfile_unix.cpp
}

!macx {
    SOURCES += messagecheckbox.cpp

    HEADERS += messagecheckbox.h
}
