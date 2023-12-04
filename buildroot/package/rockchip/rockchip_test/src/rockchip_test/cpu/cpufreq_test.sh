#!/bin/bash

DIR_DVFS=/rockchip_test/cpu

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
echo "cpu freq test:(with out stress test)                2"
echo "*****************************************************"

read -t 30 CPUFREQ_CHOICE

cpu_freq_stress_test()
{
	#test 24 hours, every cpu frequency stay 10 seconds
	bash ${DIR_DVFS}/cpu_freq_stress_test.sh 86400 10 &
}

cpu_freq_test()
{
	sh ${DIR_DVFS}/auto_cpu_freq_test.sh 1 &
}

case ${CPUFREQ_CHOICE} in
	1)
		cpu_freq_stress_test
		;;
	2)
		cpu_freq_test
		;;
	*)
		echo "not fount your input."
		;;
esac
