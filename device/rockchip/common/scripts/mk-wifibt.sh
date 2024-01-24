#!/bin/bash -e

DEFAULT_ROOTFS_DIR=buildroot/output/$RK_BUILDROOT_CFG/target

# Usage: build_wifibt <rootfs dir> [wifi chip] [bt tty dev]
build_wifibt()
{
	check_config RK_KERNEL_CFG RK_WIFIBT_CHIP || return 0

	ROOTFS_DIR="${1:-$DEFAULT_ROOTFS_DIR}"
	WIFI_CHIP="${2:-$RK_WIFIBT_CHIP}"
	BT_TTY_DEV="${3:-$RK_WIFIBT_TTY}"

	if find "$ROOTFS_DIR/lib" -name "ld-linux-armhf.so.*" &>/dev/null; then
		ROOTFS_ARCH=arm
	else
		ROOTFS_ARCH=arm64
	fi

	# Debian would call us in root (sudo) environment
	KMAKE="${SUDO_USER:+sudo -u $SUDO_USER} $KMAKE"

	# Check kernel config
	WIFI_USB=`grep "CONFIG_USB=y" kernel/.config` || true
	WIFI_SDIO=`grep "CONFIG_MMC=y" kernel/.config` || true
	WIFI_PCIE=`grep "CONFIG_PCIE_DW_ROCKCHIP=y" kernel/.config` || true
	WIFI_RFKILL=`grep "CONFIG_RFKILL=y" kernel/.config` || true
	if [ -z "WIFI_SDIO" ]; then
		echo "=== WARNNING CONFIG_MMC not set !!! ==="
	fi
	if [ -z "WIFI_RFKILL" ]; then
		echo "=== WARNNING CONFIG_USB not set !!! ==="
	fi
	if [[ "$WIFI_CHIP" =~ "U" ]];then
		if [ -z "$WIFI_USB" ]; then
			echo "=== WARNNING CONFIG_USB not set so ABORT!!! ==="
			exit 0
		fi
	fi
	echo "kernel config: $WIFI_USB $WIFI_SDIO $WIFI_RFKILL"

	RKWIFIBT=$SDK_DIR/external/rkwifibt

	echo "=========================================="
	echo "          Start building wifi/BT"
	echo "=========================================="

	echo WIFI_CHIP=$WIFI_CHIP
	echo BT_TTY_DEV=$BT_TTY_DEV

	ID=$(stat --format %u "$RKWIFIBT")
	if [ "$ID" -ne 0 ]; then
		if [ "$(id -u)" -eq 0 ]; then
			# Fixing up rkwifibt permissions
			find "$RKWIFIBT" -user 0 -exec chown -h -R $ID:$ID {} \;
		else
			echo -e "\e[36m"
			if find "$RKWIFIBT" -user 0 | grep ""; then
				echo -e "\e[31m"
				echo "$RKWIFIBT is dirty for non-root building!"
				echo "Please clear it:"
				echo "cd $RKWIFIBT"
				echo "git add -f ."
				echo "sudo git reset --hard"
				echo -e "\e[0m"
				exit 1
			fi
			echo -e "\e[0m"
		fi
	fi

	$KMAKE $RK_KERNEL_CFG $RK_KERNEL_CFG_FRAGMENTS

	if [[ "$WIFI_CHIP" =~ "ALL_AP" ]];then
		echo "building bcmdhd sdio"
		$KMAKE M=$RKWIFIBT/drivers/bcmdhd CONFIG_BCMDHD=m \
			CONFIG_BCMDHD_SDIO=y CONFIG_BCMDHD_PCIE=
		if [ -n "$WIFI_PCIE" ]; then
			echo "building bcmdhd pcie"
			$KMAKE M=$RKWIFIBT/drivers/bcmdhd CONFIG_BCMDHD=m \
				CONFIG_BCMDHD_PCIE=y CONFIG_BCMDHD_SDIO=
		fi
		if [ -n "$WIFI_USB" ]; then
			echo "building rtl8188fu usb"
			$KMAKE M=$RKWIFIBT/drivers/rtl8188fu modules
		fi
		echo "building rtl8189fs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8189fs modules
		echo "building rtl8723ds sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8723ds modules
		echo "building rtl8821cs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8821cs modules
		echo "building rtl8822cs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8822cs modules
		echo "building rtl8852bs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8852bs modules \
			DRV_PATH=$RKWIFIBT/drivers/rtl8852bs
		if [ -n "$WIFI_PCIE" ]; then
			echo "building rtl8852be pcie"
			$KMAKE M=$RKWIFIBT/drivers/rtl8852be modules \
				DRV_PATH=$RKWIFIBT/drivers/rtl8852be
		fi
	fi

	if [[ "$WIFI_CHIP" =~ "ALL_CY" ]];then
		echo "building CYW4354"
		ln -sf chips/CYW4354_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
		echo "building CYW4373"
		ln -sf chips/CYW4373_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
		echo "building CYW43438"
		ln -sf chips/CYW43438_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
		echo "building CYW43455"
		ln -sf chips/CYW43455_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
		echo "building CYW5557X"
		ln -sf chips/CYW5557X_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
		if [ -n "$WIFI_PCIE" ]; then
			echo "building CYW5557X_PCIE"
			ln -sf chips/CYW5557X_PCIE_Makefile \
				$RKWIFIBT/drivers/infineon/Makefile
			$KMAKE M=$RKWIFIBT/drivers/infineon
			echo "building CYW54591_PCIE"
			ln -sf chips/CYW54591_PCIE_Makefile \
				$RKWIFIBT/drivers/infineon/Makefile
			$KMAKE M=$RKWIFIBT/drivers/infineon
		fi
		echo "building CYW54591"
		ln -sf chips/CYW54591_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon

		if [ -n "$WIFI_USB" ]; then
			echo "building rtl8188fu usb"
			$KMAKE M=$RKWIFIBT/drivers/rtl8188fu modules
		fi
		echo "building rtl8189fs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8189fs modules
		echo "building rtl8723ds sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8723ds modules
		echo "building rtl8821cs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8821cs modules
		echo "building rtl8822cs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8822cs modules
		echo "building rtl8852bs sdio"
		$KMAKE M=$RKWIFIBT/drivers/rtl8852bs modules \
			DRV_PATH=$RKWIFIBT/drivers/rtl8852bs
		if [ -n "$WIFI_PCIE" ]; then
			echo "building rtl8852be pcie"
			$KMAKE M=$RKWIFIBT/drivers/rtl8852be modules \
				DRV_PATH=$RKWIFIBT/drivers/rtl8852be
		fi
	fi

	if [[ "$WIFI_CHIP" =~ "AP6" ]];then
		if [[ "$WIFI_CHIP" = "AP6275_PCIE" ]];then
			echo "building bcmdhd pcie driver"
			$KMAKE M=$RKWIFIBT/drivers/bcmdhd CONFIG_BCMDHD=m \
				CONFIG_BCMDHD_PCIE=y CONFIG_BCMDHD_SDIO=
		else
			echo "building bcmdhd sdio driver"
			$KMAKE M=$RKWIFIBT/drivers/bcmdhd CONFIG_BCMDHD=m \
				CONFIG_BCMDHD_SDIO=y CONFIG_BCMDHD_PCIE=
		fi
	fi

	if [[ "$WIFI_CHIP" = "CYW4354" ]];then
		echo "building CYW4354"
		ln -sf chips/CYW4354_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW4373" ]];then
		echo "building CYW4373"
		ln -sf chips/CYW4373_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW43438" ]];then
		echo "building CYW43438"
		ln -sf chips/CYW43438_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW43455" ]];then
		echo "building CYW43455"
		ln -sf chips/CYW43455_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW5557X" ]];then
		echo "building CYW5557X"
		ln -sf chips/CYW5557X_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW5557X_PCIE" ]];then
		echo "building CYW5557X_PCIE"
		ln -sf chips/CYW5557X_PCIE_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW54591" ]];then
		echo "building CYW54591"
		ln -sf chips/CYW54591_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "CYW54591_PCIE" ]];then
		echo "building CYW54591_PCIE"
		ln -sf chips/CYW54591_PCIE_Makefile \
			$RKWIFIBT/drivers/infineon/Makefile
		$KMAKE M=$RKWIFIBT/drivers/infineon
	fi

	if [[ "$WIFI_CHIP" = "RTL8188FU" ]];then
		echo "building rtl8188fu driver"
		$KMAKE M=$RKWIFIBT/drivers/rtl8188fu modules
	fi

	if [[ "$WIFI_CHIP" = "RTL8189FS" ]];then
		echo "building rtl8189fs driver"
		$KMAKE M=$RKWIFIBT/drivers/rtl8189fs modules
	fi

	if [[ "$WIFI_CHIP" = "RTL8723DS" ]];then
		$KMAKE M=$RKWIFIBT/drivers/rtl8723ds modules
	fi

	if [[ "$WIFI_CHIP" = "RTL8821CS" ]];then
		$KMAKE M=$RKWIFIBT/drivers/rtl8821cs modules
	fi

	if [[ "$WIFI_CHIP" = "RTL8822CS" ]];then
		$KMAKE M=$RKWIFIBT/drivers/rtl8822cs modules
	fi

	if [[ "$WIFI_CHIP" = "RTL8852BS" ]];then
		$KMAKE M=$RKWIFIBT/drivers/rtl8852bs modules
	fi

	if [[ "$WIFI_CHIP" = "RTL8852BE" ]];then
		$KMAKE M=$RKWIFIBT/drivers/rtl8852be modules
	fi

	echo "building realtek bt drivers"
	$KMAKE M=$RKWIFIBT/drivers/bluetooth_uart_driver
	if [ -n "$WIFI_USB" ]; then
		$KMAKE M=$RKWIFIBT/drivers/bluetooth_usb_driver
	fi

	mkdir -p $ROOTFS_DIR/etc/init.d
	mkdir -p $ROOTFS_DIR/usr/bin/
	mkdir -p $ROOTFS_DIR/usr/lib/modules/
	mkdir -p $ROOTFS_DIR/lib/firmware/rtlbt/
	mkdir -p $ROOTFS_DIR/system/lib/modules/
	mkdir -p $ROOTFS_DIR/system/etc/firmware/

	echo "create link system->vendor"
	rm -rf $ROOTFS_DIR/vendor
	ln -sf system $ROOTFS_DIR/vendor

	echo "copy prebuilt tools/sh to rootfs"
	cp $RKWIFIBT/bin/$ROOTFS_ARCH/* $ROOTFS_DIR/usr/bin/
	cp $RKWIFIBT/sh/wifi_start.sh $ROOTFS_DIR/usr/bin/
	cp $RKWIFIBT/sh/wifi_ap6xxx_rftest.sh $ROOTFS_DIR/usr/bin/
	cp $RKWIFIBT/conf/wpa_supplicant.conf $ROOTFS_DIR/etc/
	cp $RKWIFIBT/conf/dnsmasq.conf $ROOTFS_DIR/etc/

	if [[ "$WIFI_CHIP" = "ALL_CY" ]];then
		echo "copy infineon/realtek firmware/nvram to rootfs"
		cp $RKWIFIBT/drivers/infineon/*.ko \
			$ROOTFS_DIR/system/lib/modules/ || true
		cp $RKWIFIBT/firmware/infineon/*/* \
			$ROOTFS_DIR/system/etc/firmware/ || true

		cp $RKWIFIBT/sh/bt_load_broadcom_firmware $ROOTFS_DIR/usr/bin/
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_init.sh
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_pcba_test

		#reatek
		cp $RKWIFIBT/firmware/realtek/*/* $ROOTFS_DIR/lib/firmware/
		cp $RKWIFIBT/firmware/realtek/*/* \
			$ROOTFS_DIR/lib/firmware/rtlbt/
		cp $RKWIFIBT/drivers/bluetooth_uart_driver/hci_uart.ko \
			$ROOTFS_DIR/usr/lib/modules/
		if [ -n "$WIFI_USB" ]; then
			cp $RKWIFIBT/drivers/bluetooth_usb_driver/rtk_btusb.ko \
				$ROOTFS_DIR/usr/lib/modules/
		fi

		rm -rf $ROOTFS_DIR/etc/init.d/S36load_wifi_modules
		cp $RKWIFIBT/S36load_all_wifi_modules $ROOTFS_DIR/etc/init.d/
		sed -i "s/BT_TTY_DEV/\/dev\/${BT_TTY_DEV}/g" \
			$ROOTFS_DIR/etc/init.d/S36load_all_wifi_modules
	fi

	if [[ "$WIFI_CHIP" = "ALL_AP" ]];then
		echo "copy ap6xxx firmware/nvram to rootfs"
		cp $RKWIFIBT/drivers/bcmdhd/*.ko $ROOTFS_DIR/system/lib/modules/
		cp $RKWIFIBT/firmware/broadcom/*/wifi/* \
			$ROOTFS_DIR/system/etc/firmware/ || true
		cp $RKWIFIBT/firmware/broadcom/*/bt/* \
			$ROOTFS_DIR/system/etc/firmware/ || true

		cp $RKWIFIBT/sh/bt_load_broadcom_firmware $ROOTFS_DIR/usr/bin/
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_init.sh
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_pcba_test

		#reatek
		echo "copy realtek firmware/nvram to rootfs"
		cp $RKWIFIBT/drivers/rtl*/*.ko $ROOTFS_DIR/system/lib/modules/
		cp -rf $RKWIFIBT/firmware/realtek/*/* $ROOTFS_DIR/lib/firmware/
		cp -rf $RKWIFIBT/firmware/realtek/*/* \
			$ROOTFS_DIR/lib/firmware/rtlbt/
		cp $RKWIFIBT/drivers/bluetooth_uart_driver/hci_uart.ko \
			$ROOTFS_DIR/usr/lib/modules/
		if [ -n "$WIFI_USB" ]; then
			cp $RKWIFIBT/drivers/bluetooth_usb_driver/rtk_btusb.ko \
				$ROOTFS_DIR/usr/lib/modules/
		fi

		echo "copy S36load_wifi_modules to rootfs"
		rm -rf $ROOTFS_DIR/etc/init.d/S36load_wifi_modules
		cp $RKWIFIBT/S36load_all_wifi_modules $ROOTFS_DIR/etc/init.d/
		sed -i "s/BT_TTY_DEV/\/dev\/${BT_TTY_DEV}/g" \
			$ROOTFS_DIR/etc/init.d/S36load_all_wifi_modules
	fi

	if [[ "$WIFI_CHIP" =~ "RTL" ]];then
		echo "Copy RTL file to rootfs"
		if [ -d "$RKWIFIBT/firmware/realtek/$WIFI_CHIP" ]; then
			cp $RKWIFIBT/firmware/realtek/$WIFI_CHIP/* \
				$ROOTFS_DIR/lib/firmware/rtlbt/
			cp $RKWIFIBT/firmware/realtek/$WIFI_CHIP/* \
				$ROOTFS_DIR/lib/firmware/
		else
			echo "INFO: $WIFI_CHIP isn't bluetooth?"
		fi

		WIFI_KO_DIR=$(echo $WIFI_CHIP | tr '[A-Z]' '[a-z]')

		cp $RKWIFIBT/drivers/$WIFI_KO_DIR/*.ko \
			$ROOTFS_DIR/system/lib/modules/

		cp $RKWIFIBT/sh/bt_load_rtk_firmware $ROOTFS_DIR/usr/bin/
		sed -i "s/BT_TTY_DEV/\/dev\/${BT_TTY_DEV}/g" \
			$ROOTFS_DIR/usr/bin/bt_load_rtk_firmware
		if [ -n "$WIFI_USB" ]; then
			cp $RKWIFIBT/drivers/bluetooth_usb_driver/rtk_btusb.ko \
				$ROOTFS_DIR/usr/lib/modules/
			sed -i "s/BT_DRV/rtk_btusb/g" \
				$ROOTFS_DIR/usr/bin/bt_load_rtk_firmware
		else
			cp $RKWIFIBT/drivers/bluetooth_uart_driver/hci_uart.ko \
				$ROOTFS_DIR/usr/lib/modules/
			sed -i "s/BT_DRV/hci_uart/g" \
				$ROOTFS_DIR/usr/bin/bt_load_rtk_firmware
		fi
		cp $ROOTFS_DIR/usr/bin/bt_load_rtk_firmware \
			$ROOTFS_DIR/usr/bin/bt_init.sh
		cp $ROOTFS_DIR/usr/bin/bt_load_rtk_firmware \
			$ROOTFS_DIR/usr/bin/bt_pcba_test
		rm -rf $ROOTFS_DIR/etc/init.d/S36load_all_wifi_modules
		cp $RKWIFIBT/S36load_wifi_modules $ROOTFS_DIR/etc/init.d/
		sed -i "s/WIFI_KO/\/system\/lib\/modules\/$WIFI_CHIP.ko/g" \
			$ROOTFS_DIR/etc/init.d/S36load_wifi_modules
	fi

	if [[ "$WIFI_CHIP" =~ "CYW" ]];then
		echo "Copy CYW file to rootfs"
		#firmware
		cp $RKWIFIBT/firmware/infineon/$WIFI_CHIP/* \
			$ROOTFS_DIR/system/etc/firmware/
		cp $RKWIFIBT/drivers/infineon/*.ko \
			$ROOTFS_DIR/system/lib/modules/
		#bt
		cp $RKWIFIBT/sh/bt_load_broadcom_firmware $ROOTFS_DIR/usr/bin/
		sed -i "s/BT_TTY_DEV/\/dev\/${BT_TTY_DEV}/g" \
			$ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware
		sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\//g" \
			$ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_init.sh
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_pcba_test
		#wifi
		rm -rf $ROOTFS_DIR/etc/init.d/S36load_all_wifi_modules
		cp $RKWIFIBT/S36load_wifi_modules $ROOTFS_DIR/etc/init.d/
		sed -i "s/WIFI_KO/\/system\/lib\/modules\/$WIFI_CHIP.ko/g" \
			$ROOTFS_DIR/etc/init.d/S36load_wifi_modules
	fi

	if [[ "$WIFI_CHIP" =~ "AP6" ]];then
		echo "Copy AP file to rootfs"
		#firmware
		cp $RKWIFIBT/firmware/broadcom/$WIFI_CHIP/wifi/* \
			$ROOTFS_DIR/system/etc/firmware/
		cp $RKWIFIBT/firmware/broadcom/$WIFI_CHIP/bt/* \
			$ROOTFS_DIR/system/etc/firmware/
		cp $RKWIFIBT/drivers/bcmdhd/*.ko $ROOTFS_DIR/system/lib/modules/
		#bt
		cp $RKWIFIBT/sh/bt_load_broadcom_firmware $ROOTFS_DIR/usr/bin/
		sed -i "s/BT_TTY_DEV/\/dev\/${BT_TTY_DEV}/g" \
			$ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware
		sed -i "s/BTFIRMWARE_PATH/\/system\/etc\/firmware\//g" \
			$ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_init.sh
		cp $ROOTFS_DIR/usr/bin/bt_load_broadcom_firmware \
			$ROOTFS_DIR/usr/bin/bt_pcba_test
		#wifi
		rm -rf $ROOTFS_DIR/etc/init.d/S36load_all_wifi_modules
		cp $RKWIFIBT/S36load_wifi_modules $ROOTFS_DIR/etc/init.d/
		if [[ "$WIFI_CHIP" =~ "AP" ]];then
			sed -i "s/WIFI_KO/\/system\/lib\/modules\/bcmdhd.ko/g" \
				$ROOTFS_DIR/etc/init.d/S36load_wifi_modules
		else
			sed -i "s/WIFI_KO/\/system\/lib\/modules\/bcmdhd_pcie.ko/g" \
				$ROOTFS_DIR/etc/init.d/S36load_wifi_modules
		fi
	fi

	finish_build build_wifibt $@
}

# Hooks

usage_hook()
{
	echo -e "wifibt[:<dst dir>[:<chip>[:<tty>]]] \tbuild Wifi/BT"
}

BUILD_CMDS="wifibt"
build_hook()
{
	shift
	build_wifibt "$([ -d "$1" ] || echo $DEFAULT_ROOTFS_DIR)" $@
}

source "${BUILD_HELPER:-$(dirname "$(realpath "$0")")/../build-hooks/build-helper}"

build_wifibt $@
