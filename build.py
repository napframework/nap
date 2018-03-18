import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil

WORKING_DIR = '.'

THIRDPARTY = 'thirdparty'
BUILD_DIR = 'build'
CLEAN_BUILD = False
LINUX_BUILD_TYPE = 'Debug'


def isLocalGitRepo(d):
    if not os.path.exists(d): return False
    try:
        call(d, ['git', 'rev-parse'])
    except Exception as e:
        return False
    return True

def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % ' '.join(cmd))
    proc = subprocess.Popen(cmd, cwd=cwd)
    proc.communicate()
    if proc.returncode != 0:
        sys.exit(proc.returncode)

def main(targets):
    # clear build directory when a clean build is required
    print('Clean? %s' % CLEAN_BUILD)
    if CLEAN_BUILD and os.path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)

    # generate solutions
    if platform.startswith('linux'):    
        call(WORKING_DIR, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % LINUX_BUILD_TYPE])
    elif platform == 'darwin':
        call(WORKING_DIR, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])
    else:
        # create dir if it doesn't exist
        if not os.path.exists(BUILD_DIR):
            os.makedirs(BUILD_DIR)

        # generate prject
        call(WORKING_DIR,
             ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

    for t in targets:

        if platform.startswith('linux'):
            # Linux
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['make', t, '-j%s' % cpu_count()])

        elif platform == 'darwin':
            # macOS
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['xcodebuild', '-project', 'Project.xcodeproj', '-target', t, '-configuration', 'Debug'])

        else:
            # Windows
            d = WORKING_DIR
            call(d, ['cmake', '--build', BUILD_DIR, '--target', t])


def parseCmdArgs():
    """Extracts all targets from the command line input arguments
    syntax is: target:project, ie: target:napcore"""
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
    targets_ = parseCmdArgs()

    # run main
    main(targets_)
