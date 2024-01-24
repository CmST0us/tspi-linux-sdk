#!/bin/bash
DIRECTORY="/dev/serial/by-id"
counter=0
Serial_port=""

Switching_mode()
{
    PRODUCT_NAME=$(cat "$device_dir/product")
    MANUFACTURER=$(cat "$device_dir/manufacturer")
    SERIAL_NUMBER=$(cat "$device_dir/serial")

        if [ "$SERIAL_NUMBER" == "" ];then
               Serial="usb-${MANUFACTURER}_${PRODUCT_NAME}"
        else
               Serial="usb-${MANUFACTURER}_${PRODUCT_NAME}_$SERIAL_NUMBER"
        fi

        for file in $(ls "$DIRECTORY" | grep "$Serial" | sort);do
             if [ "$counter" -eq 2 ];then
                    Serial_port=$file
                    echo "Serial_port:$Serial_port"
                    break
             fi
                counter=$((counter+1))
         done

}

for device_dir in /sys/bus/usb/devices/*; do
    if [ -e "$device_dir/idProduct" ];then
	    product_id=$(cat "$device_dir/idProduct")

        if [ "$product_id" = "0801" ] || [ "$product_id" = "0800" ]; then
            echo "This is a 5G module!"
	    ping -c 2 -W 3 -I rmnet_mhi0.1 8.8.8.8
            if [ ! "$?" == "0" ];then
                echo "5G Connect faile!!!"
	    	Switching_mode "$device_dir"
	        if [ -e "$DIRECTORY/$Serial_port" ];then
		        if [ -e "/dev/mhi_BHI" ];then
		 	    echo "pcie is ok!!!"
		            /usr/bin/quectel-CM -i rmnet_mhi0 -n pdn >> /tmp/5G.log 2>&1 &
			    sleep 2
		    	else
			    echo -e "AT+QCFG=\"data_interface\",1,0\r\n" > $DIRECTORY/$Serial_port
			    sleep 2
			    exit 0
		        fi
	        else
		        echo "The Serial_port not found!!!"
			exit 0
	        fi
	    else
		 echo "5G Connect success!!!"
	    fi
        fi

	if [ "$product_id" = "0900" ]; then
	    echo "This is a RM500U module!"
	    ping -c 2 -W 3 -I pcie0 8.8.8.8
                if [ ! "$?" == "0" ];then
                    echo "RM500U Connect faile!!!"
		    Switching_mode "$device_dir"
		    if [ -e "$DIRECTORY/$Serial_port" ];then
			    echo -e "AT+QCFG=\"pcie/mode\",0\r\n" > $DIRECTORY/$Serial_port
			    sleep 2
			    /usr/bin/quectel-CM >> /tmp/5G.log 2>&1 &
		    else
			    echo "The Serial_port not found!!!"
			    exit 0
		    fi
	       else
		     echo "RM500U Connect success!!!"
		fi
	fi
   fi
done




