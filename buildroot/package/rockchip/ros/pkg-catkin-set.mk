################################################################################
# ROS catkin packages set infrastructure
################################################################################

define inner-catkin-packages-set

$(eval $(shell sed -n '/VERSION =/p' $(5)))
$(eval $(shell sed -n '/SITE =/p' $(5)))
$(eval $(shell sed -n '/SOURCE =/p' $(5)))

$(2)_VERSION = $(VERSION)
$(2)_SITE = $(SITE)
$(2)_SOURCE = $(SOURCE)

$(call inner-catkin-package,$(1),$(2),$(3),$(4))
endef

catkin-packages-set = $(call inner-catkin-packages-set,$(pkgname),$(call UPPERCASE,$(pkgname)),$(call UPPERCASE,$(pkgname)),target,$(pkgdir)/../version.in.kinetic)
ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
catkin-packages-set = $(call inner-catkin-packages-set,$(pkgname),$(call UPPERCASE,$(pkgname)),$(call UPPERCASE,$(pkgname)),target,$(pkgdir)/../version.in.indigo)
endif
