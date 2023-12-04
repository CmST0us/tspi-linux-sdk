################################################################################
#
# python-em
#
################################################################################

PYTHON_EM_VERSION = 0.4.0
PYTHON_EM_SITE = https://files.pythonhosted.org/packages/b9/b2/b64a81bec764c8590ee3cce8a9446cebdf1e3f24d5696d2915b2248921d3
PYTHON_EM_SOURCE = em-$(PYTHON_EM_VERSION).tar.gz
PYTHON_EM_LICENSE = LGPL-3.0
PYTHON_EM_LICENSE_FILES = LICENSE.txt
PYTHON_EM_SETUP_TYPE = distutils

$(eval $(host-python-package))
