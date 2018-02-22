#!/usr/bin/env python
import os
import subprocess
from sys import platform
import shutil
import distutils.dir_util
import shutil

# some globals
CONFIG_FILE = "/nap.clang-format"
CONTENT_DIR = "/content"
OUTPUT_DIR = "/../html"
SEARCH_TARGET = OUTPUT_DIR + "/search/search.css"
SEARCH_SOURCE = "/css/search.css"


def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd, shell=True)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out


# working dir = dir script resides in
def getWorkingDir():
	return os.path.dirname(os.path.realpath(__file__))


# path to doxygen executable, should be installed by homebrew or apt when running
# linux / osx. Binary is bundled for windows in repo
def getDoxygenPath():
	if platform in ["linux", "linux2", "darwin"]:
		return "doxygen"

	#windows
	doxy_path = getWorkingDir() + "/bin/doxygen.exe"
	if not os.path.exists(doxy_path):
		raise Exception("can't find doxygen executable: " + doxy_path)
	return doxy_path


# path to doxygen config file
def getDoxygenConfigFile():
	doxy_conf =  getWorkingDir() + CONFIG_FILE
	if not os.path.exists(doxy_conf):
		raise Exception("can't find doxygen configuration file: " + doxy_conf)
	return doxy_conf


# copy content
def copyContent():
	source = getWorkingDir() + CONTENT_DIR
	target = getWorkingDir() + OUTPUT_DIR + CONTENT_DIR
	print("copy: %s -> %s" % (source, target))
	try:
		distutils.dir_util.copy_tree(source, target)
	except Exception as error:
		print("unable to copy content, are you running as admin? %s" % error)


def copySearch():
	source = getWorkingDir() + SEARCH_SOURCE
	target = getWorkingDir() + SEARCH_TARGET
	print("copy: %s -> %s" % (source, target))
	try:
		shutil.copyfile(source, target)
	except Exception as error:
		print("unable to copy search stylesheet, are you running as admin? %s" % error)

# main run
if __name__ == '__main__':

	# find doxygen executable
    doxy_path = getDoxygenPath()

    # find doxygen exe script
    doxy_conf = getDoxygenConfigFile()

    # create subprocess arguments
    doxy_arg = "%s %s" % (doxy_path, doxy_conf)

    # generate docs
    call(getWorkingDir(), doxy_arg)

    # copy content
    copyContent()

    # copy search
    copySearch()




