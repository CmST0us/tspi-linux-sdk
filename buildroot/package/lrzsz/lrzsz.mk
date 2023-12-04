################################################################################
#
# lrzsz
#
################################################################################

LRZSZ_VERSION = 0.12.20
LRZSZ_SITE = http://www.ohse.de/uwe/releases
LRZSZ_CONF_OPTS = --disable-timesync
LRZSZ_LICENSE = GPL-2.0+
LRZSZ_LICENSE_FILES = COPYING

LRZSZ_CFLAGS = $(filter-out -D_FORTIFY_SOURCE=1 -D_FORTIFY_SOURCE=2, $(TARGET_CFLAGS))
LRZSZ_CPPFLAGS = $(filter-out -D_FORTIFY_SOURCE=1 -D_FORTIFY_SOURCE=2, $(TARGET_CPPFLAGS))

LRZSZ_CONF_ENV += \
    CFLAGS="$(LRZSZ_CFLAGS)" \
    CPPFLAGS="$(LRZSZ_CPPFLAGS)"

define LRZSZ_POST_CONFIGURE_HOOKS
	$(SED) "s/-lnsl//;" $(@D)/src/Makefile
	$(SED) "s~\(#define ENABLE_SYSLOG.*\)~/* \1 */~;" $(@D)/config.h
endef

define LRZSZ_BUILD_HOOKS
	$(TARGET_MAKE_ENV) $(MAKE) CROSS_COMPILE="$(TARGET_CROSS)" prefix="$(TARGET_DIR)" -C $(@D)
endef

define LRZSZ_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/src/lrz $(TARGET_DIR)/usr/bin/rz
	$(INSTALL) -m 0755 -D $(@D)/src/lsz $(TARGET_DIR)/usr/bin/sz
	ln -sf rz $(TARGET_DIR)/usr/bin/lrz
	ln -sf sz $(TARGET_DIR)/usr/bin/lsz
endef

$(eval $(autotools-package))
