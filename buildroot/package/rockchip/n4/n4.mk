N4_SITE = $(TOPDIR)/../app/n4
N4_SITE_METHOD = local
N4_INSTALL_STAGING = NO

N4_DEPENDENCIES = host-ninja libserialport libv4l rkmedia

define INSTALL_N4_KERNEL_HEADER
cp $(TOPDIR)/../kernel/include/uapi/linux/rk-usbacm-msg.h $(STAGING_DIR)/usr/include/linux/
endef

N4_PRE_CONFIGURE_HOOKS += INSTALL_N4_KERNEL_HEADER

#N4_CONF_OPTS = -DWARNINGS_AS_ERRORS=ON

N4_CONF_OPTS += -G Ninja

N4_NINJA_OPTS += $(if $(VERBOSE),-v) -j$(PARALLEL_JOBS)

define N4_BUILD_CMDS
$(TARGET_MAKE_ENV) $($(PKG)_MAKE_ENV) ninja $($(PKG)_NINJA_OPTS) -C $($(PKG)_BUILDDIR)
endef

define N4_INSTALL_TARGET_CMDS
$(TARGET_MAKE_ENV) DESTDIR=$(TARGET_DIR) ninja $($(PKG)_NINJA_OPTS) -C $($(PKG)_BUILDDIR) install
endef

define N4_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 $(N4_PKGDIR)/S51n4 $(TARGET_DIR)/etc/init.d/S51n4
endef

$(eval $(cmake-package))
