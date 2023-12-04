################################################################################
#
# python-numba
#
################################################################################

PYTHON_NUMBA_VERSION = 0.50.1 # For python-llvmlite 0.33
PYTHON_NUMBA_SITE = $(call github,numba,numba,$(PYTHON_NUMBA_VERSION))
PYTHON_NUMBA_LICENSE = BSD-2-Clause
PYTHON_NUMBA_LICENSE_FILES = LICENSE
PYTHON_NUMBA_SETUP_TYPE = setuptools

PYTHON_NUMBA_DEPENDENCIES += host-python-numpy python-numpy python-llvmlite

# OpenMP not supported in buildroot yet
PYTHON_NUMBA_ENV += NUMBA_NO_OPENMP=1

$(eval $(python-package))
