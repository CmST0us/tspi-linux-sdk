################################################################################
#
# frecon
#
################################################################################

FRECON_VERSION = 34a1f7a64782fd511640c830308e6de9fc1d042d
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

define FRECON_INSTALL_TARGET_CMDS
	cp $(@D)/frecon $(TARGET_DIR)/usr/bin/
	cp -rp $(FRECON_PKGDIR)/frecon $(TARGET_DIR)/etc/
endef

define FRECON_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 $(FRECON_PKGDIR)/S35frecon \
		$(TARGET_DIR)/etc/init.d/S35frecon
endef

$(eval $(generic-package))
