#!/bin/bash

TOOLS_ARM_DIR=./build
TOOLS_X86_DIR=./build
USR_LIB_DIR=/usr/lib
USR_LIB64_DIR=/usr/lib64

cat /proc/cpuinfo | grep -E "Intel|AMD"
if [ $? -ne 0 ]; then
    echo "use arm version"
    if [ -d "$TOOLS_ARM_DIR" ]; then
        cp $TOOLS_ARM_DIR/* /tmp/
        chmod 777 /tmp/rknn_ssd
        chmod 777 /tmp/rknn_mobilenet
        chmod 777 /tmp/npu_transfer_proxy
        if [ -d "$USR_LIB64_DIR" ]
        then
            sudo cp -dpR $TOOLS_ARM_DIR/lib64/* $USR_LIB64_DIR/
        else
            sudo cp -dpR $TOOLS_ARM_DIR/lib64/* $USR_LIB_DIR/
        fi
    else
        echo "warning: $TOOLS_ARM_DIR does not exist"
    fi
else
    echo "use x86 version"
    if [ -d "$TOOLS_X86_DIR" ]; then
        cp $TOOLS_X86_DIR/* /tmp/
        cp $TOOLS_X86_DIR/lib64/* /tmp/
        chmod 777 /tmp/rknn_ssd
        chmod 777 /tmp/rknn_mobilenet
        chmod 777 /tmp/npu_transfer_proxy
        #export LD_LIBRARY_PATH=/tmp
    else
        echo "warning: $TOOLS_X86_DIR does not exist"
    fi
fi

