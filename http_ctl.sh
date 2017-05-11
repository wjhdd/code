#!/bin/bash
LOCAL_PATH=$(pwd)
BIN=${LOCAL_PATH}/http
CONF=${LOCAL_PATH}/conf/http.conf

function http_start()
{
	[ -f http.pid ] &&{
		printf "http is exist...\n"
			return 
	}
	IP=`grep -E '^IP:' $CONF | awk -F: '{print $2}' `
		PORT=`grep -E '^PORT:' $CONF | awk -F: '{print $2}' `
		$BIN $IP $PORT
		pidof http >http.pid
		printf "http is runing...\n"
}

function http_stop()
{
	[ ! -f http.pid ] && {
		printf "http not is exist...\n"
			return 
	}
	PID=$(cat http.pid)
		kill -9 $PID
		rm -rf http.pid
		printf "http is dead...\n"
}

function usage()
{
	printf "%s [start(-s) | stop(-t) | restart(-rt)]\n" "$0"
}

[ $# -ne 1 ] &&{
		usage
		exit 1
}

case $1 in
		start|-s)
		http_start
		;;
		stop |-t)
		http_stop
		;;
		restart|-rt)
		http_stop
		http_start
		;;
		*)
		usage
		;;
esac

