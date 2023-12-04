TENSORFLOW_SITE = $(TOPDIR)/../external/tensorflow
TENSORFLOW_VERSION = master
TENSORFLOW_SITE_METHOD = local

TENSORFLOW_LICENSE_FILES = COPYING
TENSORFLOW_LICENSE = GPLv2.1+

define TENSORFLOW_DOWNLOAD_DEPENDENCIES
	cd $(@D) && ./tensorflow/contrib/lite/download_dependencies.sh
endef
TENSORFLOW_POST_RSYNC_HOOKS += TENSORFLOW_DOWNLOAD_DEPENDENCIES

define TENSORFLOW_FIX_LOCAL_INCLUDE
	sed -i "/\/usr\/local\//d" $(@D)/tensorflow/contrib/lite/Makefile
endef
TENSORFLOW_POST_RSYNC_HOOKS += TENSORFLOW_FIX_LOCAL_INCLUDE

define TENSORFLOW_BUILD_CMDS
	cd $(@D) && \
	$(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-f $(@D)/tensorflow/contrib/lite/Makefile
endef

define TENSORFLOW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/local/tensorflow
	cp $(BUILD_DIR)/tensorflow-$(TENSORFLOW_VERSION)/demo_test/usr/lib/libneuralnetworks.so $(TARGET_DIR)/usr/lib/
	cp -r $(BUILD_DIR)/tensorflow-$(TENSORFLOW_VERSION)/demo_test/usr/lib/nn_drivers $(TARGET_DIR)/usr/lib/
	cp $(BUILD_DIR)/tensorflow-$(TENSORFLOW_VERSION)/demo_test/tmp/* $(TARGET_DIR)/usr/local/tensorflow/
	cp $(BUILD_DIR)/tensorflow-$(TENSORFLOW_VERSION)/tensorflow/contrib/lite/examples/label_image/3rdparty/opencv/lib/* $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 755 $(@D)/tensorflow/contrib/lite/gen/bin/label_image $(TARGET_DIR)/usr/local/tensorflow
endef

$(eval $(generic-package))
