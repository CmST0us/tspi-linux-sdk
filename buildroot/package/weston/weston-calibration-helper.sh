#!/bin/bash -x

cat <<EOF >/etc/profile.d/weston-calibration.sh
# Calibration for $1
export WESTON_TOUCH_CALIBRATION="$2 $3 $4 $5 $6 $7"
EOF
