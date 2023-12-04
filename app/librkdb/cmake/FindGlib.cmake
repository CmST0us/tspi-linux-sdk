find_package(PkgConfig QUIET)
pkg_check_modules(PKG_GLIB QUIET "glib-2.0")
set(GLIB_DEFINITIONS ${PKG_GLIB_CFLAGS_OTHER})
