#!/usr/bin/env python3
import argparse
import os
from subprocess import call
from sys import platform
import sys

LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'Xcode'
MSVC_BUILD_DIR = 'msvc64'
CODEBLOCKS_BUILD_DIR = 'codeblocks'
DEFAULT_LINUX_BUILD_TYPE = 'Debug'

def generate(forced_path, linux_build_type, use_codeblocks):
    nap_root = get_nap_root()
    cmake = get_cmake_path()
        
    if use_codeblocks:
        build_dir = forced_path if forced_path else os.path.join(nap_root, CODEBLOCKS_BUILD_DIR)
        call(['%s -H%s -B%s -G "CodeBlocks - Unix Makefiles"' % (cmake, nap_root, build_dir)], shell=True)
        print("Warning: NAP support for Code::Blocks is experimental and unsupported")
    elif platform.startswith('linux'):
        build_dir = forced_path if forced_path else os.path.join(nap_root, LINUX_BUILD_DIR)
        build_type = linux_build_type.lower().capitalize()
        call(['%s -H%s -B%s -DCMAKE_BUILD_TYPE=%s' % (cmake, nap_root, build_dir, build_type)], shell=True)
    elif platform == 'darwin':
        build_dir = forced_path if forced_path else os.path.join(nap_root, MACOS_BUILD_DIR)
        call(['%s -H%s -B%s -G Xcode' % (cmake, nap_root, build_dir)], shell=True)
    else:
        build_dir = forced_path if forced_path else os.path.join(nap_root, MSVC_BUILD_DIR)
        cmd = '%s -H%s -B%s -G "Visual Studio 14 2015 Win64" -DPYBIND11_PYTHON_VERSION=3.5' % (cmake, nap_root, build_dir)
        call(cmd, shell=True)
    
def get_cmake_path():
    """Fetch the path to the CMake binary"""
    nap_root = get_nap_root()
    if platform.startswith('linux'):
        cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'linux', 'install', 'bin', 'cmake')
    elif platform == 'darwin':
        cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'osx', 'install', 'bin', 'cmake')   
    else:
        cmake_path = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake', 'msvc', 'install', 'bin', 'cmake')
    return cmake_path
    
def get_nap_root():
    """Get absolute path to NAP root"""
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))    
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-path', type=str,
                        default=None,
                        action='store',
                        help="Force custom build path")
    parser.add_argument('--codeblocks', action="store_true",
                        help="Experimental & unsupported: Generate Code::Blocks solution")
    if platform.startswith('linux'):
        parser.add_argument('-t', '--linux-build-type', type=str,
                            default=DEFAULT_LINUX_BUILD_TYPE,
                            action='store', nargs='?',
                            choices=['release', 'debug'],
                            help="Linux build type (default: %s)" % DEFAULT_LINUX_BUILD_TYPE.lower())
    args = parser.parse_args()
    
    linux_build_type = args.linux_build_type if platform.startswith('linux') else None   
    generate(args.build_path, linux_build_type, args.codeblocks)
