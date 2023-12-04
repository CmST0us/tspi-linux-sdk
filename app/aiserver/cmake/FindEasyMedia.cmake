find_package(PkgConfig QUIET)
pkg_check_modules(PKG_EASYMEDIA QUIET "libeasymedia")
set(EASYMEDIA_DEFINITIONS ${PKG_EASYMEDIA_CFLAGS_OTHER})

find_path(EASYMEDIA_INCLUDE_DIR
    NAMES buffer.h
    HINTS ${PKG_EASYMEDIA_INCLUDEDIR} ${PKG_EASYMEDIA_INCLUDE_DIRS}
    PATH_SUFFIXES easymedia)

find_library(EASYMEDIA_LIBRARY
    NAMES easymedia
    HINTS ${PKG_EASYMEDIA_LIBDIR} ${PKG_EASYMEDIA_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EasyMedia DEFAULT_MSG
    EASYMEDIA_LIBRARY EASYMEDIA_INCLUDE_DIR)
mark_as_advanced(EasyMedia_FOUND EASYMEDIA_INCLUDE_DIR EASYMEDIA_LIBRARY)

if(NOT TARGET EasyMedia::EasyMedia)
    add_library(EasyMedia::EasyMedia UNKNOWN IMPORTED)
    set_property(TARGET EasyMedia::EasyMedia PROPERTY IMPORTED_LOCATION ${EASYMEDIA_LIBRARY})
    set_property(TARGET EasyMedia::EasyMedia PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${EASYMEDIA_INCLUDE_DIR}")
endif()
