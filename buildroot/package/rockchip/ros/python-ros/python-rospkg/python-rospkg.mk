PYTHON_ROSPKG_VERSION = 1.0.40
PYTHON_ROSPKG_SITE = http://download.ros.org/downloads/rospkg
PYTHON_ROSPKG_SOURCE = rospkg-${PYTHON_ROSPKG_VERSION}.tar.gz
PYTHON_ROSPKG_INSTALL_STAGING = YES
PYTHON_ROSPKG_SETUP_TYPE = setuptools

${eval ${python-package}}
