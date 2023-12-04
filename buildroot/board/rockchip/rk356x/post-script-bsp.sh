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
mkdir -p $BSPDIR/mali/include
mkdir -p $BSPDIR/mali/lib

cd $BSPDIR
# copy libs
cp $LIBDIR/libdrm.* ./lib/ -dv
cp $LIBDIR/libv4l2.* ./lib/ -dv
cp $LIBDIR/libv4lconvert.* ./lib -dv

cp $LIBDIR/librga.* ./lib/ -dv
cp $LIBDIR/librockchip_mpp.* ./lib/ -dv

cp $LIBDIR/libasound.* ./lib/ -dv
cp $LIBDIR/libavformat.* ./lib/ -dv
cp $LIBDIR/libavcodec.* ./lib/ -dv
cp $LIBDIR/libswresample.* ./lib/ -dv
cp $LIBDIR/libavutil.* ./lib/ -dv

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
#cp $SDK_DIR/external/rkmedia/examples/* ./example/ -vrf

#copy iqfiles to examples
#cp $SDK_DIR/external/camera_engine_rkaiq/iqfiles ./example -vrf
#copy vqefiles to examples
#cp $SDK_DIR/external/common_algorithm/audio/rkap_aec/para ./example/vqefiles -vrf

#gpu lib
cp $TARGET_DIR/usr/lib/libEGL.*  ./mali/lib/ -rf
cp $TARGET_DIR/usr/lib/libGLESv1_CM.*  ./mali/lib/ -rf
cp $TARGET_DIR/usr/lib/libGLESv2.*  ./mali/lib/ -rf
cp $TARGET_DIR/usr/lib/libmali.*  ./mali/lib/ -rf
cp $TARGET_DIR/usr/lib/libMali.*  ./mali/lib/ -rf
cp $TARGET_DIR/usr/lib/pkgconfig  ./mali/lib/ -rf

#rknpu ko
cp $TARGET_DIR/etc/init.d/S60NPU_init ./npu/ko/ -v
cp $TARGET_DIR/etc/init.d/S05NPU_init ./npu/ko/ -v
cp $TARGET_DIR/lib32/modules/rknpu.ko ./npu/ko/ -v

# rknpu lib
cp $LIBDIR/librknnrt.so ./npu/lib/ -dv

cd -
