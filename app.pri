#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T19:24:55
#
#-------------------------------------------------

# basic config
TEMPLATE = app

QT += network
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

MOC_DIR = moc
OBJECTS_DIR = obj
RCC_DIR = rcc
UI_DIR = ui

greaterThan(QT_MAJOR_VERSION, 4): {
    DEFINES += IS_QT5
    IS_QT5 = 1

    QT += widgets concurrent
    *-clang*: cache()
}

CONFIG(release, debug|release): {
    IS_RELEASE_BUILD = 1
    DEFINES += QT_NO_DEBUG_OUTPUT \
               QT_NO_WARNING_OUTPUT
}

# app version
NVER1 = 0
NVER2 = 6
NVER3 = 6

greaterThan(NVER3, 0): VERSION = $$sprintf("%1.%2.%3", $$NVER1, $$NVER2, $$NVER3)
else                 : VERSION = $$sprintf("%1.%2", $$NVER1, $$NVER2)

DEFINES += NVER_STRING=$$sprintf("\"\\\"%1\\\"\"", $$VERSION)

# common files
SOURCES += src/main.cpp \
           src/medianxlofflinetools.cpp \
           src/resurrectpenaltydialog.cpp \
           src/qd2charrenamer.cpp \
           src/enums.cpp \
           src/itemdatabase.cpp \
           src/propertiesviewerwidget.cpp \
           src/itemsviewerdialog.cpp \
           src/itemstoragetablemodel.cpp \
           src/itemstoragetableview.cpp \
           src/itemspropertiessplitter.cpp \
           src/helpers.cpp \
           src/finditemsdialog.cpp \
           src/reversebitwriter.cpp \
           src/reversebitreader.cpp \
           src/itemparser.cpp \
           src/propertiesdisplaymanager.cpp \
           src/findresultswidget.cpp \
           src/application.cpp \
           src/experienceindicatorgroupbox.cpp \
           src/plugyitemssplitter.cpp \
           src/gearitemssplitter.cpp \
           src/kexpandablegroupbox.cpp \
           src/showselecteditemdelegate.cpp \
           src/disenchantpreviewdialog.cpp \
           src/disenchantpreviewmodel.cpp \
           src/stashsortingoptionsdialog.cpp \
           src/helpwindowdisplaymanager.cpp \
           src/skilltreedialog.cpp \
           src/allstatsdialog.cpp

HEADERS += src/medianxlofflinetools.h \
           src/resurrectpenaltydialog.h \
           src/qd2charrenamer.h \
           src/enums.h \
           src/colorsmanager.hpp \
           src/itemdatabase.h \
           src/structs.h \
           src/propertiesviewerwidget.h \
           src/itemsviewerdialog.h \
           src/itemstoragetablemodel.h \
           src/itemstoragetableview.h \
           src/itemspropertiessplitter.h \
           src/helpers.h \
           src/finditemsdialog.h \
           src/languagemanager.hpp \
           src/reversebitwriter.h \
           src/reversebitreader.h \
           src/itemparser.h \
           src/resourcepathmanager.hpp \
           src/propertiesdisplaymanager.h \
           src/findresultswidget.h \
           src/characterinfo.hpp \
           src/application.h \
           src/fileassociationmanager.h \
           src/messagecheckbox.h \
           src/experienceindicatorgroupbox.h \
           src/plugyitemssplitter.h \
           src/gearitemssplitter.h \
           src/kexpandablegroupbox.h \
           src/showselecteditemdelegate.h \
           src/disenchantpreviewdialog.h \
           src/progressbarmodal.hpp \
           src/itemnamestreewidget.hpp \
           src/disenchantpreviewmodel.h \
           src/checkboxsortfilterproxymodel.hpp \
           src/stashsortingoptionsdialog.h \
           src/helpwindowdisplaymanager.h \
           src/skilltreedialog.h \
           src/allstatsdialog.h

