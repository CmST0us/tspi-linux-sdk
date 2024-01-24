cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

set(SERVER_SRC_FILES rkbtsource_server.cpp)
set(CLIENT_SRC_FILES rkbtsource.cpp)

if(CPU_ARCH STREQUAL "arm")
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../DeviceIO/lib32")
endif()

if(CPU_ARCH STREQUAL "aarch64")
  link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../DeviceIO/lib64" "${CMAKE_CURRENT_SOURCE_DIR}/../libcomm")
endif()
add_definitions("-fpermissive")

add_executable(rkbtsource_server ${SERVER_SRC_FILES})
target_include_directories(rkbtsource_server PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../DeviceIO/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../libcomm" )
target_link_libraries(rkbtsource_server DeviceIo asound comm)

add_executable(rkbtsource ${CLIENT_SRC_FILES})
target_include_directories(rkbtsource PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../DeviceIO/include")
target_link_libraries(rkbtsource DeviceIo asound)

install(TARGETS rkbtsource_server DESTINATION bin)
install(TARGETS rkbtsource DESTINATION bin)
