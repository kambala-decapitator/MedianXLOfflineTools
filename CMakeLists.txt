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

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)
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

# install

install(TARGETS MedianXLOfflineTools
	BUNDLE DESTINATION "."
	RUNTIME DESTINATION "."
)

if(APPLE)
	set(resourcesDir "$<TARGET_BUNDLE_DIR_NAME:MedianXLOfflineTools>/Contents/Resources")

	if(QT_VERSION_MAJOR EQUAL 4)
		install(FILES "resources/mac/locversion.plist" DESTINATION ${resourcesDir})
	endif()
	install(FILES "resources/mac/disable_auto_load.command" DESTINATION ".")
else()
	set(resourcesDir ".")

	if(WIN32)
		install(FILES "resources/win/disable_auto_load.reg" DESTINATION ${resourcesDir})
	endif()
endif()
install(DIRECTORY "resources/data" DESTINATION ${resourcesDir})
install(DIRECTORY "resources/translations" DESTINATION ${resourcesDir}
	FILES_MATCHING PATTERN "*.qm"
)

# TODO: use qt_generate_deploy_app_script() with Qt 6
set(installedExeWindows "\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:MedianXLOfflineTools>")
if(APPLE)
	set(deployQtToolName "macdeployqt")
	set(deployQtOptions "\"\${CMAKE_INSTALL_PREFIX}/$<TARGET_BUNDLE_DIR_NAME:MedianXLOfflineTools>\"")
elseif(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
	set(deployQtToolName "windeployqt")
	set(deployQtOptions "--no-compiler-runtime \"${installedExeWindows}\"")
endif()
if(deployQtToolName)
	if(NOT QT_QMAKE_EXECUTABLE)
		get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake LOCATION)
	endif()
	cmake_path(GET QT_QMAKE_EXECUTABLE PARENT_PATH qtBinDir)

	find_program(deployQtTool NAMES ${deployQtToolName} PATHS ${qtBinDir})
	install(CODE "
		execute_process(COMMAND \"${deployQtTool}\" ${deployQtOptions} -verbose=2)
	")
endif()

# Qt 4 doesn't have windeployqt
if(WIN32 AND QT_VERSION_MAJOR EQUAL 4)
	install(CODE "
		include(DeployQt4)
		set(QT_BINARY_DIR ${QT_BINARY_DIR})
		set(QT_LIBRARY_DIR ${QT_LIBRARY_DIR})
		fixup_qt4_executable(${installedExeWindows}
			$<IF:$<CONFIG:Debug>,${QT_QJPEG_PLUGIN_DEBUG},${QT_QJPEG_PLUGIN_RELEASE}>
		)
	")
endif()

# CPack

if(WIN32)
	set(CPACK_GENERATOR "7Z")
else()
	set(CPACK_GENERATOR "TXZ")
endif()
include(CPack)
