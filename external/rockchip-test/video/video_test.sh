#!/bin/bash

DIR_VIDEO=`dirname $0`

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
echo "video test:                                              1"
echo "video test with FPS display:                             2"
echo "video max FPS test without display:                      3"
echo "multivideo test:                                         4"
echo "video stresstest:                                        5"
echo "***********************************************************"

read -t 30 VIDEO_CHOICE

video_test()
{
	sh ${DIR_VIDEO}/test_gst_video.sh
}

video_test_fps()
{
	sh ${DIR_VIDEO}/test_gst_video_fps.sh
}

video_test_maxfps()
{
	sh ${DIR_VIDEO}/test_gst_video_maxfps.sh
}

multivideo_test()
{
	sh ${DIR_VIDEO}/test_gst_multivideo.sh
}

video_stresstest()
{
	sh ${DIR_VIDEO}/video_stresstest.sh
}

case ${VIDEO_CHOICE} in
	1)
		video_test
		;;
	2)
		video_test_fps
		;;
	3)
		video_test_maxfps
		;;
	4)
		multivideo_test
		;;
	5)
		video_stresstest
		;;
	*)
		echo "not found your input."
		;;
esac
