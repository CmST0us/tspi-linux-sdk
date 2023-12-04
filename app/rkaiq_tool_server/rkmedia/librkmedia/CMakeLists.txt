cmake_minimum_required(VERSION 2.8)

project(librkmedia C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig QUIET)
pkg_check_modules(ROCKCHIP_MPP REQUIRED rockchip_mpp)
include_directories(rkmpp)
include_directories(${ROCKCHIP_MPP_INCLUDE_DIRS}/rockchip/)
link_libraries(rockchip_mpp)

pkg_check_modules(LIBV4L2 libv4l2)
add_definitions(-DHAVE_LIBV4L2)
include_directories(${LIBV4L2_INCLUDE_DIRS}/)
link_libraries(v4l2)

add_definitions(-DLIVE555=ON 
 -DLIVE555_SERVER=ON
 -DLIVE555_SERVER_H264=ON
 -DLIVE555_SERVER_H265=ON)
 
include_directories(${PROJECT_SOURCE_DIR}/../include/groupsock)
include_directories(${PROJECT_SOURCE_DIR}/../include/liveMedia)
include_directories(${PROJECT_SOURCE_DIR}/../include/UsageEnvironment)
include_directories(${PROJECT_SOURCE_DIR}/../include/BasicUsageEnvironment)
link_libraries(${PROJECT_SOURCE_DIR}/../lib/${ARCH}/libliveMedia.a)
link_libraries(${PROJECT_SOURCE_DIR}/../lib/${ARCH}/libBasicUsageEnvironment.a)
link_libraries(${PROJECT_SOURCE_DIR}/../lib/${ARCH}/libgroupsock.a)
link_libraries(${PROJECT_SOURCE_DIR}/../lib/${ARCH}/libUsageEnvironment.a)


include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/include/easymedia)

aux_source_directory(${PROJECT_SOURCE_DIR}/ 				RKMEDIA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/flow 			RKMEDIA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/live555/server 	RKMEDIA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkmpp 			RKMEDIA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/stream/camera 	RKMEDIA_SOURCE)


add_library(rkmedia SHARED ${RKMEDIA_SOURCE})
target_link_libraries(rkmedia pthread)
#install(TARGETS rkmedia RUNTIME DESTINATION lib)
