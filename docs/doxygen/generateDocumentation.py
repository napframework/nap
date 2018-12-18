#!/usr/bin/env python
import os
import subprocess
from sys import platform
import shutil
import distutils.dir_util
import shutil
import sys

# some globals
CONFIG_FILE = "/nap.clang-format"
CONTENT_DIR = "/content"
OUTPUT_DIR = "/../html"
SEARCH_TARGET = OUTPUT_DIR + "/search/search.css"
SEARCH_SOURCE = "/css/search.css"

# errors
ERROR_INVALID_NAP_VERSION = 2

def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd, shell=True)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out


def call_collecting_output(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd, shell=True, stdout=subprocess.PIPE)
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
	doxy_conf = getWorkingDir() + CONFIG_FILE
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


def populateNAPVersionToEnvVars():
    """Populate NAP framework version from cmake/version.cmake to environment variables NAP_VERSION_FULL and NAP_VERSION_MAJOR"""

    # Fetch version from version.cmake
    version_file = os.path.join(getWorkingDir(), os.pardir, os.pardir, 'cmake/version.cmake')
    version_unparsed = call_collecting_output(getWorkingDir(), 'cmake -P %s' % version_file)
    chunks = version_unparsed.decode('ascii', 'ignore').split(':')
    if len(chunks) < 2:
        print("Error passing invalid output from version.cmake: %s" % version_unparsed)
        sys.exit(ERROR_INVALID_NAP_VERSION)
    version = chunks[1].strip()
    os.environ["NAP_VERSION_FULL"] = version
    os.environ["NAP_VERSION_MAJOR"] = '.'.join(version.split('.')[:-1])

# main run
if __name__ == '__main__':

	# find doxygen executable
    doxy_path = getDoxygenPath()

    # find doxygen exe script
    doxy_conf = getDoxygenConfigFile()

    # create subprocess arguments
    doxy_arg = "%s %s" % (doxy_path, doxy_conf)

    # populate NAP framework version from cmake/version.cmake to environment variables NAP_VERSION_FULL (eg. 0.1.0)
    # and NAP_VERSION_MAJOR (eg. 0.1) accessible in doxygen markdown like $(NAP_VERSION_FULL), $(NAP_VERSION_MAJOR)
    populateNAPVersionToEnvVars()

    # generate docs
    call(getWorkingDir(), doxy_arg)

    # copy content
    copyContent()

    # copy search
    copySearch()




