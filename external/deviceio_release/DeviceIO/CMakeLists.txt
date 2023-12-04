cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(DeviceIo)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/DeviceIo DESTINATION ${CMAKE_INSTALL_STAGING}/usr/include/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/DeviceIo DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
