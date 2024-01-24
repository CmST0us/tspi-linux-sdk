#!/bin/bash

echo =====================================
echo SPI test
echo =====================================

cd /dev

for spidev in `ls -d spi*`; do

    cat /sys/class/spidev/${spidev}/device/uevent | grep "OF_NAME"
    /rp_test/spi/spidev_test -D $spidev -l -v -p wwww.rpdzkj.com
done

