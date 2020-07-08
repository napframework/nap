#!/usr/bin/env python

import argparse
import os
from subprocess import call
import sys 

from nap_shared import read_console_char, get_python_path

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='regenerate')
    parser.add_argument("MODULE_PATH", type=str, help=argparse.SUPPRESS)
    if sys.platform.startswith('linux'):    
        parser.add_argument('BUILD_TYPE', nargs='?', default='Debug')
    args = parser.parse_args()

    module_name = os.path.basename(args.MODULE_PATH.strip('\\'))
    nap_root = os.path.abspath(os.path.join(args.MODULE_PATH, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'platform', 'regenerate_module_by_name.py')

    # Determine our Python interpreter location
    python = get_python_path()

    cmd = [python, script_path, module_name] 
    # Add our build type for Linux
    if sys.platform.startswith('linux'):    
        cmd.append(args.BUILD_TYPE)
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and not sys.platform.startswith('linux'):
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    # Expose exit code
    sys.exit(exit_code)
