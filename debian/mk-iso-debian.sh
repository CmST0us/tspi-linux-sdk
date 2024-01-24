#! /bin/bash

ISO_FILE=debian-11.4.0-arm64-DVD-1.iso

DIR="$( cd "$( dirname "$0"  )" && pwd  )"
SDK_PATH=$DIR/..
INITRD=$DIR/rd
DEBIAN_DIR=$DIR/iso
KERNEL_NAME=kernel

if [ ! -f "$DIR/$ISO_FILE" ]; then
echo "Download Debian ISO"
wget https://cdimage.debian.org/debian-cd/current/arm64/iso-dvd/$ISO_FILE -P $DIR/
if [ $? -ne 0 ]
then
    echo "please check link: https://cdimage.debian.org/debian-cd/current/arm64/iso-dvd/$ISO_FILE"
	exit
else
	echo "Download sucess"
fi
fi

if [ ! -d "$DEBIAN_DIR" ]; then
echo “decompress ISO”
xorriso -osirrox on -indev $DIR/$ISO_FILE -extract / $DEBIAN_DIR
fi

echo "unpack initrd.gz"
mkdir -p $INITRD
cp $DEBIAN_DIR/install.a64/initrd.gz $INITRD/
cd $INITRD
gunzip initrd.gz --quiet
cpio -di < initrd
rm initrd

echo "install modules into initrd"
cd $SDK_PATH/$KERNEL_NAME
make modules_install INSTALL_MOD_PATH=$INITRD --quiet

echo "pack initrd.gz"
cd $INITRD
cp $SDK_PATH/debian/overlay-debug/usr/local/bin/io $INITRD/bin/
find . | cpio --quiet -o -H newc --owner 0:0 | gzip > $DEBIAN_DIR/install.a64/initrd.gz
cd $SDK_PATH
rm -r $INITRD

if [ ! -d $SDK_PATH/$KERNEL_NAME/debian ]; then
	echo -e "\033[36m Please compile the kernel deb before: ./build.sh kernel \033[0m"
	exit
fi

echo "update kernel deb"
rm -f $DEBIAN_DIR/pool/main/l/linux-signed-arm64/linux-image-5*
rm -f $DEBIAN_DIR/pool/main/l/linux/linux-headers-*
rm -f $DEBIAN_DIR/pool/main/l/linux/linux-libc-*
rm -f $SDK_PATH/$KERNEL_NAME/linux-image-*dbg*.deb
cp $SDK_PATH/$KERNEL_NAME/linux-image-*.deb $DEBIAN_DIR/pool/main/l/linux-signed-arm64/
cp $SDK_PATH/$KERNEL_NAME/linux-headers-*.deb $DEBIAN_DIR/pool/main/l/linux/
cp $SDK_PATH/$KERNEL_NAME/linux-libc-*.deb $DEBIAN_DIR/pool/main/l/linux/
cp $SDK_PATH/$KERNEL_NAME/arch/arm64/boot/Image $DEBIAN_DIR/install.a64/vmlinuz

KERNEL_VERSION=$(cat $SDK_PATH/$KERNEL_NAME/include/config/kernel.release)

if [ "$KERNEL_VERSION" == "5.4.18" ]; then
	echo "insert cmdline into grub.cfg"
	sed -i 's/vmlinuz/vmlinuz earlycon=uart8250,mmio32,0xfeb50000 console=ttyS2,1500000n8/g' $DEBIAN_DIR/boot/grub/grub.cfg
fi

echo "modify package Depends $KERNEL_VERSION kernel"
for file in $( find $DEBIAN_DIR/pool/main/l/linux-signed-arm64/ -type f -name '*deb' | sort )
do
rm -rf tmp
dpkg-deb -I $file | grep "Kernel-Version: " &&
dpkg-deb -R $file tmp &&
sed -i '/^Kernel-Version: /c\Kernel-Version: '"$KERNEL_VERSION"'' tmp/DEBIAN/control &&
dpkg-deb -b tmp $file &&
rm -rf tmp

dpkg-deb -I $file | grep "Depends: linux-image-" &&
dpkg-deb -R $file tmp &&
sed -i '/^Depends: linux-image-/c\Depends: linux-image-'"$KERNEL_VERSION"'' tmp/DEBIAN/control &&
dpkg-deb -b tmp $file &&
rm -rf tmp
done
rm -rf tmp

# ------------------rkwifibt------------
echo -e "\033[36m Install rkwifibt.................... \033[0m"
mkdir -p $DEBIAN_DIR/pool/main/rockchip/
cp $SDK_PATH/debian/packages/arm64/rkwifibt/*.deb $DEBIAN_DIR/pool/main/rockchip/

echo "update Release info"
cd $DEBIAN_DIR/
cat << EOF >> deb.conf
Dir {
  ArchiveDir ".";
  OverrideDir ".";
  CacheDir ".";
};

TreeDefault {
  Directory "pool/";
};

BinDirectory "pool/main" {
  Packages "dists/bullseye/main/binary-arm64/Packages";
};

Default {
  Packages {
  Extensions ".deb";
  Compress ". gzip";
  };
};

Contents {
  Compress "gzip";
};
EOF
apt-ftparchive generate deb.conf
rm deb.conf

cat << EOF >> udeb.conf
Dir {
  ArchiveDir ".";
  OverrideDir ".";
  CacheDir ".";
};

TreeDefault {
  Directory "pool/";
};

BinDirectory "pool/main" {
  Packages "dists/bullseye/main/debian-installer/binary-arm64/Packages";
};

Default {
  Packages {
  Extensions ".udeb";
  Compress ". gzip";
  };
};

Contents {
  Compress "gzip";
};
EOF
apt-ftparchive generate udeb.conf
rm udeb.conf

cat << EOF >> release.conf
APT::FTPArchive::Release::Codename "bullseye";
APT::FTPArchive::Release::Origin "Debian";
APT::FTPArchive::Release::Components "main";
APT::FTPArchive::Release::Label "Debian";
APT::FTPArchive::Release::Architectures "arm64";
APT::FTPArchive::Release::Suite "bullseye";
EOF

apt-ftparchive -c release.conf release dists/bullseye > ../Release
rm release.conf
mv ../Release dists/bullseye/Release
rm dists/bullseye/main/binary-arm64/Packages
rm dists/bullseye/main/debian-installer/binary-arm64/Packages

find -follow -type f ! -name md5sum.txt -print0 | xargs -0 md5sum > md5sum.txt

echo "repack ISO"
xorriso -as mkisofs -r  -V 'Debian 11 ARM64' -o $SDK_PATH/rockdev/rk3588-$ISO_FILE -J -joliet-long -cache-inodes -e /boot/grub/efi.img -no-emul-boot -append_partition 2 0xef $DEBIAN_DIR/boot/grub/efi.img -partition_cyl_align all $DEBIAN_DIR
