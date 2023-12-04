#!/bin/sh
#

[ -f /etc/profile.d/enable_coredump.sh ] && source /etc/profile.d/enable_coredump.sh

export enable_encoder_debug=0

cvr_app &
