cmake_minimum_required(VERSION 3.8.2)

project(rkfacial)

set(SRC
    database.c
    rockface_control.cpp
    load_feature.c
    play_wav.c
    camrgb_control.c
    camir_control.c
    rga_control.c
    video_common.c
    usb_camera.c
    vpu_decode.c
    db_monitor.cpp
    vpu_encode.c
    snapshot.c
    rkfacial.c
    turbojpeg_decode.c
    rkdrm_display.c
    display.c
    face_config.c
    draw_rect.c
    image_read.c
)

include_directories(${DRM_HEADER_DIR})

set(LIB rockface rknn_api drm rga pthread sqlite3 asound rockchip_mpp turbojpeg m)

if (DEFINED CAMERA_ENGINE_RKISP)
    set(LIB ${LIB} rkisp rkisp_api)
    add_definitions(-DCAMERA_ENGINE_RKISP)
endif()

if (DEFINED CAMERA_ENGINE_RKAIQ)
    set(SRC ${SRC} aiq_control.cpp)
    set(LIB ${LIB} rkaiq rkisp_api)
    add_definitions(-DCAMERA_ENGINE_RKAIQ)
    include_directories(${CAMERA_ENGINE_RKAIQ_ALGOS})
    include_directories(${CAMERA_ENGINE_RKAIQ_COMMON})
    include_directories(${CAMERA_ENGINE_RKAIQ_IQ_PARSER})
    include_directories(${CAMERA_ENGINE_RKAIQ_UAPI})
    include_directories(${CAMERA_ENGINE_RKAIQ_XCORE})
endif()

if(DEFINED USE_WEB_SERVER)
include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
pkg_check_modules (SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)
set(LIB ${LIB}
    IPCProtocol gdbus
    PkgConfig::GLIB
    PkgConfig::DBUS
    PkgConfig::SQLITE3
    PkgConfig::JSON-C
)
add_definitions(-DUSE_WEB_SERVER)
endif()

if(DEFINED IR_TEST_DATA)
add_definitions(-DIR_TEST_DATA)
endif()

if(DEFINED FACE_MASK)
add_definitions(-DFACE_MASK)
endif()

add_library(rkfacial SHARED ${SRC})
target_link_libraries(rkfacial ${LIB})

install(TARGETS rkfacial DESTINATION lib)
install(FILES rkfacial.h rga_control.h turbojpeg_decode.h display.h draw_rect.h DESTINATION include/rkfacial)

install(DIRECTORY wav/cn/ DESTINATION ../etc)
