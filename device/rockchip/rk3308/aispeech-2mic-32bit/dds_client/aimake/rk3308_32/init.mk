#
# RK3308 32bit linux configuration
#
RK3308_LINUX_NDK_HOME =../../../../../buildroot/output/rockchip_rk3308_32_release/host/usr
TOOLCHAINS = $(RK3308_LINUX_NDK_HOME)
PLATFORM = $(RK3308_LINUX_NDK_HOME)
LIBEXEC  = $(RK3308_LINUX_NDK_HOME)/libexec/gcc/arm-rockchip-linux-gnueabihf/6.4.0/
CXX_STL = $(RK3308_LINUX_NDK_HOME)/arm-rockchip-linux-gnueabihf/include/c++/6.4.0

CC  = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-gcc
LD  = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-ld
CPP = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-cpp
CXX = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-c++
AR  = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-ar
AS  = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-as
NM  = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-nm
STRIP = $(TOOLCHAINS)/bin/arm-rockchip-linux-gnueabihf-strip


#CFLAGS := -fsigned-char -mfloat-abi=softfp -mfpu=neon
CFLAGS := -fsigned-char

CXXFLAGS := $(CFLAGS) -I $(CXX_STL)

LDFLAGS := -L$(PLATFORM)/arm-rockchip-linux-gnueabihf/lib -L$(LIBEXEC) -lpthread -ldl -lrt --sysroot=$(RK3308_LINUX_NDK_HOME)/arm-rockchip-linux-gnueabihf/sysroot

