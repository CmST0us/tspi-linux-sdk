################################################################################
#
# android-tools
#
################################################################################

ANDROID_TOOLS_SITE = https://launchpad.net/ubuntu/+archive/primary/+files
ANDROID_TOOLS_VERSION = 4.2.2+git20130218
ANDROID_TOOLS_SOURCE = android-tools_$(ANDROID_TOOLS_VERSION).orig.tar.xz
ANDROID_TOOLS_EXTRA_DOWNLOADS = android-tools_$(ANDROID_TOOLS_VERSION)-3ubuntu41.debian.tar.gz
HOST_ANDROID_TOOLS_EXTRA_DOWNLOADS = $(ANDROID_TOOLS_EXTRA_DOWNLOADS)
ANDROID_TOOLS_LICENSE = Apache-2.0
ANDROID_TOOLS_LICENSE_FILES = debian/copyright
ANDROID_TOOLS_DEPENDENCIES = host-pkgconf
HOST_ANDROID_TOOLS_DEPENDENCIES = host-pkgconf

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_STATIC),y)
ANDROID_TOOLS_LDFLAGS = -static -ldl
endif

# Extract the Debian tarball inside the sources
define ANDROID_TOOLS_DEBIAN_EXTRACT
	$(call suitable-extractor,$(notdir $(ANDROID_TOOLS_EXTRA_DOWNLOADS))) \
		$(ANDROID_TOOLS_DL_DIR)/$(notdir $(ANDROID_TOOLS_EXTRA_DOWNLOADS)) | \
		$(TAR) -C $(@D) $(TAR_OPTIONS) -
endef

HOST_ANDROID_TOOLS_POST_EXTRACT_HOOKS += ANDROID_TOOLS_DEBIAN_EXTRACT
ANDROID_TOOLS_POST_EXTRACT_HOOKS += ANDROID_TOOLS_DEBIAN_EXTRACT

# Apply the Debian patches before applying the Buildroot patches
define ANDROID_TOOLS_DEBIAN_PATCH
	$(APPLY_PATCHES) $(@D) $(@D)/debian/patches \*
endef

HOST_ANDROID_TOOLS_PRE_PATCH_HOOKS += ANDROID_TOOLS_DEBIAN_PATCH
ANDROID_TOOLS_PRE_PATCH_HOOKS += ANDROID_TOOLS_DEBIAN_PATCH

ifeq ($(BR2_PACKAGE_HOST_ANDROID_TOOLS_FASTBOOT),y)
HOST_ANDROID_TOOLS_BUILD_TARGETS += fastboot
HOST_ANDROID_TOOLS_INSTALL_TARGETS += build-fastboot/fastboot
HOST_ANDROID_TOOLS_DEPENDENCIES += host-zlib host-libselinux
endif

ifeq ($(BR2_PACKAGE_HOST_ANDROID_TOOLS_ADB),y)
HOST_ANDROID_TOOLS_BUILD_TARGETS += adb
HOST_ANDROID_TOOLS_INSTALL_TARGETS += build-adb/adb
HOST_ANDROID_TOOLS_DEPENDENCIES += host-zlib host-openssl
endif

ifeq ($(BR2_PACKAGE_HOST_ANDROID_TOOLS_EXT4_UTILS),y)
HOST_ANDROID_TOOLS_BUILD_TARGETS += ext4_utils
HOST_ANDROID_TOOLS_INSTALL_TARGETS += \
	$(addprefix build-ext4_utils/,make_ext4fs ext4fixup ext2simg img2simg simg2img simg2simg)
HOST_ANDROID_TOOLS_DEPENDENCIES += host-libselinux
endif

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_FASTBOOT),y)
ANDROID_TOOLS_TARGETS += fastboot
ANDROID_TOOLS_DEPENDENCIES += zlib libselinux
endif

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_ADB),y)
ANDROID_TOOLS_TARGETS += adb
ANDROID_TOOLS_DEPENDENCIES += zlib openssl
endif

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_ADBD),y)
ANDROID_TOOLS_TARGETS += adbd
ANDROID_TOOLS_DEPENDENCIES += zlib openssl
endif

# Build each tool in its own directory not to share object files

