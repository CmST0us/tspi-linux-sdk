#!/bin/sh

DDR_DIR=/rockchip_test/ddr

RESULT_DIR=/data/cfg/rockchip_test
RESULT_LOG=${RESULT_DIR}/memtester.log

if [ ! -e "/data/cfg/rockchip_test" ]; then
	echo "no /data/cfg/rockchip_test"
	mkdir -p /data/cfg/rockchip_test
fi

#run memtester test
echo "**********************DDR MEMTESTER TEST****************************"
echo "**********************run: memtester 128M***************************"
echo "**********************DDR MEMTESTER TEST****************************"
memtester 128M 2>&1 | tee $RESULT_LOG &

echo "***DDR MEMTESTER TEST START: you can see the log at $RESULT_LOG*****"
