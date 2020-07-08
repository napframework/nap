#!/usr/bin/env python

import argparse
import os
from subprocess import call
import sys 

from nap_shared import read_console_char, get_python_path

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='regenerate')
    parser.add_argument("PROJECT_PATH", type=str, help=argparse.SUPPRESS)
    if sys.platform.startswith('linux'):    
        parser.add_argument('BUILD_TYPE', nargs='?', default='Debug')
    else:    
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")       
        parser.add_argument("-np", "--no-pause", action="store_true",
                            help="Don't pause afterwards on failed generation")
    args = parser.parse_args()

    project_name = os.path.basename(args.PROJECT_PATH.strip('\\'))
    nap_root = os.path.abspath(os.path.join(args.PROJECT_PATH, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'platform', 'regenerate_project_by_name.py')

    # If we're on Windows or macOS and we're generating a solution for the first time show the generated solution
    show_solution = sys.platform in ('win32', 'darwin') and not args.no_show

    # Determine our Python interpreter location
    python = get_python_path()

    cmd = [python, script_path, project_name] 
    # Add our build type for Linux
    if sys.platform.startswith('linux'):    
        cmd.append(args.BUILD_TYPE)
    # If we don't want to show the solution and we weren't not on Linux specify that
    if not show_solution and not sys.platform.startswith('linux'):
        cmd.append('--no-show')
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and not sys.platform.startswith('linux') and not args.no_pause:
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    # Expose exit code
    sys.exit(exit_code)
