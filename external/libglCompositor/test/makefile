CC=aarch64-linux-g++
#CC=/home/lzx/rk/linux_rk3326_SDK/buildroot/output/rockchip_rk3326_64/host/bin/aarch64-linux-g++
CFLAGS=-Iinclude -I/home/hds/RK3326-linux/buildroot/output/host/usr/aarch64-rockchip-linux-gnu/sysroot/usr/include/libdrm
OBJ = gl_test.cpp
LIBS=-lm -ldl -lmali -ldrm -lpthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gltest: $(OBJ)
	$(CC) -std=c++11 -o $@ $^ $(CFLAGS) $(LIBS)