define HOST_ANDROID_TOOLS_BUILD_CMDS
	$(foreach t,$(HOST_ANDROID_TOOLS_BUILD_TARGETS),\
		mkdir -p $(@D)/build-$(t) && \
		$(HOST_MAKE_ENV) $(HOST_CONFIGURE_OPTS) $(MAKE) SRCDIR=$(@D) \
			-C $(@D)/build-$(t) -f $(@D)/debian/makefiles/$(t).mk$(sep))
endef

define ANDROID_TOOLS_BUILD_CMDS
	$(foreach t,$(ANDROID_TOOLS_TARGETS),\
		mkdir -p $(@D)/build-$(t) && \
		$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) \
			LDFLAGS="$(ANDROID_TOOLS_LDFLAGS)" $(MAKE) SRCDIR=$(@D) \
			-C $(@D)/build-$(t) -f $(@D)/debian/makefiles/$(t).mk$(sep))
endef

define HOST_ANDROID_TOOLS_INSTALL_CMDS
	$(foreach t,$(HOST_ANDROID_TOOLS_INSTALL_TARGETS),\
		$(INSTALL) -D -m 0755 $(@D)/$(t) $(HOST_DIR)/bin/$(notdir $(t))$(sep))
endef

define ANDROID_TOOLS_INSTALL_TARGET_CMDS
	$(foreach t,$(ANDROID_TOOLS_TARGETS),\
		$(INSTALL) -D -m 0755 $(@D)/build-$(t)/$(t) $(TARGET_DIR)/usr/bin/$(t)$(sep))
endef

ifeq ($(BR2_PACKAGE_ANDROID_TOOLS_ADBD),y)
define ANDROID_TOOLS_INSTALL_TARGET_SHELL
	mkdir -p $(TARGET_DIR)/etc/profile.d
	echo "[ -x /bin/bash ] && export ADBD_SHELL=/bin/bash" > \
		$(TARGET_DIR)/etc/profile.d/adbd.sh
endef
ANDROID_TOOLS_PRE_INSTALL_TARGET_HOOKS += ANDROID_TOOLS_INSTALL_TARGET_SHELL

ifneq ($(BR2_PACKAGE_ANDROID_TOOLS_TCP_PORT),0)
define ANDROID_TOOLS_INSTALL_TARGET_TCP_PORT
	echo "export ADB_TCP_PORT=$(BR2_PACKAGE_ANDROID_TOOLS_TCP_PORT)" >> \
		$(TARGET_DIR)/etc/profile.d/adbd.sh
endef
ANDROID_TOOLS_POST_INSTALL_TARGET_HOOKS += ANDROID_TOOLS_INSTALL_TARGET_TCP_PORT
endif

ADBD_AUTH_PASSWORD = $(call qstrip,$(BR2_PACKAGE_ANDROID_TOOLS_AUTH_PASSWORD))
ifneq ($(ADBD_AUTH_PASSWORD),)
ADBD_AUTH_PASSWORD_MD5=$(shell echo $(ADBD_AUTH_PASSWORD) | md5sum)

define ANDROID_TOOLS_INSTALL_TARGET_PASSWORD
	$(INSTALL) -D -m 0755 $(ANDROID_TOOLS_PKGDIR)/adbd-auth \
		$(TARGET_DIR)/usr/bin/adbd-auth
	sed -i "s/AUTH_PASSWORD/${ADBD_AUTH_PASSWORD_MD5}/g" \
		$(TARGET_DIR)/usr/bin/adbd-auth
endef
ANDROID_TOOLS_POST_INSTALL_TARGET_HOOKS += ANDROID_TOOLS_INSTALL_TARGET_PASSWORD
endif

ADBD_RSA_KEY = $(call qstrip,$(BR2_PACKAGE_ANDROID_TOOLS_AUTH_RSA_KEY))
ifneq ($(ADBD_RSA_KEY),)
define ANDROID_TOOLS_INSTALL_TARGET_RSA_KEY
	echo "export ADBD_RSA_KEY_FILE=/etc/adbkey.pub" >> \
		$(TARGET_DIR)/etc/profile.d/adbd.sh
	$(INSTALL) -D -m 0644 $(ADBD_RSA_KEY) $(TARGET_DIR)/etc/adbkey.pub
endef
ANDROID_TOOLS_POST_INSTALL_TARGET_HOOKS += ANDROID_TOOLS_INSTALL_TARGET_RSA_KEY
endif

endif

$(eval $(generic-package))
$(eval $(host-generic-package))
