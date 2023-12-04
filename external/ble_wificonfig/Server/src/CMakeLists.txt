file(GLOB_RECURSE Gdbus_SRC "${ble_wificonfig_SOURCE_DIR}/src/gdbus/*.c")
file(GLOB_RECURSE Src_SRC "${ble_wificonfig_SOURCE_DIR}/src/src/*.c")

add_executable(ble_wificonfig 
        "${ble_wificonfig_SOURCE_DIR}/src/WifiUtil.cpp"
        "${ble_wificonfig_SOURCE_DIR}/src/WifiInfo.cpp"
        "${ble_wificonfig_SOURCE_DIR}/src/wifi_util_wrapper.cpp"
        "${ble_wificonfig_SOURCE_DIR}/src/gatt-service.c"
	 ${Gdbus_SRC}
         ${Src_SRC})

find_package(PkgConfig)
pkg_check_modules (GLIB2 glib-2.0 REQUIRED)
pkg_check_modules (DBUS dbus-1 REQUIRED)
#pkg_check_modules (DBUS-GLIB dbus-glib-1 REQUIRED)
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})
#include_directories(${DBUS-GLIB_INCLUDE_DIRS})
target_link_libraries (ble_wificonfig ${GLIB2_LIBRARIES})
target_link_libraries (ble_wificonfig ${DBUS_LIBRARIES})
#target_link_libraries (ble_wificonfig ${DBUS-GLIB_LIBRARIES})

install(TARGETS ble_wificonfig DESTINATION bin)

target_include_directories(ble_wificonfig PUBLIC 
	"${ble_wificonfig_SOURCE_DIR}/include"
	"${CMAKE_THREAD_LIBS_INIT}"
	${RAPIDJSON_INCLUDE_DIR})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wno-unused-variable -fpermissive")

target_link_libraries(ble_wificonfig pthread) 
