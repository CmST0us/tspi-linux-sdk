#!/bin/bash

BUILDROOT_DIR=`pwd`
SDK_DIR=`dirname $BUILDROOT_DIR`

echo "==========build oem to rootfs=========="
echo "$BUILDROOT_DIR"
echo "$SDK_DIR"
echo "$RK_TARGET_PRODUCT"
echo "$RK_CFG_RAMBOOT"

rm -rf $SDK_DIR/buildroot/output/$RK_CFG_RAMBOOT/target/oem/
cp -rf $SDK_DIR/buildroot/output/$RK_CFG_RAMBOOT/oem/. $SDK_DIR/buildroot/output/$RK_CFG_RAMBOOT/target/oem/

exit 0
