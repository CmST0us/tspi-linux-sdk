# unpackage busybox tarball
tar xjf busybox-1.27.2-patch-reboot-arg.tar.bz2

# copy rockchip's busybox defconfig
# busybox_spi_nor_defconfig used for spi nor
# busybox_emmc_defconfig used for eMMC (default)
cp busybox-1.27.2-patch/configs/busybox_defconfig busybox-1.27.2/configs/busybox_defconfig

# change directory to busybox
cd busybox-1.27.2

# config defconfig
make busybox_defconfig

# compile, Notice: the cross compile tool is in the prebuilts directory of SDK
make ARCH=arm install CROSS_COMPILE=~/RV1109-SDK/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf- -j32

cd ..

# unpackage base root filesystem which is pre-built bin, e.g. target-emmc-v1.1.0.tar.bz2
tar xjf target-emmc-v1.1.0.tar.bz2

# copy busybox target bin and libs to target directory (option)
cp busybox-1.27.2/_install/* target/ -rfa

# package root filesystem with squashfs
mksquashfs target rootfs.squashfs -noappend -comp xz

# for spi nor
# package root filesystem with squashfs
# --pad : partition size
mkfs.jffs2 -r target -o rootfs.jffs2 --pad=0x400000 -n

# package root filesystem with ext4, e.g.
tar xjf tools.tar.bz2
./tools/mkfs-ext4/do-mkfs.ext4.sh target rootfs.ext4 64M

# the command of unpackage squashfs filesystem : unsquashfs ./rootfs.squashfs
