#!/usr/bin/env python3
import argparse
import os
from subprocess import call
import sys
import shutil

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_nap_root, get_default_build_dir, get_system_generator, BuildType, Platform

def get_build_directory(forced_path, clean):
    build_dir = forced_path if forced_path else get_default_build_dir()
    if clean and os.path.exists(build_dir):
        print("Clearing: {}".format(build_dir))
        shutil.rmtree(build_dir)
    return build_dir

def list_generators():
    cmake = get_cmake_path()
    cmd = '%s --help' % cmake
    call(cmd, shell=True)

def generate(forced_path, enable_python, additional_dirs, build_type, clean):
    cmake = get_cmake_path()
    nap_root = get_nap_root()

    # Get platform specific build directory
    build_dir = get_build_directory(forced_path, clean)
    cmd = ['%s' % cmake,
                '-H%s' % nap_root,
                '-B%s' % build_dir,
                '-G%s' % str(get_system_generator())]

    # Add build config if selected or default
    if build_type:
        cmd.append('-DCMAKE_BUILD_TYPE=%s' % build_type)

    # Add NAP specific options
    cmd.append('-DNAP_ENABLE_PYTHON=%s' % enable_python)
    cmd.append('-DADDITIONAL_SUB_DIRECTORIES=%s' % additional_dirs)
    call(cmd, cwd=nap_root)

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

    parser.add_argument('-t', '--build-type',
        type=str,
        default=None,
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type for single solution generators such as Makefile, default: {0}".format(BuildType.get_default()))

    parser.add_argument('-p', '--enable-python', action="store_true",
        help="Enable python integration using pybind (deprecated)")

    parser.add_argument('-d', '--additional-dirs',
        nargs='+',
        type=str,
        default=[],
        help="List of additional sub directories to add to the build",
        metavar="'dirx' 'diry'")

    # Parse command line arguments
    args = parser.parse_args()
 
    # Convert additional sub directories to CMake list type
    additional_dirs = ';'.join(args.additional_dirs)

    # Force build type selection when generator is single
    build_type = args.build_type
    if not build_type and get_system_generator().is_single():
        build_type = BuildType.get_default()

    # Generate solution
    generate(args.build_path, int(args.enable_python), additional_dirs, build_type, args.clean)
