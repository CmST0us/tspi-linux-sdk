################################################################################
#
# UnixBench
#
################################################################################

UNIXBENCH_VERSION = master
UNIXBENCH_SITE = https://github.com/kdlucas/byte-unixbench
UNIXBENCH_SITE_METHOD = git

UNIXBENCH_LICENSE = GPL-2.0+
UNIXBENCH_LICENSE_FILES = LICENSE.txt
UNIXBENCH_DEPENDENCIES = perl

UNIXBENCH_MAKE_OPTS = \
	UB_GCC_OPTIONS="-O3 -ffast-math" \
	CC="$(TARGET_CC)"

define UNIXBENCH_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/UnixBench $(UNIXBENCH_MAKE_OPTS)
endef

UNIXBENCH_TARGET_DIR = \
	$(TARGET_DIR)/opt/unixbench

define UNIXBENCH_INSTALL_TARGET_CMDS
	$(INSTALL) -d -m 755 \
		$(UNIXBENCH_TARGET_DIR)/results \
		$(UNIXBENCH_TARGET_DIR)/tmp \
		$(UNIXBENCH_TARGET_DIR)/pgms \
		$(UNIXBENCH_TARGET_DIR)/testdir
	$(INSTALL) -m 755 $(@D)/UnixBench/Run -t $(UNIXBENCH_TARGET_DIR)
	$(INSTALL) -m 755 $(@D)/UnixBench/pgms/* -t $(UNIXBENCH_TARGET_DIR)/pgms/
	$(INSTALL) -m 644 $(@D)/UnixBench/pgms/index.base -t $(UNIXBENCH_TARGET_DIR)/pgms/
	$(INSTALL) -m 644 $(@D)/UnixBench/pgms/unixbench.logo -t $(UNIXBENCH_TARGET_DIR)/pgms/
	$(INSTALL) -m 644 $(@D)/UnixBench/testdir/* -t $(UNIXBENCH_TARGET_DIR)/testdir/
endef

$(eval $(generic-package))
