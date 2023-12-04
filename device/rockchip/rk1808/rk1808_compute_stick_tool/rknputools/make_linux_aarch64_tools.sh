#!/bin/bash

export RK_RKNN_API_PLT=Linux
export RK_RKNN_API_ARCH=arm
export RK_NPU_TRANSFER_PROXY_ARCH=linux-aarch64

set -e

SCRIPT_DIR=$(dirname $(realpath $BASH_SOURCE))
TOP_DIR=$(realpath $SCRIPT_DIR/../../../../..)
cd $TOP_DIR

CURRENT_DIR=$TOP_DIR/device/rockchip/rk1808/rk1808_compute_stick_tool/rknputools
MKTOOLSFILE=$CURRENT_DIR/mkrknputools.sh
RKNN_API_DIR=$TOP_DIR/external/RKNPUTools/rknn-api/$RK_RKNN_API_PLT
NPU_TRANSFER_DIR=$TOP_DIR/external/RKNPUTools/npu_transfer_proxy/$RK_NPU_TRANSFER_PROXY_ARCH

if [ -d "$RKNN_API_DIR" ]
then
    if [ -d "$NPU_TRANSFER_DIR" ]
    then
        if [ -f "$MKTOOLSFILE" ]; then
            $MKTOOLSFILE $RK_RKNN_API_PLT $RK_RKNN_API_ARCH $RK_NPU_TRANSFER_PROXY_ARCH
        fi
    else
        echo "warning: $NPU_TRANSFER_DIR not found!"
    fi
else
    echo "warning: $RKNN_API_DIR not found!"
fi
