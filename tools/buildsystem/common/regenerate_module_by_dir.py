#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys 

from nap_shared import read_console_char, get_python_path, BuildType, Platform, get_system_generator

def regenerate_module_by_dir(module_path, build_type):
    module_name = os.path.basename(module_path.strip('\\'))
    nap_root = os.path.abspath(os.path.join(module_path, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'common', 'regenerate_module_by_name.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Create command
    cmd = [python, script_path, module_name] 
    if build_type:
        cmd.append(build_type)
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and not Platform.get() == Platform.Linux:
        print("Press key to close...")
        read_console_char()
        
    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='regenerate')
    parser.add_argument("MODULE_PATH", type=str, help=argparse.SUPPRESS)
    parser.add_argument('-t', '--build-type',
        type=str,
        default=None,
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type for single solution generators such as Makefile, default: {0}".format(BuildType.get_default()))
    
    args = parser.parse_args()

    # Force build type selection when generator is single
    build_type = args.build_type
    if not build_type and get_system_generator().is_single():
        build_type = BuildType.get_default()

    exit_code = regenerate_module_by_dir(args.MODULE_PATH, build_type)
    sys.exit(exit_code)
