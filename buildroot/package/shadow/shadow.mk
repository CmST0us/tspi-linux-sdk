################################################################################
#
# shadow
#
################################################################################

SHADOW_VERSION = 4.11.1
SHADOW_SITE = https://github.com/shadow-maint/shadow/releases/download/v$(SHADOW_VERSION)
SHADOW_SOURCE = shadow-$(SHADOW_VERSION).tar.xz
SHADOW_LICENSE = BSD-3-Clause
SHADOW_LICENSE_FILES = COPYING

SHADOW_CONF_OPTS += \
	--disable-man \
	--without-btrfs \
	--without-skey \
	--without-tcb

ifeq ($(BR2_STATIC_LIBS),y)
SHADOW_CONF_OPTS += --enable-static
else
SHADOW_CONF_OPTS += --disable-static
endif

ifeq ($(BR2_SHARED_LIBS),y)
SHADOW_CONF_OPTS += --enable-shared
else
SHADOW_CONF_OPTS += --disable-shared
endif

ifeq ($(BR2_PACKAGE_SHADOW_SHADOWGRP),y)
SHADOW_CONF_OPTS += --enable-shadowgrp
else
SHADOW_CONF_OPTS += --disable-shadowgrp
endif

ifeq ($(BR2_PACKAGE_SHADOW_ACCOUNT_TOOLS_SETUID),y)
SHADOW_CONF_OPTS += --enable-account-tools-setuid
SHADOW_ACCOUNT_TOOLS_SETUID = \
	/usr/sbin/chgpasswd f 4755 0 0 - - - - - \
	/usr/sbin/chpasswd f 4755 0 0 - - - - - \
	/usr/sbin/groupadd f 4755 0 0 - - - - - \
	/usr/sbin/groupdel f 4755 0 0 - - - - - \
	/usr/sbin/groupmod f 4755 0 0 - - - - - \
	/usr/sbin/newusers f 4755 0 0 - - - - - \
	/usr/sbin/useradd f 4755 0 0 - - - - - \
	/usr/sbin/usermod f 4755 0 0 - - - - -
else
SHADOW_CONF_OPTS += --disable-account-tools-setuid
endif

ifeq ($(BR2_PACKAGE_SHADOW_UTMPX),y)
SHADOW_CONF_OPTS += --enable-utmpx
else
SHADOW_CONF_OPTS += --disable-utmpx
endif

ifeq ($(BR2_PACKAGE_SHADOW_SUBORDINATE_IDS),y)
SHADOW_CONF_OPTS += --enable-subordinate-ids
SHADOW_SUBORDINATE_IDS_PERMISSIONS =  \
	/usr/bin/newuidmap f 4755 0 0 - - - - - \
	/usr/bin/newgidmap f 4755 0 0 - - - - -
else
SHADOW_CONF_OPTS += --disable-subordinate-ids
endif

ifeq ($(BR2_PACKAGE_ACL),y)
SHADOW_CONF_OPTS += --with-acl
SHADOW_DEPENDENCIES += acl
else
SHADOW_CONF_OPTS += --without-acl
endif

ifeq ($(BR2_PACKAGE_ATTR),y)
SHADOW_CONF_OPTS += --with-attr
SHADOW_DEPENDENCIES += attr
else
SHADOW_CONF_OPTS += --without-attr
endif

ifeq ($(BR2_PACKAGE_AUDIT),y)
SHADOW_CONF_OPTS += --with-audit
SHADOW_DEPENDENCIES += audit
else
SHADOW_CONF_OPTS += --without-audit
endif

ifeq ($(BR2_PACKAGE_CRACKLIB),y)
SHADOW_CONF_OPTS += --with-libcrack
SHADOW_DEPENDENCIES += cracklib
else
SHADOW_CONF_OPTS += --without-libcrack
endif

ifeq ($(BR2_PACKAGE_LIBSELINUX),y)
SHADOW_CONF_OPTS += --with-selinux
SHADOW_DEPENDENCIES += libselinux libsemanage
else
SHADOW_CONF_OPTS += --without-selinux
endif

ifeq ($(BR2_PACKAGE_LINUX_PAM),y)
SHADOW_CONF_OPTS += --with-libpam
SHADOW_DEPENDENCIES += linux-pam
else
SHADOW_CONF_OPTS += --without-libpam
endif

