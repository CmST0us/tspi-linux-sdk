
check_time() {
while [ 1 ]
do
        uptime=`cat /proc/uptime | awk -F ' ' '{print $1}' | awk -F '.' '{print $1}'`
        echo uptime is $uptime
        if [ $uptime -lt 20 ]; then
                sleep 1
        else
                break
        fi        
done
}

check_time

case "$1" in
    start)
    
        if pidof bluetoothd; then
        killall bluetoothd
        fi

        sleep 1

        if pidof gatt-service; then
        killall gatt-service
        fi

        hciconfig hci0 down 
        sleep 1
        hciconfig hci0 up 
        
        #hciconfig hci0 piscan &

        /usr/libexec/bluetooth/bluetoothd -C -E -d -n &

        #sleep 2

        /usr/bin/gatt-service &
        
        ;;

    stop)

        if pidof bluetoothd; then
        killall bluetoothd
        fi

        sleep 1

        if pidof gatt-service; then
        killall gatt-service
        fi
        
        ;;
        
    ok)
    
        while [ 1 ]
        do
                if ! pidof gatt-service; then
                      killall bluetoothd
                      break
                fi
                sleep 1
        done
        
        ;;
    *)
esac
exit 0
