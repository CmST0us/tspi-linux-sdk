PYTHON_CATKIN_PKG_VERSION = 0.4.5
PYTHON_CATKIN_PKG_SOURCE = catkin_pkg-$(PYTHON_CATKIN_PKG_VERSION).tar.gz
PYTHON_CATKIN_PKG_SITE = https://files.pythonhosted.org/packages/fa/e9/6977c1cd0ae22ca164dfed9007439ad89fc81d759e79286ebd024d48883b
PYTHON_CATKIN_PKG_SETUP_TYPE = setuptools

HOST_PYTHON_CATKIN_PKG_DEPENDENCIES = host-python-pyparsing host-python-dateutil

$(eval $(host-python-package))
$(eval $(python-package))
