find_package(PkgConfig QUIET)
pkg_check_modules(LIVEMEDIA QUIET "live555")

include(FindPackageHandleStandardArgs)
find_path(LIVEMEDIA_INCLUDE_DIR
    NAMES
    liveMedia.hh
    PATH
    include
    PATH_SUFFIXES
    liveMedia)
find_path(GROUPSOCK_INCLUDE_DIR
    NAMES
    Groupsock.hh
    PATH
    include
    PATH_SUFFIXES
    groupsock)
find_path(BASICUSAGEENVIRONMENT_INCLUDE_DIR
    NAMES
    BasicUsageEnvironment.hh
    PATH
    include
    PATH_SUFFIXES
    BasicUsageEnvironment)
find_path(USAGEENVIRONMENT_INCLUDE_DIR
    NAMES
    UsageEnvironment.hh
    PATH
    include
    PATH_SUFFIXES
    UsageEnvironment)
find_library(LIVEMEDIA_LIBRARY
    liveMedia)
find_library(GROUPSOCK_LIBRARY
    groupsock)
find_library(BASICUSAGEENVIRONMENT_LIBRARY
    BasicUsageEnvironment)
find_library(USAGEENVIRONMENT_LIBRARY
    UsageEnvironment)
find_package_handle_standard_args(LIVEMEDIA
    DEFAULT_MSG
    LIVEMEDIA_INCLUDE_DIR
    LIVEMEDIA_LIBRARY
    GROUPSOCK_INCLUDE_DIR
    GROUPSOCK_LIBRARY
    BASICUSAGEENVIRONMENT_INCLUDE_DIR
    BASICUSAGEENVIRONMENT_LIBRARY
    USAGEENVIRONMENT_INCLUDE_DIR
    USAGEENVIRONMENT_LIBRARY
    )
mark_as_advanced(
    LIVEMEDIA_INCLUDE_DIR
    LIVEMEDIA_LIBRARY
    GROUPSOCK_INCLUDE_DIR
    GROUPSOCK_LIBRARY
    BASICUSAGEENVIRONMENT_INCLUDE_DIR
    BASICUSAGEENVIRONMENT_LIBRARY
    USAGEENVIRONMENT_INCLUDE_DIR
    USAGEENVIRONMENT_LIBRARY)

if(LIVEMEDIA_FOUND)
    set(LIVEMEDIA_LIBRARIES    ${LIVEMEDIA_LIBRARY};${GROUPSOCK_LIBRARY};${BASICUSAGEENVIRONMENT_LIBRARY};${USAGEENVIRONMENT_LIBRARY})
    set(LIVEMEDIA_INCLUDE_DIRS ${LIVEMEDIA_INCLUDE_DIR};${GROUPSOCK_INCLUDE_DIR};${BASICUSAGEENVIRONMENT_INCLUDE_DIR};${USAGEENVIRONMENT_INCLUDE_DIR})

    if(LIVEMEDIA_LIBRARY AND NOT TARGET LiveMedia::liveMedia)
        if(IS_ABSOLUTE "${LIVEMEDIA_LIBRARY}")
            add_library(LiveMedia::liveMedia UNKNOWN IMPORTED)
            set_target_properties(LiveMedia::liveMedia PROPERTIES IMPORTED_LOCATION
                "${LIVEMEDIA_LIBRARY}")
        else()
            add_library(LiveMedia::liveMedia INTERFACE IMPORTED)
            set_target_properties(LiveMedia::liveMedia PROPERTIES IMPORTED_LIBNAME
                "${LIVEMEDIA_LIBRARY}")
        endif()
        set_target_properties(LiveMedia::liveMedia PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
            "${LIVEMEDIA_INCLUDE_DIR}")
    endif()

    if(GROUPSOCK_LIBRARY AND NOT TARGET LiveMedia::groupsock)
        if(IS_ABSOLUTE "${GROUPSOCK_LIBRARY}")
            add_library(LiveMedia::groupsock UNKNOWN IMPORTED)
            set_target_properties(LiveMedia::groupsock PROPERTIES IMPORTED_LOCATION
                "${GROUPSOCK_LIBRARY}")
        else()
            add_library(LiveMedia::groupsock INTERFACE IMPORTED)
            set_target_properties(LiveMedia::groupsock PROPERTIES IMPORTED_LIBNAME
                "${GROUPSOCK_LIBRARY}")
        endif()
        set_target_properties(LiveMedia::groupsock PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
            "${GROUPSOCK_INCLUDE_DIR}")
    endif()

    if(BASICUSAGEENVIRONMENT_LIBRARY AND NOT TARGET LiveMedia::BasicUsageEnvironment)
        if(IS_ABSOLUTE "${BASICUSAGEENVIRONMENT_LIBRARY}")
            add_library(LiveMedia::BasicUsageEnvironment UNKNOWN IMPORTED)
            set_target_properties(LiveMedia::BasicUsageEnvironment PROPERTIES IMPORTED_LOCATION
                "${BASICUSAGEENVIRONMENT_LIBRARY}")
        else()
            add_library(LiveMedia::BasicUsageEnvironment INTERFACE IMPORTED)
            set_target_properties(LiveMedia::BasicUsageEnvironment PROPERTIES IMPORTED_LIBNAME
                "${BASICUSAGEENVIRONMENT_LIBRARY}")
        endif()
        set_target_properties(LiveMedia::BasicUsageEnvironment PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
            "${BASICUSAGEENVIRONMENT_INCLUDE_DIR}")
    endif()

    if(USAGEENVIRONMENT_LIBRARY AND NOT TARGET LiveMedia::UsageEnvironment)
        if(IS_ABSOLUTE "${USAGEENVIRONMENT_LIBRARY}")
            add_library(LiveMedia::UsageEnvironment UNKNOWN IMPORTED)
            set_target_properties(LiveMedia::UsageEnvironment PROPERTIES IMPORTED_LOCATION
                "${USAGEENVIRONMENT_LIBRARY}")
        else()
            add_library(LiveMedia::UsageEnvironment INTERFACE IMPORTED)
            set_target_properties(LiveMedia::UsageEnvironment PROPERTIES IMPORTED_LIBNAME
                "${USAGEENVIRONMENT_LIBRARY}")
        endif()
        set_target_properties(LiveMedia::UsageEnvironment PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
            "${LIVEMEDIA_INCLUDE_DIR}")
    endif()
endif()
