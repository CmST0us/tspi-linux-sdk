cmake_minimum_required(VERSION 3.8)
project(ispipc)

include(FindPkgConfig)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

if(CONFIG_DBUS)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
endif()

add_compile_options(-fPIC)
add_compile_options(-Wno-unused-variable
                                        -Wno-unused-but-set-variable
                                        -Wno-reorder
                                        -Wno-unused-function
					-fpermissive)

add_subdirectory(server)

if(CONFIG_DBUS)
#add_subdirectory(client)
#add_subdirectory(demo)
endif()
