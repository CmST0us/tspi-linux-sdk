find_package(PkgConfig QUIET)
pkg_check_modules(PKG_DBUS_CPP QUIET "dbus-c++-1")
set(DBUS_CPP_DEFINITIONS ${PKG_DBUS_CPP_CFLAGS_OTHER})

find_path(DBUS_CPP_INCLUDE_DIR
    NAMES dbus-c++/dbus.h
    HINTS ${PKG_DBUS_CPP_INCLUDEDIR} ${PKG_DBUS_CPP_INCLUDE_DIRS}
    PATH_SUFFIXES dbus-c++-1)

find_library(DBUS_CPP_LIBRARY
    NAMES dbus-c++-1
    HINTS ${PKG_DBUS_CPP_LIBDIR} ${PKG_DBUS_CPP_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dbus_cpp DEFAULT_MSG
    DBUS_CPP_LIBRARY DBUS_CPP_INCLUDE_DIR)
mark_as_advanced(dbus_cpp_FOUND DBUS_CPP_INCLUDE_DIR DBUS_CPP_LIBRARY)

if(NOT TARGET dbus_cpp::dbus_cpp)
    add_library(dbus_cpp::dbus_cpp UNKNOWN IMPORTED)
    set_property(TARGET dbus_cpp::dbus_cpp PROPERTY IMPORTED_LOCATION ${DBUS_CPP_LIBRARY})
    set_property(TARGET dbus_cpp::dbus_cpp PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DBUS_CPP_INCLUDE_DIR}")
endif()
