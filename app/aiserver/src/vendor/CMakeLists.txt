cmake_minimum_required( VERSION 2.8.8 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(SRC_DEPEND_LIBS rockit dl)

# vendor custom node
option(ENABLE_SAMPLE_NODE  "enable sample node" OFF)
if (${ENABLE_SAMPLE_NODE})
    set(SRC_FILES_VENDOR
        SampleNode.cpp
    )
endif()

# node
option(ENABLE_SAMPLE_NODE_EPTZ  "enable node eptz" ON)
if (${ENABLE_SAMPLE_NODE_EPTZ})
    include_directories(filter/eptz)
    link_directories(filter/eptz)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/filter/eptz/libeptz.so)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/filter/eptz/libeptz.so DESTINATION ../../oem/usr/lib/)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/filter/eptz/eptz_zoom.conf")
        set(ETC_DST "${PROJECT_SOURCE_DIR}/../../target/etc/")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/filter/eptz/eptz_zoom.conf DESTINATION ${ETC_DST})
    endif()
    set(SRC_FILES_VENDOR
        ${SRC_FILES_VENDOR}
        filter/eptz/RTNodeVFilterEptzDemo.cpp
    )
endif()

option(ENABLE_SAMPLE_NODE_FEC  "enable node fec" OFF)
if ({ENABLE_SAMPLE_NODE_FEC})
    aux_source_directory(filter/fec/src SRC_FILES_VENDOR)
    include_directories(filter/fec)
    include_directories(filter/fec/headers)
    include_directories(../utils/drm)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/filter/fec/libdistortion.so DESTINATION ../../oem/usr/lib/)
    set(SRC_DEPEND_LIBS ${SRC_DEPEND_LIBS} drm)
endif()

option(ENABLE_SAMPLE_NODE_ROCKX  "enable node rockx" OFF)
if (${ENABLE_SAMPLE_NODE_ROCKX})
    if (${USE_ROCKX})
        set(SRC_FILES_VENDOR
            ${SRC_FILES_VENDOR}
            filter/rockx/RTVFilterRockx.cpp
            filter/rockx/RTNodeVFilterRockx.cpp
        )
        message(STATUS "Build WITH rockx ")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/filter/rockx/aicamera.json DESTINATION ../../oem/usr/share/aiserver/)
    else()
        message(STATUS "Build NONE rockx ")
    endif()
endif()

# vendor face line node
option(ENABLE_SAMPLE_NODE_FACE_LINE  "enable face_line node" ON)
if (${ENABLE_SAMPLE_NODE_FACE_LINE})
    set(SRC_FILES_VENDOR
        ${SRC_FILES_VENDOR}
        filter/face_line/RTNodeVFilterFaceLineDemo.cpp )
    set(SRC_DEPEND_LIBS ${SRC_DEPEND_LIBS} rga)
endif()

# vendor faceae node
option(ENABLE_SAMPLE_NODE_FACEAE  "enable faceae node" OFF)
if (${ENABLE_SAMPLE_NODE_FACEAE})
    if (${DBSERVER_SUPPORT})
       set(SRC_FILES_VENDOR
            ${SRC_FILES_VENDOR}
           filter/faceae/RTNodeVFilterFaceAEDemo.cpp )
    endif()
endif()

option(ENABLE_SAMPLE_NODE_VIDEO_OUTPUT "enable node video output" OFF)
if(${ENABLE_SAMPLE_NODE_VIDEO_OUTPUT})
    set(SRC_DEPEND_LIBS ${SRC_DEPEND_LIBS} drm)
    set(INCLUDE_RKVO
        filter/rkvo/headers
        ../utils/drm)
    include_directories(${INCLUDE_RKVO})
    set(SRC_FILES_VENDOR
        ${SRC_FILES_VENDOR}
        filter/rkvo/RTNodeVFilterVideoOutput.cpp
        filter/rkvo/drmDsp.c
        filter/rkvo/drmDsp/bo.c
        filter/rkvo/drmDsp/dev.c
        filter/rkvo/drmDsp/modeset.c)
    set(SRC_DEPEND_LIBS ${SRC_DEPEND_LIBS} drm)
    message("Build with rkvo")
endif()

# zoom node
set(SRC_FILES_VENDOR
    ${SRC_FILES_VENDOR}
    filter/zoom/RTNodeVFilterZoom.cpp
)

add_library(${VENDOR_STATIC} STATIC ${SRC_FILES_VENDOR})
set_target_properties(${VENDOR_STATIC} PROPERTIES FOLDER "vendor")
target_link_libraries(${VENDOR_STATIC} ${SRC_DEPEND_LIBS})
