#!/usr/bin/env python3

import os
import sys
import socket
import time
import subprocess

# address (most likely broadcast) of the backup timer
BACKUP_TIME_IP = "192.168.14.255"
TIMER_IS_BROADCAST = True

# address of the individual backup controllers
BACKUP_LIGH_IP = ["192.168.14.7", "192.168.14.8", "192.168.14.9"]
BACKUP_TIME_PORT = 9761
BACKUP_LIGH_PORT = 6467

# backup cmds
TIME_ON_CMD = "R\\07\\00!\\001\\01\\01\\a6"
LIGH_ON_CMD = "!Dag Verlichting#"

# turn backup systems on
def turnontimer():
	print("turning on backup timer: {0}:{1} {2}".format(BACKUP_TIME_IP, BACKUP_TIME_PORT, TIME_ON_CMD))
	nsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	if TIMER_IS_BROADCAST:
		nsocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
	nsocket.sendto(str.encode(TIME_ON_CMD), (BACKUP_TIME_IP, BACKUP_TIME_PORT))

# turn the lights on
def turnonlights():
	lsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	for light_ip in BACKUP_LIGH_IP:
		print("turning on light program: {0}:{1} {2}".format(light_ip, BACKUP_LIGH_PORT, LIGH_ON_CMD))
		lsocket.sendto(str.encode(LIGH_ON_CMD), (light_ip, BACKUP_LIGH_PORT))


if __name__ == "__main__":
	print(os.path.realpath(__file__))

	subprocess.call(['touch', '/opt/dump/lightservice.txt'])

	# turn on backup timer
	try:
		turnontimer()
	except Exception as err:
		print("Unable to turn on timer: {0}".format(err))

	# turn on backup lights
	try:
		turnonlights()
	except Exception as err:
		print("Unable to turn on lights: {0}".format(err))
