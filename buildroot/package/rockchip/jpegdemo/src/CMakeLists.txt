# vim: syntax=cmake
cmake_minimum_required(VERSION 2.8.11)

include_directories(.)
include_directories("/home/asx/linux/3128h/external/mpp/inc")
include_directories("/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src")
include_directories("/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/allocator")
include_directories("/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/test")
include_directories("/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/dec/inc")
include_directories("/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/dec/vpu_inc")
include_directories("/home/asx/linux/3128h/buildroot/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include" "/home/asx/linux/3128h/buildroot/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include/libdrm")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ADD_DEFINITIONS(-fpermissive)

link_libraries("libdrm.so")
link_libraries("librockchip_mpp.so")
link_libraries("librockchip_vpu.so")
link_libraries("libjpeghw_dec.so")
link_libraries("libpthread.so")
add_executable(jpeg_demo
    main.cpp
    )

INSTALL(TARGETS jpeg_demo RUNTIME DESTINATION bin)
