find_package(PkgConfig QUIET)
pkg_check_modules(PKG_DBUS QUIET "dbus-1")
set(DBUS_DEFINITIONS ${PKG_DBUS_CFLAGS_OTHER})

#find_path(DBUS_INCLUDE_DIR
#    NAMES dbus/dbus.h
#    HINTS ${PKG_DBUS_INCLUDEDIR} ${PKG_DBUS_INCLUDE_DIRS}
#    PATH_SUFFIXES dbus-1.0)
#
#find_library(DBUS_LIBRARY
#    NAMES dbus-1
#    HINTS ${PKG_DBUS_LIBDIR} ${PKG_DBUS_LIBRARY_DIRS})
#
#include(FindPackageHandleStandardArgs)
#find_package_handle_standard_args(dbus_1 DEFAULT_MSG
#    DBUS_LIBRARY DBUS_INCLUDE_DIR)
#mark_as_advanced(dbus_1_FOUND DBUS_INCLUDE_DIR DBUS_LIBRARY)
#
#if(NOT TARGET dbus_1::dbus_1)
#    add_library(dbus_1::dbus_1 UNKNOWN IMPORTED)
#    set_property(TARGET dbus_1::dbus_1 PROPERTY IMPORTED_LOCATION ${DBUS_LIBRARY})
#    set_property(TARGET dbus_1::dbus_1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DBUS_INCLUDE_DIR}")
#endif()
