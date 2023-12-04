################################################################################
#
# hplip
#
################################################################################

HPLIP_VERSION = 3.20.6
HPLIP_SITE = http://downloads.sourceforge.net/hplip/hplip
HPLIP_AUTORECONF = YES
HPLIP_DEPENDENCIES = cups libusb jpeg host-pkgconf
HPLIP_LICENSE = GPL-2.0, BSD-3-Clause, MIT
HPLIP_LICENSE_FILES = COPYING

ifeq ($(BR2_arm),y)
HPLIP_MACHINE=arm32
else ifeq ($(BR2_aarch64),y)
HPLIP_MACHINE=arm64
endif

HPLIP_CONF_OPTS = \
	--disable-qt4 \
	--disable-scan-build \
	--disable-gui-build \
	--disable-doc-build \
	--disable-network-build \
	--enable-hpcups-install \
	--disable-hpijs-install \
	--enable-cups-ppd-install \
	--enable-cups-drv-install \
	--disable-foomatic-ppd-install \
	--disable-foomatic-drv-install \
	--disable-foomatic-rip-hplip-install \
	--enable-new-hpcups \
	--enable-lite-build \
	--enable-class-driver

# build system does not support cups-config
HPLIP_CONF_ENV = LIBS=`$(STAGING_DIR)/usr/bin/cups-config --libs`

ifeq ($(BR2_PACKAGE_DBUS),y)
HPLIP_CONF_OPTS += --enable-dbus-build
HPLIP_DEPENDENCIES += dbus
else
HPLIP_CONF_OPTS += --disable-dbus-build
endif

define HPLIP_POST_INSTALL_TARGET_FIXUP
	mkdir -p $(TARGET_DIR)/usr/share/hplip/data/models
	cp $(@D)/data/models/* $(TARGET_DIR)/usr/share/hplip/data/models

	ln -sf hbpl1-$(HPLIP_MACHINE).so \
		$(TARGET_DIR)/usr/share/hplip/prnt/plugins/hbpl1.so
	ln -sf lj-$(HPLIP_MACHINE).so \
		$(TARGET_DIR)/usr/share/hplip/prnt/plugins/lj.so

	echo -e "[plugin]\ninstalled=1\neula=1\nversion=$(HPLIP_VERSION)" > \
		$(@D)/hplip.state
	$(INSTALL) -D -m 0644 $(@D)/hplip.state \
		$(TARGET_DIR)/var/lib/hp/hplip.state
endef
HPLIP_POST_INSTALL_TARGET_HOOKS += HPLIP_POST_INSTALL_TARGET_FIXUP

$(eval $(autotools-package))
