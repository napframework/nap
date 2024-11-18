#!/usr/bin/env python3
import os
from multiprocessing import cpu_count
import shutil
import subprocess
import sys
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_nap_root, BuildType, Platform, get_default_build_dir, max_build_parallelization, get_system_generator

ERROR_CONFIGURE = 2

def call(cwd, cmd, shell=False):
    proc = subprocess.Popen(cmd, cwd=cwd, shell=shell)
    proc.communicate()
    if proc.returncode != 0:
        sys.exit(proc.returncode)

def main(target, clean_build, build_type, enable_python):
    # Solution generation cmd
    nap_root = get_nap_root()
    if Platform.get() == Platform.Windows:
        gen_cmd = ['{}\\generate_solution.bat'.format(get_nap_root())]
    else:
        gen_cmd = ['./generate_solution.sh']

    # Generate solution
    build_dir = get_default_build_dir()
    gen_cmd.extend(['--build-path=%s' % build_dir, '-t', build_type])
    if enable_python:
        gen_cmd.append('-p')
    if clean_build:
        gen_cmd.append('-c')
    call(nap_root, gen_cmd)
        
    # Build target
    build_cmd = [get_cmake_path(), '--build', build_dir, '--target', target]
    if not get_system_generator().is_single():
        build_cmd.extend(['--config', build_type])
    call(nap_root, max_build_parallelization(build_cmd))

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", 
        type=str, 
        help="The project name (default='all')", 
        default="all", 
        nargs="?")
    
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