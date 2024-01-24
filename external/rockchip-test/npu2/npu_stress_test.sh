#!/bin/bash

COMPATIBLE=$(cat /proc/device-tree/compatible)


while true
do
if [[ $COMPATIBLE =~ "rk3588" ]]; then
    rknn_common_test /usr/share/model/RK3588/mobilenet_v1.rknn /usr/share/model/dog_224x224.jpg 10
else
    rknn_common_test /usr/share/model/RK356X/mobilenet_v1.rknn /usr/share/model/dog_224x224.jpg 10
fi
done