FORMS += src/medianxlofflinetools.ui \
         src/resurrectpenaltydialog.ui \
         src/qd2charrenamer.ui \
         src/propertiesviewerwidget.ui \
         src/finditemsdialog.ui \
         src/stashsortingoptionsdialog.ui \
         src/allstatsdialog.ui

RESOURCES += resources/medianxlofflinetools.qrc

TRANSLATIONS += resources/translations/medianxlofflinetools_ru.ts \
                resources/translations/medianxlofflinetools.ts

OTHER_FILES += TODO.txt

# QMAKE_CXXFLAGS += -std=c++11

# platform-specific
win32 {
    SOURCES += src/medianxlofflinetools_win.cpp \
               src/fileassociationmanager_win.cpp

    HEADERS += src/windowsincludes.h

    LIBS += -lshell32 \ # SHChangeNotify()
            -lole32   \ # CoCreateInstance() and other COM shit
            -luser32    # AllowSetForegroundWindow()

    RC_FILE = resources/win/medianxlofflinetools.rc
    OTHER_FILES += $$RC_FILE

    # defines (mostly for .rc file)
    DEFINES += NVER1=$$NVER1 \
               NVER2=$$NVER2 \
               NVER3=$$NVER3 \
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
}

macx {
    OBJECTIVE_SOURCES += src/application_mac.mm \
                         src/messagecheckbox_mac_p.mm \
                         src/medianxlofflinetools_mac.mm \
                         src/machelpers.mm \
                         src/fileassociationmanager_mac.mm

    OBJECTIVE_HEADERS += src/machelpers.h

    LIBS += -framework ApplicationServices \ # LSGetApplicationForInfo()
            -framework AppKit                # NSWindow calls to disable Lion window restoration and NSAlert

    INFO_PLIST_NAME = Info.plist
    QMAKE_INFO_PLIST = resources/mac/$$INFO_PLIST_NAME
    OTHER_FILES += $$QMAKE_INFO_PLIST

    ICON = resources/mac/icon.icns

    CONTENTS_PATH  = $${TARGET}.app/Contents
    RESOURCES_PATH = $$CONTENTS_PATH/Resources

    !isEmpty(IS_QT5) {
        QMAKE_MAC_SDK = macosx
    }
    else {
        *-clang* {
            QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
        }
        else {
            # release build is intended to be compiled with PPC support
            !isEmpty(IS_RELEASE_BUILD) {
                MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
                if (!exists($$MAC_SDK)): MAC_SDK = /Developer/Xcode3.2.6/SDKs/MacOSX10.5.sdk
                QMAKE_MAC_SDK = $$MAC_SDK

                CONFIG += x86 ppc
                CONFIG -= x86_64
                QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
            }
        }
    }

    INFO_PLIST_PATH = $$CONTENTS_PATH/$$INFO_PLIST_NAME
    # literal $ is either $$ or \$$
    QMAKE_POST_LINK = sed -e \'s/@CMAKE_PROJECT_VERSION@/$$VERSION/\' -i \'\' $$INFO_PLIST_PATH;

    PROJECT_DATA = resources/data
    PROJECT_TR   = resources/translations
    BUNDLE_DATA  = $$RESOURCES_PATH/data
    BUNDLE_TR    = $$RESOURCES_PATH/translations

    isEmpty(IS_RELEASE_BUILD) {
    }
    else {
        appresources.files += $$PROJECT_DATA
        appresources.files += $$PROJECT_TR

        # remove unused files
        QMAKE_POST_LINK += rm -rf $$BUNDLE_TR/*.ts;
    }

    appresources.files += resources/mac/locversion.plist
    appresources.path = Contents/Resources
    QMAKE_BUNDLE_DATA += appresources
}
else: SOURCES += src/messagecheckbox_p.cpp

isEmpty(IS_RELEASE_BUILD): DEFINES += DATA_PATH=$$sprintf("\"\\\"%1\\\"\"", $$_PRO_FILE_PWD_/resources)
