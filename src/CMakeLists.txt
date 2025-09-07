target_sources(MedianXLOfflineTools PRIVATE
	allstatsdialog.cpp
	allstatsdialog.h
	allstatsdialog.ui
	application.cpp
	application.h
	characterinfo.hpp
	checkboxsortfilterproxymodel.hpp
	colorsmanager.cpp
	colorsmanager.h
	disenchantpreviewdialog.cpp
	disenchantpreviewdialog.h
	disenchantpreviewmodel.cpp
	disenchantpreviewmodel.h
	enums.cpp
	enums.h
	experienceindicatorgroupbox.cpp
	experienceindicatorgroupbox.h
	fileassociationmanager.h
	finditemsdialog.cpp
	finditemsdialog.h
	finditemsdialog.ui
	findresultswidget.cpp
	findresultswidget.h
	gearitemssplitter.cpp
	gearitemssplitter.h
	helpers.cpp
	helpers.h
	helpwindowdisplaymanager.cpp
	helpwindowdisplaymanager.h
	itemdatabase.cpp
	itemdatabase.h
	itemnamestreewidget.hpp
	itemparser.cpp
	itemparser.h
	itemspropertiessplitter.cpp
	itemspropertiessplitter.h
	itemstoragetablemodel.cpp
	itemstoragetablemodel.h
	itemstoragetableview.cpp
	itemstoragetableview.h
	itemsviewerdialog.cpp
	itemsviewerdialog.h
	kexpandablegroupbox.cpp
	kexpandablegroupbox.h
	languagemanager.hpp
	main.cpp
	medianxlofflinetools.cpp
	medianxlofflinetools.h
	medianxlofflinetools.ui
	messagecheckbox.h
	plugyitemssplitter.cpp
	plugyitemssplitter.h
	progressbarmodal.hpp
	propertiesdisplaymanager.cpp
	propertiesdisplaymanager.h
	propertiesviewerwidget.cpp
	propertiesviewerwidget.h
	propertiesviewerwidget.ui
	qd2charrenamer.cpp
	qd2charrenamer.h
	qd2charrenamer.ui
	resourcepathmanager.hpp
	resurrectpenaltydialog.cpp
	resurrectpenaltydialog.h
	resurrectpenaltydialog.ui
	reversebitreader.cpp
	reversebitreader.h
	reversebitwriter.cpp
	reversebitwriter.h
	showselecteditemdelegate.cpp
	showselecteditemdelegate.h
	skilltreedialog.cpp
	skilltreedialog.h
	stashsortingoptionsdialog.cpp
	stashsortingoptionsdialog.h
	stashsortingoptionsdialog.ui
	structs.h
)

if(WIN32)
	target_sources(MedianXLOfflineTools PRIVATE
		fileassociationmanager_win.cpp
		medianxlofflinetools_win.cpp
		messagecheckbox_p.cpp
		windowsincludes.h
	)
	target_link_libraries(MedianXLOfflineTools PRIVATE
		ole32 # COM shit
		shell32 # SHChangeNotify()
		user32 # AllowSetForegroundWindow()
	)
elseif(APPLE)
	target_sources(MedianXLOfflineTools PRIVATE
		application_mac.mm
		fileassociationmanager_mac.mm
		machelpers.h
		machelpers.mm
		medianxlofflinetools_mac.mm
		messagecheckbox_mac_p.mm
	)
	target_link_libraries(MedianXLOfflineTools PRIVATE
		"-framework ApplicationServices" # LSGetApplicationForInfo()
		"-framework AppKit" # NSWindow calls to disable Lion window restoration and NSAlert
	)
else()
	target_sources(MedianXLOfflineTools PRIVATE
		messagecheckbox_p.cpp
	)
endif()
