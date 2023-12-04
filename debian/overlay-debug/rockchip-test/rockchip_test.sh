#!/bin/bash
### file: rockchip-test.sh
### function: ddr cpu gpio flash bt audio recovery s2r sdio/pcie(wifi)
###           ethernet reboot ddrfreq npu camera video and so on.

moudle_env()
{
   export  MODULE_CHOICE
}

module_choice()
{
    echo "******************************************************"
    echo "***                                                ***"
    echo "***          *****************************         ***"
    echo "***          *    ROCKCHIPS TEST TOOLS   *         ***"
    echo "***          *  V1.0 updated on 20220324 *         ***"
    echo "***          *****************************         ***"
    echo "***                                                ***"
    echo "*****************************************************"


    echo "*****************************************************"
    echo "cpu test:             1 (cpufreq stresstest)"
    echo "ddr test:             2 (memtester & stressapptest)"
    echo "gpu test:             3 (use glmark2)"
    echo "npu test:             4 (npu2:rk3588)"
    echo "auto reboot test:     5 (reboot tests)"
    echo "suspend_resume test:  6 (suspend & resume)"
    echo "nand power lost test: 7 (S5 stress tests)"
    echo "flash stress test:    8 (flash tests)"
    echo "audio test:           9 (audio tests)"
    echo "recovery test:        10 (default wipe all)"
    echo "bluetooth test:       11 (bluetooth on&off test)"
    echo "wifi test:            12 (wifi on&off test)"
    echo "ethernet test:        13 (ethernet tests)"
    echo "camera test:          14 (use rkaiq_demo)"
    echo "video test:           15 (use gstreamer-wayland and app_demo)"
    echo "chromium test:        16 (chromium with video hardware acceleration)"
    echo "hardware infomation:  17 (to get the hardware infomation)"
    echo "*****************************************************"

    echo  "please input your test moudle: "
    read -t 30  MODULE_CHOICE
}

npu_stress_test()
{
    bash /rockchip-test/npu/npu_test.sh
}

npu2_stress_test()
{
    bash /rockchip-test/npu2/npu_test.sh
}

ddr_test()
{
    bash /rockchip-test/ddr/ddr_test.sh
}

cpu_test()
{
    bash /rockchip-test/cpu/cpufreq_test.sh
}

flash_stress_test()
{
    bash /rockchip-test/flash_test/flash_stress_test.sh 5 20000&
}

recovery_test()
{
    bash /rockchip-test/recovery/recovery_test.sh
}

suspend_resume_test()
{
    bash /rockchip-test/suspend_resume/suspend_resume.sh
}

wifi_test()
{
    bash /rockchip-test/wifibt/wifi_onoff.sh
}

ethernet_test()
{
    bash /rockchip-test/ethernet/eth_test.sh
}

bluetooth_test()
{
    bash /rockchip-test/wifibt/bt_onoff.sh &
}

audio_test()
{
    bash /rockchip-test/audio/audio_functions_test.sh
}

auto_reboot_test()
{
    fcnt=/userdata/cfg/rockchip/reboot_cnt;
    if [ -e "$fcnt" ]; then
	rm -f $fcnt;
    fi
    bash /rockchip-test/auto_reboot/auto_reboot.sh
}

camera_test()
{
    bash /rockchip-test/camera/camera_test.sh
}

video_test()
{
    bash /rockchip-test/video/video_test.sh
}

gpu_test()
{
    bash /rockchip-test/gpu/gpu_test.sh
}

chromium_test()
{
    bash /rockchip-test/chromium/chromium_test.sh
}

power_lost_test()
{
        fcnt=/data/config/rockchip-test/reboot_cnt;
        if [ -e "$fcnt" ]; then
                rm -f $fcnt;
        fi
        bash /rockchip-test/flash_test/power_lost_test.sh &
}

sys_info_get()
{
    bash /rockchip-test/system_infomation/get_sys_info.sh
}
module_test()
{
	case ${MODULE_CHOICE} in
		1)
			cpu_test
			;;
		2)
			ddr_test
			;;
		3)
			gpu_test
			;;
		4)
			npu2_stress_test
			;;
		5)
			auto_reboot_test
			;;
		6)
			suspend_resume_test
			;;
		7)
			power_lost_test
			;;
		8)
			flash_stress_test
			;;
		9)
			audio_test
			;;
		10)
			recovery_test
			;;
		11)
			bluetooth_test
			;;
		12)
			wifi_test
			;;
		13)
			ethernet_test
			;;
		14)
			camera_test
			;;
		15)
			video_test
			;;
		16)
			chromium_test
			;;
		17)
			sys_info_get
			;;
	esac
}

module_choice
module_test
