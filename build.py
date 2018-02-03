import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil
from threading import Thread
import time

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
    except Exception as e:
        return False
    return True


def readpipe(pipe, q):
    for line in iter(pipe.readline, b''):
        q.append((pipe, line))


def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % ' '.join(cmd))

    proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # store process output in queue as (pipe, line) and keep errors list
    q = []
    errors = []
    # kick off threads to consume process output streams
    tout = Thread(target=readpipe, args=(proc.stdout, q))
    terr = Thread(target=readpipe, args=(proc.stderr, q))
    tout.start()
    terr.start()

    def flushpipes():
        """Grab subprocess output from queue and redirect"""
        while q:
            pipe, line = q.pop(0)
            line = line.decode('utf-8')
            if pipe == proc.stdout:
                sys.stdout.write(line)
            elif pipe == proc.stderr:
                sys.stderr.write(line)
                errors.append(line)
        time.sleep(0.1)

    # gather and output
    while proc.poll() is None:
        flushpipes()

    tout.join()
    terr.join()

    sys.stdout.flush()
    sys.stderr.flush()

    # subprocess may have dumped lines while our loop was exiting
    flushpipes()

    # if there were problems, display summary and exit with subprocess' exit code
    if errors:
        sys.stderr.write('\n'
                         'Error Summary:\n'
                         '==============\n'
                         '\n'
                         '%s\n' % (''.join(errors)))
        sys.exit(proc.returncode)


def main(targets):
    # clear build directory when a clean build is required
    print('Clean? %s' % CLEAN_BUILD)
    if CLEAN_BUILD and os.path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)

    # generate solutions
    if platform in ["linux", "linux2"]:
        call(WORKING_DIR, ['cmake', '-H.', '-B%s' % BUILD_DIR])
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

        if platform in ["linux", "linux2"]:
            # Linux
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['make', t, '-j%s' % cpu_count()])

        elif platform == 'darwin':
            # OSX
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
