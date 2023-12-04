#!/bin/bash

DIR_GPU=/rockchip-test/gpu

info_view()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***            GPU TEST                           ***"
    echo "***                                               ***"
    echo "*****************************************************"
}

info_view
echo "***********************************************************"
echo "glmark2 fullscreen test:					1"
echo "glmark2 offscreen test:					2"
echo "glmark2 stress test:					3"
echo "***********************************************************"

read -t 30 GPU_CHOICE

glmark2_fullscreen_test()
{
	bash ${DIR_GPU}/test_fullscreen_glmark2.sh
}

glmark2_offscreen_test()
{
	bash ${DIR_GPU}/test_offscreen_glmark2.sh
}

glmark2_stress_test()
{
	bash ${DIR_GPU}/test_stress_glmark2.sh
}

case ${GPU_CHOICE} in
	1)
		glmark2_fullscreen_test
		;;
	2)
		glmark2_offscreen_test
		;;
	3)
		glmark2_stress_test
		;;
	*)
		echo "not fount your input."
		;;
esac
