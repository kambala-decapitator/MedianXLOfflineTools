add_library(QtSingleApplication OBJECT
	qtsingleapplication/qtlocalpeer.cpp
	qtsingleapplication/qtlocalpeer.h
	qtsingleapplication/qtlockedfile.cpp
	qtsingleapplication/qtlockedfile.h
	qtsingleapplication/QtSingleApplication
	qtsingleapplication/qtsingleapplication.cpp
	qtsingleapplication/qtsingleapplication.h
)
if(WIN32)
	target_sources(QtSingleApplication PRIVATE
		qtsingleapplication/qtlockedfile_win.cpp
	)
else()
	target_sources(QtSingleApplication PRIVATE
		qtsingleapplication/qtlockedfile_unix.cpp
	)
endif()
