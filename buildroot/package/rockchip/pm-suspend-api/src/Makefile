
all: libsuspend_api.so

dbus_suspend_api.o: dbus_suspend_api.c
	$(CC) -c dbus_suspend_api.c $(CFLAGS) -fPIC -g -Wall

libsuspend_api.so: dbus_suspend_api.o
	$(CC) -o $@ dbus_suspend_api.o $(CFLAGS) -fPIC -shared

.PHONY: clean

clean:
	rm -f *.o *~ libsuspend_api.so

.PHONY: install

install:
	cp -f dbus_suspend_api.h $(HOST_DIR)/aarch64-rockchip-linux-gnu/sysroot/usr/include/
	cp -f libsuspend_api.so $(HOST_DIR)/aarch64-rockchip-linux-gnu/sysroot/usr/lib/
	cp -f libsuspend_api.so $(TARGET_DIR)/usr/lib/
	cp -f 00rockchip-suspend $(TARGET_DIR)/usr/lib/pm-utils/sleep.d/

.PHONY: uninstall

uninstall:
	rm -f $(TARGET_DIR)/usr/lib/libsuspend_api.so
	rm -f $(TARGET_DIR)/usr/lib/pm-utils/sleep.d/00rockchip-suspend
