#-------------------------------------------------
#
# Project created by QtCreator 2011-08-01T19:24:55
#
#-------------------------------------------------

TARGET = MedianXLOfflineTools
TEMPLATE = app
VERSION = 0.2


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
    itemparser.cpp

HEADERS  += medianxlofflinetools.h \
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
	resourcepathmanager.hpp

FORMS    += medianxlofflinetools.ui \
    resurrectpenaltydialog.ui \
    qd2charrenamer.ui \
    propertiesviewerwidget.ui \
	finditemsdialog.ui

TRANSLATIONS += medianxlofflinetools_ru.ts

RESOURCES += medianxlofflinetools.qrc


macx {
	OTHER_FILES += Resources/mac/Info.plist
	
	ICON = Resources/mac/icon.icns
	QMAKE_INFO_PLIST = Resources/mac/Info.plist

    CONFIG(release, debug|release) {
        message(release build)
        QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
        CONFIG += x86 ppc
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
    }
}
win32 {
	# RC_FILE = Resources/win/icon.rc
	# LIBS += "c:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib/shell32.lib"
}

# QMAKE_CXXFLAGS += -std=c++0x
