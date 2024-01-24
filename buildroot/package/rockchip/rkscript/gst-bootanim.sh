#!/bin/sh

VIDEO=$(ls /etc/bootanim.d/*.mp4 2>/dev/null)

[ -z "$VIDEO" ] && exit

VSINK_ARGS=kmssink

# Comment this for using overlay plane
VSINK_ARGS="$VSINK_ARGS force-modesetting=true"

# Uncomment this for fullscreen
# VSINK_ARGS="$VSINK_ARGS fullscreen=true"

gst-play-1.0 $VIDEO -q --no-interactive --audiosink=fakesink \
	--videosink="$VSINK_ARGS"&

# Let the caller know that we are started
touch ${TAG_FILE:-/dev/null}
