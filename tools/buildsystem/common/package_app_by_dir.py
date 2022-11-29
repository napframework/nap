#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys

from nap_shared import read_console_char, get_python_path, get_nap_root

def package_project_by_dir(project_path, include_napkin, zip_package, show, pause_on_package):
    project_name = os.path.basename(project_path.strip('\\'))
    nap_root = get_nap_root()
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'common', 'package_app_by_name.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Build our command
    cmd = [python, script_path, project_name]
    if not include_napkin:
        cmd.append('--no-napkin')
    if not zip_package:
        cmd.append('--no-zip')
    if not show:
        cmd.append('--no-show')
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if pause_on_package:
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='package')
    parser.add_argument("PROJECT_PATH", type=str, help=argparse.SUPPRESS)
    parser.add_argument("-ns", "--no-show", action="store_true",
                        help="Don't show the generated package")
    parser.add_argument("-nn", "--no-napkin", action="store_true",
                        help="Don't include napkin")
    parser.add_argument("-nz", "--no-zip", action="store_true",
                        help="Don't zip package")  
    if not sys.platform.startswith('linux'):    
        parser.add_argument("-np", "--no-pause", action="store_true",
                            help="Don't pause afterwards")
    args = parser.parse_args()

    pause_on_package = False
    if not sys.platform.startswith('linux') and not args.no_pause:
        pause_on_package = True
    exit_code = package_project_by_dir(args.PROJECT_PATH, not args.no_napkin, not args.no_zip, not args.no_show, pause_on_package)

    # Expose exit code
    sys.exit(exit_code)
