find_package(PkgConfig QUIET)
pkg_check_modules(PKG_GLIB QUIET "glib-2.0")
set(GLIB_DEFINITIONS ${PKG_GLIB_CFLAGS_OTHER})

find_path(GLIB_INCLUDE_DIR
    NAMES glib.h
    HINTS ${PKG_GLIB_INCLUDEDIR} ${PKG_GLIB_INCLUDE_DIRS}
    PATH_SUFFIXES glib-2.0)

find_path(GLIB_LIB_INCLUDE_DIR
    NAMES glibconfig.h
    HINTS ${PKG_GLIB_LIBDIR} ${PKG_GLIB_LIBRARY_DIRS}
    PATH_SUFFIXES glib-2.0/include)

find_library(GLIB_LIBRARY
    NAMES glib-2.0
    HINTS ${PKG_GLIB_LIBDIR} ${PKG_GLIB_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glib_2 DEFAULT_MSG
    GLIB_LIBRARY GLIB_INCLUDE_DIR GLIB_LIB_INCLUDE_DIR )
mark_as_advanced(glib_2_FOUND GLIB_INCLUDE_DIR GLIB_LIB_INCLUDE_DIR GLIB_LIBRARY)

if(NOT TARGET glib_2::glib_2)
    add_library(glib_2::glib_2 UNKNOWN IMPORTED)
    set_property(TARGET glib_2::glib_2 PROPERTY IMPORTED_LOCATION ${GLIB_LIBRARY})
    set_property(TARGET glib_2::glib_2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${GLIB_INCLUDE_DIR}" "${GLIB_LIB_INCLUDE_DIR}")
endif()
