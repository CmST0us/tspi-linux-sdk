################################################################################
#
# Punybench
#
################################################################################

PUNYBENCH_VERSION = master
PUNYBENCH_SITE = $(TOPDIR)/../external/punybench
PUNYBENCH_SITE_METHOD = local

PUNYBENCH_LICENSE = GPLv2
PUNYBENCH_LICENSE_FILES = LICENSE/gpl.txt

PUNYBENCH_BUILD_OPTS = \
	-Wno-error=deprecated-declarations \
	-Wno-error=maybe-uninitialized \

PUNYBENCH_MAKE = \
	$(MAKE) BOARD=$(BR2_ARCH) \

PUNYBENCH_MAKE_OPTS = \
	CC="$(TARGET_CC) $(PUNYBENCH_BUILD_OPTS)" \

define PUNYBENCH_CONFIGURE_CMDS
	# Do nothing
endef

define PUNYBENCH_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(PUNYBENCH_MAKE) -C $(@D) \
		$(PUNYBENCH_MAKE_OPTS)
endef

PUNYBENCH_POST_INSTALL_TARGET_HOOKS += PUNYBENCH_INSTALL_SCRIPT_HOOK

define PUNYBENCH_INSTALL_SCRIPT_HOOK
	$(INSTALL) -D -m 755 $(@D)/scripts/punybench.sh -t $(TARGET_DIR)/opt/punybench/script/
endef

$(eval $(autotools-package))
