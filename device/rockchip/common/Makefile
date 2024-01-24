# Makefile for Rockchip Linux SDK
#
# Based on buildroot-2022.08's Makefile
#
# Copyright (C) 1999-2005 by Erik Andersen <andersen@codepoet.org>
# Copyright (C) 2006-2014 by the Buildroot developers <buildroot@uclibc.org>
# Copyright (C) 2014-2020 by the Buildroot developers <buildroot@buildroot.org>
# Copyright (C) 2022 by Jeffy Chen <jeffy.chen@rock-chips.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#--------------------------------------------------------------
# Just run 'make menuconfig', configure stuff, then run 'make'.
# You shouldn't need to mess with anything beyond this point...
#--------------------------------------------------------------

# Delete default rules. We don't use them. This saves a bit of time.
.SUFFIXES:

# we want bash as shell
SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	 else if [ -x /bin/bash ]; then echo /bin/bash; \
	 else echo sh; fi; fi)

# Strip quotes and then whitespaces
qstrip = $(strip $(subst ",,$(1)))
#"))

# Needed for the foreach loops to loop over the list of hooks, so that
# each hook call is properly separated by a newline.
define sep


endef

# absolute path
COMMON_DIR := $(abspath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
DEVICE_DIR := $(abspath $(COMMON_DIR)/..)
SDK_DIR := $(abspath $(DEVICE_DIR)/../..)
CHIP_DIR := $(DEVICE_DIR)/.chip
CONFIG_DIR := $(COMMON_DIR)/configs
CONFIG_CONFIG_IN = Config.in
CONFIG = $(SDK_DIR)/buildroot/support/kconfig
OUTDIR := $(SDK_DIR)/output
BUILD_DIR := $(OUTDIR)/kconf
SCRIPT_TARGETS := $(shell $(SDK_DIR)/build.sh core-usage | grep -o "^[a-z0-9_.-]*")

RK_CONFIG = $(OUTDIR)/.config

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands
ifeq ("$(origin V)", "command line")
	KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
	KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
	Q =
ifndef VERBOSE
	VERBOSE = 1
endif
export VERBOSE
else
	Q = @
endif

# Default to build script's default target
.PHONY: build
build:
	$(Q)$(SDK_DIR)/build.sh

# kconfig uses CONFIG_SHELL
CONFIG_SHELL := $(SHELL)

export SHELL CONFIG_SHELL Q KBUILD_VERBOSE

HOSTCC := gcc
HOSTCXX := g++

export HOSTCC HOSTCXX

# silent mode requested?
QUIET := $(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),-q)

$(BUILD_DIR)/auto.conf: $(RK_CONFIG)
	make -j1 $(EXTRAMAKEARGS) HOSTCC="$(HOSTCC)" HOSTCXX="$(HOSTCXX)" syncconfig

$(BUILD_DIR)/%onf:
	mkdir -p $(@D)/lxdialog
	make CC="$(HOSTCC)" HOSTCC="$(HOSTCC)" \
	    obj=$(@D) -C $(CONFIG) -f Makefile.br $(@F)

TARGET_DEFCONFIG := $(filter %_defconfig,$(MAKECMDGOALS))
ifneq ($(TARGET_DEFCONFIG),)
RK_DEFCONFIG := $(realpath $(CHIP_DIR)/$(TARGET_DEFCONFIG))
else
RK_DEFCONFIG := $(shell [ -e ${RK_CONFIG} ] && \
	grep RK_DEFCONFIG ${RK_CONFIG} | cut -d'=' -f2)
endif
DEFCONFIG := $(call qstrip,$(RK_DEFCONFIG))
DEFCONFIG_NAME := $(notdir $(DEFCONFIG))

# Setup some default values
RK_CHIP_FAMILY := $(notdir $(realpath $(CHIP_DIR)))
RK_CHIP := $(shell echo ${DEFCONFIG_NAME} | \
	cut -d'_' -f2 | grep -E "^(px|rk|rv)")

COMMON_CONFIG_ENV = \
	RK_DEFCONFIG='$(call qstrip,$(RK_DEFCONFIG))' \
	RK_CHIP_FAMILY='$(call qstrip,$(RK_CHIP_FAMILY))' \
	RK_CHIP='$(call qstrip,$(RK_CHIP))' \
	KCONFIG_AUTOCONFIG=$(BUILD_DIR)/auto.conf \
	KCONFIG_AUTOHEADER=$(BUILD_DIR)/autoconf.h \
	KCONFIG_TRISTATE=$(BUILD_DIR)/tristate.config \
	srctree=$(CONFIG_DIR) BR2_CONFIG=$(RK_CONFIG)

menuconfig: $(BUILD_DIR)/mconf
	$(Q)$(COMMON_CONFIG_ENV) $< $(CONFIG_CONFIG_IN)

.PHONY: menuconfig

oldconfig syncconfig olddefconfig: $(BUILD_DIR)/conf
	$(Q)$(COMMON_CONFIG_ENV) $< --$@ $(CONFIG_CONFIG_IN)

.PHONY: oldconfig syncconfig olddefconfig

%_defconfig: $(BUILD_DIR)/conf $(CHIP_DIR)/%_defconfig
	$(Q)$(COMMON_CONFIG_ENV) $< --defconfig=$(CHIP_DIR)/$@ $(CONFIG_CONFIG_IN)

.PHONY: %_defconfig

savedefconfig: $(BUILD_DIR)/conf
	$(Q)$(COMMON_CONFIG_ENV) $< \
		--savedefconfig=$(if $(DEFCONFIG),$(DEFCONFIG),$(CHIP_DIR)/defconfig) \
		$(CONFIG_CONFIG_IN)
	$(Q)sed '/^RK_DEFCONFIG=/d' $(if $(DEFCONFIG),$(DEFCONFIG),$(CHIP_DIR)/defconfig)

.PHONY: savedefconfig

.PHONY: help
help:
	@echo '  menuconfig             - interactive curses-based configurator'
	@echo '  oldconfig              - resolve any unresolved symbols in .config'
	@echo '  syncconfig             - Same as oldconfig, but quietly, additionally update deps'
	@echo '  olddefconfig           - Same as syncconfig but sets new symbols to their default value'
	@echo '  savedefconfig          - Save current config to RK_DEFCONFIG (minimal config)'
	@echo '  clean                  - Cleanup'
	@echo
	@echo 'from build.sh:'
	@$(SDK_DIR)/build.sh core-usage | grep "^[a-z]"

define script_target
$(1):
	$(Q)$(SDK_DIR)/build.sh $$@
endef
$(eval $(foreach t,$(SCRIPT_TARGETS),$(call script_target,$(t))$(sep)))

.PHONY: $(SCRIPT_TARGETS)

.PHONY: clean
clean:
ifeq ($(RK_SESSION),)
	$(Q)$(SDK_DIR)/build.sh cleanall
endif

.PHONY: distclean
distclean: clean
	$(Q)rm -rf $(BUILD_DIR) $(RK_CONFIG)*
