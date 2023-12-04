#!/bin/sh

DIR_CAMERA=/rockchip_test/camera

info_view()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***            CAMERA TEST                        ***"
    echo "***                                               ***"
    echo "*****************************************************"
}

info_view
echo "*****************************************************"
echo "camera rkisp test:                                  1"
echo "camera usb test:                                    2"
echo "camera stresstest:                                  3"
echo "*****************************************************"

read -t 30 CAMERA_CHOICE

camera_rkisp_test()
{
	sh ${DIR_CAMERA}/camera_rkisp_test.sh
}

camera_usb_test()
{
	sh ${DIR_CAMERA}/camera_usb_test.sh
}

camera_stresstest()
{
	sh ${DIR_CAMERA}/camera_stresstest.sh 1000
}

case ${CAMERA_CHOICE} in
	1)
		camera_rkisp_test
		;;
	2)
		camera_usb_test
		;;
	3)
		camera_stresstest
		;;
	*)
		echo "not fount your input."
		;;
esac
