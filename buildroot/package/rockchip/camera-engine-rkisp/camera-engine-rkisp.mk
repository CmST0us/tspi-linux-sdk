################################################################################
#
# Rockchip Camera Engine RKisp For Linux
#
################################################################################

CAMERA_ENGINE_RKISP_VERSION = 1.0
CAMERA_ENGINE_RKISP_SITE = $(TOPDIR)/../external/camera_engine_rkisp
CAMERA_ENGINE_RKISP_SITE_METHOD = local

CAMERA_ENGINE_RKISP_LICENSE = Apache V2.0
CAMERA_ENGINE_RKISP_LICENSE_FILES = NOTICE

CAMERA_ENGINE_RKISP_INSTALL_STAGING = YES

CAMERA_ENGINE_RKISP_MAKE_OPTS = \
	TARGET_GCC="$(TARGET_CC)" \
	TARGET_GPP="$(TARGET_CXX)" \
	TARGET_AR="$(TARGET_AR)" \
	TARGET_LD="$(TARGET_LD)" \

ifeq ($(BR2_PACKAGE_RK3326),y)
CAMERA_ENGINE_RKISP_CONF_OPTS = \
		IS_RKISP_v12=true
endif

ifeq ($(BR2_PACKAGE_PX30),y)
CAMERA_ENGINE_RKISP_CONF_OPTS = \
		IS_RKISP_v12=true
endif

ifneq ($(call qstrip,$(BR2_PACKAGE_CAMERA_ENGINE_RKISP_IQFILE)),)
CAMERA_ENGINE_RKISP_IQFILE = $(call qstrip,$(BR2_PACKAGE_CAMERA_ENGINE_RKISP_IQFILE))
else
CAMERA_ENGINE_RKISP_IQFILE = *.xml
endif

ifeq ($(call qstrip,$(BR2_ARCH)),arm)
CAMERA_ENGINE_RKISP_LIB = lib32
CAMERA_ENGINE_RKISP_GLIB_H = glib-2.0-32
CAMERA_ENGINE_RKISP_MAKE_OPTS += ARCH=arm
else
CAMERA_ENGINE_RKISP_LIB = lib64
CAMERA_ENGINE_RKISP_GLIB_H = glib-2.0-64
CAMERA_ENGINE_RKISP_MAKE_OPTS += ARCH=aarch64
endif

ifeq ($(BR2_PACKAGE_TINYXML2),y)
CAMERA_ENGINE_RKISP_CONF_OPTS = \
			IS_NEED_COMPILE_TINYXML2=false
CAMERA_ENGINE_RKISP_DEPENDENCIES = tinyxml2
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKISP_3A_SERVICE), y)
CAMERA_ENGINE_RKISP_INSTALL_3A_SCRIPT=y
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKISP_3A_SPEEDUP_DIR_ETC), y)
CAMERA_ENGINE_RKISP_IQ_DB_DIR = "/etc/iqfiles-db"
CAMERA_ENGINE_RKISP_3A_SCRIPT_FILENAME = S04rkisp_3A
else ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKISP_3A_SPEEDUP_DIR_USERDATA), y)
CAMERA_ENGINE_RKISP_IQ_DB_DIR = "/userdata/iqfiles-db"
CAMERA_ENGINE_RKISP_3A_SCRIPT_FILENAME = S40rkisp_3A
else
CAMERA_ENGINE_RKISP_IQ_DB_DIR = ""
CAMERA_ENGINE_RKISP_3A_SCRIPT_FILENAME = S40rkisp_3A
endif

export BUILD_OUPUT_GSTREAMER_LIBS:=$(@D)/ext/rkisp/usr/$(CAMERA_ENGINE_RKISP_LIB)/gstreamer-1.0
export BUILD_OUPUT_EXTERNAL_LIBS:=$(@D)/ext/rkisp/usr/$(CAMERA_ENGINE_RKISP_LIB)

