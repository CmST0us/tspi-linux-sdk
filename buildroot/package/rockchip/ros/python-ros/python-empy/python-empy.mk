################################################################################
#
# python-empy
#
################################################################################

PYTHON_EMPY_VERSION = 3.3.3
PYTHON_EMPY_SITE = http://www.alcyone.com/software/empy
PYTHON_EMPY_SOURCE = empy-$(PYTHON_EMPY_VERSION).tar.gz
PYTHON_EMPY_LICENSE = LGPL-3.0
PYTHON_EMPY_LICENSE_FILES = LICENSE.txt
PYTHON_EMPY_SETUP_TYPE = distutils

$(eval $(host-python-package))
