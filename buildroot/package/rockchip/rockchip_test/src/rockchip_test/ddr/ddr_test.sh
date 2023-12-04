#!/bin/sh

DIR_DDR=/rockchip_test/ddr

info_view()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***            DDR TEST                           ***"
    echo "***                                               ***"
    echo "*****************************************************"
}

info_view
echo "*****************************************************"
echo "memtester test:                                 1"
echo "stressapptest:                                  2"
echo "*****************************************************"

read -t 30 DDR_CHOICE

memtester_test()
{
	sh ${DIR_DDR}/memtester_test.sh
}

stressapptest_test()
{
	sh ${DIR_DDR}/stressapptest_test.sh
}

case ${DDR_CHOICE} in
	1)
		memtester_test
		;;
	2)
		stressapptest_test
		;;
	*)
		echo "not fount your input."
		;;
esac
