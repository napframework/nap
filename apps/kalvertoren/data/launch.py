import subprocess
import os
import sys
import socket
import time

# THIS SCRIPT LAUNCHES THE APP AND KEEPS IT RUNNING
# ONLY WORKS WITH PACKAGED KALVERTOREN RELEASES

# address (most likely broadcast) of the backup timer
BACKUP_TIME_IP = "192.168.14.255"
TIMER_IS_BROADCAST = True

# address of the individual backup controllers
BACKUP_LIGH_IP = ["192.168.14.7", "192.168.14.8", "192.168.14.9"]
BACKUP_TIME_PORT = 9761
BACKUP_LIGH_PORT = 6467

# backup cmds
TIME_OFF_CMD = "R\\07\\00!\\001\\01\\01\\a5"
TIME_ON_CMD = "R\\07\\00!\\001\\01\\01\\a6"
LIGH_OFF_CMD = "!kunst#"

# constructs path and makes sure it is valid
def constructPath():

	# get directory that holds this script
	script_path = os.path.dirname(os.path.realpath(__file__))
	
	# Go up one dir and attache executable
	exe_path = "{0}/../{1}".format(script_path, "kalvertoren")
	if not os.path.exists(exe_path):
		raise Exception("Executable doesn't exist: {0}".format(exe_path))
	return exe_path


# turns backup systems off
def turnofftimer():

	# first send timer off command
	print("turning off backup timer: {0}:{1}, {2}".format(BACKUP_TIME_IP, BACKUP_TIME_PORT, TIME_OFF_CMD))
	nsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	if TIMER_IS_BROADCAST:
		nsocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
	nsocket.sendto(str.encode(TIME_OFF_CMD), (BACKUP_TIME_IP, BACKUP_TIME_PORT))


# turn backup systems on
def turnontimer():

	# send timer on command
	print("turning on backup timer: {0}:{1} {2}".format(BACKUP_TIME_IP, BACKUP_TIME_PORT, TIME_ON_CMD))
	nsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	if TIMER_IS_BROADCAST:
		nsocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
	nsocket.sendto(str.encode(TIME_ON_CMD), (BACKUP_TIME_IP, BACKUP_TIME_PORT))


# turn of all the light devices
def turnofflights():
	
	# now tell all the backup light devices to turn off
	lsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	for light_ip in BACKUP_LIGH_IP:
		print("turning off light program: {0}:{1} {2}".format(light_ip, BACKUP_LIGH_PORT, LIGH_OFF_CMD))
		lsocket.sendto(str.encode(LIGH_OFF_CMD), (light_ip, BACKUP_LIGH_PORT))


# run main call
def run(exePath):

	# turn off timers
	try:
		turnofftimer()
	except Exception as err:
		print("Unable to turn off timer: {0}".format(err))
		print("Not starting app!")
		return

	# turn off all existing lights
	try:
		turnofflights()
	except Exception as err:
		print("Unable to turn off lights: {0}".format(err))
		print("Not starting app!")

		# make sure the timer is back on!
		turnontimer()
		return

	# wait for the systems to respond
	time.sleep(5)

	# run the app
	while(True):
		proc = subprocess.Popen(exePath)
		proc.wait()
		if(proc.returncode == 0):
			print("App {0} exited gracefully".format(exePath))
			break
		if(proc.returncode == -6):
			print("Linux is being an ass, exiting...")
			break
		print("App {0} Crashed! ReturnCode: {1}".format(exePath, proc.returncode))

	# turn on backup lights
	try:
		turnontimer()
	except Exception as err:
		print("Unable to turn on timer: {0}".format(err))


if __name__ == "__main__":
	print(os.path.realpath(__file__))

	# get app path
	exe_path = constructPath();

	# run forever, on a crash the app is restarted automatically
	# when quit gracefully the loop is cancelled
	#run(exe_path)
	run(exe_path)
