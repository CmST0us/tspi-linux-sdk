cmake_minimum_required(VERSION 3.8)

project(CallFunIpc)

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

include_directories(
	${PROJECT_SOURCE_DIR}/./)

link_directories(
	${PROJECT_SOURCE_DIR}/./lib)

link_libraries(
	PkgConfig::GLIB
	PkgConfig::DBUS
	PkgConfig::JSON-C
	gdbus
	pthread
)
aux_source_directory(./ SRCS)
add_library(${PROJECT_NAME}
	SHARED ${SRCS})

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib)
set(HEADERS
	call_fun_ipc.h
	)
install(FILES ${HEADERS} DESTINATION include)

if(DEMO)

add_executable(call_fun_ipc_demo_client demo/call_fun_ipc_demo_client.c)
target_link_libraries(call_fun_ipc_demo_client ${PROJECT_NAME})
install(TARGETS call_fun_ipc_demo_client
	RUNTIME DESTINATION bin)

add_executable(call_fun_ipc_demo_server demo/call_fun_ipc_demo_server.c)
target_link_libraries(call_fun_ipc_demo_server ${PROJECT_NAME})
install(TARGETS call_fun_ipc_demo_server
	RUNTIME DESTINATION bin)

install(FILES demo/dbusconfig/call_fun_ipc_demo.conf
	DESTINATION share/dbus-1/system.d)

endif()