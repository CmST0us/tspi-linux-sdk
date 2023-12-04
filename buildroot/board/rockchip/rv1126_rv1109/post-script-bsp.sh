#!/bin/bash

BUILDROOT_DIR=`pwd`
SDK_DIR=`dirname $BUILDROOT_DIR`

export LIBDIR=$TARGET_DIR/usr/lib/
export HEADER_DIR=$STAGING_DIR/usr/include
export BSPDIR=$TARGET_DIR/../BSP
export BINDIR=$TARGET_DIR/usr/bin/

echo "$BUILDROOT_DIR"
echo "$SDK_DIR"
echo "$LIBDIR"
echo "$BINDIR"

rm -rf $BSPDIR
mkdir $BSPDIR
mkdir -p $BSPDIR/lib
mkdir -p $BSPDIR/include
mkdir -p $BSPDIR/example
mkdir -p $BSPDIR/npu/ko
mkdir -p $BSPDIR/npu/lib
mkdir -p $BSPDIR/npu/include

cd $BSPDIR
# copy libs
cp $LIBDIR/libdrm.* ./lib/ -dv
cp $LIBDIR/libv4l2.* ./lib/ -dv
cp $LIBDIR/libv4lconvert.* ./lib -dv

cp $LIBDIR/librga.* ./lib/ -dv
cp $LIBDIR/librockchip_mpp.* ./lib/ -dv

cp $LIBDIR/libasound.* ./lib/ -dv
cp $LIBDIR/libthird_media.* ./lib/ -dv

cp $LIBDIR/libRKAP* ./lib/ -dv
cp $LIBDIR/libmd_share.so ./lib/ -dv
cp $LIBDIR/libod_share.so ./lib/ -dv

cp $LIBDIR/librkaiq.* ./lib/ -dv
cp $LIBDIR/libeasymedia.* ./lib/ -dv

#copy headers
cp $HEADER_DIR/rga ./include/ -vrf
#mkdir ./include/rkaiq
#cp $HEADER_DIR/rkaiq/uAPI ./include/rkaiq/ -vrf
cp $HEADER_DIR/rkaiq ./include/ -rf
cp $SDK_DIR/external/rkmedia/include/rkmedia ./include/ -vrf

#copy examples
cp $SDK_DIR/external/rkmedia/examples/* ./example/ -vrf

#copy iqfiles to examples
cp $SDK_DIR/external/camera_engine_rkaiq/iqfiles ./example -vrf
#copy vqefiles to examples
mkdir -p ./example/vqefiles
cp $SDK_DIR/external/common_algorithm/audio/rkap_3a/para* ./example/vqefiles -vrf

########################### npu ##############################

#rknpu ko
cp $TARGET_DIR/etc/init.d/S60NPU_init ./npu/ko/ -v
cp $TARGET_DIR/lib32/modules/galcore.ko ./npu/ko/ -v

# rknpu lib
cp $LIBDIR/cl_viv_vx_ext.h ./npu/lib/ -dv
cp $LIBDIR/libArchModelSw.so ./npu/lib/ -dv
cp $LIBDIR/libGAL.so ./npu/lib/ -dv
cp $LIBDIR/libNN* ./npu/lib/ -dv
cp $LIBDIR/libOpen* ./npu/lib/ -dv
cp $LIBDIR/librknn_runtime.so ./npu/lib/ -dv
cp $LIBDIR/libVSC* ./npu/lib/ -dv

# rockx
cp $LIBDIR/librknn_api.so ./npu/lib/ -dv
cp $LIBDIR/librockx.so ./npu/lib/ -dv
cp $LIBDIR/person_detection_v2.data ./npu/lib/ -dv
cp $LIBDIR/face_detection_v2.data ./npu/lib/ -dv
cp $HEADER_DIR/rockx ./npu/include/ -vrf
#
cd -
