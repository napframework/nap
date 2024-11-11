#!/usr/bin/env python3
import argparse
import os
from subprocess import call
from sys import platform
import sys
import shutil

LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'Xcode'
MSVC_BUILD_DIR = 'msvc64'
DEFAULT_BUILD_TYPE = 'Release'

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_nap_root

def get_build_directory(forced_path, clean):
    if platform.startswith('linux'):
        build_loc = LINUX_BUILD_DIR
    elif platform.startswith('darwin'):
        build_loc = MACOS_BUILD_DIR
    else:
        build_loc = MSVC_BUILD_DIR

    build_dir = forced_path if forced_path else os.path.join(get_nap_root(), build_loc)
    if clean and os.path.exists(build_dir):
        print("Clearing: {}".format(build_dir))
        shutil.rmtree(build_dir)
    return build_dir


def list_generators():
    cmake = get_cmake_path()
    cmd = '%s --help' % cmake
    call(cmd, shell=True)


def generate(forced_path, enable_python, additional_dirs, build_type, clean, generator):
    cmake = get_cmake_path()
    nap_root = get_nap_root()
    
    # Get platform specific build directory
    build_dir = get_build_directory(forced_path, clean)

    # Cmake generate command
    cmd = '%s -H%s -B%s' % (cmake, nap_root, build_dir)

    # Add generator if selected
    if generator is not None:
        cmd += ' -G\"%s\"' % generator

    # Add build config if selected or default
    if build_type is not None:
        cmd += ' -DCMAKE_BUILD_TYPE=%s' % build_type

    # Add NAP specific options
    cmd += ' -DNAP_ENABLE_PYTHON=%s -DADDITIONAL_SUB_DIRECTORIES=%s' % (enable_python, additional_dirs)
    call(cmd, shell=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--build-path',
        type=str,
        default=None,
        action='store',
        help="Use custom build path instead of system default",
        metavar="'dir'")

    parser.add_argument('-c', '--clean',
        default=False,
        action='store_true',
        help="Clear build directory before generating solution")

    parser.add_argument('-l', '--list',
        default = False,
        action='store_true',
        help="List available CMake solution generators on this platform"
        )

    parser.add_argument('-g', '--generator',
        type=str,
        default=None,
        help="CMake solution generator to use, add '-l' to print a list of compatible generators",
        action='store',
        metavar="'CMake Generator'"
        )

    parser.add_argument('-t', '--build-type',
        type=str,
        default=None,
        action='store', nargs='?',
        choices=['Release', 'Debug'],
        help="Build type for single solution generators such as Makefile, default: {0}".format(DEFAULT_BUILD_TYPE))

    parser.add_argument('-p', '--enable-python', action="store_true",
        help="Enable python integration using pybind (deprecated)")

    parser.add_argument('-d', '--additional-dirs',
        nargs='+',
        type=str,
        default=[],
        help="List of additional sub directories to add to the build",
        metavar="'dirx' 'diry'")

    args = parser.parse_args()

    # Print list of available generators and return
    if args.list:
        list_generators()
        sys.exit()
 
    # Convert additional sub directories to CMake list type
    additional_dirs = ';'.join(args.additional_dirs)

    # Select system default build type if build_type is not provided
    # On linux the default is make -> single generator, on windows it's visual studio -> multi-generator
    build_type = args.build_type
    if platform.startswith('linux') and build_type is None:
        build_type = DEFAULT_BUILD_TYPE

    # Generate solution
    generate(args.build_path, int(args.enable_python), additional_dirs, build_type, args.clean, args.generator)
