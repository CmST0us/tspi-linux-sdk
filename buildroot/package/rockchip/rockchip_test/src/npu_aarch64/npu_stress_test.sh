#!/bin/bash
MODEL_LIST="
caffe/alexnet
caffe/inception_v1
caffe/inception_v3
caffe/inception_v4
caffe/mobilenet_v1
caffe/mobilenet_v2
caffe/resnet-50
caffe/squeezenet_v1.1
caffe/vgg16
caffe/vgg16_ssd
caffe/yolo_v2_voc
tensorflow/densenet
tensorflow/inception_v1
tensorflow/mobilenet_v1
tensorflow/mobilenet_v2
tensorflow/mobilenetv2_ssd
tflite/mobilenet_v1
tflite/mobilenet_v2
tflite/mobilenetv2_ssd
tflite/ssd_mobilenet_v1
object-detect/faster_rcnn_zf
object-detect/mobilenet_ssd
extra-models/cpm
extra-models/pva_faster_rcnn
extra-models/segent
extra-models/ssd_mobilenet_fpn
720p/inception_v4-720p
720p/inception_v4-tf-720p
720p/vgg16-tf-720p
"
MODEL_LIST=${1:-vgg_16_maxpool}

#default count 8640000, about 48 hours
COUNT=${2:-8640000}

echo "==========COUNT: $COUNT"
export VSI_NN_LOG_LEVEL=0
export VIV_VX_ENABLE_SWTILING_PHASE1=1
export VIV_VX_ENABLE_SWTILING_PHASE2=1
export NN_EXT_DDR_READ_BW_LIMIT=5
export NN_EXT_DDR_WRITE_BW_LIMIT=5
export NN_EXT_DDR_TOTAL_BW_LIMIT=5

# fix if run in other path
cd /rockchip_test/npu

for model in $MODEL_LIST
do
  echo "###"$model"###"
  MODEL_PATH=$model
  RKNN_MODEL=`find $MODEL_PATH -name "*.rknn"`
  IMAGE_FILE=`find $MODEL_PATH -name "*.jpg"`
  PERF=$(./rknn_inference $RKNN_MODEL $IMAGE_FILE $COUNT 2>&1)
  echo $PERF
done
dmesg |grep "GPU[0] hang"
if [ $? != 0 ]; then
  echo "====npu stress test PASS====="
else
  echo "====npu stress test FAIL===="
fi
