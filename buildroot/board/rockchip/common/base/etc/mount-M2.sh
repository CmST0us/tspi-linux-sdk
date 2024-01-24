#!/bin/sh
if [ ! -d /sys/class/nvme/nvme0 ]; then
    echo "SSD device not found!"
    exit -1
fi

dd_output=""
ssd_maxpart=""
temp_size=0

cd /sys/class/nvme/

# Loop through all nvmeX devices
for nvme_dev in nvme*; do
    echo "Found nvme device $(cat ${nvme_dev}/model)"
    echo -e "\tSSD firmware: $(cat ${nvme_dev}/firmware_rev)"
    echo -e "\tSSD serial: $(cat ${nvme_dev}/serial)"
    cd "$nvme_dev"

    # Loop through all nvmeXnX devices
    for nvme in nvme*; do
        blk_size=$(cat ${nvme}/queue/physical_block_size)
        sector=$(cat ${nvme}/size)
        nvme_size=$((blk_size * sector))

        echo -e "\tSSD available size: $((nvme_size / 1024 / 1024 / 1024))G"
        cd "$nvme"

	# Loop through all nvmeXnXpX partitions
        for nvme_part in ${nvme}p*; do
            echo -e "\tSSD partitions:"

            part_sector=$(cat "$nvme_part/size")
            part_size=$((part_sector * blk_size))

            if [[ $part_size -lt 1024 ]];then
               part_size_str="${part_size}B"
            elif [[ $part_size -lt 1024*1024 ]];then
                part_size_str="$((part_size / 1024))K"
            elif [[ $part_size -lt 1024*1024*1024 ]];then
                part_size_str="$((part_size / 1024 / 1024))M"
            else
                part_size_str="$((part_size / 1024 / 1024 / 1024))G"
            fi

            mount_point=$(df | grep "/dev/$nvme_part" -w | awk '{print $6}')
            if [ "$mount_point" == "" ];then
                dmesg -n 1
                echo -e "\t\t$nvme_part is not mounted, mounting..."
                mount_point=/mnt/nvme/$nvme_part
                [ ! -d $mount_point ] && mkdir -p $mount_point
                mount /dev/$nvme_part $mount_point
                if [ $? -ne 0 ]; then
                    mount_point="not mounted, maybe file system of part not support to mount"
                fi
                dmesg -n 8
            fi

            if [[ $part_size -gt $tmp_size ]];then
                ssd_maxpart=$nvme_part
                tmp_size=$part_size
                dd_output=$mount_point
            fi

            echo -e "\t\t$nvme_part: $((part_size / 1024 / 1024 / 1024))G, mount point: $mount_point"
        done
    done
done
