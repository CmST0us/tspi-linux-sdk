################################################################################
#
# Chromium wayland
#
################################################################################
ifeq ($(BR2_aarch64),y)
CHROMIUM_WAYLAND_ARCH = aarch64
else
CHROMIUM_WAYLAND_ARCH = armhf
endif

CHROMIUM_WAYLAND_VERSION = 101.0.4951.54
CHROMIUM_WAYLAND_SITE = $(TOPDIR)/../external/chromium
CHROMIUM_WAYLAND_SOURCE = chromium-ozone-wayland_$(CHROMIUM_WAYLAND_VERSION)_$(CHROMIUM_WAYLAND_ARCH).tar.xz
CHROMIUM_WAYLAND_SITE_METHOD = file
CHROMIUM_WAYLAND_LICENSE_FILES = LICENSE

# The archive has no leading component.
CHROMIUM_WAYLAND_STRIP_COMPONENTS = 0

define CHROMIUM_WAYLAND_MERGE_DOWNLOAD
	cat $(CHROMIUM_WAYLAND_SITE)/$(CHROMIUM_WAYLAND_SOURCE)* > \
		$(DL_DIR)/$(CHROMIUM_WAYLAND_SOURCE)
endef
CHROMIUM_WAYLAND_PRE_DOWNLOAD_HOOKS += CHROMIUM_WAYLAND_MERGE_DOWNLOAD

define CHROMIUM_WAYLAND_COPY_LICENSES
	cp -rfp $(CHROMIUM_WAYLAND_SITE)/LICENSE $(@D)/
	cp -rfp $(CHROMIUM_WAYLAND_SITE)/licenses $(@D)/
endef
CHROMIUM_WAYLAND_POST_EXTRACT_HOOKS += CHROMIUM_WAYLAND_COPY_LICENSES

define CHROMIUM_WAYLAND_INSTALL_TARGET_CMDS
	cp -rp $(@D)/usr/ $(TARGET_DIR)/

	sed -i 's/\(CHROME_EXTRA_ARGS=\)/\1" $${CHROMIUM_FLAGS} "/' \
		$(TARGET_DIR)/usr/lib/chromium/chromium-wrapper

	$(INSTALL) -D -m 644 $(CHROMIUM_WAYLAND_PKGDIR)/chromium-wayland.sh \
		$(TARGET_DIR)/etc/profile.d/chromium-wayland.sh
endef

ifeq ($(BR2_PACKAGE_LIBV4L_RKMPP),y)
define CHROMIUM_WAYLAND_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 $(CHROMIUM_WAYLAND_PKGDIR)/S99chromium-wayland.sh \
		$(TARGET_DIR)/etc/init.d/S99chromium-wayland.sh
endef
endif

$(eval $(generic-package))
