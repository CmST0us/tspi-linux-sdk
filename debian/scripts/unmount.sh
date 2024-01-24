#!/bin/bash -e

SCRIPTS_DIR=$(realpath $(dirname $0))
DEBIAN_DIR=$(realpath $SCRIPTS_DIR/..)
WORK_DIR=$(realpath $DEBIAN_DIR/work)

main(){
while mount | grep -q $WORK_DIR; do
	local LOCS
	LOCS=$(mount | grep $WORK_DIR | cut -f 3 -d ' ' | sort -r)
	for loc in $LOCS; do
		sudo umount "$loc"
	done
done
}

main