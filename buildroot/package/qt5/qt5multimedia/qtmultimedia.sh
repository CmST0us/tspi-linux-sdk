# The env variables below can be override by the init script(e.g. S50launcher)

# The kmssink has better performance but is more limited.
# export PREFERED_VIDEOSINK=kmssink
export PREFERED_VIDEOSINK=waylandsink

export QT_GSTREAMER_WIDGET_VIDEOSINK=${PREFERED_VIDEOSINK}
export QT_GSTREAMER_WINDOW_VIDEOSINK=${PREFERED_VIDEOSINK}

# Neither of them is perfect.
# The playbin2 (decodebin2) cannot handle multiqueue overrun for some
# broken streams:
# https://bugzilla.gnome.org/show_bug.cgi?id=775474
# The playbin3 (decodebin3) might cause memory corruption or ghostpad
# activation deadlock in looping.
# export QT_GSTREAMER_PLAYBIN=playbin
export QT_GSTREAMER_PLAYBIN=playbin3
