#!/bin/bash

export PREFERED_VIDEOSINK=xvimagesink
export QT_GSTREAMER_WIDGET_VIDEOSINK=${PREFERED_VIDEOSINK}
export QT_GSTREAMER_WINDOW_VIDEOSINK=${PREFERED_VIDEOSINK}

echo performance | tee $(find /sys/ -name *governor) /dev/null || true

multivideoplayer -platform xcb
