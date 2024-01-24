#!/bin/sh

# $1 is chip
CHIP_ID=""
HIGH_LEVEL="3.3V" # get from chip trm
LOW_LEVEL="1.8V"  # get from chip trm

TOOL_HEXDUMP="NO"
VERSION="V1.0.1"
program_name="$0"

function help_msg()
{
	echo "Please input chip."
	echo "$program_name <chip>"
	echo "chip: rv1126,rv1109,rk3308,rk3308b,rk3308bs"
	exit 0
}

function echo_msg()
{
	echo -e "\e[1;31m    $1 \e[0m"
}

function get_reg()
{
	if [ -n "$1" ]; then
		_val=`io -4 -r $1`
		_val=`echo ${_val##*:}`
		_val=`echo 0x${_val}`
		echo "$_val"
	else
		echo "get register error."
		exit -1
	fi
}

function print_val()
{
	tag="$2"
	if [ $(( $1 )) -eq 0 ];then
		echo_msg "$tag $HIGH_LEVEL"
	else
		echo_msg "$tag $LOW_LEVEL"
	fi
}

function list_iodomain_rv1126_rv1109()
{
	iodomain_val=`get_reg 0xFE020140`
	pmuio1_vsel=$(( 0x1 << 9 ))
	pmuio0_vsel=$(( 0x1 << 8 ))
	vccio7_vsel=$(( 0x1 << 7 ))
	vccio6_vsel=$(( 0x1 << 6 ))
	vccio5_vsel=$(( 0x1 << 5 ))
	vccio4_vsel=$(( 0x1 << 4 ))
	vccio3_vsel=$(( 0x1 << 3 ))
	vccio2_vsel=$(( 0x1 << 2 ))
	vccio1_vsel=$(( 0x1 << 1 ))
	flash_vosel=$(( 0x1 << 0 ))

	print_val $(( $iodomain_val & $pmuio1_vsel )) "pmuio1_vsel:"
	print_val $(( $iodomain_val & $pmuio0_vsel )) "pmuio0_vsel:"
	print_val $(( $iodomain_val & $vccio7_vsel )) "vccio7_vsel:"
	print_val $(( $iodomain_val & $vccio6_vsel )) "vccio6_vsel:"
	print_val $(( $iodomain_val & $vccio5_vsel )) "vccio5_vsel:"
	print_val $(( $iodomain_val & $vccio4_vsel )) "vccio4_vsel:"
	print_val $(( $iodomain_val & $vccio3_vsel )) "vccio3_vsel:"
	print_val $(( $iodomain_val & $vccio2_vsel )) "vccio2_vsel:"

	if [ $(( $iodomain_val & $flash_vosel )) -eq 0 ];then
		# read iomux
		gpio0b3_iomux_val=`get_reg 0xFE020008`
		gpio0b3_iomux_val_off=$(( 0x1 << 12 ))
		if [ $(( $gpio0b3_iomux_val & $gpio0b3_iomux_val_off )) -eq 0 ];then
			# read gpio data direction
			gpio0b3_data_direction_val=`get_reg 0xFF460008`
			gpio0b3_data_direction_val_off=$(( 0x1 << 11 ))
			if [ $(( $gpio0b3_data_direction_val & $gpio0b3_data_direction_val_off )) -eq 0 ];then
				# read gpio data register
				gpio0b3_data_reg_val=`get_reg 0xFF460070`
				gpio0b3_data_reg_val_off=$(( 0x1 << 11 ))
				print_val $(( $gpio0b3_data_reg_val & $gpio0b3_data_reg_val_off )) "vccio1_vsel:"
			else
				echo "Check vccio1_vsel error!!!"
			fi
		else
			echo "Check vccio1_vsel error!!!"
		fi
	fi
	if [ $(( $iodomain_val & $flash_vosel )) -eq 1 ];then
		print_val $(( $iodomain_val & $vccio1_vsel )) "vccio1_vsel:"
	fi
}

