#!/usr/bin/env python

import argparse
import os
from subprocess import call
import sys 

from nap_shared import read_console_char, get_python_path

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='build')
    parser.add_argument("PROJECT_PATH", type=str, help=argparse.SUPPRESS)
    parser.add_argument('-t', '--build-type', type=str.lower, default='debug',
            choices=['release', 'debug'], help="Build type (default debug)")
    if not sys.platform.startswith('linux'):    
        parser.add_argument("-np", "--no-pause", action="store_true",
                            help="Don't pause afterwards on failed generation")
    args = parser.parse_args()

    project_name = os.path.basename(os.path.abspath(args.PROJECT_PATH.strip('\\')))
    nap_root = os.path.abspath(os.path.join(args.PROJECT_PATH, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'platform', 'cli_single_project_build.py')

    # Determine our Python interpreter location
    python = get_python_path()

    cmd = [python, script_path, '--build-type=%s' % args.build_type, project_name]
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and not sys.platform.startswith('linux') and not args.no_pause:
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    # Expose exit code
    sys.exit(exit_code)
