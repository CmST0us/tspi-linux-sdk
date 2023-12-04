#!/bin/bash

COMPATIBLE=$(cat /proc/device-tree/compatible)


while true
do
if [[ $COMPATIBLE =~ "rk3588" ]]; then
    /rockchip-test/npu2/aarch64/rknn_common_test /rockchip-test/npu2/model/RK3588/vgg16_max_pool_fp16.rknn /rockchip-test/npu2/model/dog_224x224.jpg 10;
else
    /rockchip-test/npu2/aarch64/rknn_common_test /rockchip-test/npu2/model/RK356X/mobilenet_v1.rknn /rockchip-test/npu2/model/dog_224x224.jpg 10;
fi
done
