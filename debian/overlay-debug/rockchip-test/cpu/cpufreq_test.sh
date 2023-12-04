#!/bin/bash

DIR_DVFS=/rockchip-test/cpu

info_view()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***            CPU TEST                           ***"
    echo "***                                               ***"
    echo "*****************************************************"
}

info_view
echo "*****************************************************"
echo "cpu freq stress test:                               1"
echo "*****************************************************"

read -t 30 CPUFREQ_CHOICE

cpu_freq_stress_test()
{
	#test 24 hours, every cpu frequency stay 10 seconds
	bash ${DIR_DVFS}/cpu_freq_stress_test.sh 86400 10 &
}

case ${CPUFREQ_CHOICE} in
	1)
		cpu_freq_stress_test
		;;
	*)
		echo "not fount your input."
		;;
esac
