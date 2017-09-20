import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil

WORKING_DIR = '.'

THIRDPARTY = 'thirdparty'
THIRDPARTY_DIR = '%s/../thirdparty' % WORKING_DIR
THIRDPARTY_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/thirdparty.git'
NAP_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/nap.git'
NAP_BRANCH = 'build'
BUILD_DIR = 'build'
CLEAN_BUILD = False


def isLocalGitRepo(d):
    if not os.path.exists(d): return False
    try:
        call(d, ['git', 'rev-parse'])
    except:
        return False
    return True


def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out


def installDependenciesLinux():
    d = WORKING_DIR
    call('.', ['sudo', 'apt-get', '--assume-yes', 'install',
               'cmake',
               'build-essential',
	       'python3-dev',
               'libsdl2-dev',
               'libglew-dev',
               'libassimp-dev',
               'libglm-dev',
               'libtclap-dev',
               'libfreeimage-dev',
               'ffmpeg'
               ])


def isBrewInstalled():
    try:
        return os.path.exists(call(WORKING_DIR, ['which', 'brew']))
    except:
        return False


def installDependenciesOSX():
    d = WORKING_DIR
    for pack in ['cmake', 'sdl2', 'glew', 'glm', 'assimp', 'tclap', 'ffmpeg']:
        try:
            call(d, ['brew', 'install', pack])
        except:
            print('Failed installing %s' % pack)


def installDependencies():
    print('Installing dependencies')
    if platform == "linux" or platform == "linux2":
        installDependenciesLinux()
    elif platform == "darwin":
        installDependenciesOSX()
    elif platform == "win32":
        # Windows...
        pass


def main(targets):
    # install osx / linux specific dependendies
    installDependencies()

    # generate solutions
    if platform in ["linux", "linux2", "darwin"]:
        call(WORKING_DIR, ['cmake', '-H.', '-B%s' % BUILD_DIR])
    else:
        bd = '%s/%s' % (WORKING_DIR, BUILD_DIR)
        
        # clear build directory when a clean build is required
        print(CLEAN_BUILD)
        if CLEAN_BUILD and os.path.exists(bd):
            shutil.rmtree(bd)

        # create dir if it doesn't exist
        if not os.path.exists(bd):
            os.makedirs(bd)

        # generate prject
        call(WORKING_DIR, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

    #copy targets
    build_targets = targets

    # add targets here
    # build_targets.append("hello")

    for t in targets:
        # osx / linux
        if platform in ["linux", "linux2", "darwin"]:
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['make', t, '-j%s' % cpu_count()])
        # windows
        else:
            d = WORKING_DIR
            call(d, ['cmake', '--build', BUILD_DIR, '--target', t])


# Extracts all targets from the command line input arguments, syntax is: target:project, ie: target:napcore
def extractTargets():
    targets = []
    for arg in sys.argv:
        # if the argument clean has been given, perform a clean build
        if arg == "clean":
            print("performing clean build")
            global CLEAN_BUILD
            CLEAN_BUILD = True
            continue

        # not a target
        if not "target" in arg:
            continue

        # try to split
        result = str.split(arg, ':')
        if len(result) == 1:
            print("invalid target: %s, can't be split using delimiter ':'" % arg)
            continue
        
        # add
        print("adding build target: %s" % result[1])
        targets.append(result[1])
    return targets


# main run
if __name__ == '__main__':

    # extract command line targets
    targets = extractTargets()

    # run main
    main(targets)
