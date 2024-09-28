#!/bin/bash -e

# buildroot would fixup owner in its fakeroot script
POST_OS_DISALLOWED="recovery pcba buildroot"

source "${POST_HELPER:-$(dirname "$(realpath "$0")")/../post-hooks/post-helper}"

echo "Fixing up owner for $TARGET_DIR..."

ID=$(stat --format %u "$SDK_DIR")
if [ "$ID" -ne 0 ]; then
	NAME=$(grep -E "^[^:]*:x:$ID:" /etc/passwd | cut -d':' -f1)
	echo "Fixing up uid=$ID($NAME) to 0(root)..."
	find . -user $ID -exec chown -h -R 0:0 {} \;
fi

if [ -d home ]; then
	for u in $(ls home/); do
		ID=$(grep "^$u:" etc/passwd | cut -d':' -f3 || true)
		[ "$ID" ] || continue
		echo "Fixing up /home/$u for uid=$ID($u)..."
		chown -h -R $ID:$ID home/$u
	done
fi

ID=$(stat --format %u "$RK_OUTDIR")
if [ "$(id -u)" -eq 0 -a "$ID" -ne 0 ]; then
	echo "Fixing up owner for $RK_OUTDIR..."
	find "$RK_OUTDIR" -user 0 -exec chown -h -R $ID:$ID {} \;
fi
