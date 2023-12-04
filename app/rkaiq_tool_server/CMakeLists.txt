cmake_minimum_required(VERSION 2.8)

project(rkaiq_tool_server C CXX)

add_compile_options(${ISP_HW_VERSION})

set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories(
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/netserver/
	${PROJECT_SOURCE_DIR}/rkaiq/
	${PROJECT_SOURCE_DIR}/camera/
	${PROJECT_SOURCE_DIR}/mediactl/)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti ${CMAKE_CXX_FLAGS}")
endif(CMAKE_COMPILER_IS_GNUCXX)

aux_source_directory(${PROJECT_SOURCE_DIR}/logger/ LOGGER_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/netserver NET_SERVER_SOURCE)
set(RKAIQ_SOURCE
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api/rkaiq_media.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api/rkaiq_socket.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/multiframe_process.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_online_protocol.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_raw_protocol.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_protocol.cpp
    )
aux_source_directory(${PROJECT_SOURCE_DIR}/camera/ CAMERA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/mediactl/ MEDIACTL_SOURCE)

option(BUILD_SANITIZER_STATIC "compile with sanitizer (static library linker)" OFF)
if(BUILD_SANITIZER_STATIC)
	add_definitions(-fsanitize=address -static-libasan -g -ggdb -gdwarf -funwind-tables -rdynamic -O0 )
	link_libraries(libasan.a dl m rt)
endif()

option(BUILD_DEBUG "compile with debug" OFF)
if(BUILD_SANITIZER_STATIC)
        add_definitions(-g -ggdb -gdwarf -funwind-tables -rdynamic -O0 )
endif()

option(BUILD_RKAIQ_API "depend on rkaiq api library" OFF)
if(BUILD_RKAIQ_API)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api RKAIQ_SOURCE)
link_libraries(rkaiq)
add_definitions(-DENABLE_RKAIQ_UAPI)
endif()
find_path(RKAIQ_API_INCLUDE_DIR
    NAMES rk_aiq_user_api_imgproc.h
    HINTS ${CMAKE_FIND_ROOT_PATH}
    PATH_SUFFIXES rkaiq/uAPI)
include_directories(${RKAIQ_API_INCLUDE_DIR}/)
include_directories(${RKAIQ_API_INCLUDE_DIR}/..)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../xcore)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../common)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../algos)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../iq_parser)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../iq_parser_v2)
include_directories(${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api)

option(BUILD_RKMEDIA "depend on rkmedia library" ON)
option(BUILD_RKMEDIA_INDE "depend on rkmedia library independent" ON)
if(BUILD_RKMEDIA)
add_definitions(-DENABLE_RSTP_SERVER)
if(BUILD_RKMEDIA_INDE)
include_directories(${PROJECT_SOURCE_DIR}/rkmedia)
include_directories(${PROJECT_SOURCE_DIR}/rkmedia/include)
include_directories(${PROJECT_SOURCE_DIR}/rkmedia/librkmedia/include)
include_directories(${PROJECT_SOURCE_DIR}/rkmedia/librkmedia/include/easymedia)
add_subdirectory(rkmedia/librkmedia)
set(RKMEDIA_SOURCE
	${RKMEDIA_SOURCE}
	rkmedia/link_flow.cc
	rkmedia/sink_flow.cc
	rkmedia/source_flow.cc
	rkmedia/rtsp_server.cpp)
link_libraries(rkmedia)
else()
find_package(PkgConfig QUIET)
pkg_check_modules(PKG_EASYMEDIA QUIET "libeasymedia")
set(EASYMEDIA_DEFINITIONS ${PKG_EASYMEDIA_CFLAGS_OTHER})
find_path(EASYMEDIA_INCLUDE_DIR
    NAMES buffer.h
    HINTS ${PKG_EASYMEDIA_INCLUDEDIR} ${PKG_EASYMEDIA_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/rkmedia/include
    PATH_SUFFIXES easymedia)
include_directories(${PROJECT_SOURCE_DIR}/rkmedia/)
link_libraries(easymedia)
include_directories(${EASYMEDIA_INCLUDE_DIR}/)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkmedia/ RKMEDIA_SOURCE)
endif()
endif()

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.cc *.h)
add_custom_target(
    clangformat
    COMMAND /usr/bin/clang-format
    -style=LLVM
    -i
    ${ALL_SOURCE_FILES})

set(RKAIQ_TOOL_SERVER_SOURCE
	${LOGGER_SOURCE}
	${NET_SERVER_SOURCE}
	${RKAIQ_SOURCE}
	${CAMERA_SOURCE}
	${RKMEDIA_SOURCE}
	${MEDIACTL_SOURCE}
	rkaiq_tool_server.cpp)

add_executable(rkaiq_tool_server ${RKAIQ_TOOL_SERVER_SOURCE})
if(BUILD_RKMEDIA_INDE)
add_dependencies(rkaiq_tool_server rkmedia)
endif()
target_link_libraries(rkaiq_tool_server pthread)
install(TARGETS rkaiq_tool_server RUNTIME DESTINATION bin)

option(BUILD_TEST "build test demo" OFF)
if(BUILD_TEST)
	add_subdirectory(test)
endif()
