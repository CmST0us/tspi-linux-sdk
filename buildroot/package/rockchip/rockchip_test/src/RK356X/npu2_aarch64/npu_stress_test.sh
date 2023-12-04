#!/bin/bash

export LD_LIBRARY_PATH=/rockchip_test/npu2/lib
chmod +x /rockchip_test/npu2/rknn_mobilenet_demo


while true
do   
/rockchip_test/npu2/rknn_mobilenet_demo /rockchip_test/npu2/model/RK356X/mobilenet_v1.rknn /rockchip_test/npu2/model/dog_224x224.jpg;
sleep 1
done
