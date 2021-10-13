#!/usr/bin/env python3
import os
import subprocess
from platform import machine
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil

LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'Xcode'
MSVC_BUILD_DIR = 'msvc64'
THIRDPARTY = 'thirdparty'

def call(cwd, cmd, shell=False):
    print('Dir: %s' % cwd)
    print('Command: %s' % ' '.join(cmd))
    proc = subprocess.Popen(cmd, cwd=cwd, shell=shell)
    proc.communicate()
    if proc.returncode != 0:
        sys.exit(proc.returncode)

def get_cmake_path():
    """Fetch the path to the CMake binary"""

    cmake_thirdparty_root = os.path.join(os.pardir, THIRDPARTY, 'cmake')
    if platform.startswith('linux'):
        arch = machine()
        if arch == 'x86_64':
            return os.path.join(cmake_thirdparty_root, 'linux', 'x86_64', 'bin', 'cmake')
        elif arch == 'aarch64':
            return os.path.join(cmake_thirdparty_root, 'linux', 'arm64', 'bin', 'cmake')
        else:
            return os.path.join(cmake_thirdparty_root, 'linux', 'armhf', 'bin', 'cmake')
    elif platform == 'darwin':
        return os.path.join(cmake_thirdparty_root, 'macos', 'x86_64', 'bin', 'cmake')
    else:
        return os.path.join(cmake_thirdparty_root, 'msvc', 'x86_64', 'bin', 'cmake')

def get_nap_root():
    """Get absolute path to NAP root"""
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))    

def parse_command_arguments():
    """Extracts all targets from the command line input arguments
    syntax is: target:project, ie: target:napcore"""
    targets = []
    clean_build = False
    linux_build_type = 'debug'
    for arg in sys.argv:
        # If the argument clean has been given, perform a clean build
        if arg == "clean":
            print("Performing clean build")
            clean_build = True
            continue

        if arg == "linuxreleasebuild" and platform.startswith('linux'):
            print("Performing Linux release build")
            linux_build_type = 'release'
            continue

        # Not a target
        if not "target" in arg:
            continue

        # Try to split
        parts = str.split(arg, ':')
        if len(parts) == 1:
            print("Invalid target: %s, can't be split using delimiter ':'" % arg)
            continue

        # Add
        print("Adding build target: %s" % parts[1])
        targets.append(parts[1])
        
    if len(targets) == 0:
        print("Warning: No targets specified, building default targets. Use generate_solution if you don't want to build any targets.")

    return (targets, clean_build, linux_build_type)

def main(targets, clean_build, linux_build_type):
    build_dir = None
    if platform.startswith('linux'):
        build_dir = LINUX_BUILD_DIR
    elif platform == 'darwin':
        build_dir = MACOS_BUILD_DIR
    else:
        build_dir = MSVC_BUILD_DIR
    nap_root = get_nap_root()
    build_dir = os.path.join(nap_root, build_dir)

    # Clear build directory when a clean build is required
    if clean_build and os.path.exists(build_dir):
        shutil.rmtree(build_dir)
        
    # Generate solution
    if platform.startswith('linux'):
        rc = call(nap_root, ['./generate_solution.sh', '--build-path=%s' % build_dir, '-t', linux_build_type])
    elif platform == 'darwin':
        rc = call(nap_root, ['./generate_solution.sh', '--build-path=%s' % build_dir])
    else:
        rc = call(nap_root, ['generate_solution.bat', '--build-path=%s' % build_dir])        
        
    # Build
    if len(targets) > 0:
        for t in targets:
            if platform.startswith('linux'):
                # Linux
                call(build_dir, ['make', t, '-j%s' % cpu_count()])
            elif platform == 'darwin':
                # macOS
                call(build_dir, ['xcodebuild', '-project', 'NAP.xcodeproj', '-target', t, '-configuration', 'Debug'])
            else:
                # Windows
                cmake = get_cmake_path()
                call(nap_root, [cmake, '--build', build_dir, '--target', t])
    else:
        if platform.startswith('linux'):
            # Linux
            call(build_dir, ['make', '-j%s' % cpu_count()])
        elif platform == 'darwin':
            # macOS
            call(build_dir, ['xcodebuild', '-project', 'NAP.xcodeproj', '-configuration', 'Debug'])
        else:
            # Windows
            cmake = get_cmake_path()
            call(nap_root, [cmake, '--build', build_dir])

if __name__ == '__main__':
    if '--help' in sys.argv or '/?' in sys.argv:
        print("usage: build [clean] [target:first_target] [target:another_target]")
        print("")
        print("optional arguments:")
        print("  clean:                 Clean the build")
        print("  target:targetname:     Name of target to build, can be specified any number of times")
        if platform.startswith('linux'):
            print("  linuxreleasebuild:     Perform release build on Linux")
        
        sys.exit(0)

    # Extract command line targets
    (targets, clean_build, linux_build_type) = parse_command_arguments()

    # Run main
    main(targets, clean_build, linux_build_type)
