project(RK_PCBA)
cmake_minimum_required(VERSION 2.8)

message(${PROJECT_SOURCE_DIR})
message(${PROJECT_BINARY_DIR})

# 通过git获取版本宏 BUILD_LOG_VERSION
execute_process(COMMAND git log -1 --pretty=format:%h
    TIMEOUT 5
    OUTPUT_VARIABLE BUILD_LOG_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "building from git log: ${BUILD_LOG_VERSION}")
add_definitions(-DBUILD_LOG_VERSION=\"${BUILD_LOG_VERSION}\")

execute_process(COMMAND date "+%Y-%m-%d-%H:%M:%S"
    TIMEOUT 5
    OUTPUT_VARIABLE BUILD_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "building date: ${BUILD_DATE}")
add_definitions(-DBUILD_DATE=\"${BUILD_DATE}\")

if (DEFINED  PCBA_PX3SE)
add_definitions("-DPCBA_PX3SE")
endif()

if (DEFINED  PCBA_3308)
add_definitions("-DPCBA_3308")
endif()

if (DEFINED  PCBA_3229GVA)
add_definitions("-DPCBA_3229GVA")
endif()

if (DEFINED  PCBA_3326)
add_definitions("-DPCBA_3326")
endif()

if (DEFINED  PCBA_1808)
add_definitions("-DPCBA_1808")
endif()

if (DEFINED  PCBA_PX30)
add_definitions("-DPCBA_PX30")
endif()

if (DEFINED  PCBA_3288)
add_definitions("-DPCBA_3288")
endif()

if (DEFINED  PCBA_3328)
add_definitions("-DPCBA_3328")
endif()

if (DEFINED  PCBA_3399)
add_definitions("-DPCBA_3399")
endif()

if (DEFINED  PCBA_3399PRO)
add_definitions("-DPCBA_3399PRO")
endif()

if (DEFINED  PCBA_1126_1109)
add_definitions("-DPCBA_1126_1109")
endif()

if (DEFINED  PCBA_356X)
add_definitions("-DPCBA_356X")
endif()

if (DEFINED  PCBA_3588)
add_definitions("-DPCBA_3588")
endif()


set(INSTALL_TARGET_DIR /data)

set(SRC_LIST
	echo_audio_play_test.c
	echo_audio_record_test.c
	echo_audio_test.c
	echo_bt_test.c
	echo_cpu_test.c
	echo_ddr_test.c
	echo_emmc_test.c
	echo_key_test.c
	echo_led_test.c
	echo_rotary_test.c
	echo_rtc_test.c
	echo_wlan_test.c
	echo_sdcard_test.c
	echo_usbhost_test.c
	echo_auto_test.c
)

foreach(obj_source ${SRC_LIST})
	STRING( REGEX REPLACE "(.*)(\\..*)$" "\\1" obj_target ${obj_source} )
add_executable( ${obj_target} ${obj_source} )
install(TARGETS ${obj_target} DESTINATION ${INSTALL_TARGET_DIR})
endforeach(obj_source)

set(SRC_LIST echo_pcbatest_server.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( echo_pcbatest_server ${SRC_LIST} )

if (DEFINED  PCBA_3308)
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR PCBA_3308 ========  ")
set(SRC_LIST mic_test_Linux/record_test.c mic_test_Linux/vibrate_test.c)
add_library(mictest ${SRC_LIST})
set_property(TARGET mictest PROPERTY POSITION_INDEPENDENT_CODE ON)

set(RINGTEST Y)
set(SRC_LIST echo_ringmic_test.c)
add_executable( echo_ringmic_test ${SRC_LIST} )
target_link_libraries(echo_ringmic_test mictest)

set(SRC_LIST echo_ringmic_pdm_test.c)
add_executable( echo_ringmic_pdm_test ${SRC_LIST} )
target_link_libraries(echo_ringmic_pdm_test mictest)

set(SRC_LIST echo_ir_test.c)
add_executable( echo_ir_test ${SRC_LIST} )

install(TARGETS echo_ringmic_pdm_test mictest echo_ir_test DESTINATION ${INSTALL_TARGET_DIR})
else()
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR  NULL========  ")
endif()

if (DEFINED  PCBA_PX3SE)
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR PCBA_PX3SE ========  ")

set(RINGTEST Y)
set(SRC_LIST echo_ringmic_test.c)
link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/rk_pcba_test/libmictest.so)
add_executable( echo_ringmic_test ${SRC_LIST} )
else()
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR NULL ========  ")
endif()

MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "==CMAKE_CURRENT_SOURCE_DIR=")
MESSAGE(${CMAKE_STRIP}  "== CMAKE_STRIP")
MESSAGE(${CMAKE_C_COMPILER} "==CMAKE_C_COMPILER")

if (RINGTEST)
install(TARGETS echo_pcbatest_server echo_ringmic_test DESTINATION ${INSTALL_TARGET_DIR})
else()
install(TARGETS echo_pcbatest_server DESTINATION ${INSTALL_TARGET_DIR})
endif()

if (DEFINED  PCBA_1808)
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR PCBA_1808 ========  ")
set(SRC_LIST echo_npu_usb_screen_test.c)
add_executable( echo_npu_usb_screen_test ${SRC_LIST} )
install(TARGETS echo_npu_usb_screen_test DESTINATION ${INSTALL_TARGET_DIR})
endif()

if (DEFINED PCBA_356X)
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ===========SOURCE DIR PCBA_356X ===========")
endif()

if (DEFINED PCBA_3588)
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ===========SOURCE DIR PCBA_3588 ===========")
endif()

# pcba with ui
if (DEFINED  PCBA_WITH_UI)
#if(${PCBA_WITH_UI} STREQUAL "ON")
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR  PCBA_WITH_UI ========  ")
MESSAGE(${CMAKE_SOURCE_DIR} "  ====== CMAKE_SOURCE_DIR ========  ")
set(SRC_LIST
	pcba_minui/pcba_core.c
	pcba_minui/ui.c
	pcba_minui/safe_iop.c
	pcba_minui/strlcpy.c
	pcba_minui/strlcat.c
	pcba_minui/minzip/DirUtil.c
	pcba_minui/minzip/Hash.c
	pcba_minui/minzip/Inlines.c
	pcba_minui/minzip/SysUtil.c
	pcba_minui/minzip/Zip.c
	pcba_minui/mtdutils/mounts.c
	pcba_minui/mtdutils/mtdutils.c
	pcba_minui/mtdutils/rk29.c
	pcba_minui/minui/events.c
	pcba_minui/minui/graphics.c
	pcba_minui/minui/resources.c
	pcba_minui/minui/graphics_drm.c
	pcba_minui/script_parser.c
	pcba_minui/script.c
	pcba_minui/touch_test.c
	pcba_minui/rtc_test.c
	pcba_minui/wlan_test.c
	pcba_minui/bt_test.c
	pcba_minui/ddr_test.c
	pcba_minui/sdcard_test.c
	pcba_minui/key_test.c
	pcba_minui/screen_test.c
	pcba_minui/udisk_test.c
	pcba_minui/emmc_test.c
	pcba_minui/audio_play_test.c
	pcba_minui/audio_record_test.c
)

include_directories(
	${CMAKE_SYSROOT}/usr/include/libdrm/
	${CMAKE_CURRENT_SOURCE_DIR}/pcba_minui
	${CMAKE_CURRENT_SOURCE_DIR}/pcba_minui/minui
	${CMAKE_CURRENT_SOURCE_DIR}/pcba_minui/Language
)
#link_libraries(z c pthread png drm)
add_executable(pcba-core ${SRC_LIST})
target_link_libraries( pcba-core -lz -lc -lpthread -ldrm -lpng)
install(TARGETS pcba-core  DESTINATION ${INSTALL_TARGET_DIR})


else()
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "  ====== SOURCE_DIR No PCBA_WITH_UI ========  ")
endif()
