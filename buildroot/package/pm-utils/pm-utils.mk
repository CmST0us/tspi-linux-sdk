################################################################################
#
# pm-utils
#
################################################################################

PM_UTILS_VERSION = 1.4.1
PM_UTILS_SITE = http://pm-utils.freedesktop.org/releases
PM_UTILS_SOURCE = pm-utils-$(PM_UTILS_VERSION).tar.gz
PM_UTILS_LICENSE = GPL-2.0+
PM_UTILS_LICENSE_FILES = COPYING

# Run pm-utils hooks for system-sleep
define PM_UTILS_INSTALL_INIT_SYSTEMD
        $(INSTALL) -m 0755 -D -t $(TARGET_DIR)/lib/systemd/system-sleep/ \
		$(PM_UTILS_PKGDIR)/00pm-utils
endef

$(eval $(autotools-package))
