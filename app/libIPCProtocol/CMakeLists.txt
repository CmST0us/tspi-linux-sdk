cmake_minimum_required(VERSION 3.8)

project(IPCProtocol)

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
pkg_check_modules (SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

include_directories(
	${PROJECT_SOURCE_DIR}/./
	${CMAKE_SYSROOT}/usr/include/dbus-c++-1
)

link_directories(
	${PROJECT_SOURCE_DIR}/./lib)

link_libraries(
	PkgConfig::GLIB
	PkgConfig::DBUS
	PkgConfig::SQLITE3
	PkgConfig::JSON-C
	dbus-c++-1
	rkdb
	gdbus
	pthread
)

add_definitions(-DDBUS_HAS_RECURSIVE_MUTEX)

aux_source_directory(./ SRCS)
add_library(${PROJECT_NAME}
	SHARED ${SRCS})

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib)
set(HEADERS
	dbserver.h
	netserver.h
	ispserver.h
	storage_manager.h
	system_manager.h
	mediaserver.h
	IPCProtocol.h
	dbus_signal.h
	libipcpro_log_control.h
	)
install(FILES ${HEADERS} DESTINATION include)

option(COMPILE_DEMO "Enable compile demo" OFF)
if(COMPILE_DEMO)
	add_executable(dbserver_test demo/dbserver_test.c)
	target_link_libraries(dbserver_test ${PROJECT_NAME})
	install(TARGETS dbserver_test
		RUNTIME DESTINATION bin)

	add_executable(netserver_test demo/netserver_test.c)
	target_link_libraries(netserver_test ${PROJECT_NAME})
	install(TARGETS netserver_test
		RUNTIME DESTINATION bin)

	add_executable(storage_manager_test demo/storage_manager_test.c)
	target_link_libraries(storage_manager_test ${PROJECT_NAME})
	install(TARGETS storage_manager_test
		RUNTIME DESTINATION bin)

	add_executable(system_manager_test demo/system_manager_test.c)
	target_link_libraries(system_manager_test ${PROJECT_NAME})
	install(TARGETS system_manager_test
		RUNTIME DESTINATION bin)

	add_executable(mediaserver_test demo/mediaserver_test.c)
	target_link_libraries(mediaserver_test ${PROJECT_NAME})
	install(TARGETS mediaserver_test
		RUNTIME DESTINATION bin)
endif()
