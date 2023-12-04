#!/bin/bash

DIR_ETH=/rockchip-test/ethernet

info_view()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***            Ethernet TEST                      ***"
    echo "***                                               ***"
    echo "*****************************************************"
}

info_view
echo "***********************************************************"
echo "Ethernet delayline test:					1"
echo "***********************************************************"

read -t 30 ETH_CHOICE

ethernet_delayline_test()
{
	bash ${DIR_ETH}/test_ethernet_delayline.sh
}

case ${ETH_CHOICE} in
	1)
		ethernet_delayline_test
		;;
	*)
		echo "not fount your input."
		;;
esac
