#!/bin/sh
VERIFIED=/tmp/.adb_auth_verified

if [ -f "$VERIFIED" ]; then
    echo "success."
    exit
fi

for i in $(seq 1 3); do
    read -p "$(hostname -s)'s password: " PASSWD
    if [ "$(echo $PASSWD | md5sum)" = "AUTH_PASSWORD" ]; then
        echo "success."
        touch $VERIFIED
        exit
    fi

    echo "password incorrect!"
done

false
