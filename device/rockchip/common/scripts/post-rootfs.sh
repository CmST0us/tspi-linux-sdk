#!/bin/bash -e

echo "Executing $(basename "$BASH_SOURCE")..."

SCRIPT_DIR="$(dirname "$(realpath "$BASH_SOURCE")")"

# HACK: Allow host tools, e.g. python2 in yocto building
export PATH="$PATH:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin" \

# Trigger build.sh's post-rootfs hooks
RK_SESSION=latest "$SCRIPT_DIR/build.sh" post-rootfs $@
