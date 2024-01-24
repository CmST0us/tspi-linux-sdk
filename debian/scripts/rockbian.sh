#!/bin/bash -e

while getopts "c:" flag
do
	case "$flag" in
		clean)
			echo "clean build files"
			;;
		rebuild)
			echo "rebuild"
			REBUILD=1
			;;
		arch)
			echo "set arch"
			ARCH="$OPTARG"
			;;
		mirror)
			echo "set mirror"
			MIRROR="$OPTARG"
			;;
		*)
			;;
	esac
done

SCRIPTS_DIR=$(realpath $(dirname $0))
DEBIAN_DIR=$(realpath $SCRIPTS_DIR/..)
WORK_DIR=$(realpath $DEBIAN_DIR/work)

if [ x$1 = xclean ];then
./scripts/unmount.sh
sudo rm -rf $WORK_DIR/rkstage* build.log $WORK_DIR/export-image/*.img 
exit 0
fi

if [ ! -d $WORK_DIR ]; then
	git clone https://github.com/RPi-Distro/pi-gen.git $WORK_DIR
	cd $WORK_DIR && git checkout f01430c9d8f67a4b9719cc00e74a2079d3834d5d -b work && git am $SCRIPTS_DIR/patches/* && cd $DEBIAN_DIR
	mkdir -p $WORK_DIR/cache
fi

./scripts/unmount.sh
sudo TOP_DIR=$(realpath $DEBIAN_DIR/..) MIRROR=${MIRROR:-"http://mirrors.ustc.edu.cn/debian/"} ARCH=${ARCH:-armhf} $WORK_DIR/build.sh -c $SCRIPTS_DIR/config
./scripts/unmount.sh
