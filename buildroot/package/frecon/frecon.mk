################################################################################
#
# frecon
#
################################################################################

FRECON_VERSION = c6150b5173371cde887c6ef7f4be20e866b21686
FRECON_SITE = https://chromium.googlesource.com/chromiumos/platform/frecon
FRECON_SITE_METHOD = git
FRECON_LICENSE = ChromiumOS
FRECON_LICENSE_FILES = LICENSE

FRECON_DEPENDENCIES = host-python3 libdrm libpng libtsm udev

FRECON_MAKE_ENV = \
	PKG_CONFIG=$(PKG_CONFIG_HOST_BINARY) \
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) OUT=$(@D)/ \
	CHROMEOS=0 DRM_NO_MASTER=1 USE_UNIFONT=1

ifeq ($(BR2_PACKAGE_FRECON_USE_GETTY),)
FRECON_MAKE_ENV += USE_GETTY=0
endif

define FRECON_BUILD_CMDS
	$(FRECON_MAKE_ENV) $(MAKE) -C $(@D)
endef

ifeq ($(BR2_PACKAGE_FRECON_VTS),y)
FRECON_ARGS += --enable-vts
endif

ifeq ($(BR2_PACKAGE_FRECON_VT1),y)
FRECON_ARGS += --enable-vt1
endif

ifneq ($(BR2_PACKAGE_FRECON_ROTATE),0)
FRECON_ENV += export FRECON_FB_ROTATE=$(BR2_PACKAGE_FRECON_ROTATE)
endif

ifneq ($(BR2_PACKAGE_FRECON_SCALE),1)
FRECON_ENV += export FRECON_FB_SCALE=$(BR2_PACKAGE_FRECON_SCALE)
endif

define FRECON_INSTALL_TARGET_CMDS
	cp $(@D)/frecon $(TARGET_DIR)/usr/bin/
	cp -rp $(FRECON_PKGDIR)/frecon $(TARGET_DIR)/etc/
endef

define FRECON_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 $(FRECON_PKGDIR)/S35frecon \
		$(TARGET_DIR)/etc/init.d/S35frecon
	$(SED) 's/\(FRECON_ARGS=\).*/\1"$(FRECON_ARGS)"/' \
		$(TARGET_DIR)/etc/init.d/S35frecon
endef

define FRECON_INSTALL_TARGET_ENV
	echo $(FRECON_ENV) | xargs -n 2 > \
                $(TARGET_DIR)/etc/profile.d/frecon.sh
endef
FRECON_POST_INSTALL_TARGET_HOOKS += FRECON_INSTALL_TARGET_ENV

$(eval $(generic-package))
