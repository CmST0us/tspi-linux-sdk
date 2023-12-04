cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

# Set project information 
project(alexaClient VERSION 1.0 LANGUAGES CXX)

add_compile_options(-std=c++11)

# Alexa Demo targets. 
add_subdirectory("ThirdParty")
add_subdirectory("Server")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/ap_notification DESTINATION ${CMAKE_INSTALL_PREFIX})
install(PROGRAMS ${CMAKE_SOURCE_DIR}/bt_realtek_wificonfig DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
