#!/bin/bash
FLAGS=
# PCIE: ROCKCHIP_PCIE30
# Display Support: ROCKCHIP_VOPEN
case "$1" in
        rk3568 | RK3568 | 3568)
                CHIP=3568;
                ;;
        ACPI)
                CHIP=3588;
                FLAGS="-D ROCKCHIP_ACPIEN"
                ;;
        *)
                CHIP=3588
#                FLAGS="-D ROCKCHIP_PCIE30"
#                FLAGS="-D ROCKCHIP_VOPEN"
                ;;
esac
echo Start to build rk$CHIP UEFI

CMD=`realpath $0`
COMMON_DIR=`dirname $CMD`
TOP_DIR=$(realpath $COMMON_DIR/..)
echo $TOP_DIR
export GCC5_AARCH64_PREFIX=$TOP_DIR/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export WORKSPACE=$COMMON_DIR
export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms:$WORKSPACE/edk2-non-osi
. edk2/edksetup.sh BaseTools &&
make -C edk2/BaseTools &&
build -a AARCH64 -t GCC5 -p edk2-platforms/Platform/Rockchip/RK$CHIP/RK$CHIP.dsc $FLAGS &&
echo "cd u-boot and re-packet uboot.img: ./scripts/fit-repack.sh -f uboot.img -d unpack/" &&
cd ../u-boot &&
mkdir -p unpack &&
./make.sh rk$CHIP CROSS_COMPILE=$TOP_DIR/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu- &&
./make.sh --idblock
cp uboot.img uboot_uefi.img
cp $WORKSPACE/Build/RK$CHIP/DEBUG_GCC5/FV/BL33_AP_UEFI.Fv unpack/uboot &&
./scripts/fit-repack.sh -f uboot_uefi.img -d unpack/
mv uboot_uefi.img $WORKSPACE/uboot_uefi.img
echo "****Build NOR FLASH IMAGE****"
cp $WORKSPACE/Build/RK$CHIP/DEBUG_GCC5/FV/NOR_FLASH_IMAGE.fd $WORKSPACE/RK3588_NOR_FLASH.img
dd if=$WORKSPACE/RK3588_NOR_FLASH.img of=$WORKSPACE/NV_DATA.img bs=1K skip=7936
dd if=$WORKSPACE/rk3588_spi_nor_gpt.img of=$WORKSPACE/RK3588_NOR_FLASH.img
dd if=idblock.bin of=$WORKSPACE/RK3588_NOR_FLASH.img bs=1K seek=32
dd if=idblock.bin of=$WORKSPACE/RK3588_NOR_FLASH.img bs=1K seek=544
dd if=$WORKSPACE/uboot_uefi.img of=$WORKSPACE/RK3588_NOR_FLASH.img bs=1K seek=1024
dd if=$WORKSPACE/NV_DATA.img of=$WORKSPACE/RK3588_NOR_FLASH.img bs=1K seek=7936
echo "FLAGS=:" $FLAGS
echo "*********************************************************************"
echo "** uefi document : https://10.10.10.29/c/rk/internal-docs/+/149726 **"
echo "*********************************************************************"
