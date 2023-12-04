################################################################################
#
# linux-serial-test
#
################################################################################

LINUX_SERIAL_TEST_VERSION = master
LINUX_SERIAL_TEST_SITE = https://github.com/cbrake/linux-serial-test
LINUX_SERIAL_TEST_SITE_METHOD = git
LINUX_SERIAL_TEST_LICENSE = MIT
LINUX_SERIAL_TEST_LICENSE_FILES = LICENSE

$(eval $(cmake-package))
