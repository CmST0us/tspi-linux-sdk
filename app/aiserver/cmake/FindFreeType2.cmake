find_package(PkgConfig QUIET)
pkg_check_modules(PKG_FREEFTYPE QUIET "freetype")
set(FREEFTYPE_DEFINITIONS ${PKG_FREEFTYPE_CFLAGS_OTHER})

find_path(FREEFTYPE_INCLUDE_DIR
    NAMES ft2build.h
    HINTS ${PKG_FREEFTYPE_INCLUDEDIR} ${PKG_FREEFTYPE_INCLUDE_DIRS}
    PATH_SUFFIXES freetype2)

find_library(FREEFTYPE_LIBRARY
    NAMES freetype
    HINTS ${PKG_FREEFTYPE_LIBDIR} ${PKG_FREEFTYPE_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freetype_2 DEFAULT_MSG
    FREEFTYPE_LIBRARY FREEFTYPE_INCLUDE_DIR )
mark_as_advanced(Freetype_2_FOUND FREEFTYPE_INCLUDE_DIR FREEFTYPE_LIBRARY)

if(NOT TARGET Freetype_2::Freetype_2)
    add_library(Freetype_2::Freetype_2 UNKNOWN IMPORTED)
    set_property(TARGET Freetype_2::Freetype_2 PROPERTY IMPORTED_LOCATION ${FREEFTYPE_LIBRARY})
    set_property(TARGET Freetype_2::Freetype_2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${FREEFTYPE_INCLUDE_DIR}")
endif()
