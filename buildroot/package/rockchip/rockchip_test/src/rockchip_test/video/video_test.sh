#!/bin/sh

DIR_VIDEO=/rockchip_test/video

info_view()
{
    echo "*****************************************************"
    echo "***                                               ***"
    echo "***            VIDEO TEST                         ***"
    echo "***                                               ***"
    echo "*****************************************************"
}

info_view
echo "***********************************************************"
echo "video test:						1"
echo "multivideo with singlewindow test				2"
echo "multivideo with multiwindow test				3"
echo "***********************************************************"

read -t 30 VIDEO_CHOICE

video_test()
{
	sh ${DIR_VIDEO}/test_gst_video.sh
}

multivideo_singlewindow_test()
{
	cd ${DIR_VIDEO}
	./test_multivideo_singlewindow.sh test
}

multivideo_multiwindow_test()
{
	cd ${DIR_VIDEO}
	./test_multivideo_multiwindow.sh test
}

case ${VIDEO_CHOICE} in
	1)
		video_test
		;;
	2)
		multivideo_singlewindow_test
		;;
	3)
		multivideo_multiwindow_test
		;;
	*)
		echo "not fount your input."
		;;
esac
