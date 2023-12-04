#!/bin/sh

CMD=`realpath $0`
CMD_DIR=`dirname $CMD`
BUILDROOT_DIR=`pwd`
SDK_DIR=`dirname $BUILDROOT_DIR`
RV1108_DEVICES_BOARD_DIR=$SDK_DIR/device/rockchip/$RK_TARGET_PRODUCT/overlay-board

echo "======================================================="
echo "SDK_DIR                  = $SDK_DIR"
echo "BUILDROOT_DIR            = $BUILDROOT_DIR"
echo "RV1108_DEVICES_BOARD_DIR = $RV1108_DEVICES_BOARD_DIR"
echo "RK_TARGET_PRODUCT        = $RK_TARGET_PRODUCT"
echo "RK_TARGET_BOARD_VERSION  = $RK_TARGET_BOARD_VERSION"
echo "======================================================="

# Remove unneeded files
echo "======================================================="
echo "Remove unneeded files"
echo "======================================================="
rm $TARGET_DIR/etc/init.d/S01logging > /dev/null 2>&1
rm $TARGET_DIR/etc/init.d/S20urandom > /dev/null 2>&1
rm $TARGET_DIR/etc/init.d/S40network > /dev/null 2>&1
rm $TARGET_DIR/etc/init.d/S41dhcpcd  > /dev/null 2>&1
rm $TARGET_DIR/etc/init.d/S50sshd    > /dev/null 2>&1
rm $TARGET_DIR/etc/init.d/S80dnsmasq > /dev/null 2>&1
rm $TARGET_DIR/usr/bin/sqlite3 > /dev/null 2>&1
TEMP_DELETE_TARGET="ts_calibrate ts_finddev ts_harvest ts_print \
		    ts_print_mt ts_print_raw ts_test ts_test_mt ts_uinput ts_verify"
cd $TARGET_DIR/usr/bin/ && rm $TEMP_DELETE_TARGET > /dev/null 2>&1 && cd -

# copy target form device/rockchicp/rv1108/overlay-board
echo "======================================================="
echo "copy target form device/rockchicp/rv1108/overlay-board"
echo "======================================================="
if [ ! -d  $RV1108_DEVICES_BOARD_DIR ]; then
        echo "$RV1108_DEVICES_BOARD_DIR no exit"
else
	cd $RV1108_DEVICES_BOARD_DIR
	if [ -d  rv1108-$RK_TARGET_BOARD_VERSION ]; then
		echo "copy rv1108-$RK_TARGET_BOARD_VERSION"
		rsync -av --exclude userdata rv1108-$RK_TARGET_BOARD_VERSION/* $TARGET_DIR/
	else
		echo "rv1108-$RK_TARGET_BOARD_VERSION no exit"
	fi
	cd -
fi

#pack root parttion target
$CMD_DIR/pack-root.sh