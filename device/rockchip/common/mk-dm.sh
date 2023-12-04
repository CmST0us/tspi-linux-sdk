#!/bin/bash

set -ex

MODE=$1
INPUT=`readlink -f $2`

OUTPUT=`dirname $INPUT`
COMMON_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        COMMON_DIR=$(dirname $CMD)
fi
cd $COMMON_DIR
cd ../../..
TOP_DIR=$(pwd)

BOARD_CONFIG=$TOP_DIR/device/rockchip/.BoardConfig.mk
source $BOARD_CONFIG

TEMPDIR=${OUTPUT}/tempfile
if [ "$MODE" = "DM-E" ]; then
	ROOTFS=${OUTPUT}/enc.img
	cipher=aes-cbc-plain
	key=`cat u-boot/keys/system_enc_key`
else
	ROOTFS=${OUTPUT}/dmv.img
fi
ROOT_HASH=${TEMPDIR}/root.hash
ROOT_HASH_OFFSET=${TEMPDIR}/root.offset
INIT_FILE=${TOP_DIR}/buildroot/board/rockchip/common/security-ramdisk-overlay/init
ROOTFS_INFO=`ls -l ${INPUT}`

PACK=TRUE
if [ -e ${OUTPUT}/rootfs.info ]; then
	if [ "`cat ${OUTPUT}/rootfs.info`" = "`ls -l ${INPUT}`" ]; then
		PACK=FALSE
	else
		echo "`ls -l $INPUT`" > ${OUTPUT}/rootfs.info
	fi
else
	echo "`ls -l $INPUT`" > ${OUTPUT}/rootfs.info
fi

function pack_dmv() {
	cp ${INPUT} ${ROOTFS}
	HASH_OFFSET=$[(ROOTFS_SIZE / 1024 / 1024 + 2) * 1024 * 1024]
	tempfile=`mktemp /tmp/temp.XXXXXX`
	veritysetup --hash-offset=${HASH_OFFSET} format ${ROOTFS} ${ROOTFS} > ${tempfile}
	cat ${tempfile} | grep "Root hash" | awk '{printf $3}' > ${ROOT_HASH}

	cp ${tempfile} ${TEMPDIR}/tempfile
	rm ${tempfile}
	echo ${HASH_OFFSET} > ${ROOT_HASH_OFFSET}
}

function pack_dme() {
	sectors=`ls -l ${INPUT} | awk '{printf $5}'`
	sectors=$[(sectors + (21 * 1024 * 1024) - 1) / 512] # remain 20M for partition info / unit: 512 bytes

	loopdevice=`losetup -f`
	mappername=encfs-$(shuf -i 1-10000000000000000000 -n 1)
	dd if=/dev/null of=${ROOTFS} seek=${sectors} bs=512
	sudo -S losetup ${loopdevice} ${ROOTFS} < u-boot/keys/root_passwd
	sudo -S dmsetup create $mappername --table "0 $sectors crypt $cipher $key 0 $loopdevice 0 1 allow_discards" < u-boot/keys/root_passwd
	sudo -S dd if=${INPUT} of=/dev/mapper/${mappername} conv=fsync < u-boot/keys/root_passwd
	sync && sudo -S dmsetup remove ${mappername} < u-boot/keys/root_passwd
	sudo -S losetup -d ${loopdevice} < u-boot/keys/root_passwd

	rm ${TEMPDIR}/enc.info || true
	echo "sectors=${sectors}" > ${TEMPDIR}/enc.info
	echo "cipher=${cipher}" >> ${TEMPDIR}/enc.info
	echo "key=${key}" >> ${TEMPDIR}/enc.info
}

if [ "$PACK" = "TRUE" ]; then
	test -d ${TEMPDIR} || mkdir -p ${TEMPDIR}
	ROOTFS_SIZE=`ls ${INPUT} -l | awk '{printf $5}'`

	if [ "$MODE" = "DM-V" ]; then
		pack_dmv
	elif [ "$MODE" = "DM-E" ]; then
		pack_dme
	fi

	ln -rsf ${ROOTFS} ${OUTPUT}/security-system.img
fi

cp ${TOP_DIR}/buildroot/board/rockchip/common/security-ramdisk-overlay/init.in ${INIT_FILE}

if [ "$MODE" = "DM-V" ]; then
	TMP_HASH=`cat ${ROOT_HASH}`
	TMP_OFFSET=`cat ${ROOT_HASH_OFFSET}`
	sed -i "s/OFFSET=/OFFSET=${TMP_OFFSET}/" ${INIT_FILE}
	sed -i "s/HASH=/HASH=${TMP_HASH}/" ${INIT_FILE}
	sed -i "s/ENC_EN=/ENC_EN=false/" ${INIT_FILE}
elif [ "$MODE" = "DM-E" ]; then
	source ${TEMPDIR}/enc.info

	sed -i "s/ENC_EN=/ENC_EN=true/" ${INIT_FILE}
	sed -i "s/CIPHER=/CIPHER=${cipher}/" ${INIT_FILE}

	echo "Generate misc with key"
	${COMMON_DIR}/mk-misc.sh ${COMMON_DIR}/../rockimg/${RK_MISC} ${COMMON_DIR}/../rockimg/misc.img 64 $(cat ${TOP_DIR}/u-boot/keys/system_enc_key)
fi

sed -i "s/# exec busybox switch_root/exec busybox switch_root/" ${INIT_FILE}