function list_iodomain_rk3308_rk3308b_rk3308bs()
{
	iodomain_val=`get_reg 0xFF000300`
	flash_vosel=$(( 0x1 << 8 ))
	vccio5_vsel=$(( 0x1 << 5 ))
	vccio4_vsel=$(( 0x1 << 4 ))
	vccio3_vsel=$(( 0x1 << 3 ))
	vccio2_vsel=$(( 0x1 << 2 ))
	vccio1_vsel=$(( 0x1 << 1 ))
	vccio0_vsel=$(( 0x1 << 0 ))

	if [ $(( $iodomain_val & $flash_vosel )) -eq $(( 0x1 << 8 )) ];then
		print_val $(( $iodomain_val & $vccio5_vsel )) "vccio5_vsel:"
		print_val $(( $iodomain_val & $vccio4_vsel )) "vccio4_vsel:"
		print_val $(( $iodomain_val & $vccio3_vsel )) "vccio3_vsel:"
		print_val $(( $iodomain_val & $vccio2_vsel )) "vccio2_vsel:"
		print_val $(( $iodomain_val & $vccio1_vsel )) "vccio1_vsel:"
		print_val $(( $iodomain_val & $vccio0_vsel )) "vccio0_vsel:"
        fi
	if [ $(( $iodomain_val & $flash_vosel )) -eq 0 ];then
		# read iomux
		gpio0a4_iomux_val=`get_reg 0xFF000000`
		gpio0a4_iomux_val_off=$(( 0x1 << 8 ))
		if [ $(( $gpio0a4_iomux_val & $gpio0a4_iomux_val_off )) -eq 0 ];then
			# read gpio data direction
			gpio0a4_data_direction_val=`get_reg 0xFF220004`
			gpio0a4_data_direction_val_off=$(( 0x1 << 4 ))
			if [ $(( $gpio0a4_data_direction_val & $gpio0b3_data_direction_val_off )) -eq 0 ];then
				# read gpio data register
				gpio0a4_data_reg_val=`get_reg 0xFF220050`
				gpio0a4_data_reg_val_off=$(( 0x1 << 4 ))
				print_val $(( $iodomain_val & $vccio5_vsel )) "vccio5_vsel:"
				print_val $(( $iodomain_val & $vccio4_vsel )) "vccio4_vsel:"
				print_val $(( $gpio0a4_data_reg_val & $gpio0a4_data_reg_val_off )) "vccio3_vsel:"
				print_val $(( $iodomain_val & $vccio2_vsel )) "vccio2_vsel:"
				print_val $(( $iodomain_val & $vccio1_vsel )) "vccio1_vsel:"
				print_val $(( $iodomain_val & $vccio0_vsel )) "vccio0_vsel:"
			else
				echo "Check vccio1_vsel error!!!"
			fi
		else
			echo "Check vccio1_vsel error!!!"
		fi
	fi
}

function list_iodomain_rk3326_rk3326s()
{
	iodomain0_val=`get_reg 0xFF140180`
	iodomain1_val=`get_reg 0xFF010100`

	pmuio2_vsel=$(( 0x1 << 15 ))
	pmuio1_vsel=$(( 0x1 << 14 ))

	vccio5_vsel=$(( 0x1 << 6 ))
	vccio4_vsel=$(( 0x1 << 5 ))
	vccio3_vsel=$(( 0x1 << 4 ))
	vccio2_vsel=$(( 0x1 << 3 ))
	vccio1_vsel=$(( 0x1 << 2 ))

	print_val $(( $iodomain1_val & $pmuio2_vsel )) "pmuio2_vsel:"
	print_val $(( $iodomain1_val & $pmuio1_vsel )) "pmuio1_vsel:"
	print_val $(( $iodomain0_val & $vccio5_vsel )) "vccio5_vsel:"
	print_val $(( $iodomain0_val & $vccio4_vsel )) "vccio4_vsel:"
	print_val $(( $iodomain0_val & $vccio3_vsel )) "vccio3_vsel:"
	print_val $(( $iodomain0_val & $vccio2_vsel )) "vccio2_vsel:"
	print_val $(( $iodomain0_val & $vccio1_vsel )) "vccio1_vsel:"
}

