#!/usr/bin/env python3
import os
from platform import machine
from multiprocessing import cpu_count
import shutil
import subprocess
from sys import platform
import sys
import argparse

LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'Xcode'
MSVC_BUILD_DIR = 'msvc64'
THIRDPARTY = 'thirdparty'
DEFAULT_BUILD_TYPE = 'Release'

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

def main(target, clean_build, build_type, enable_python):
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

    # Get arguments to generate solution
    solution_args = []
    if platform.startswith('linux'):
        solution_args = ['./generate_solution.sh', '--build-path=%s' % build_dir, '-t', build_type.lower()]
    elif platform == 'darwin':
        solution_args = ['./generate_solution.sh', '--build-path=%s' % build_dir]
    else:
        solution_args = ['generate_solution.bat', '--build-path=%s' % build_dir]

    # Enable python if requested 
    if enable_python:
        solution_args.append('-p')
    
    # Generate solution
    rc = call(nap_root, solution_args)
        
    # Build
    build_config = build_type.capitalize()
    if platform.startswith('linux'):
        # Linux
        call(build_dir, ['make', target, '-j%s' % cpu_count()])
    elif platform == 'darwin':
        # macOS
        call(build_dir, ['xcodebuild', '-project', 'NAP.xcodeproj', '-target', target, '-configuration', build_config])
    else:
        # Windows
        cmake = get_cmake_path()
        call(nap_root, [cmake, '--build', build_dir, '--target', target, '--config', build_config])

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str, help="The project name")
    parser.add_argument('-t', '--build-type', type=str.lower, default=DEFAULT_BUILD_TYPE,
            choices=['release', 'debug'], help="Build configuration (default=%s)" % DEFAULT_BUILD_TYPE.lower())
    parser.add_argument('-c', '--clean', default=False, action="store_true", help="Clean before build")
    parser.add_argument('-p', '--enable-python', action="store_true", help="Enable python integration using pybind (deprecated)")

    args = parser.parse_args()

    print("Project to build: {0}, clean: {1}, type: {2}".format(
        args.PROJECT_NAME, args.clean, args.build_type))

    # Run main
    main(args.PROJECT_NAME, args.clean, args.build_type, args.enable_python)
