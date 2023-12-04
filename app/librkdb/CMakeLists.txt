cmake_minimum_required(VERSION 3.8)

project(librkdb LANGUAGES C)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(DESCRIPTION "librkdb projects")

include(clang-format)
include(cppcheck)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    message(FATAL_ERROR "This project can only be built for linux")
endif()

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

link_libraries(
	PkgConfig::GLIB
	PkgConfig::SQLITE3
	PkgConfig::JSON-C
	)
add_library(rkdb SHARED rkdb.c)
install(TARGETS rkdb
	LIBRARY DESTINATION lib)
install(FILES rkdb.h
	DESTINATION include)

include_directories(
	${PROJECT_SOURCE_DIR}/./)

option(LIBRKDB_EXAMPLE_BUILD  "Build tests executables" OFF)
if (${LIBRKDB_EXAMPLE_BUILD})

add_executable(rkdb_demo demo/main.c)
target_link_libraries(rkdb_demo rkdb)
install(TARGETS rkdb_demo
	RUNTIME DESTINATION bin)
endif()
