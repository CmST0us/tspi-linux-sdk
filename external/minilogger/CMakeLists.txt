cmake_minimum_required(VERSION 3.8...3.16 FATAL_ERROR)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

project(minilogger
    LANGUAGES C CXX
    VERSION 1.0.0)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake)

option(MINILOGGER_USE_LIBUNWIND "use libunwind as unwinder" OFF)
if (APPLE)
    set(MINILOGGER_USE_LIBUNWIND OFF)
endif()
if (MINILOGGER_USE_LIBUNWIND)
set(MIN_Libunwind_VERSION 1.31)
find_package(Libunwind REQUIRED)
endif()
find_package(GLib2 REQUIRED)

add_library(minilogger
    STATIC
    src/backtrace.cpp
    src/log.cpp)

add_library(MiniLogger::MiniLogger ALIAS minilogger)

if (NOT APPLE)
    target_compile_options(minilogger PRIVATE -rdynamic)
endif()
target_compile_options(minilogger PRIVATE -fno-exceptions -fno-rtti)
target_compile_options(minilogger PRIVATE -fstack-protector-all -fstack-check)
target_compile_options(minilogger PRIVATE -Wno-variadic-macros)
target_compile_options(minilogger PRIVATE -g -ggdb -gdwarf)
target_compile_options(minilogger PRIVATE -funwind-tables)
target_compile_options(minilogger PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)

target_compile_features(minilogger PRIVATE cxx_auto_type)
target_compile_features(minilogger PRIVATE cxx_std_11)

target_include_directories(minilogger
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(minilogger
    PUBLIC
    GLib2::glib
)

if (MINILOGGER_USE_LIBUNWIND)
target_compile_options(minilogger PRIVATE -DUSE_LIBUNWIND)
target_link_libraries(minilogger
    PUBLIC
    ${LIBUNWIND_LIBRARIES}
)
endif()

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/MiniLogger)

install(TARGETS minilogger
    EXPORT minilogger-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(minilogger PROPERTIES EXPORT_NAME MiniLogger)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT minilogger-targets
    FILE
    MiniLoggerTargets.cmake
    NAMESPACE
    MiniLogger::
    DESTINATION
    ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MiniLoggerConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)


configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/MiniLoggerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MiniLoggerConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FindGLib2.cmake
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FindLibunwind.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MiniLoggerConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MiniLoggerConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindLibunwind.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FindLibunwind.cmake
    COPYONLY)

export(EXPORT minilogger-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/MiniLoggerTargets.cmake
    NAMESPACE MiniLogger::)

export(PACKAGE MiniLogger)

enable_testing()
add_subdirectory(test)
