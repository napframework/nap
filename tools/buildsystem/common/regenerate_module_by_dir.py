#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys 

from nap_shared import read_console_char, get_python_path

def regenerate_module_by_dir(module_path, linux_build_type=None):
    module_name = os.path.basename(module_path.strip('\\'))
    nap_root = os.path.abspath(os.path.join(module_path, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'common', 'regenerate_module_by_name.py')

    # Determine our Python interpreter location
    python = get_python_path()

    cmd = [python, script_path, module_name] 
    # Add our build type for Linux
    if linux_build_type != None:
        cmd.append(linux_build_type)
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and not sys.platform.startswith('linux'):
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='regenerate')
    parser.add_argument("MODULE_PATH", type=str, help=argparse.SUPPRESS)
    if sys.platform.startswith('linux'):    
        parser.add_argument('BUILD_TYPE', nargs='?', default='Release')
    args = parser.parse_args()

    linux_build_type = None
    if sys.platform.startswith('linux'):    
        linux_build_type = args.BUILD_TYPE
    exit_code = regenerate_module_by_dir(args.MODULE_PATH, linux_build_type)

    # Expose exit code
    sys.exit(exit_code)
