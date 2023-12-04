################################################################################
#
# python-llvmlite
#
################################################################################

PYTHON_LLVMLITE_VERSION = v0.33.0 # For LLVM-9.0
PYTHON_LLVMLITE_SITE = $(call github,numba,llvmlite,$(PYTHON_LLVMLITE_VERSION))
PYTHON_LLVMLITE_LICENSE = BSD-2-Clause
PYTHON_LLVMLITE_LICENSE_FILES = LICENSE
PYTHON_LLVMLITE_SETUP_TYPE = setuptools

PYTHON_LLVMLITE_DEPENDENCIES += llvm

PYTHON_LLVMLITE_ENV += \
	LLVM_CONFIG=$(STAGING_DIR)/usr/bin/llvm-config \
	CXX="$(TARGET_CXX)" CXXFLAGS="$(TARGET_CXXFLAGS) -fPIC" \
	LDFLAGS="$(TARGET_LDFLAGS)"

$(eval $(python-package))
