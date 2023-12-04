hide := @
ECHO := echo

SYS_PATH := ../../../buildroot/output/rockchip_puma/host/arm-buildroot-linux-gnueabihf/sysroot
OUT_PATH := ../../../buildroot/output/rockchip_puma/target
STAGOUT_PATH := ../../buildroot/output/rockchip_puma/staging
INC_FILES := -I $(SYS_PATH)/usr/include \
             -I $(SYS_PATH)/usr/include/dbus-1.0 \
             -I $(SYS_PATH)/usr/lib/dbus-1.0/include \
             -I $(SYS_PATH)/usr/include/glib-2.0 \
             -I $(SYS_PATH)/usr/lib/glib-2.0/include \
             -I $(SYS_PATH)/usr/include/libdrm \
             -I $(SYS_PATH)/usr/include/libxml2 \
             -I $(OUT_PATH)/usr/include \
             -I $(STAGOUT_PATH)/usr/include \
             -I gdbus
LIB_FILES := -L $(OUT_PATH)/usr/lib

#LD_FLAGS := -lpthread -ljpeg -lpng12 -lm -ldbus-1 -ldbus-glib-1 -lglib-2.0 -lgio-2.0 -lgobject-2.0 -lreadline -ldrm -lminigui_ths -lfreetype -lts -lrga
#SRC_FILES := agent.c dbus_helpers.c netctl.c gdbus/*.c ui/*.c
LD_FLAGS := -lpthread -lm -ldbus-1 -ldbus-glib-1 -lglib-2.0 -lgio-2.0 -lgobject-2.0 -lreadline -lsqlite3 -ljson-c -lIPCProtocol

DBSERVER_TEST_SRC_FILES := dbserver_test.c
DBSERVER_TEST_BIN_FILE := dbserver_test

NETSERVER_TEST_SRC_FILES := netserver_test.c
NETSERVER_TEST_BIN_FILE := netserver_test

STORAGE_MANAGER_TEST_SRC_FILES := storage_manager_test.c
STORAGE_MANAGER_TEST_BIN_FILE := storage_manager_test

DBSERVER_INITDATA_LD_FLAGS := -lpthread -lm -ldbus-1 -ldbus-glib-1 -lglib-2.0 -lgio-2.0 -lgobject-2.0 -lreadline -lsqlite3 -ljson-c -lIPCProtocol -lxml2
DBSERVER_INITDATA_SRC_FILES := dbserver_initdata.c
DBSERVER_INITDATA_BIN_FILE := dbserver_initdata

out:
	$(hide)$(ECHO) "Build bin ..."
	./../../../buildroot/output/rockchip_puma/host/usr/bin/arm-buildroot-linux-gnueabihf-gcc $(DBSERVER_TEST_SRC_FILES) $(INC_FILES) $(LIB_FILES) $(LD_FLAGS) -o $(DBSERVER_TEST_BIN_FILE)
	./../../../buildroot/output/rockchip_puma/host/usr/bin/arm-buildroot-linux-gnueabihf-gcc $(NETSERVER_TEST_SRC_FILES) $(INC_FILES) $(LIB_FILES) $(LD_FLAGS) -o $(NETSERVER_TEST_BIN_FILE)
	./../../../buildroot/output/rockchip_puma/host/usr/bin/arm-buildroot-linux-gnueabihf-gcc $(STORAGE_MANAGER_TEST_SRC_FILES) $(INC_FILES) $(LIB_FILES) $(LD_FLAGS) -o $(STORAGE_MANAGER_TEST_BIN_FILE)
	./../../../buildroot/output/rockchip_puma/host/usr/bin/arm-buildroot-linux-gnueabihf-gcc $(DBSERVER_INITDATA_SRC_FILES) $(INC_FILES) $(LIB_FILES) $(DBSERVER_INITDATA_LD_FLAGS) -o $(DBSERVER_INITDATA_BIN_FILE)
	$(hide)$(ECHO) "Build bin Done ..."
