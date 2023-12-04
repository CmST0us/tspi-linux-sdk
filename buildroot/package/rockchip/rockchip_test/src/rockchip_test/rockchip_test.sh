#!/bin/sh
### file: rockchip_test.sh
### author: yhx@rock-chips.com wxt@rock-chips.com
### function: ddr cpu gpio flash bt audio recovery s2r sdio/pcie(wifi)
###           ethernet reboot ddrfreq npu camera video
### date: 20190107

moudle_env()
{
   export  MODULE_CHOICE
}

module_choice()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***        ********************                   ***"
    echo "***       *ROCKCHIPS TEST TOOLS*                  ***"
    echo "***        *                  *                   ***"
    echo "***        ********************                   ***"
    echo "***                                               ***"
    echo "*****************************************************"


    echo "*****************************************************"
    echo "ddr test :            1 (memtester & stressapptest)"
    echo "cpufreq test:         2 (cpufreq stresstest)"
    echo "flash stress test:    3"
    echo "bluetooth test:       4 (bluetooth on&off test)"
    echo "audio test:           5"
    echo "recovery test:        6 (default wipe all)"
    echo "suspend_resume test:  7 (suspend & resume)"
    echo "wifi test:            8"
    echo "ethernet test:        9"
    echo "auto reboot test:     10"
    echo "ddr freq scaling test 11"
    echo "npu test              12"
    echo "npu2 test             13 (rk356x or rk3588)"
    echo "camera test           14 (use rkisp_demo)"
    echo "video test            15 (use gstreamer-wayland and app_demo)"
    echo "gpu test              16 (use glmark2)"
    echo "chromium test         17 (chromium with video hardware acceleration)"
    echo "nand power lost test: 18"
    echo "*****************************************************"

    echo  "please input your test moudle: "
    read -t 30  MODULE_CHOICE
}

npu_stress_test()
{
    sh /rockchip_test/npu/npu_test.sh
}

npu2_stress_test()
{
    sh /rockchip_test/npu2/npu_test.sh
}

ddr_test()
{
    sh /rockchip_test/ddr/ddr_test.sh
}

cpufreq_test()
{
    sh /rockchip_test/cpu/cpufreq_test.sh
}

flash_stress_test()
{
    bash /rockchip_test/flash_test/flash_stress_test.sh 5 20000&
}

recovery_test()
{
    sh /rockchip_test/recovery_test/auto_reboot.sh
}

suspend_resume_test()
{
    sh /rockchip_test/suspend_resume/suspend_resume.sh
}

wifi_test()
{
    sh /rockchip_test/wifi/wifi_test.sh
}

ethernet_test()
{
    sh /test_plan/ethernet/eth_test.sh
}

bluetooth_test()
{
    sh /rockchip_test/bluetooth/bt_onoff.sh &
}

audio_test()
{
    sh /rockchip_test/audio/audio_functions_test.sh
}

auto_reboot_test()
{
    fcnt=/data/config/rockchip_test/reboot_cnt;
    if [ -e "$fcnt" ]; then
	rm -f $fcnt;
    fi
    sh /rockchip_test/auto_reboot/auto_reboot.sh
}

ddr_freq_scaling_test()
{
    bash /rockchip_test/ddr/ddr_freq_scaling.sh
}

camera_test()
{
    sh /rockchip_test/camera/camera_test.sh
}

video_test()
{
    sh /rockchip_test/video/video_test.sh
}

gpu_test()
{
    sh /rockchip_test/gpu/gpu_test.sh
}

chromium_test()
{
    sh /rockchip_test/chromium/chromium_test.sh
}

power_lost_test()
{
        fcnt=/data/config/rockchip_test/reboot_cnt;
        if [ -e "$fcnt" ]; then
                rm -f $fcnt;
        fi
        sh /rockchip_test/flash_test/power_lost_test.sh &
}

module_test()
{
    case ${MODULE_CHOICE} in
        1)
            ddr_test
            ;;
        2)
            cpufreq_test
            ;;
        3)
            flash_stress_test
            ;;
        4)
            bluetooth_test
            ;;
        5)
            audio_test
            ;;
        6)
            recovery_test
            ;;
        7)
            suspend_resume_test
            ;;
        8)
            wifi_test
            ;;
        9)
            ethernet_test
            ;;
        10)
            auto_reboot_test
            ;;
	11)
	    ddr_freq_scaling_test
	    ;;
	12)
	    npu_stress_test
	    ;;
	13)
	    npu2_stress_test
	    ;;
	14)
	    camera_test
	    ;;
	15)
	    video_test
	    ;;
	16)
	    gpu_test
	    ;;
	17)
	    chromium_test
	    ;;
	18)
	    power_lost_test
	    ;;
    esac
}

module_choice
module_test