function list_iodomain_rk3566_rk3568()
{
	iodomain0_val=`get_reg 0xFDC20140`
	iodomain1_val=`get_reg 0xFDC20144`
	iodomain2_val=`get_reg 0xFDC20148`

	pmuio2_vsel=$(( 0x1 << 1 ))

	vccio7_vsel=$(( 0x1 << 7 ))
	vccio6_vsel=$(( 0x1 << 6 ))
	vccio5_vsel=$(( 0x1 << 5 ))
	vccio4_vsel=$(( 0x1 << 4 ))
	vccio3_vsel=$(( 0x1 << 3 ))
	vccio2_vsel=$(( 0x1 << 2 ))
	vccio1_vsel=$(( 0x1 << 1 ))

	print_val $(( $iodomain2_val & $pmuio2_vsel )) "pmuio2_vsel:"
	print_val $(( $iodomain0_val & $vccio7_vsel )) "vccio7_vsel:"
	print_val $(( $iodomain0_val & $vccio6_vsel )) "vccio6_vsel:"
	print_val $(( $iodomain0_val & $vccio5_vsel )) "vccio5_vsel:"
	print_val $(( $iodomain0_val & $vccio4_vsel )) "vccio4_vsel:"
	print_val $(( $iodomain0_val & $vccio3_vsel )) "vccio3_vsel:"
	print_val $(( $iodomain0_val & $vccio2_vsel )) "vccio2_vsel:"
	print_val $(( $iodomain0_val & $vccio1_vsel )) "vccio1_vsel:"
}

function chk_rv1126_rv1109()
{
	nvem_path="/sys/bus/nvmem/devices/rockchip-otp0/nvmem"
	iff=`hexdump -C $nvem_path | grep -Ew "00000000  52 56 11 26|00000000  52 56 11 09"`
	if [ -n "$iff" ]; then
		CHIP_ID=rv1126_rv1109
		HIGH_LEVEL="3.3V"
		LOW_LEVEL="1.8V"
		return 0
	fi
	return 1
}

function chk_rk3308_rk3308b_rk3308bs()
{
	nvem_path="/sys/bus/nvmem/devices/rockchip-otp0/nvmem"
	iff=`hexdump -C $nvem_path | grep -Ew "00000000  52 4b 33 08"`
	if [ -n "$iff" ]; then
		CHIP_ID=rk3308
		HIGH_LEVEL="3.3V"
		LOW_LEVEL="1.8V"
		return 0
	fi
	return 1
}

function chk_rk3326_rk3326s()
{
	nvem_path="/sys/bus/nvmem/devices/rockchip-otp0/nvmem"
	iff=`hexdump -C $nvem_path | grep -Ew "00000000  52 4b 33 26"`
	if [ -n "$iff" ]; then
		CHIP_ID=rk3326
		HIGH_LEVEL="3.3V"
		LOW_LEVEL="1.8V"
		return 0
	fi
	return 1
}

function chk_rk3566_rk3568()
{
	nvem_path="/sys/bus/nvmem/devices/rockchip-otp0/nvmem"
	iff=`hexdump -C $nvem_path | grep -Ew "00000000  52 4b 35 68|00000000  52 4b 35 66"`
	if [ -n "$iff" ]; then
		CHIP_ID=rk356x
		HIGH_LEVEL="3.3V"
		LOW_LEVEL="1.8V"
		return 0
	fi
	return 1
}

