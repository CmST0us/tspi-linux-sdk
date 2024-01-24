#!/bin/bash

echo ===========================
echo SATA test
echo ===========================

cd /sys/class/scsi_device/
tmp_size=0
dd_output=""
sata_maxpart=""

for sata_device in `ls -d *`; do
    sata_name=`cat $sata_device/device/model`
    echo "Found SATA device $sata_name"
    
    blk_size=`cat $sata_device/device/block/sd*/queue/physical_block_size`
    sector=`cat $sata_device/device/block/sd*/size`

    size=$((blk_size * sector))    

    echo -e "\t SATA availabel size: $((size / 1024 / 1024 / 1024))G"

    cd $sata_device/device/block/sd*/    

    echo -e "\t SATA parts:"

    for part in `ls -d sd*`; do
        part_sector=`cat $part/size`
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

        mount_point=`df | grep "/dev/$part" -w | awk '{print $NF}'`

        if [ "$mount_point" == "" ]; then
            mount_point="/mnt/sata/$part"
            [ ! -d "$mount_point" ] && mkdir -p "$mount_point"

            #disable kernel log
            dmesg -n 1
            mount /dev/$part "$mount_point" > /dev/null 2>&1
            if [ $? -ne 0 ]; then
                mount_point="not mounted, maybe file system of part not support to mount"
            fi
 
            #enable kernel log
            dmesg -n 8
        fi

        if [[ $part_size -gt $tmp_size ]];then
            sata_maxpart=$part
            tmp_size=$part_size
            dd_output="$mount_point"
        fi

        printf "\t\t%-7s %-10s %s %s\n" "$part:" "$part_size_str" "mount point:" "$mount_point"
    done
    
    echo ==============================================
    echo SATA device write/read 1G data rate                              
    echo ==============================================               
    echo Write rate:                                                    
    cmd="dd if=/dev/zero of="${dd_output}/test.out" bs=512k count=2000"        
    echo $cmd                                                           
    $cmd                                                          
                                                              
    echo Clean cache for test read                    
    dmesg -n 1
    sync && echo 3 | tee /proc/sys/vm/drop_caches                                                        
    dmesg -n 8
                                                                                                     
    echo Read rate:                                                                                      
    cmd="dd if="${dd_output}/test.out" of=/dev/zero bs=512k count=2000"        
    echo $cmd                                       
    $cmd    

    rm -rf "${dd_output}/test.out"

done
