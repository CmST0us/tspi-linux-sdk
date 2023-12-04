hide := @
ECHO := echo

SYS_PATH := ../../../buildroot/output/rockchip_puma/host/arm-buildroot-linux-gnueabihf/sysroot
OUT_PATH := ../../../buildroot/output/rockchip_puma/target
INC_FILES := -I $(SYS_PATH)/usr/include \
             -I $(SYS_PATH)/usr/include/dbus-1.0 \
             -I $(SYS_PATH)/usr/lib/dbus-1.0/include \
             -I $(SYS_PATH)/usr/include/glib-2.0 \
             -I $(SYS_PATH)/usr/lib/glib-2.0/include \
             -I $(SYS_PATH)/usr/include/libdrm \
             -I $(OUT_PATH)/usr/include \
             -I gdbus
LIB_FILES := -L $(OUT_PATH)/usr/lib

LD_FLAGS := -lpthread -lm -ldbus-1 -ldbus-glib-1 -lglib-2.0 -lgio-2.0 -lgobject-2.0 -lreadline -ljson-c -lsqlite3 -lrkdb
SRC_FILES := main.c

BIN_FILE := rkdb_demo

ledtest:
	$(hide)$(ECHO) "Build clock ..."
	./../../../buildroot/output/rockchip_puma/host/usr/bin/arm-buildroot-linux-gnueabihf-gcc $(SRC_FILES) $(INC_FILES) $(LIB_FILES) $(LD_FLAGS) -o $(BIN_FILE)
	$(hide)$(ECHO) "Build clock Done ..."
