find_package(PkgConfig QUIET)
pkg_check_modules(PKG_EXIV2 QUIET "exiv2")
set(EXIV2_DEFINITIONS ${PKG_EXIV2_CFLAGS_OTHER})

find_path(EXIV2_INCLUDE_DIR
    NAMES exiv2.hpp
    HINTS ${PKG_EXIV2_INCLUDEDIR} ${PKG_EXIV2_INCLUDE_DIRS}
    PATH_SUFFIXES exiv2)

find_library(EXIV2_LIBRARY
    NAMES exiv2
    HINTS ${PKG_EXIV2_LIBDIR} ${PKG_EXIV2_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(exiv2 DEFAULT_MSG
    EXIV2_LIBRARY EXIV2_INCLUDE_DIR )
mark_as_advanced(exiv2 EXIV2_INCLUDE_DIR EXIV2_LIBRARY)

if(NOT TARGET Exiv2::Exiv2)
    add_library(Exiv2::Exiv2 UNKNOWN IMPORTED)
    set_property(TARGET Exiv2::Exiv2 PROPERTY IMPORTED_LOCATION ${EXIV2_LIBRARY})
    set_property(TARGET Exiv2::Exiv2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${EXIV2_INCLUDE_DIR}")
endif()
