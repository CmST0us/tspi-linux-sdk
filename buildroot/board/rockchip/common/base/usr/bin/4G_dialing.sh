#!/bin/bash
DIRECTORY="/dev/serial/by-id"
Serial_port=""
counter=0

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

        if [ "$product_id" = "6002" ] || [ "$product_id" = "6001" ] || [ "$product_id" = "6005" ]; then
            echo "This is a EC200 module!"
            for interface in ${device_dir}/*/net/*; do
		name=$(basename $interface)
		echo "name:$name"
		    
	    done 
	    if [ ! -e "/sys/class/net/$name" ];then
            	echo "$name not found!!!"
		Switching_mode "$device_dir"
		if [ -e $DIRECTORY/$Serial_port ];then
                      echo -e "AT+QCFG=\"usbnet\",1" > $DIRECTORY/$Serial_port
                      sleep 1
                      /usr/bin/quectel-CM >> /tmp/4G.log 2>&1 &
                else
                      echo "The Serial_port not found!!!"
                fi

   	    else
        	echo "This is EC200 Connection test"
        	ping -c 2 -W 3 -I $name 8.8.8.8
        	if [ ! "$?" == "0" ];then
            	    echo "EC200 Connect faile!!!"
		    Switching_mode "$device_dir"
	            if [ -e $DIRECTORY/$Serial_port ];then
		         #echo -e "AT+QCFG=\"usbnet\",0" > $DIRECTORY/$Serial_port
		         sleep 1
		         /usr/bin/quectel-CM >> /tmp/4G.log 2>&1 &
	            else
		         echo "The Serial_port not found!!!"
	            fi

	    	 else
		    	echo "EC200 Connect success!"
		    	exit 1
                 fi
	     fi
        fi
   
	if [ "$product_id" = "0125" ];then
	    echo "This is a EC20 module!"
	    for interface in ${device_dir}/*/net/*; do
                name=$(basename $interface)
                echo "name:$name"

            done
            	if [ ! -e "/sys/class/net/$name" ];then
                    echo "$name not found!!!"
                    Switching_mode "$device_dir"
                    if [ -e $DIRECTORY/$Serial_port ];then
                           echo -e "AT+QCFG=\"usbnet\",0" > $DIRECTORY/$Serial_port
                           sleep 1
                           /usr/bin/quectel-CM >> /tmp/4G.log 2>&1 &
                    else
                           echo "The Serial_port not found!!!"
                    fi
                else
                    echo "This is EC20 Connection test"
                    ping -c 2 -W 3 -I $name 8.8.8.8
                        if [ ! "$?" == "0" ];then
                            echo "EC20 Connect faile!!!"
			    Switching_mode "$device_dir"
		 	    if [ -e $DIRECTORY/$Serial_port ];then
                        	  #echo -e "AT+QCFG=\"usbnet\",0" > $DIRECTORY/$Serial_port
                        	  sleep 1
                        	  /usr/bin/quectel-CM >> /tmp/4G.log 2>&1 &
                 	    else
                        	  echo "The Serial_port not found!!!"
                 	    fi
	    		else 
		    	    echo "EC20 Connect success!"
	    	    	    exit 1
		    	fi
                 fi
     	  fi
   fi
done
