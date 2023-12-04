cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)

project(rk_eptz_demo)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-fno-rtti)
add_definitions("-fpermissive -g")
include_directories(./)
include_directories(${RKNPU_HEADER_DIR})
include_directories(${ROCKX_HEADER_DIR})
option(NDEBUG OFF)
if(NDEBUG)
  add_definitions("-DNDEBUG")
endif()
set(CMAKE_BUILD_TYPE "Debug")

set(EPTZ_TARGET eptz_demo)

aux_source_directory(src EPTZ_SRC)

set(EPTZ_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(EPTZ_LIBRARY easymedia rkuvc rga pthread)
set(EPTZ_LIBRARY ${EPTZ_LIBRARY} SDL2 SDL2_ttf SDL2_gfx)

if(ROCKX)
  add_definitions(-DHAVE_ROCKX=1)
  set(EPTZ_LIBRARY ${EPTZ_LIBRARY} rockx)
endif()

add_executable(${EPTZ_TARGET} ${EPTZ_SRC})
target_include_directories(${EPTZ_TARGET} PUBLIC ${EPTZ_INC})

target_link_libraries(${EPTZ_TARGET} ${EPTZ_LIBRARY})

install(TARGETS ${EPTZ_TARGET} RUNTIME DESTINATION "bin")
install(DIRECTORY fonts DESTINATION "/usr/lib"
        FILES_MATCHING PATTERN "*.ttf")
