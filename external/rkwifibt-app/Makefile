CFLAGS := -Wall -g

CC ?= /YOUPATH/buildroot/output/rockchip_rk3326_64/host/bin/aarch64-buildroot-linux-gnu-gcc
SYSROOT ?= --sysroot=/YOUPATH/buildroot/output/rockchip_rk3326_64/host/aarch64-buildroot-linux-gnu/sysroot

all: clean rkwifibt_test

OBJS := \
	test/main.o \
	test/rk_wifi_test.o \
	test/bt_test.o \
	test/rk_ble_app.o \
	test/softap/softap.o

ifeq ($(ARCH), arm64)
#ARCH=arm64
CFLAGS += -lpthread -lasound -L lib64/ -lrkwifibt -I include/
else
#ARCH=arm
CFLAGS += -lpthread -lasound -L lib32/ -lrkwifibt -I include/
endif

rkwifibt_test: $(OBJS)
	$(CC) -o rkwifibt_test $(OBJS) --sysroot=$(SYSROOT) $(CFLAGS)  -fPIC

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) rkwifibt_test
