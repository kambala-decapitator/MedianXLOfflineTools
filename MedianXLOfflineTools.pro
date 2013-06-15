#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T19:24:55
#
#-------------------------------------------------

# basic config
TARGET = MedianXLOfflineTools
TEMPLATE = app

QT += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets # for Qt 5

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

# app version
NVER1 = 0
NVER2 = 4
NVER3 = 0
NVER4 = 0

      greaterThan(NVER4, 0): NVER_STRING_LAST = $$sprintf("%1.%2", $$NVER3, $$NVER4)
else: greaterThan(NVER3, 0): NVER_STRING_LAST = $$sprintf("%1", $$NVER3)

isEmpty(NVER_STRING_LAST): VERSION = $$sprintf("%1.%2", $$NVER1, $$NVER2)
else                     : VERSION = $$sprintf("%1.%2.%3", $$NVER1, $$NVER2, $$NVER_STRING_LAST)

DEFINES += NVER_STRING=$$sprintf("\"\\\"%1\\\"\"", $$VERSION)

# files
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
           experienceindicatorgroupbox.cpp \
           plugyitemssplitter.cpp \
           gearitemssplitter.cpp \
           kexpandablegroupbox.cpp \
           showselecteditemdelegate.cpp \
           disenchantpreviewdialog.cpp \
           disenchantpreviewmodel.cpp \
           dupescandialog.cpp \
           stashsortingoptionsdialog.cpp

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
           messagecheckbox.h \
           experienceindicatorgroupbox.h \
           plugyitemssplitter.h \
           gearitemssplitter.h \
           kexpandablegroupbox.h \
           showselecteditemdelegate.h \
           disenchantpreviewdialog.h \
           progressbarmodal.hpp \
           itemnamestreewidget.hpp \
           disenchantpreviewmodel.h \
           checkboxsortfilterproxymodel.hpp \
           dupescandialog.h \
           stashsortingoptionsdialog.h

FORMS += medianxlofflinetools.ui \
         resurrectpenaltydialog.ui \
         qd2charrenamer.ui \
         propertiesviewerwidget.ui \
         finditemsdialog.ui \
         skillplandialog.ui \
         stashsortingoptionsdialog.ui

RESOURCES += resources/medianxlofflinetools.qrc

TRANSLATIONS += resources/translations/medianxlofflinetools_ru.ts \
                resources/translations/medianxlofflinetools.ts

OTHER_FILES += TODO.txt

# QMAKE_CXXFLAGS += -std=c++11

win32 {
    SOURCES += medianxlofflinetools_win.cpp \
               qtsingleapplication/qtlockedfile_win.cpp \
               fileassociationmanager_win.cpp

    HEADERS += windowsincludes.h

    LIBS += -lshell32 \ # SHChangeNotify()
            -lole32   \ # CoCreateInstance() and other COM shit
            -luser32    # AllowSetForegroundWindow()

    RC_FILE = resources/win/medianxlofflinetools.rc

    DEFINES += NVER1=$$NVER1 \
               NVER2=$$NVER2 \
               NVER3=$$NVER3 \
               NVER4=$$NVER4

#    CONFIG(release, debug|release) {
#        appresources.path = $$DESTDIR
#        appresources.files += resources/translations/*.qm
#        appresources.files += resources/data/*
#        appresources.files -= resources/data/items
#        INSTALLS += appresources

#        copyfiles.commands += @call xcopy resources\\translations\\*.qm $$DESTDIR\\resources\\translations\\ /Y /S
#        copyfiles.commands += @call xcopy resources/data/* $$DESTDIR/resources/data/ /Y /S
#        copyfiles.commands += @call rmdir $$DESTDIR/resources/data/items /S /Q
#        QMAKE_EXTRA_TARGETS += copyfiles
#    }
}

macx {
    OBJECTIVE_SOURCES += application_mac.mm \
                         messagecheckbox_mac_p.mm \
                         medianxlofflinetools_mac.mm \
                         machelpers.mm \
                         fileassociationmanager_mac.mm

    OBJECTIVE_HEADERS += machelpers.h

    LIBS += -framework ApplicationServices \ # LSGetApplicationForInfo()
            -framework AppKit                # NSWindow calls to disable Lion window resoration and NSAlert

    INFO_PLIST_NAME = Info.plist
    QMAKE_INFO_PLIST = resources/mac/$$INFO_PLIST_NAME
    OTHER_FILES += $$QMAKE_INFO_PLIST

    ICON = resources/mac/icon.icns

    # for Xcode 4.3+
    MAC_SDK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
    if (!exists($$MAC_SDK)) {
        MAC_SDK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
    }
    # for earlier versions
    if (!exists($$MAC_SDK)) {
        MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
    }
    if (!exists($$MAC_SDK)) {
        MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
    }

    # release build is intended to be compiled on 10.6 (or even earlier) for PPC support
    CONFIG(release, debug|release) {
        message(release build)
        MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
        CONFIG += x86 ppc
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5

        appresources.files += resources/translations
        appresources.files += resources/data
    }

    if (!exists($$MAC_SDK)) {
        error(Selected Mac OS X SDK does not exist at $$MAC_SDK!)
    }
    QMAKE_MAC_SDK = $$MAC_SDK

    appresources.files += resources/mac/locversion.plist
    appresources.path = Contents/Resources
    QMAKE_BUNDLE_DATA += appresources

    QMAKE_PRE_LINK = sed -e \'s/@APP_VERSION@/$$VERSION/\' -i \'\' $${TARGET}.app/Contents/$$INFO_PLIST_NAME
}
else: SOURCES += messagecheckbox_p.cpp

unix: SOURCES += qtsingleapplication/qtlockedfile_unix.cpp
