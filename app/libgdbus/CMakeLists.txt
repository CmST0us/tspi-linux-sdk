cmake_minimum_required(VERSION 3.8)

project(gdbus)

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)

include_directories(
	${PROJECT_SOURCE_DIR}/./)

link_directories(
	${PROJECT_SOURCE_DIR}/./lib)

aux_source_directory(./ SRCS)
add_library(${PROJECT_NAME} SHARED ${SRCS})

target_link_libraries(${PROJECT_NAME}
	PkgConfig::GLIB
	PkgConfig::DBUS
	)

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib)

install(FILES gdbus.h
	DESTINATION include)
