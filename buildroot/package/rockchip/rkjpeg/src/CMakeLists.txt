# vim: syntax=cmake
cmake_minimum_required(VERSION 2.8.11)
include_directories(.)
include_directories(/home/asx/linux/3128h/external/mpp/inc)
include_directories(/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/dec/jpeg)
include_directories(/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/test)
include_directories(/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/allocator)
include_directories(/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/dec/config)
include_directories(/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/dec/inc)
include_directories(/home/asx/linux/3128h/buildroot/package/rockchip/rkjpeg/src/dec/vpu_inc)
include_directories("/home/asx/linux/3128h/buildroot/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include" "/home/asx/linux/3128h/buildroot/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include/libdrm")

set(JPEGD_SRC
    dec/jpeg/jpegdechdrs.c
    dec/jpeg/jpegdecinternal.c
    dec/jpeg/jpegdecutils.c
    dec/jpeg/jpegdecscan.c
    dec/jpeg/dwl_test.c
    dec/jpeg/jpegdecapi.c
    dec/vpu_inc/vpu.c
    dec/vpu_inc/reg.cpp
    allocator/allocator_drm.c
    test/hw_jpegdecapi.c
    test/pv_jpegdec_api.cpp
    )
link_libraries("libdrm.so")
link_libraries("librockchip_vpu.so")
link_libraries("librockchip_mpp.so")
add_library(jpeghw_dec SHARED 
	${JPEGD_SRC}
    )

INSTALL(TARGETS jpeghw_dec ARCHIVE DESTINATION lib LIBRARY DESTINATION lib COMPONENT library)
