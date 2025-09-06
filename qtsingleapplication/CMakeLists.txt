add_library(QtSingleApplication OBJECT
	qtlocalpeer.cpp
	qtlocalpeer.h
	QtSingleApplication
	qtsingleapplication.cpp
	qtsingleapplication.h
)
target_include_directories(QtSingleApplication PUBLIC ".")
linkQt(QtSingleApplication)
