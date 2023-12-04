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
if [ -z $RK_CFG_TOOLCHAIN ]
then
        echo "RK_CFG_TOOLCHAIN is empty, skip building buildroot toolchain!"
        exit 0
fi
source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_TOOLCHAIN
$TOP_DIR/buildroot/utils/brmake toolchain gdb sdk -j8
if [ $? -ne 0 ]; then
    exit 1
fi
echo "toolchain located at: $TOP_DIR/buildroot/output/$RK_CFG_TOOLCHAIN/host"
