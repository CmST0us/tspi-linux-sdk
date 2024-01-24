#! /system/bin/sh

# Function to run the loop
run_loop() {
    DIR=$1
    EXECUTABLE=$2
    MODEL=$3
    IMAGE=$4

    # Check if the directory exists
    if [ -d "$DIR" ]; then
        cd $DIR
        export LD_LIBRARY_PATH=./lib
        while true
        do
            ./$EXECUTABLE $MODEL $IMAGE
        done
        # Return to the original directory
        cd -
    fi
}

# Save the current directory
CURRENT_DIR=$(pwd)

# Run the loops
run_loop "/" "/rockchip-test/npu2/rknn_demo.sh" "/usr/share/model/RK3588/vgg16_max_pool_fp16.rknn"  "/usr/share/model/dog_224x224.jpg"
run_loop "/" "/rockchip-test/npu2/rknn_demo.sh" "/usr/share/model/RK356X/mobilenet_v1.rknn" "/usr/share/model/cat_224x224.jpg"