#!/bin/bash

set -e

SCRIPT_DIR=$(dirname $(realpath $BASH_SOURCE))
TOP_DIR=$(realpath $SCRIPT_DIR/../../../../..)
cd $TOP_DIR

source $TOP_DIR/device/rockchip/.BoardConfig.mk
BUILD_DIR=build
TOOLS_OUT_DIR=$TOP_DIR/device/rockchip/rk1808/rk1808_compute_stick_tool/rknputools/$BUILD_DIR
LIB_OUT_DIR=$TOOLS_OUT_DIR/lib64

RKNPUTOOLS_DIR=$TOP_DIR/external/RKNPUTools
RKNN_API_DIR=$RKNPUTOOLS_DIR/rknn-api/$RK_RKNN_API_PLT
RKNN_SDK_DIR=$RKNN_API_DIR/rknn_api_sdk
RKNN_MOBILENET=$RKNN_SDK_DIR/$BUILD_DIR/rknn_mobilenet
RKNN_SSD=$RKNN_SDK_DIR/$BUILD_DIR/rknn_ssd
NPU_TRANSFER_PROXY=$RKNPUTOOLS_DIR/npu_transfer_proxy/$RK_NPU_TRANSFER_PROXY_ARCH/npu_transfer_proxy


if [ -d "$TOOLS_OUT_DIR" ]; then
    echo "delete dir $BUILD_DIR"
    rm -rf $TOOLS_OUT_DIR
fi
mkdir -p $LIB_OUT_DIR

# Require buildroot host tools to do image packing.
if [ ! -d "$TARGET_OUTPUT_DIR" ]; then
    echo "Source buildroot/build/envsetup.sh"
    source $TOP_DIR/buildroot/build/envsetup.sh $RK_CFG_BUILDROOT
fi

if [ -d $RKNN_SDK_DIR ]; then
    cd $RKNN_SDK_DIR
    if [ -d $BUILD_DIR ]; then
        rm -rf $BUILD_DIR
    fi
    mkdir $BUILD_DIR; cd $BUILD_DIR

    echo -n "compile $RK_RKNN_API_ARCH rknn api..."
    if [ "$RK_RKNN_API_ARCH" == "arm" ]
    then
        cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ ..
        make
    else
        cmake ..; make
    fi

    echo "copy $RK_RKNN_API_PLT images and resource to $BUILD_DIR"
    if [ -f $RKNN_MOBILENET ]
    then
        echo -n "copy rknn_mobilenet..."
        cp $RKNN_MOBILENET $TOOLS_OUT_DIR/rknn_mobilenet
        echo "done."
    else
        echo "warning: $RKNN_MOBILENET not found!"
    fi

    if [ -f $RKNN_SSD ]
    then
        echo -n "copy rknn_ssd..."
        cp $RKNN_SSD $TOOLS_OUT_DIR/rknn_ssd
        echo "done."
    else
        echo "warning: $RKNN_SSD not found!"
    fi
fi

if [ -f $NPU_TRANSFER_PROXY ]
then
    echo -n "copy $RK_NPU_TRANSFER_PROXY_ARCH npu_transfer_proxy..."
    cp $NPU_TRANSFER_PROXY $TOOLS_OUT_DIR/npu_transfer_proxy
    echo "done."
else
    echo "warning: $NPU_TRANSFER_PROXY not found!"
fi

if [ -d $RKNN_API_DIR ]
then
    echo -n "copy libs and resource..."
    cp $RKNN_API_DIR/tmp/* $TOOLS_OUT_DIR/
    cp $RKNN_SDK_DIR/rknn_api/$RK_RKNN_API_ARCH/lib64/* $LIB_OUT_DIR/
    cp $RKNN_SDK_DIR/3rdparty/opencv/$RK_RKNN_API_ARCH/lib64/* $LIB_OUT_DIR/
    echo "done."
else
    echo -e "\e[31m error: $RKNN_API_DIR not found! \e[0m"
fi
