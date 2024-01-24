#!/bin/bash -e

# mk-image.sh requires -d option to pack e2fs for non-root user
if mke2fs -h 2>&1 | grep -wq "\-d"; then
	exit 0
fi

echo -e "\e[35m"
echo "Your mke2fs is too old: $(mke2fs -V 2>&1 | head -n 1)"
echo "Please update it:"
echo "git clone https://git.kernel.org/pub/scm/fs/ext2/e2fsprogs.git --depth 1 -b v1.47.0"
echo "cd e2fsprogs"
echo "./configure"
echo "sudo make install -j8"
echo -e "\e[0m"
exit 1
