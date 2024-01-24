#!/bin/bash


if [ -z "${WESTON_DRM_MIRROR+set}" ]; then
    echo "WESTON_DRM_MIRROR is not set."
    /rp_test/display/dualscreen-display
else
    echo "WESTON_DRM_MIRROR is set, un set it"
    unset WESTON_DRM_MIRROR
    sh /etc/init.d/S49weston stop

    sh /etc/init.d/S49weston start
    
    sleep 3s    

    /rp_test/display/dualscreen-display    
fi