define CAMERA_ENGINE_RKISP_CONFIGURE_CMDS
	rm -rf $(@D)/ext/rkisp/usr/lib
	rm -rf $(@D)/ext/rkisp/usr/include/glib-2.0
	cp -rf $(@D)/ext/rkisp/usr/include/$(CAMERA_ENGINE_RKISP_GLIB_H) $(@D)/ext/rkisp/usr/include/glib-2.0
	cp -rf $(@D)/ext/rkisp/usr/$(CAMERA_ENGINE_RKISP_LIB) $(@D)/ext/rkisp/usr/lib
endef

define CAMERA_ENGINE_RKISP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
	$(CAMERA_ENGINE_RKISP_MAKE_OPTS) \
	$(CAMERA_ENGINE_RKISP_CONF_OPTS)
endef

RKgstDir = $(TARGET_DIR)/usr/lib/gstreamer-1.0
RKafDir = $(TARGET_DIR)/usr/lib/rkisp/af
RKaeDir = $(TARGET_DIR)/usr/lib/rkisp/ae
RKawbDir = $(TARGET_DIR)/usr/lib/rkisp/awb

define CAMERA_ENGINE_RKISP_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/build/lib/librkisp.so $(STAGING_DIR)/usr/lib/
	$(INSTALL) -D -m 644 $(@D)/build/lib/librkisp_api.so $(STAGING_DIR)/usr/lib/
	mkdir -p $(STAGING_DIR)/usr/include/camera-engine-rkisp/interface
	$(foreach header,$(wildcard $($(PKG)_BUILDDIR)/interface/*.h),$(INSTALL) -D -m 644 $(header) $(STAGING_DIR)/usr/include/camera-engine-rkisp/interface;)
	$(INSTALL) -D -m 644 $(@D)/apps/rkisp_api/rkisp_api.h $(STAGING_DIR)/usr/include/camera-engine-rkisp/interface
endef

define CAMERA_ENGINE_RKISP_INSTALL_TARGET_CMDS
	mkdir -p $(RKgstDir)
	mkdir -p $(RKafDir)
	mkdir -p $(RKaeDir)
	mkdir -p $(RKawbDir)
	mkdir -p $(TARGET_DIR)/etc/iqfiles

	if [ x${CAMERA_ENGINE_RKISP_INSTALL_3A_SCRIPT} != x ]; then \
		$(INSTALL) -D -m 755 $(TOPDIR)/package/rockchip/camera-engine-rkisp/SXXrkisp_3A \
			$(TARGET_DIR)/etc/init.d/$(CAMERA_ENGINE_RKISP_3A_SCRIPT_FILENAME); \
		mkdir -p $(TARGET_DIR)/${CAMERA_ENGINE_RKISP_IQ_DB_DIR}; \
		sed -i "s:CAMERA_ENGINE_RKISP_XML_DB=.*:CAMERA_ENGINE_RKISP_XML_DB=${CAMERA_ENGINE_RKISP_IQ_DB_DIR}:g" \
			$(TARGET_DIR)/etc/init.d/$(CAMERA_ENGINE_RKISP_3A_SCRIPT_FILENAME); \
	fi

	$(INSTALL) -D -m 755 $(@D)/build/bin/rkisp_demo $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/build/bin/rkisp_3A_server $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 644 $(@D)/iqfiles/$(CAMERA_ENGINE_RKISP_IQFILE) $(TARGET_DIR)/etc/iqfiles/
	$(INSTALL) -D -m 644 $(@D)/build/lib/librkisp.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 644 $(@D)/build/lib/librkisp_api.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 644 $(@D)/plugins/3a/rkiq/af/$(CAMERA_ENGINE_RKISP_LIB)/librkisp_af.so $(RKafDir)/
	$(INSTALL) -D -m 644 $(@D)/plugins/3a/rkiq/aec/$(CAMERA_ENGINE_RKISP_LIB)/librkisp_aec.so $(RKaeDir)/
	$(INSTALL) -D -m 644 $(@D)/plugins/3a/rkiq/awb/$(CAMERA_ENGINE_RKISP_LIB)/librkisp_awb.so $(RKawbDir)/
endef

$(eval $(autotools-package))
