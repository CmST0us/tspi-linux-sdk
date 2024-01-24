cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR) 
project(rkwifibt)

message("build rkwifibt app ...")

set(TEST_FILES 
	test/main.c
	test/bt_test.c
	test/rk_ble_app.c
	test/rk_wifi_test.c
	test/softap/softap.c
)
set(SERVER_FILES 
	test/rkbtsource_server.c
)
set(CLIENT_FILES 
	test/rkbtsource.c
)

find_package(PkgConfig)
pkg_check_modules (GLIB2 glib-2.0 REQUIRED)
pkg_check_modules (DBUS dbus-1 REQUIRED)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${WPA_SUPPLICANT_INCLUDE_DIRS})

#add_definitions("-Wall -rdynamic -g -gdwarf -O0 -fpermissive -Wno-unused-variable -Wno-write-strings -Wno-unused-function -Wno-write-strings")
add_definitions("-Wall -g -rdynamic -fPIC -gdwarf -O0 -Wno-unused-function -Wno-unused-variable -D_GNU_SOURCE")

add_executable(rkwifibt_app_test ${TEST_FILES})
target_include_directories(rkwifibt_app_test PUBLIC "include" )
target_link_libraries(rkwifibt_app_test pthread rkwifibt asound)

add_executable(rkwifibt_server ${SERVER_FILES})
target_include_directories(rkwifibt_server PUBLIC "include" )
target_link_libraries(rkwifibt_server pthread rkwifibt)

add_executable(rkwifibt_client ${CLIENT_FILES})
target_include_directories(rkwifibt_client PUBLIC "include" )
target_link_libraries(rkwifibt_client pthread rkwifibt)

# install target
#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS rkwifibt_app_test DESTINATION bin)
install(TARGETS rkwifibt_server DESTINATION bin)
install(TARGETS rkwifibt_client DESTINATION bin)
