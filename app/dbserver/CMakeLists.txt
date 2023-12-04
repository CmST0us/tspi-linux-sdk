cmake_minimum_required(VERSION 3.8)

project(dbserver)

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
pkg_check_modules (SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

include_directories(
	${PROJECT_SOURCE_DIR}/./)

link_directories(
	${PROJECT_SOURCE_DIR}/./lib)

aux_source_directory(./ SRCS)
add_executable(${PROJECT_NAME}
	${SRCS})

option(ENABLE_MINILOGGER  "enbale minilogger" ON)
if (ENABLE_MINILOGGER)
    find_package(MiniLogger REQUIRED)
    add_definitions(-DENABLE_MINILOGGER)
    target_link_libraries(${PROJECT_NAME} MiniLogger::MiniLogger)
endif()

option(SANITIZER_STATIC "compile with sanitizer (static library linker)" OFF)
if(SANITIZER_STATIC)
	add_definitions(-fsanitize=address -static-libasan -g -ggdb -gdwarf -funwind-tables -rdynamic -O0)
	add_definitions(-fno-stack-protector -fno-omit-frame-pointer -fsanitize-recover=address)
	# add_definitions(-fsanitize=undefined)
	target_link_libraries(${PROJECT_NAME} libasan.a dl m rt)
endif()

option(SANITIZER_DYNAMIC "compile with sanitizer (dynamic library linker)" OFF)
if(SANITIZER_DYNAMIC)
	# NOTE: copy libasan.so with manual operation
	add_definitions(-fsanitize=address -g -ggdb -gdwarf -funwind-tables -rdynamic -O0)
	add_definitions(-fno-stack-protector -fno-omit-frame-pointer -fsanitize-recover=address)
	# add_definitions(-fsanitize=undefined)
	target_link_libraries(${PROJECT_NAME} asan)
endif()

target_link_libraries(${PROJECT_NAME}
	PkgConfig::GLIB
	PkgConfig::DBUS
	PkgConfig::SQLITE3
	PkgConfig::JSON-C
	rkdb
	gdbus
	pthread
	)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin)
install(FILES dbusconfig/dbserver.conf
	DESTINATION share/dbus-1/system.d)
