#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T19:24:55
#
#-------------------------------------------------

# basic config
TARGET = MedianXLOfflineTools
TEMPLATE = app

QT += network

greaterThan(QT_MAJOR_VERSION, 4): {
    DEFINES += IS_QT5
    IS_QT5 = 1

    QT += widgets
    *-clang*: cache()
}

CONFIG(release, debug|release): {
    IS_RELEASE_BUILD = 1
    DEFINES += QT_NO_DEBUG_OUTPUT \
               QT_NO_WARNING_OUTPUT
}

# app version
NVER1 = 0
NVER2 = 4
NVER3 = 1
NVER4 = 0

      greaterThan(NVER4, 0): NVER_STRING_LAST = $$sprintf("%1.%2", $$NVER3, $$NVER4)
else: greaterThan(NVER3, 0): NVER_STRING_LAST = $$sprintf("%1", $$NVER3)

isEmpty(NVER_STRING_LAST): VERSION = $$sprintf("%1.%2", $$NVER1, $$NVER2)
else                     : VERSION = $$sprintf("%1.%2.%3", $$NVER1, $$NVER2, $$NVER_STRING_LAST)

DEFINES += NVER_STRING=$$sprintf("\"\\\"%1\\\"\"", $$VERSION)

# common files
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
           stashsortingoptionsdialog.cpp \
           helpwindowdisplaymanager.cpp

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
           stashsortingoptionsdialog.h \
           helpwindowdisplaymanager.h

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

# platform-specific
win32 {
    SOURCES += medianxlofflinetools_win.cpp \
               qtsingleapplication/qtlockedfile_win.cpp \
               fileassociationmanager_win.cpp

    HEADERS += windowsincludes.h

    LIBS += -lshell32 \ # SHChangeNotify()
            -lole32   \ # CoCreateInstance() and other COM shit
            -luser32    # AllowSetForegroundWindow()

    RC_FILE = resources/win/medianxlofflinetools.rc
    OTHER_FILES += $$RC_FILE

    # defines (mostly for .rc file)
    DEFINES += NVER1=$$NVER1 \
               NVER2=$$NVER2 \
               NVER3=$$NVER3 \
               NVER4=$$NVER4 \
               BUILDING_FROM_PRO \ # to set app version in .rc correctly
               NOMINMAX # disables min/max macros which fixes error with QDateTime


    defineReplace(toNativeSeparators) {
        path = $$1
        path ~= s,/,\\,g
        return($$path)
    }

    isEmpty(IS_RELEASE_BUILD) {
        DEFINES += _DEBUG

        OUT_FOLDER = debug
    }
    else {
        DEFINES += _USING_V110_SDK71_ # for WinXP support in MSVS2012

        OUT_FOLDER = release
    }

    # create symbolic link to 'resources' folder in the folder of .exe
    LINK_DST = $$OUT_PWD/$$OUT_FOLDER/resources
    !exists($$LINK_DST): QMAKE_POST_LINK = mklink /D $$toNativeSeparators($$LINK_DST) $$toNativeSeparators($$_PRO_FILE_PWD_/resources)
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

    CONTENTS_PATH  = $${TARGET}.app/Contents
    RESOURCES_PATH = $$CONTENTS_PATH/Resources

    !isEmpty(IS_QT5) {
        # Qt 5 uses new approach to QMAKE_MAC_SDK, so default value is usually ok
#        QMAKE_MAC_SDK = macosx10.8

        # make dock menu work on Qt5. code from https://qt.gitorious.org/qt/qtmacextras
        OBJECTIVE_SOURCES += qtmacextras/qmacfunctions.mm \
                             qtmacextras/qmacfunctions_mac.mm

        OBJECTIVE_HEADERS += qtmacextras/qmacfunctions.h \
                             qtmacextras/qmacfunctions_p.h \
                             qtmacextras/qmacextrasglobal.h

        QT += gui-private # to remove this dependency, compile qtmacextras in a library
    }
    else {
        SDK_PATH_OLD = /Developer/SDKs
        SDK_PATH_NEW = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform$$SDK_PATH_OLD

        # for Xcode 4.3+
        MAC_SDK = $$SDK_PATH_NEW/MacOSX10.8.sdk
        if (!exists($$MAC_SDK)): MAC_SDK = $$SDK_PATH_NEW/MacOSX10.7.sdk
        # for earlier versions
        if (!exists($$MAC_SDK)): MAC_SDK = $$SDK_PATH_OLD/MacOSX10.6.sdk
        if (!exists($$MAC_SDK)): MAC_SDK = $$SDK_PATH_OLD/MacOSX10.5.sdk

        # release build is intended to be compiled with PPC support
        !isEmpty(IS_RELEASE_BUILD) {
            MAC_SDK = $$SDK_PATH_OLD/MacOSX10.5.sdk
            if (!exists($$MAC_SDK)): MAC_SDK = /Developer/Xcode3.2.6/SDKs/MacOSX10.5.sdk

            CONFIG += x86 ppc
            QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
        }

        if (!exists($$MAC_SDK)): error(Selected Mac OS X SDK does not exist at $$MAC_SDK!)
        QMAKE_MAC_SDK = $$MAC_SDK
    }

    COPYRIGHT = Copyright © kambala 2011-2013
    INFO_PLIST_PATH = $$CONTENTS_PATH/$$INFO_PLIST_NAME
    QMAKE_POST_LINK = sed -e \'s/@APP_VERSION@/$$VERSION/\' -e \'s/@COPYRIGHT@/$$COPYRIGHT/\' -i \'\' $$INFO_PLIST_PATH;

    PROJECT_DATA = resources/data
    PROJECT_TR   = resources/translations
    BUNDLE_DATA  = $$RESOURCES_PATH/data
    BUNDLE_TR    = $$RESOURCES_PATH/translations

    isEmpty(IS_RELEASE_BUILD) {
        # create symlinks instead of copying in debug mode
        QMAKE_POST_LINK += [ -L $$BUNDLE_DATA ] || ln -s $$_PRO_FILE_PWD_/$$PROJECT_DATA $$BUNDLE_DATA;
        QMAKE_POST_LINK += [ -L $$BUNDLE_TR ]   || ln -s $$_PRO_FILE_PWD_/$$PROJECT_TR   $$BUNDLE_TR;
    }
    else {
        appresources.files += $$PROJECT_DATA
        appresources.files += $$PROJECT_TR

        # remove unused files
        QMAKE_POST_LINK += rm -rf $$BUNDLE_DATA/items $$BUNDLE_TR/*.ts;
    }

    appresources.files += resources/mac/locversion.plist
    appresources.path = Contents/Resources
    QMAKE_BUNDLE_DATA += appresources
}
else: SOURCES += messagecheckbox_p.cpp

unix: SOURCES += qtsingleapplication/qtlockedfile_unix.cpp
