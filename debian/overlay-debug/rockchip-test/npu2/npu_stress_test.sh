#!/bin/bash

COMPATIBLE=$(cat /proc/device-tree/compatible)
BIN=rknn_stress_test
TOTAL_TIME=10

while true
do
if [[ $COMPATIBLE =~ "rk3588" ]]; then
    rknn_common_test /usr/share/model/RK3588/mobilenet_v1.rknn /usr/share/model/dog_224x224.jpg 10
elif [[ $COMPATIBLE =~ "rk3562" ]]; then
    ln -rsf /usr/share/model/RK3562/data/* /data/
    $BIN /usr/share/model/RK3562/cfg/mobilenet_v2_i8.cfg $TOTAL_TIME
    $BIN /usr/share/model/RK3562/cfg/mobilenet_v2_fp16.cfg $TOTAL_TIME
    $BIN /usr/share/model/RK3562/cfg/resnet50_i8.cfg $TOTAL_TIME
    $BIN /usr/share/model/RK3562/cfg/resnet50_fp16.cfg $TOTAL_TIME
    $BIN /usr/share/model/RK3562/cfg/vgg16_max_pool_i8.cfg $TOTAL_TIME
    $BIN /usr/share/model/RK3562/cfg/vgg16_max_pool_fp16.cfg $TOTAL_TIME
else
    rknn_common_test /usr/share/model/RK356X/mobilenet_v1.rknn /usr/share/model/dog_224x224.jpg 10
fi
done
