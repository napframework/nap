#!/usr/bin/env python3
import argparse
import os
from platform import machine
from subprocess import call
from sys import platform
import sys

LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'Xcode'
MSVC_BUILD_DIR = 'msvc64'
CODEBLOCKS_BUILD_DIR = 'codeblocks'
DEFAULT_LINUX_BUILD_TYPE = 'Release'

def generate(forced_path, additional_dirs, linux_build_type):
    nap_root = get_nap_root()
    cmake = get_cmake_path()
        
    if platform.startswith('linux'):
        build_dir = forced_path if forced_path else os.path.join(nap_root, LINUX_BUILD_DIR)
        build_type = linux_build_type.lower().capitalize()
        call(['%s -H%s -B%s -DCMAKE_BUILD_TYPE=%s -DADDITIONAL_SUB_DIRECTORIES=%s' % (cmake, nap_root, build_dir, build_type, additional_dirs)], shell=True)
    elif platform == 'darwin':
        build_dir = forced_path if forced_path else os.path.join(nap_root, MACOS_BUILD_DIR)
        call(['%s -H%s -B%s -G Xcode -DADDITIONAL_SUB_DIRECTORIES=%s' % (cmake, nap_root, build_dir, additional_dirs)], shell=True)
    else:
        build_dir = forced_path if forced_path else os.path.join(nap_root, MSVC_BUILD_DIR)
        cmd = '%s -H%s -B%s -G "Visual Studio 16 2019" -DPYBIND11_PYTHON_VERSION=3.5 -DADDITIONAL_SUB_DIRECTORIES=%s' % (cmake, nap_root, build_dir, additional_dirs)
        call(cmd, shell=True)
    
def get_cmake_path():
    """Fetch the path to the CMake binary"""
    nap_root = get_nap_root()
    if platform.startswith('linux'):
        arch = machine()
        if arch == 'x86_64':
            cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'linux', 'x86_64', 'bin', 'cmake')
        elif arch == 'aarch64':
            cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'linux', 'arm64', 'bin', 'cmake')
        else:
            cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'linux', 'armhf', 'bin', 'cmake')
    elif platform == 'darwin':
        cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'macos', 'x86_64', 'bin', 'cmake')   
    else:
        cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'msvc', 'x86_64', 'bin', 'cmake')
    return cmake_path
    
def get_nap_root():
    """Get absolute path to NAP root"""
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))    
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-path', 
        type=str, 
        default=None,
        action='store',
        help="Force custom build path")

    if platform.startswith('linux'):
        parser.add_argument('-t', '--linux-build-type', 
            type=str,
            default=DEFAULT_LINUX_BUILD_TYPE,
            action='store', nargs='?',
            choices=['release', 'debug'],
            help="Linux build type (default: %s)" % DEFAULT_LINUX_BUILD_TYPE.lower())

    parser.add_argument('-d', '--additional-dirs', 
        nargs='+',
        type=str,
        default=[],
        help="List of additional sub directories to add to the build")

    # parse command line arguments
    args = parser.parse_args()

    # convert additional sub directories to CMAKE list type
    additional_dirs = ';'.join(args.additional_dirs)

    # get linux build type
    linux_build_type = args.linux_build_type if platform.startswith('linux') else None   

    # generate solution
    generate(args.build_path, additional_dirs, linux_build_type)
