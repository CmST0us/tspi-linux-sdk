################################################################################
#
# libcap-ng
#
################################################################################

LIBCAMERA_VERSION = 72278369b30af757c8ab756cc6c9cadcf15684d5
LIBCAMERA_SITE = git://linuxtv.org/libcamera.git
LIBCAMERA_LICENSE = GPL-2.0+ (programs), LGPL-2.1+ (library)
LIBCAMERA_LICENSE_FILES = \
	licenses/apache-2.0.txt \
	licenses/cc-by-sa-v4.0.txt \
	licenses/developer-certificate-of-origin.txt \
	licenses/gnu-gpl-2.0.txt \
	licenses/gnu-lgpl-2.1.txt

LIBCAMERA_INSTALL_STAGING = YES
LIBCAMERA_DEPENDENCIES = \
	host-python \
	host-python-pyyaml \
	host-meson

LIBCAMERA_CONF_OPTS = \
	--prefix=/usr \
	--libdir='/usr/lib' \
	--buildtype $(if $(BR2_ENABLE_DEBUG),debug,release) \
	--cross-file $(HOST_DIR)/etc/meson/cross-compilation.conf

LIBCAMERA_NINJA_OPTS = $(if $(VERBOSE),-v) -j$(PARALLEL_JOBS)

define LIBCAMERA_CONFIGURE_CMDS
	rm -rf $(@D)/build
	mkdir -p $(@D)/build
	$(TARGET_MAKE_ENV) meson $(LIBCAMERA_CONF_OPTS) $(@D) $(@D)/build
endef

define LIBCAMERA_BUILD_CMDS
	PYTHON="$(HOST_DIR)/bin/python" \
	$(TARGET_MAKE_ENV) ninja $(LIBCAMERA_NINJA_OPTS) -C $(@D)/build
endef

define LIBCAMERA_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) DESTDIR=$(TARGET_DIR) ninja $(LIBCAMERA_NINJA_OPTS) \
		-C $(@D)/build install
endef

define LIBCAMERA_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) DESTDIR=$(STAGING_DIR) ninja $(LIBCAMERA_NINJA_OPTS) \
		-C $(@D)/build install
endef

$(eval $(generic-package))
