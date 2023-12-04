#!/bin/sh
#
#
check_died() {
	while true;do
		pid=`pidof duer_linux alsa_audio_main_service`
		if [ "$pid" = "" ];then
			echo "dueros all died."
			break
		fi
		sleep 1
	done
}

enable_coredump() {
	if [ $dueros_debug -eq 1 ]; then
		#echo "/userdata/core-%s-%p-%e-%t" > /proc/sys/kernel/core_pattern
		echo "|/oem/core_helper %s %p %e %t" > /proc/sys/kernel/core_pattern
		ulimit -c unlimited
	fi
}

disable_coredump() {
	if [ $dueros_debug -eq 1 ]; then
		ulimit -S -c 0 > /dev/null 2>&1
	fi
}

case "$1" in
  start)
	echo "Starting $0..."

	# start audio preProcess
	ln -s /oem/baidu_spil_rk3308/* /data/ -f
	cd /data
	mkdir -p local/ipc

	enable_coredump

	./setup.sh
	./alsa_audio_main_service 6mic_loopback &

	# start dueros
	mkdir -p /data/duer/test && cd /data/duer
	ln -snf /oem/duer/* ./
	./duer_linux &
	;;
  stop)
	echo "Stop $0..."
	killall alsa_audio_main_service
	killall duer_linux
	;;
  restart|reload)
	killall alsa_audio_main_service
	killall duer_linux
	check_died
	cd /data && ./alsa_audio_main_service 6mic_loopback &
	cd /data/duer && ./duer_linux &
	;;
  *)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

exit $?
