ifeq ($(BR2_PACKAGE_RV1108),y)

include package/rockchip/rv1108_package/.BoardConfig.mk
include $(sort $(wildcard package/rockchip/rv1108_package/*/*.mk))

RV_SDK_DIR=$(TOPDIR)/..
RV_OUTPUT_DIR=$(TOPDIR)/output/rockchip_rv1108
RV_IMAGE_DIR=$(RV_OUTPUT_DIR)/images
RV_LOADER_DIR=$(TOPDIR)/../loader
RV_KERNEL_DIR=$(TOPDIR)/../kernel
RV_USERDATA_DIR=$(RV_OUTPUT_DIR)/userdata
RV_MODULES_DIR=$(RV_OUTPUT_DIR)/modules
RV_ROOT_DIR=$(RV_OUTPUT_DIR)/root
RV_BUILD_DIR=$(TOPDIR)/../build
RV_DEVICE_PRODUCT_DIR=$(TOPDIR)/../device/rockchip/$(RK_TARGET_PRODUCT)
RV_COMMON_USERDATA_DIR=$(RV_DEVICE_PRODUCT_DIR)/userdata
RV_COMMON_ROOT_DATA_DIR=$(RV_DEVICE_PRODUCT_DIR)/root
RV_DEVICE_PRODUCT_BOARD_DIR=$(RV_DEVICE_PRODUCT_DIR)/overlay-board/rv1108-$(RK_TARGET_BOARD_VERSION)
RV_BOARD_USERDATA_DIR=$(RV_DEVICE_PRODUCT_BOARD_DIR)/userdata
RV_BOARD_ROOT_DIR=$(RV_DEVICE_PRODUCT_BOARD_DIR)/root

### build loader
ifeq ($(RK_LOADER_BUILD_TYPE),emmc)
    EMMC_ONLY=1
else ifeq ($(RK_LOADER_BUILD_TYPE),nor)
    NOR_ONLY=1
else
    ALL_SUPPORT=1
endif

PREISP_RKL_DTS=$(TOPDIR)/../kernel/arch/arm/boot/dts/$(RK_KERNEL_DTS).dts
PREISP_RKL_NODE=preisp_reserved
PREISP_RKL_ADDR_VALUE := $(shell grep ${PREISP_RKL_NODE} -A 6 ${PREISP_RKL_DTS} | \
                  sed ':a;N;ba;s/\n/ /g' | grep reg | cut -d '{' -f2 | cut -d '}' -f1 | cut -d '<' -f2 | cut -d ' ' -f1)
PREISP_RKL_ADDR := $(shell if [ -z ${PREISP_RKL_ADDR_VALUE} ]; then \
		  echo "none"; else echo "${PREISP_RKL_ADDR_VALUE}"; fi)

ifeq ($(RK_LOADER_PREISP_EN),1)
	LOADER_BUILD_MAKE_ENV += PREISP_EN=$(RK_LOADER_PREISP_EN)
	LOADER_BUILD_MAKE_ENV += PREISP_RKL_ADDR=$(PREISP_RKL_ADDR)
endif

LOADER_BUILD_MAKE_ENV += \
    POWER_HOLD_GPIO_GROUP=$(RK_LOADER_POWER_HOLD_GPIO_GROUP) \
    POWER_HOLD_GPIO_INDEX=$(RK_LOADER_POWER_HOLD_GPIO_INDEX) \
    EMMC_TURNING_DEGREE=$(RK_LOADER_EMMC_TURNING_DEGREE) \
    BOOTPART_SELECT=$(RK_LOADER_BOOTPART_SELECT) \
    EMMC_ONLY=$(EMMC_ONLY) \
    NOR_ONLY=$(NOR_ONLY) \
    ALL_SUPPORT=$(ALL_SUPPORT)

loader:
	make -C $(RV_LOADER_DIR) PLAT=rv1108usbplug
	make -C $(RV_LOADER_DIR) PLAT=rv1108loader $(LOADER_BUILD_MAKE_ENV)
	cp $(RV_LOADER_DIR)/rk_tools/bin/rv11/RV1108_DDR3.bin  $(RV_IMAGE_DIR)/rv1108ddr.bin
	cp $(RV_LOADER_DIR)/Project/rv1108loader/Debug/bin/rv1108loader.bin $(RV_IMAGE_DIR)/
	cp $(RV_LOADER_DIR)/RV1108_usb_boot_V1.24.bin $(RV_IMAGE_DIR)/RV1108_usb_boot.bin

loader-clean:
	make -C $(RV_LOADER_DIR) PLAT=rv1108usbplug clean
	make -C $(RV_LOADER_DIR) PLAT=rv1108loader clean


### build kernel
kernel:
	$(info RK_KERNEL_DEFCONFIG=$(RK_KERNEL_DEFCONFIG))
	$(info RK_ARCH=$(RK_ARCH))
	make -C $(RV_KERNEL_DIR) ARCH=$(RK_ARCH) $(RK_KERNEL_DTS).img -j$(RK_JOBS)
	make -C $(RV_KERNEL_DIR) modules -j$(RK_JOBS)
	make -C $(RV_KERNEL_DIR) INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=$(RV_OUTPUT_DIR)/tmp_modules modules_install
	mkdir -p $(RV_OUTPUT_DIR)/modules
	find $(RV_OUTPUT_DIR)/tmp_modules -name "*.ko" | xargs cp -t  $(RV_OUTPUT_DIR)/modules
	rm -fr $(RV_OUTPUT_DIR)/tmp_modules

kernel-clean:
	make -C $(RV_KERNEL_DIR) clean

define mk_parttion_image
fstype=`echo $(1)`; \
echo fstype=$$fstype; \
fssize=`echo $(2)`; \
echo fssize=$$fssize; \
outputfile=`echo $(3)`; \
echo outputfile=$$outputfile; \
inputfile=`echo $(4)`; \
echo inputfile=$$inputfile; \
if [ $$fstype == "ext4" ]; then \
	make_ext4fs -l $$fssize $$outputfile $$inputfile; \
elif [ $$fstype == "jffs2" ]; then \
	fssize=`echo "obase=16;$$fssize" | cut -d 'M' -f1 | bc`; \
	jffs2_fssize=0x$${fssize}00000;\
	echo jffs2_fssize=$$jffs2_fssize; \
	mkfs.jffs2 -d $$inputfile -o $$outputfile -e 0x10000 --pad=$$jffs2_fssize -n; \
else \
	make_ext4fs -l $$fssize $$outputfile $$inputfile; \
fi;
endef

### build userdata
userdata:
	if [ ! -d $(RV_USERDATA_DIR) ]; then mkdir -p $(RV_USERDATA_DIR); else rm -fr $(RV_USERDATA_DIR)/*; fi
	if [ -f $(RV_IMAGE_DIR)/userdata.img ]; then rm $(RV_IMAGE_DIR)/userdata.img; fi
	if [ -d $(RV_COMMON_USERDATA_DIR) ]; then cp -fr $(RV_COMMON_USERDATA_DIR)/* $(RV_USERDATA_DIR) 2>&1; fi
	if [ -d $(RV_BOARD_USERDATA_DIR) ]; then cp -fr $(RV_BOARD_USERDATA_DIR)/* $(RV_USERDATA_DIR) 2>&1; fi
	$(call mk_parttion_image,$(RK_USERDATA_FILESYSTEM_TYPE),$(RK_USERDATA_FILESYSTEM_SIZE),\
		$(RV_IMAGE_DIR)/userdata.img, $(RV_USERDATA_DIR))

### build root , is not rootfs
root:
	if [ -f $(RV_IMAGE_DIR)/root.img ]; then rm $(RV_IMAGE_DIR)/root.img; fi
	if [ -d $(RV_BOARD_ROOT_DIR) ]; then cp -fr $(RV_BOARD_ROOT_DIR)/* $(RV_ROOT_DIR) 2>&1; fi
	if [ -d $(RV_COMMON_ROOT_DATA_DIR) ]; then cp -fr $(RV_COMMON_ROOT_DATA_DIR)/* $(RV_ROOT_DIR) 2>&1; fi
	if [ -d $(RV_MODULES_DIR) ]; then cp -fr  $(RV_MODULES_DIR) $(RV_ROOT_DIR)/lib 2>&1; fi
	$(call mk_parttion_image,$(RK_ROOT_FILESYSTEM_TYPE),$(RK_ROOT_FILESYSTEM_SIZE),\
                $(RV_IMAGE_DIR)/root.img, $(RV_ROOT_DIR))

root-clean:
	if [ -d $(RV_ROOT_DIR) ]; then rm -fr $(RV_ROOT_DIR); fi

fw:
	if [ ! -L $(RV_SDK_DIR)/output ]; then ln -s buildroot/output $(RV_SDK_DIR)/output; fi
	if [ -f $(RV_IMAGE_DIR)/dtb ]; then rm $(RV_IMAGE_DIR)/dtb; fi
	cp $(RV_KERNEL_DIR)/arch/arm/boot/dts/$(RK_KERNEL_DTS).dtb $(RV_IMAGE_DIR)/dtb
	if [ -f $(RV_IMAGE_DIR)/kernel.img ]; then rm $(RV_IMAGE_DIR)/kernel.img; fi
	$(RV_BUILD_DIR)/kernelimage --pack --kernel $(RV_KERNEL_DIR)/arch/arm/boot/Image $(RV_IMAGE_DIR)/kernel.img 0x60308000 > /dev/null
	if [ -f $(RV_IMAGE_DIR)/Firmware.img ]; then rm $(RV_IMAGE_DIR)/Firmware.img; fi
	if [ -f $(RV_BOARD_USERDATA_DIR)/firmware/preisp.rkl ]; then cp $(RV_BOARD_USERDATA_DIR)/firmware/preisp.rkl $(RV_IMAGE_DIR);fi
	$(RV_BUILD_DIR)/firmwareMerger -p $(RV_BUILD_DIR)/setting_ini/$(RK_SETTING_INI) $(RV_IMAGE_DIR)

fww:
	cd $(TOPDIR)/../tools/Linux_Upgrade_Tool_* && ./linux_upgrade.sh

sync:
	$(TOPDIR)/../.repo/repo/repo sync

all: loader kernel userdata root fw


clean: loader-clean kernel-clean root-clean

reinstall: root-clean

# prepare for gdb debug env
RV_GDBDEBUG_DIR=$(RV_OUTPUT_DIR)/gdbdebug

prepare-gdb:
	if [ ! -d $(RV_GDBDEBUG_DIR) ]; then mkdir $(RV_GDBDEBUG_DIR); fi
	echo "add-auto-load-safe-path $(RV_GDBDEBUG_DIR)" > ~/.gdbinit;
	echo "set sysroot $(STAGING_DIR)" > $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "set solib-absolute-prefix $(STAGING_DIR)" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "set solib-search-path $(STAGING_DIR)" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "define enter_non_stop" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "  set pagination off" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "  set target-async on" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "  set non-stop on" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "end" >> $(RV_GDBDEBUG_DIR)/.gdbinit;
	echo "#!/bin/bash" > $(RV_GDBDEBUG_DIR)/rv_gdb.bash;
	echo "#./rv_gdb.bash corefile lock_app" >> $(RV_GDBDEBUG_DIR)/rv_gdb.bash;
	echo "LD_LIB_DIR=$(STAGING_DIR)/lib" >> $(RV_GDBDEBUG_DIR)/rv_gdb.bash;
	echo "GDB_LOAD_FILE=\`find $(STAGING_DIR)/usr -name \$$2\`" >> $(RV_GDBDEBUG_DIR)/rv_gdb.bash;
	echo "export LD_LIBRARY_PATH=${LD_LIB_DIR}" >> $(RV_GDBDEBUG_DIR)/rv_gdb.bash;
	echo "$(TARGET_CROSS)gdb --init-command=$(RV_GDBDEBUG_DIR)/.gdbinit --init-command=${GDB_LOAD_FILE} --core=\$$1" >> $(RV_GDBDEBUG_DIR)/rv_gdb.bash;
	chmod +x $(RV_GDBDEBUG_DIR)/rv_gdb.bash;


endif