function get_chip_id()
{
	# if error, exit -1
	if [ "$TOOL_HEXDUMP" = "YES" ];then
		# if check otp success, just return CHIP_ID and ignore $1
		chk_rv1126_rv1109
		if [ $? -eq 0 ];then
			return 0
		fi
		chk_rk3308_rk3308b_rk3308bs
		if [ $? -eq 0 ];then
			return 0
		fi
		chk_rk3326_rk3326s
		if [ $? -eq 0 ];then
			return 0
		fi
		chk_rk3566_rk3568
		if [ $? -eq 0 ];then
			return 0
		fi
	fi

	# if check otp failed, check CHIP_ID with $1
	case "$1" in
		rv1126|rv1109|RV1126|RV1109)
			chip_id=`io -4 -r 0XFE000110 | grep -w 00001109`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=rv1126_rv1109
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		rk3308bs|RK3308BS)
		chip_id=`io -4 -r 0XFF000800 | grep -w 0003308c`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=$1
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		rk3308b|RK3308B)
		chip_id=`io -4 -r 0XFF000800 | grep -w 00003308`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=$1
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		rk3308|RK3308)
		chip_id=`io -4 -r 0XFF000800 | grep -w 00000cea`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=$1
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		rk3326|RK3326)
		chip_id=`io -4 -r 0XFF140800 | grep -w 00003326`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=$1
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		rk3326s|RK3326S)
		chip_id=`io -4 -r 0XFF140800 | grep -w 00003326`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=$1
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		rk3566|rk3568|RK3566|RK3568)
			chip_id=`io -4 -r 0XFDC60800 | grep -w 00003566`
			if [ $? -eq 0 -a -n "$chip_id" ]; then
				CHIP_ID=$1
				HIGH_LEVEL="3.3V"
				LOW_LEVEL="1.8V"
				return 0
			fi
			;;
		*)
			help_msg
			;;
	esac

	echo "Not found CHIP_ID...exit!!!"
	exit 0
}

# input parameter
#      CHIP_ID    --> rv1126,rk3308,rk3308b,rk3308bs,rk3566,rk3568
#      HIGH_LEVEL --> "3.3V"
#      LOW_LEVEL  --> "1.8V"
function get_iodomain_val()
{
	# if error, exit -1
	echo_msg "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	echo_msg "XXXXXXXXXX      PLEASE CHECK IO-DOMAIN !!!!!!!!!!!!!!!"
	echo_msg "XXXXXXXXXX        请务必检查IO电源域配置  !!!!!!!!!!!!!!!"
	echo_msg "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	echo_msg "Get IO DOMAIN VALUE:"
	case "$CHIP_ID" in
		rv1126|rv1109|RV1126|RV1109|rv1126_rv1109|RV1126_RV1109)
			list_iodomain_rv1126_rv1109
			;;
		rk3308bs|rk3308b|rk3308|RK3308BS|RK3308B|RK3308)
			list_iodomain_rk3308_rk3308b_rk3308bs
			;;
		rk3326|rk3326s|RK3326|RK3326)
			list_iodomain_rk3326_rk3326s
			;;
		rk3566|rk3568|rk356x|RK3566|RK3568|RK356X)
			echo_msg "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
			echo_msg "注意事项：PMUIO1/PMUIO2 固定不可配"
			echo_msg "VCCIO2电平由硬件FLASH_VOL_SEL决定:"
			echo_msg "当VCCIo2供电是1.8V,则FLASH_VOL_SEL管脚必须保持为高电平;"
			echo_msg "当VCCIO2供电是3.3V,则FLASH_VOL_SEL管脚必须保持为低电平;"
			echo_msg "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

			list_iodomain_rk3566_rk3568
			;;
		*)
			help_msg
			;;
	esac
	echo_msg "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	exit 0
}

function chk_env()
{
	echo_msg "$0 version: $VERSION"
	# if error, exit -1
	which io &>/dev/null
	if [ $? -ne 0 ]; then
		echo_msg "install io command first!!!"
		exit -1
	fi

	which hexdump &>/dev/null
	if [ $? -eq 0 ]; then
		TOOL_HEXDUMP="YES"
	fi
}

if [ "$1" = "-h" -o "$1" = "--help" ]; then
	help_msg
fi
chk_env
get_chip_id $1
echo_msg "Get CHIP ID: $CHIP_ID"
get_iodomain_val
