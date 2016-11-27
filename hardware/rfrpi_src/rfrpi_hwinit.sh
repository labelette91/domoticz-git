#!/bin/sh
# This script init the GPIO for RFRPI
# When the shield is not initialized, it emit on RF433 noise
# This can perturbate other devices
# This script must be launched at bootup
# It needs wiringPi

gpio mode 0 in
gpio mode 7 out
gpio mode 5 out
gpio mode 4 out
gpio write 7 0
gpio write 5 1
gpio write 4 0
