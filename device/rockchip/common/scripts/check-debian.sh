#!/bin/bash -e

if [ ! -e "/usr/share/live/build/data/debian-cd/$RK_DEBIAN_VERSION" ]; then
	echo -e "\e[35m"
	echo "Your live-build doesn't support $RK_DEBIAN_VERSION"
	echo "Please replace it:"
	echo "sudo apt-get remove live-build"
	echo "git clone https://salsa.debian.org/live-team/live-build.git --depth 1 -b debian/1%20230131"
	echo "cd live-build"
	echo "rm -rf manpages/po/"
	echo "sudo make install -j8"
	echo -e "\e[0m"
	exit 1
fi
