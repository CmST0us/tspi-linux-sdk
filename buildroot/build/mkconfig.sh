#!/bin/bash

BUILD_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        BUILD_DIR=$(dirname $CMD)
fi
cd $BUILD_DIR/../../
TOP_DIR=$(pwd)
cd - > /dev/null
CONIFG_DIR=$TOP_DIR/buildroot/configs
CONFIG=$1
LINE=$(head -n 1 $CONIFG_DIR/$CONFIG)
ROCKCHIP=$(echo "$LINE" | cut -c1-8)
TARGET_DIR=${CONFIG%_defconfig}
mkdir -p $TOP_DIR/buildroot/output/$TARGET_DIR
DST_CONFIG=$TOP_DIR/buildroot/output/$TARGET_DIR/.rockchipconfig
SRC_CONFIG=$TOP_DIR/buildroot/configs/$CONFIG
echo "dst:$DST_CONFIG"
echo "src:$SRC_CONFIG"

function additem()
{
	ITEM=$1
	TARGET_FILE=$2
	if [ ! -f $TARGET_FILE ]
	then
		#echo "$TARGET_FILE doesn't exist, so add item directly"
		echo "$ITEM" >> $TARGET_FILE
		return
		echo "return"
	fi
	LINE_CNT=0
	#echo "check if item $ITEM in file $TARGET_FILE"
	for line in $(cat $TARGET_FILE)
	do
		SRC_BRITEM=${ITEM%=*}
		DST_BRITEM=${line%=*}
		BR2=$(echo "${line}" | cut -c1-3)
		if [ $BR2 != "BR2" ]
		then
		       	#echo "${line} is not valid, skip"	
			continue
		fi
                LINE_CNT=$[LINE_CNT+1]
                #echo "line $LINE_CNT is ${line}"
		#echo "src br item: $SRC_BRITEM, dst br item: $DST_BRITEM"
		if [ $SRC_BRITEM = $DST_BRITEM ]
		then
			#echo "${line} already in $DST_CONFIG, check if it is fs-overlay"
			if [ $SRC_BRITEM = "BR2_ROOTFS_OVERLAY" ]
			then
				#echo "merge $ITEM and ${line} to a new fs-overlay"
				SRC_FSOVERLAY=${ITEM#*\"}
				DST_FSOVERLAY=${line#*=}
				DST_FSOVERLAY=${DST_FSOVERLAY%\"*}
				REPLACE_ITEM="BR2_ROOTFS_OVERLAY=$DST_FSOVERLAY $SRC_FSOVERLAY"
			else
				REPLACE_ITEM=$ITEM
			fi
			#echo "replace line $LINE_CNT ${line} in $TARGET_FILE to $REPLACE_ITEM"
			sed -i "${LINE_CNT}c ${REPLACE_ITEM}" $TARGET_FILE
			return
		fi
	done

	if [ -z $FOUND_ITEM ]
	then
		#echo "$ITEM not found in $TARGET_FILE, add it"
		echo "$ITEM" >> $TARGET_FILE
	fi
}

function addfile()
{
	SRC=$1
	DST=$2
	#echo "add $SRC to $DST"
	for line in $(cat $SRC)
	do
		#echo "add file, add item ${line} from $SRC to $DST"
		additem ${line} $DST
	done
}

if [ $ROCKCHIP = rockchip ]
then
	rm $DST_CONFIG 2>/dev/null
	for line in $(cat $SRC_CONFIG)
	do
		if [ -f $TOP_DIR/buildroot/configs/${line} ]
		then 
			echo "merge $TOP_DIR/buildroot/configs/${line}"
			#cat $TOP_DIR/buildroot/configs/${line} >> $DST_CONFIG
			addfile $TOP_DIR/buildroot/configs/${line} $DST_CONFIG
		else
			echo "merge single item ${line}"
			additem ${line} $DST_CONFIG
		fi
	done
else
	cp $SRC_CONFIG $DST_CONFIG
fi

