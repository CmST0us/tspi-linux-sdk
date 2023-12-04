CAMERAHAL_SITE = $(TOPDIR)/../framework/camerahal
CAMERAHAL_SITE_METHOD = local
CAMERAHAL_INSTALL_STAGING = YES

CAMERAHAL_DEPENDENCIES = libion

define search_iq
dts_name=`echo $(1)`; \
build_camiq_dir=`echo $(2)`; \
target_use_bin=`echo $(3)`; \
target_camiq_dir=`echo $(4)`; \
dts_dir=`echo $${dts_name%/*}`; \
echo dts_name=$$dts_name; \
echo dts_dir=$$dts_dir; \
echo build_camiq_dir=$$build_camiq_dir; \
echo target_use_bin=$$target_use_bin; \
camera_temp=`grep "cif_isp0" -A 6 $(1) | sed ':a;N;$$!ba;s/\n/ /g' | cut -d '{' -f2 | cut -d '}' -f1 | cut -d '<' -f2 | cut -d '>' -f1 | tr -d "&"`; \
cam_id=$$camera_temp; \
temp=`echo "$$camera_temp" | grep "camera"`; \
if [[ -z "$$temp" ]]; then \
	cam_id=`grep "$$camera_temp" -A 10 $(1) | \
	grep "rockchip,rkisp-coproc-v4l2-platform-subdev" -A 10 | \
	grep "rockchip,camera-modules-attached" | \
	sed ':a;N;$$!ba;s/\n/ /g' | grep "&camera" | \
	cut -d '{' -f2 | cut -d '}' -f1 | cut -d '<' -f2 | cut -d '>' -f1 | tr -d "&"`; \
fi; \
echo cam_id=$$cam_id; \
cam_dts=`grep "&i2c" -A 30 $(1) | grep "^#include" | cut -d '"' -f2 | cut -d '"' -f1`; \
for i in $$cam_id; do \
	echo $$i; \
	if [[ ! -z "$$cam_dts" ]]; then \
		cam_iq=`grep "$$i:" -A 35 $$dts_dir/$$cam_dts | \
				sed ':a;N;$$!ba;s/\n/ /g' | \
				cut -d '{' -f2 | cut -d '}' -f1 | \
		        awk -F 'rockchip,camera-module-name' '{print $$(2)}' | \
				cut -d '"' -f2 | cut -d '"' -f1`; \
		echo cam_dts=$$cam_dts; \
		echo cam_iq=$$cam_iq; \
		cam_name=`grep "$$i:" -A 15 $$dts_dir/$$cam_dts | \
				grep "compatible" | \
				cut -d ',' -f2 | cut -d '-' -f1`; \
		echo cam_name=$$cam_name; \
	else \
		cam_iq=`grep "$$i:" -A 35 $(1) | \
				sed ':a;N;$$!ba;s/\n/ /g' | \
				cut -d '{' -f2 | cut -d '}' -f1 | \
				awk -F 'rockchip,camera-module-name' '{print $$(2)}' | \
				cut -d '"' -f2 | cut -d '"' -f1`; \
		echo cam_iq=$$cam_iq; \
		cam_name=`grep "$$i:" -A 15 $(1) | \
				grep "compatible" | \
				cut -d ',' -f2 | cut -d '-' -f1`; \
		echo cam_name=$$cam_name; \
	fi; \
	cam_xml_array+="$${cam_name}_$${cam_iq}.xml "; \
done; \
echo cam_xml_array=$$cam_xml_array; \
if [[ "bin" == $$target_use_bin ]]; then \
	echo "Use bin IQ data"; \
	for i in $$cam_xml_array; do \
		echo xml:$$i; \
		cam_xml_name=`echo $$i | cut -d '.' -f1`; \
		cp $$build_camiq_dir/CamIqData.db.$$cam_xml_name $$build_camiq_dir/CamIqData.db; \
		break; \
	done; \
else \
	echo "Use xml IQ data"; \
	for i in $$cam_xml_array; do \
		echo xml:$$i; \
		cp $$build_camiq_dir/$$i $$target_camiq_dir/; \
	done; \
fi;
endef

define CAMERAHAL_PRE_CONFIGURE_BIN_CMDS
	$(call search_iq,$(call qstrip,$(TOPDIR)/../kernel/arch/arm/boot/dts/$(RK_KERNEL_DTS).dts),\
		$(@D)/camera_engine_cifisp/IQ/bin,bin)
endef

define CAMERAHAL_PRE_INSTALL_TARGET_XML_CMDS
	mkdir -p $(TARGET_DIR)/etc/cam_iq/
	$(call search_iq,$(call qstrip,$(TOPDIR)/../kernel/arch/arm/boot/dts/$(RK_KERNEL_DTS).dts),\
		$(@D)/camera_engine_cifisp/IQ,xml,$(TARGET_DIR)/etc/cam_iq/)
endef

ifdef BR2_PACKAGE_CAMERAHAL_USE_IQ_BIN
CAMERAHAL_MAKE_OPTS += IQ_BIN_NAME="CamIqData.db"
CAMERAHAL_PRE_BUILD_HOOKS += CAMERAHAL_PRE_CONFIGURE_BIN_CMDS
else
CAMERAHAL_PRE_INSTALL_TARGET_HOOKS += CAMERAHAL_PRE_INSTALL_TARGET_XML_CMDS
endif

define CAMERAHAL_BUILD_CMDS
	-rm -rf $(TARGET_DIR)/etc/cam_iq
	cd $(@D) && \
        $(TARGET_MAKE_ENV) CROSS_COMPILE=$(TARGET_CROSS) $(MAKE) $(CAMERAHAL_MAKE_OPTS)
endef

define CAMERAHAL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m  644 $(@D)/camera_engine_cifisp/build/lib/libcam_engine_cifisp.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m  644 $(@D)/camera_engine_cifisp/build/lib/libcam_ia.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m  755 $(@D)/camera_engine_cifisp/build/bin/cnvIQ $(TARGET_DIR)/usr/bin/
	mkdir -p $(TARGET_DIR)/usr/include/CameraHal/linux
	$(foreach header,$(wildcard $(@D)/camera_engine_cifisp/HAL/include/*.h),$(INSTALL) -D -m 644 $(header) $(TARGET_DIR)/usr/include/CameraHal;)
	cp -fr $(@D)/camera_engine_cifisp/include/linux/* $(TARGET_DIR)/usr/include/CameraHal/linux
	cp -fr $(@D)/camera_engine_cifisp/include/* $(TARGET_DIR)/usr/include/
endef

define CAMERAHAL_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m  644 $(@D)/camera_engine_cifisp/build/lib/libcam_engine_cifisp.so $(STAGING_DIR)/usr/lib/
	$(INSTALL) -D -m  644 $(@D)/camera_engine_cifisp/build/lib/libcam_ia.so $(STAGING_DIR)/usr/lib/
	mkdir -p $(STAGING_DIR)/usr/include/CameraHal/linux
	$(foreach header,$(wildcard $(@D)/camera_engine_cifisp/include/*.h), $(INSTALL) -D -m 644 $(header) $(STAGING_DIR)/usr/include/CameraHal;)
	$(foreach header,$(wildcard $(@D)/camera_engine_cifisp/HAL/include/*.h),$(INSTALL) -D -m 644 $(header) $(STAGING_DIR)/usr/include/CameraHal;)
	cp -fr $(@D)/camera_engine_cifisp/include/linux/* $(STAGING_DIR)/usr/include/CameraHal/linux
	cp -fr $(@D)/camera_engine_cifisp/include/* $(STAGING_DIR)/usr/include/
endef

$(eval $(generic-package))
