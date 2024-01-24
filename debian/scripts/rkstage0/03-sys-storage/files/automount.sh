#!/bin/bash

pathtoname() {
    udevadm info -p "/sys/$1" | awk -v FS== '/DEVNAME/ {print $2}'
}

while read -r _ _ event devpath _; do
        if [[ $event == add ]]; then
            devname=$(pathtoname "$devpath")
            udisksctl mount --block-device "$devname" --no-user-interaction
        fi
done < <(stdbuf -o L udevadm monitor --udev -s block)
