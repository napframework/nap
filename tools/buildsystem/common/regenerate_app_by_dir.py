#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys

from nap_shared import read_console_char, get_python_path

def regenerate_app_by_dir(app_path, suppress_showing_solution, linux_build_type, pause_on_failure):
    app_name = os.path.basename(app_path.strip('\\'))
    nap_root = os.path.abspath(os.path.join(app_path, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'common', 'regenerate_app_by_name.py')

    # Determine our Python interpreter location
    python = get_python_path()

    cmd = [python, script_path, app_name]
    # Add our build type for Linux
    if linux_build_type != None:
        cmd.append(linux_build_type)
    # If we don't want to show the solution and we weren't not on Linux specify that
    if suppress_showing_solution:
        cmd.append('--no-show')
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and pause_on_failure:
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='regenerate')
    parser.add_argument("APP_PATH", type=str, help=argparse.SUPPRESS)
    if sys.platform.startswith('linux'):
        parser.add_argument('BUILD_TYPE', nargs='?', default='Release')
    else:
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")
        parser.add_argument("-np", "--no-pause", action="store_true",
                            help="Don't pause afterwards on failed generation")
    args = parser.parse_args()

    # If we're on Windows or macOS and we're generating a solution for the first time show the generated solution
    suppress_showing_solution = sys.platform in ('win32', 'darwin') and args.no_show

    linux_build_type = None
    if sys.platform.startswith('linux'):
        linux_build_type = args.BUILD_TYPE

    pause_on_failure = False
    if not sys.platform.startswith('linux') and not args.no_pause:
        pause_on_failure = True

    exit_code = regenerate_app_by_dir(args.APP_PATH, suppress_showing_solution, linux_build_type, pause_on_failure)

    # Expose exit code
    sys.exit(exit_code)
