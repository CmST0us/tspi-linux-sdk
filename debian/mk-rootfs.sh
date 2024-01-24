#!/bin/bash -e

if [ ! $RELEASE ]; then
	RELEASE='stretch'
fi

./mk-rootfs-$RELEASE.sh
