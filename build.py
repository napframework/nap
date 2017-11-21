import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil
import datetime

WORKING_DIR = '.'

THIRDPARTY = 'thirdparty'
THIRDPARTY_DIR = '%s/../thirdparty' % WORKING_DIR
THIRDPARTY_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/thirdparty.git'
NAP_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/nap.git'
NAP_BRANCH = 'build'
BUILD_DIR = 'build'
PACKAGING_DIR = 'packaging'
CLEAN_BUILD = False
PACKAGE = False


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

    # clear build directory when a clean build is required
    print(CLEAN_BUILD)
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
        call(WORKING_DIR, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

    if PACKAGE:
        sys.exit(packageBuild())

    #copy targets
    build_targets = targets

    # add targets here
    # build_targets.append("hello")

    for t in targets:
        # linux
        if platform in ["linux", "linux2"]:
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['make', t, '-j%s' % cpu_count()])
        # osx
        elif platform == 'darwin':
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['xcodebuild', '-project', 'Project.xcodeproj', '-target', t, '-configuration', 'Debug'])
        # windows
        else:
            d = WORKING_DIR
            call(d, ['cmake', '--build', BUILD_DIR, '--target', t])


def packageBuild():
    print("Packaging..")

    # Build package name
    timestamp = datetime.datetime.now().strftime('%d%m%YT%H%M%S')
    version = '0.1.0' # TODO pull from file
    # TODO add git revision
    package_filename = "NAP-%s-%%s-%s" % (version, timestamp)

    # Remove old packaging path if it exists
    if os.path.exists(PACKAGING_DIR):
        shutil.rmtree(PACKAGING_DIR)

    # TODO temp list of examples to iterate over.  Later we should be able to just process the whole dir.
    packaged_examples = ['tommy', 'rendertest']

    if platform in ["linux", "linux2"]:
        # TODO
        package_filename = package_filename % ('Linux') + '.tbz2'

        # d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
        # call(d, ['make', t, '-j%s' % cpu_count()])
    # osx
    elif platform == 'darwin':
        # Do the build, per configuration, installing into our packaging path
        d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
        # TODO add other configurations
        call(d, ['xcodebuild', '-configuration', 'Debug', '-target', 'install'])
        call(d, ['xcodebuild', '-configuration', 'Release', '-target', 'install'])
        # call(d, ['xcodebuild', '-configuration', 'MinSizeRel', '-target', 'install'])
        # call(d, ['xcodebuild', '-configuration', 'RelWithDebInfo', '-target', 'install'])

        # Generate Xcode projects for our examples
        # TODO work out if we want to do this
        # for example in packaged_examples:
        #     d = '%s/%s/examples/%s' % (WORKING_DIR, PACKAGING_DIR, example)
        #     call(d, ['cmake', '-H.', '-Bxcode', '-G', 'Xcode'])

        # Fix our dylib paths so fbxconverter will run from released package
        d = '%s/%s' % (WORKING_DIR, PACKAGING_DIR)
        call(d, ['python', '../dist/osx/dylibpathfix.py', 'fbxconverter_pathfix'])

        # TODO remove unwanted files (eg. .DS_Store)
        package_filename = package_filename % ('macOS')
        shutil.move(PACKAGING_DIR, package_filename)
        package_filename_with_ext =  '%s.%s' % (package_filename, 'zip')
        call(WORKING_DIR, ['zip', '-yr', package_filename_with_ext, package_filename])
        shutil.move(package_filename, PACKAGING_DIR)
        print "Packaged to %s" % package_filename_with_ext

    # windows
    else:
        # TODO
        package_filename = package_filename % ('Win64') + '.tbz2'

        # d = WORKING_DIR
        # call(d, ['cmake', '--build', BUILD_DIR, '--target', t])



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

        if arg == "package":
            print("Packaging NAP")
            global PACKAGE
            PACKAGE = True
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
