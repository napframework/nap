#!/usr/bin/env python3
import os
from platform import machine
from multiprocessing import cpu_count
import shutil
import subprocess
from sys import platform
import sys
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_nap_root, BuildType, Platform, get_default_build_dir

ERROR_CONFIGURE = 2

def call(cwd, cmd, shell=False):
    print('Dir: %s' % cwd)
    print('Command: %s' % ' '.join(cmd))
    proc = subprocess.Popen(cmd, cwd=cwd, shell=shell)
    proc.communicate()
    if proc.returncode != 0:
        sys.exit(proc.returncode)

def main(target, clean_build, build_type, enable_python):
    # Clear build directory when a clean build is required
    build_dir = get_default_build_dir()
    if clean_build and os.path.exists(build_dir):
        shutil.rmtree(build_dir)

    # Get arguments to generate solution
    solution_args = []
    if Platform.get() == Platform.Linux:
        solution_args = ['./generate_solution.sh', '--build-path=%s' % build_dir, '-t', build_type]
    elif Platform.get() == Platform.macOS:
        solution_args = ['./generate_solution.sh', '--build-path=%s' % build_dir]
    elif Platform.get() == Platform.Windows:
        solution_args = ['generate_solution.bat', '--build-path=%s' % build_dir]
    else:
        print("Error: Unsupported target platform")
        sys.exit(ERROR_CONFIGURE)

    # Enable python if requested 
    if enable_python:
        solution_args.append('-p')
    
    # Generate solution
    nap_root = get_nap_root()
    rc = call(nap_root, solution_args, True)
        
    # Build
    if Platform.get() == Platform.Linux:
        call(build_dir, ['make', target, '-j%s' % cpu_count()])
    elif Platform.get() == Platform.macOS:
        cmd = ['xcodebuild', '-project', 'NAP.xcodeproj', '-configuration', build_type]
        if target == 'all':
            cmd.append('-alltargets')
        else:
            cmd.extend(['-target', target])
        call(build_dir, cmd)
    elif Platform.get() == Platform.Windows:
        cmake = get_cmake_path()
        nap_root = get_nap_root()
        cmake = get_cmake_path()
        cmd = [cmake, '--build', build_dir, '--config', build_type]
        if target != 'all':
            cmd.extend(['--target', target])
        call(nap_root, cmd)

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str, help="The project name (default='all')", default="all", nargs="?")
    
    parser.add_argument('-t', '--build-type',
        type=str,
        default=BuildType.get_default(),
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type, default: {0}".format(BuildType.get_default()))
    
    parser.add_argument('-c', '--clean', 
        default=False, 
        action="store_true", 
        help="Clean before build")
    
    parser.add_argument('-p', '--enable-python', 
        action="store_true", 
        help="Enable Python integration using pybind (deprecated)")

    args = parser.parse_args()

    print("Project to build: {0}, clean: {1}, type: {2}".format(
        args.PROJECT_NAME, args.clean, args.build_type))

    # Run main
    main(args.PROJECT_NAME, args.clean, args.build_type, args.enable_python)