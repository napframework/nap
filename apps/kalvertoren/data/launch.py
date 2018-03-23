import subprocess
import os
import sys

# THIS SCRIPT LAUNCHES THE APP AND KEEPS IT RUNNING
# ONLY WORKS WITH PACKAGED KALVERTOREN RELEASES

# constructs path and makes sure it is valid
def constructPath():

	# get directory that holds this script
	script_path = os.path.dirname(os.path.realpath(__file__))
	
	# Go up one dir and attache executable
	exe_path = "{0}/../{1}".format(script_path, "kalvertoren")
	if not os.path.exists(exe_path):
		raise Exception("Executable doesn't exist: {0}".format(exe_path))
	return exe_path

# run main call
def run(exePath):
	while(True):
		proc = subprocess.Popen(exePath)
		proc.wait()
		if(proc.returncode == 0):
			print("App {0} exited gracefully".format(exePath))
			break
		print("App {0} Crashed! ReturnCode: {1}".format(exePath, proc.returncode))


if __name__ == "__main__":
	print(os.path.realpath(__file__))

	# get app path
	exe_path = constructPath();

	# run forever, on a crash the app is restarted automatically
	# when quit gracefully the loop is cancelled
	#run(exe_path)
	run(exe_path)
