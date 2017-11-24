import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil
import datetime
import json

WORKING_DIR = '.'

THIRDPARTY = 'thirdparty'
THIRDPARTY_DIR = '%s/../thirdparty' % WORKING_DIR
THIRDPARTY_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/thirdparty.git'
NAP_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/nap.git'
NAP_BRANCH = 'build'
BUILD_DIR = 'build'
PACKAGING_DIR = 'packaging'
ARCHIVING_DIR = 'archiving'
BUILDINFO_FILE = 'cmake/buildinfo.json'
PACKAGED_BUILDINFO_FILE = '%s/cmake/buildinfo.json' % PACKAGING_DIR
CLEAN_BUILD = False
PACKAGE = False


def isLocalGitRepo(d):
    if not os.path.exists(d): return False
    try:
        call(d, ['git', 'rev-parse'])
    except:
        return False
    return True


def call(cwd, cmd, capture_output=False):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    if capture_output:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE)
    else:
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
        # Package our build
        packaging_success = packageBuild()
        # TODO improve error propogation behaviour
        sys.exit(0 if packaging_success else 1)

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

    # Note: Packaging directly from Python for now.  CPack was investigated but it was looking difficult to make it work when
    # wanting to build multiple configurations at the same time.  If there was a reasonable CPack solution it feels like that 
    # would be cleaner than this.

    # Remove old packaging path if it exists
    if os.path.exists(PACKAGING_DIR):
        shutil.rmtree(PACKAGING_DIR, True)
    os.makedirs(PACKAGING_DIR)

    # Build package name

    if platform in ["linux", "linux2"]:
        # TODO
        package_filename = package_filename % ('Linux') + '.tbz2'

        # d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
        # call(d, ['make', t, '-j%s' % cpu_count()])
    # osx
    elif platform == 'darwin':
        # Do the build, per configuration, installing into our packaging path
        d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
        call(d, ['xcodebuild', '-configuration', 'Debug', '-target', 'install'])
        call(d, ['xcodebuild', '-configuration', 'Release', '-target', 'install'])

        # Generate Xcode projects for our examples
        # TODO temp list of examples to iterate over.  Later we should be able to just process the whole dir.
        # packaged_examples = ['tommy', 'rendertest']
        # TODO work out if we want to do this
        # for example in packaged_examples:
        #     d = '%s/%s/examples/%s' % (WORKING_DIR, PACKAGING_DIR, example)
        #     call(d, ['cmake', '-H.', '-Bxcode', '-G', 'Xcode'])

        # Fix our dylib paths so fbxconverter will run from released package
        # TODO push back into cmake
        d = '%s/%s' % (WORKING_DIR, PACKAGING_DIR)
        call(d, ['python', '../dist/osx/dylibpathfix.py', 'fbxconverter_pathfix'])

        # TODO remove unwanted files (eg. .DS_Store)
        
        package_filename = buildPackageFilenameAndBuildInfo('macOS')
        shutil.move(PACKAGING_DIR, package_filename)

        # Archive
        package_filename_with_ext = '%s.%s' % (package_filename, 'zip')
        print("Archiving to %s" % package_filename_with_ext)
        call(WORKING_DIR, ['zip', '-yr', package_filename_with_ext, package_filename])

        # Cleanup
        shutil.move(package_filename, PACKAGING_DIR)
        print("Done.")
    # windows
    else:
        call(WORKING_DIR, ['cmake', '--build', BUILD_DIR, '--target', 'install', '--config', 'Debug'])
        call(WORKING_DIR, ['cmake', '--build', BUILD_DIR, '--target', 'install', '--config', 'Release'])

        package_filename = buildPackageFilenameAndBuildInfo('Win64')

        # Rename our packaging dir to match the release
        shutil.move(PACKAGING_DIR, package_filename)

        # Create our archive dir, used to create a copy level folder within the archive
        if os.path.exists(ARCHIVING_DIR):
            shutil.rmtree(ARCHIVING_DIR, True)
        os.makedirs(ARCHIVING_DIR)
        archive_path = os.path.join(ARCHIVING_DIR, package_filename)
        shutil.move(package_filename, archive_path)

        # Create archive
        # package_filename_with_ext = '%s.%s' % (package_filename, 'zip')
        print("Archiving to %s.zip" % package_filename)
        shutil.make_archive(package_filename, 'zip', ARCHIVING_DIR)

        # Cleanup
        shutil.move(archive_path, PACKAGING_DIR)
        shutil.rmtree(ARCHIVING_DIR)
        print("Done.")

    return True

def buildPackageFilenameAndBuildInfo(platform):
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    # Update our JSON build info file and get the current version
    version = processBuildInfo(timestamp)
    package_filename = "NAP-%s-%s-%s" % (version, platform, timestamp)
    return package_filename

def processBuildInfo(timestamp):
    # Write build info back out with bumped build number
    with open(BUILDINFO_FILE) as json_file:
        build_info = json.load(json_file)

    # TODO add validation

    # Read our version
    version = build_info['version']

    # Bump build number
    if not 'buildNumber' in build_info:
        build_info['buildNumber'] = 0
    build_info['buildNumber'] += 1

    # Write build info back out with bumped build number
    with open(BUILDINFO_FILE, 'w') as outfile:
        json.dump(build_info, outfile, sort_keys=True, indent=2)

    # Add git revision and timestamp to build info and bundle into package
    # TODO add ability to not have git revision in release build
    git_revision = call(WORKING_DIR, ['git', 'rev-parse', 'HEAD'], True)
    build_info['gitRevision'] = git_revision.decode('ascii', 'ignore').strip()
    build_info['timestamp'] = timestamp
    with open(PACKAGED_BUILDINFO_FILE, 'w') as outfile:
        json.dump(build_info, outfile, sort_keys=True, indent=2)

    # Return version for population into package name
    return version
    
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
