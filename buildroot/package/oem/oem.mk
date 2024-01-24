################################################################################
#
# oem
#
################################################################################

OEM_ADD_TOOLCHAIN_DEPENDENCY = NO

define OEM_TARGET_FINALIZE_CLEAN_HOOK
	@$(call MESSAGE,"Finalizing OEM directory")
	rm -rf $(TARGET_DIR)/oem/usr/include \
		$(TARGET_DIR)/oem/usr/share/aclocal \
		$(TARGET_DIR)/oem/usr/lib/pkgconfig \
		$(TARGET_DIR)/oem/usr/share/pkgconfig \
		$(TARGET_DIR)/oem/usr/lib/cmake \
		$(TARGET_DIR)/oem/usr/share/cmake \
		$(TARGET_DIR)/oem/usr/doc
	find $(TARGET_DIR)/oem/usr/{lib,share}/ -name '*.cmake' -print0 | \
		xargs -0 rm -f
	find $(TARGET_DIR)/oem/lib/ \
		$(TARGET_DIR)/oem/usr/lib/ \
		$(TARGET_DIR)/oem/usr/libexec/ \
		\( -name '*.a' -o -name '*.la' -o -name '*.prl' \) -print0 | \
		xargs -0 rm -f
endef
OEM_TARGET_FINALIZE_HOOKS += OEM_TARGET_FINALIZE_CLEAN_HOOK

define OEM_INSTALL_TARGET_CMDS
        mkdir -p $(TARGET_DIR)/usr/share/dbus-1/system.d
        $(INSTALL) -D -m 0644 $(OEM_PKGDIR)/redirect_dbus4oem.conf \
                $(TARGET_DIR)/usr/share/dbus-1/system.d
endef

$(eval $(generic-package))
