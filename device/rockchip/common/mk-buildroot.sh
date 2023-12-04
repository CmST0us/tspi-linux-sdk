#!/bin/bash

COMMON_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        COMMON_DIR=$(dirname $CMD)
fi
cd $COMMON_DIR
cd ../../..
TOP_DIR=$(pwd)
BOARD_CONFIG=$1
source $BOARD_CONFIG
if [ -z $RK_CFG_BUILDROOT ]
then
        echo "RK_CFG_BUILDROOT is empty, skip building buildroot rootfs!"
        exit 0
fi
source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_BUILDROOT
$TOP_DIR/buildroot/utils/brmake
if [ $? -ne 0 ]; then
    echo "log saved on $TOP_DIR/br.log"
    tail -n 100 $TOP_DIR/br.log
    exit 1
fi
echo "log saved on $TOP_DIR/br.log. pack buildroot image at: $TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT/images/rootfs.$RK_ROOTFS_TYPE"
