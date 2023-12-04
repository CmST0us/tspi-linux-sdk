################################################################################
#
# python-pychromecast
#
################################################################################

PYTHON_PYCHROMECAST_VERSION = 4.0.1
PYTHON_PYCHROMECAST_SOURCE = PyChromecast-$(PYTHON_PYCHROMECAST_VERSION).tar.gz
PYTHON_PYCHROMECAST_SITE = https://files.pythonhosted.org/packages/f6/ce/9869189d7d8b1f35a6e13d4651d56a8d8155d28f289527dcfcf678af71f4
PYTHON_PYCHROMECAST_SETUP_TYPE = setuptools
PYTHON_PYCHROMECAST_LICENSE = MIT
PYTHON_PYCHROMECAST_LICENSE_FILES = LICENSE

$(eval $(python-package))
