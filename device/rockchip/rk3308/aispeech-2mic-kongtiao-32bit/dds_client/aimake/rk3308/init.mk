#
# RK3308 linux configuration
#
RK3308_LINUX_NDK_HOME = /home/sch/bak/rk3308-arm64-glibc-2018.03-toolschain
TOOLCHAINS = $(RK3308_LINUX_NDK_HOME)
PLATFORM = $(RK3308_LINUX_NDK_HOME)
LIBEXEC  = $(RK3308_LINUX_NDK_HOME)/libexec/gcc/aarch64-rockchip-linux-gnu/6.4.0
CXX_STL = $(RK3308_LINUX_NDK_HOME)/aarch64-rockchip-linux-gnu/include/c++/6.4.0

CC  = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-gcc
LD  = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-ld
CPP = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-cpp
CXX = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-c++
AR  = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-ar
AS  = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-as
NM  = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-nm
STRIP = $(TOOLCHAINS)/bin/aarch64-rockchip-linux-gnu-strip


#CFLAGS := -fsigned-char -mfloat-abi=softfp -mfpu=neon
CFLAGS := -fsigned-char -mcpu=cortex-a35+crc+crypto

CXXFLAGS := $(CFLAGS) -I $(CXX_STL)

LDFLAGS := -L$(PLATFORM)/aarch64-rockchip-linux-gnu/lib -L$(LIBEXEC) -lpthread -ldl -lrt --sysroot=$(RK3308_LINUX_NDK_HOME)/aarch64-rockchip-linux-gnu/sysroot

