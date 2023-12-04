#!/bin/bash

#/rockchip-test/npu2/aarch64/rknn_common_test /rockchip-test/npu2/model/RK3588/vgg16_max_pool_fp16.rknn /rockchip-test/npu2/model/dog_224x224.jpg 10

COMPATIBLE=$(cat /proc/device-tree/compatible)

if [[ $COMPATIBLE =~ "rk3588" ]]; then
###mipi camera is /dev/video8, can change for tests
rknn_camera -d /dev/video8 -m /rockchip-test/npu2/model/RK3588/vgg16_max_pool_fp16.rknn
else
rknn_camera -d /dev/video1 -m /rockchip-test/npu2/model/RK356X/mobilenet_v1.rknn
fi

