#!/bin/sh

if [ -f /etc/ppp/resolv.conf ]; then
	cp /etc/ppp/resolv.conf /etc/resolv.conf
elif [ -f /var/run/ppp/resolv.conf ]; then
	cp /var/run/ppp/resolv.conf /etc/resolv.conf
else
	echo nameserver $DNS1 > /etc/resolv.conf
	echo nameserver $DNS2 >> /etc/resolv.conf
fi
