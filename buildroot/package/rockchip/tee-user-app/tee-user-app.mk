################################################################################
#
# Rockchip tee-user-app For Linux
#
################################################################################

TEE_USER_APP_VERSION = develop
TEE_USER_APP_SITE = $(TOPDIR)/../external/security
TEE_USER_APP_SITE_METHOD = local

TEE_USER_APP_LICENSE = GPL-2.0
TEE_USER_APP_LICENSE_FILES = rk_tee_user/v2/LICENSE.md

TEE_USER_APP_OUTPUT=$(@D)/rk_tee_user/${BR2_PACKAGE_TEE_USER_APP_TEE_VERSION}/out
TEE_USER_APP_PRE_BIN=$(@D)/bin/optee_${BR2_PACKAGE_TEE_USER_APP_TEE_VERSION}

ifeq ($(BR2_ARCH_IS_64),y)
TEE_USER_APP_TOOLCHAIN_64 = $(TARGET_CROSS)
TEE_USER_APP_TOOLCHAIN_32 = $(BR2_PACKAGE_TEE_USER_APP_EXTRA_TOOLCHAIN)
TEE_USER_APP_ARCH=64
else
TEE_USER_APP_TOOLCHAIN_64 = $(BR2_PACKAGE_TEE_USER_APP_EXTRA_TOOLCHAIN)
TEE_USER_APP_TOOLCHAIN_32 = $(TARGET_CROSS)
TEE_USER_APP_ARCH=32
endif

define TEE_USER_APP_BUILD_CMDS
	cp $(TOPDIR)/package/rockchip/tee-user-app/extra_app/host \
		$(@D)/rk_tee_user/${BR2_PACKAGE_TEE_USER_APP_TEE_VERSION}/host/extra_app -r ; \
	cp $(TOPDIR)/package/rockchip/tee-user-app/extra_app/ta \
		$(@D)/rk_tee_user/${BR2_PACKAGE_TEE_USER_APP_TEE_VERSION}/ta/extra_app -r

	cd $(@D)/rk_tee_user/$(BR2_PACKAGE_TEE_USER_APP_TEE_VERSION) ; \
	AARCH64_TOOLCHAIN=$(TEE_USER_APP_TOOLCHAIN_64) ARM32_TOOLCHAIN=$(TEE_USER_APP_TOOLCHAIN_32) \
			  ./build.sh $(BR2_PACKAGE_TEE_USER_APP_COMPILE_CMD)
endef

define TEE_USER_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(TEE_USER_APP_OUTPUT)/extra_app/keybox_app $(TARGET_DIR)/usr/bin/
	mkdir $(TARGET_DIR)/lib/optee_armtz || true
	$(INSTALL) -D -m 755 $(TEE_USER_APP_OUTPUT)/ta/extra_app/*.ta $(TARGET_DIR)/lib/optee_armtz/
	$(INSTALL) -D -m 755 $(TEE_USER_APP_PRE_BIN)/lib/arm$(TEE_USER_APP_ARCH)/tee-supplicant $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(TEE_USER_APP_PRE_BIN)/lib/arm$(TEE_USER_APP_ARCH)/libteec.so* $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))
