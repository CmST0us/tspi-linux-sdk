cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

#PROJECT (deviceio_test)

message(${PROJECT_SOURCE_DIR})
message(${PROJECT_BINARY_DIR})

add_definitions("-Wall -g")

add_subdirectory("DeviceIO")
add_subdirectory("test")
