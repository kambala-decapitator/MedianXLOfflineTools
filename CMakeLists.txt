cmake_minimum_required(VERSION 3.18) # 3.20 for macOS

project(MedianXLOfflineTools
	VERSION 0.6.6
	LANGUAGES CXX
)
if(APPLE)
	enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# dependencies

set(qtComponents Core Gui Network)
set(qtComponents5 ${qtComponents} Widgets)

find_package(QT NAMES Qt5 COMPONENTS ${qtComponents5})
if(QT_FOUND)
	find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${qtComponents5} REQUIRED)
	set(qtComponents ${qtComponents5})
	add_compile_definitions(IS_QT5=1)

	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
else()
	set(qtComponents4 ${qtComponents})
	list(TRANSFORM qtComponents4 PREPEND "Qt")
	find_package(Qt4 REQUIRED ${qtComponents4})
	foreach(component IN ZIP_LISTS qtComponents qtComponents4)
		add_library("Qt4::${component_0}" ALIAS "Qt4::${component_1}")
	endforeach()

	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
endif()

function(linkQt tgt)
	foreach(component ${qtComponents})
		target_link_libraries(${tgt} PRIVATE
			"Qt${QT_VERSION_MAJOR}::${component}"
		)
	endforeach()
endfunction()

# targets

if(WIN32)
	add_compile_definitions(NOMINMAX)
	if(QT_VERSION_MAJOR LESS 6)
		add_compile_definitions(
			UNICODE
			_UNICODE
		)
	endif()
endif()

add_executable(MedianXLOfflineTools WIN32 MACOSX_BUNDLE
	resources/medianxlofflinetools.qrc
)
add_subdirectory(src)
linkQt(MedianXLOfflineTools)
target_compile_definitions(MedianXLOfflineTools PRIVATE
	"NVER1=${CMAKE_PROJECT_VERSION_MAJOR}"
	"NVER2=${CMAKE_PROJECT_VERSION_MINOR}"
	"NVER3=${CMAKE_PROJECT_VERSION_PATCH}"
	"NVER_STRING=\"${CMAKE_PROJECT_VERSION}\""
	"$<$<CONFIG:Debug>:DATA_PATH=\"${CMAKE_SOURCE_DIR}/resources\">"
)

if(WIN32)
	target_sources(MedianXLOfflineTools PRIVATE resources/win/medianxlofflinetools.rc)
elseif(APPLE)
	set(ICON "icon.icns")
	set(iconPath "resources/mac/${ICON}")
	set_source_files_properties(${iconPath} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	target_sources(MedianXLOfflineTools PRIVATE ${iconPath})

	set(infoPlist "Info.plist")
	set(EXECUTABLE "MedianXLOfflineTools")
	configure_file("resources/mac/${infoPlist}" ${infoPlist} @ONLY)
	set_target_properties(MedianXLOfflineTools PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/${infoPlist}"
	)
endif()

# macOS already runs apps in a single instance basically
if(NOT APPLE)
	add_subdirectory(qtsingleapplication)
	target_link_libraries(MedianXLOfflineTools PRIVATE QtSingleApplication)
    target_compile_definitions(MedianXLOfflineTools PRIVATE HAS_QTSINGLEAPPLICATION=1)
endif()
