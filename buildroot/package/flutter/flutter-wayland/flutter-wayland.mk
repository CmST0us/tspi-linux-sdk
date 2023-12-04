################################################################################
#
# Flutter wayland
#
################################################################################

FLUTTER_WAYLAND_VERSION = eee6dd1047ce85fcb9b5e517598b40bea7c76bea
FLUTTER_WAYLAND_SITE = $(call github,JeffyCN,flutter_wayland,$(FLUTTER_WAYLAND_VERSION))

FLUTTER_WAYLAND_LICENSE_FILES = LICENSE

FLUTTER_WAYLAND_DEPENDENCIES += \
	flutter-engine waylandpp libxkbcommon rapidjson libegl libgles

FLUTTER_WAYLAND_CONF_OPTS += \
	-DFLUTTER_ENGINE_LIBRARY=${STAGING_DIR}/usr/lib/libflutter_engine.so

define FLUTTER_WAYLAND_INSTALL_TARGET_ENV
	$(INSTALL) -D -m 644 $(FLUTTER_WAYLAND_PKGDIR)/flutter-wayland.sh \
		$(TARGET_DIR)/etc/profile.d/flutter-wayland.sh
endef
FLUTTER_WAYLAND_POST_INSTALL_TARGET_HOOKS += FLUTTER_WAYLAND_INSTALL_TARGET_ENV

$(eval $(cmake-package))