ifeq ($(BR2_ENABLE_LOCALE),y)
SHADOW_CONF_OPTS += --enable-nls
else
SHADOW_CONF_OPTS += --disable-nls
endif

ifeq ($(BR2_PACKAGE_SHADOW_SHA_CRYPT),y)
SHADOW_CONF_OPTS += --with-sha-crypt
else
SHADOW_CONF_OPTS += --without-sha-crypt
endif

ifeq ($(BR2_PACKAGE_SHADOW_BCRYPT),y)
SHADOW_CONF_OPTS += --with-bcrypt
else
SHADOW_CONF_OPTS += --without-bcrypt
endif

ifeq ($(BR2_PACKAGE_SHADOW_YESCRYPT),y)
SHADOW_CONF_OPTS += --with-yescrypt
else
SHADOW_CONF_OPTS += --without-yescrypt
endif

ifeq ($(BR2_PACKAGE_SHADOW_NSCD),y)
SHADOW_CONF_OPTS += --with-nscd
else
SHADOW_CONF_OPTS += --without-nscd
endif

ifeq ($(BR2_PACKAGE_SHADOW_SSSD),y)
SHADOW_CONF_OPTS += --with-sssd
else
SHADOW_CONF_OPTS += --without-sssd
endif

ifeq ($(BR2_PACKAGE_SHADOW_GROUP_NAME_MAX_LENGTH),0)
SHADOW_CONF_OPTS += --without-group-name-max-length
else
SHADOW_CONF_OPTS += --with-group-name-max-length=$(BR2_PACKAGE_SHADOW_GROUP_NAME_MAX_LENGTH)
endif

ifeq ($(BR2_PACKAGE_SHADOW_SU),y)
SHADOW_CONF_OPTS += --with-su
SHADOW_SU_PERMISSIONS = /bin/su f 4755 0 0 - - - - -
else
SHADOW_CONF_OPTS += --without-su
endif

define SHADOW_PERMISSIONS
	/usr/bin/chage f 4755 0 0 - - - - -
	/usr/bin/chfn f 4755 0 0 - - - - -
	/usr/bin/chsh f 4755 0 0 - - - - -
	/usr/bin/expiry f 4755 0 0 - - - - -
	/usr/bin/gpasswd f 4755 0 0 - - - - -
	/usr/bin/newgrp f 4755 0 0 - - - - -
	/usr/bin/passwd f 4755 0 0 - - - - -
	$(SHADOW_ACCOUNT_TOOLS_SETUID)
	$(SHADOW_SUBORDINATE_IDS_PERMISSIONS)
	$(SHADOW_SU_PERMISSIONS)
endef

define SHADOW_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/src/passwd $(TARGET_DIR)/usr/bin/passwd
	$(INSTALL) -m 0755 -D $(@D)/src/useradd $(TARGET_DIR)/usr/bin/useradd
	$(INSTALL) -m 0755 -D $(@D)/src/userdel $(TARGET_DIR)/usr/bin/userdel
	$(INSTALL) -m 0755 -D $(@D)/src/usermod $(TARGET_DIR)/usr/bin/usermod
	$(INSTALL) -m 0755 -D $(@D)/src/groupadd $(TARGET_DIR)/usr/bin/groupadd
	$(INSTALL) -m 0755 -D $(@D)/src/groupdel $(TARGET_DIR)/usr/bin/groupdel
	$(INSTALL) -m 0755 -D $(@D)/src/groupmod $(TARGET_DIR)/usr/bin/groupmod
	$(INSTALL) -m 0755 -D $(@D)/src/chage $(TARGET_DIR)/usr/bin/chage
	$(INSTALL) -m 0755 -D $(@D)/src/expiry $(TARGET_DIR)/usr/bin/expiry
	$(INSTALL) -m 0755 -D $(@D)/src/gpasswd $(TARGET_DIR)/usr/bin/gpasswd
	$(INSTALL) -m 0755 -D $(@D)/src/newuidmap $(TARGET_DIR)/usr/bin/newuidmap
	$(INSTALL) -m 0755 -D $(@D)/src/chgpasswd $(TARGET_DIR)/usr/sbin/chgpasswd
endef

$(eval $(autotools-package))
