#!/bin/sh
module="useless"
device="useless"
mode="664"
# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/rmmod ./$module.ko
/sbin/insmod ./$module.ko $* || exit 1
# remove stale nodes
rm -f ${device}
line=$(cat /proc/devices | grep useless)
set $line
major=$1
mknod ${device} c $major 1
