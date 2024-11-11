#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys

from nap_shared import read_console_char, get_python_path, get_nap_root

DEFAULT_BUILD_TYPE = 'Release'

def build_app_by_dir(app_path, build_type, pause_on_failed_build):
    app_name = os.path.basename(os.path.abspath(app_path.strip('\\')))
    nap_root = get_nap_root()
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'cli_single_app_build', 'cli_single_app_build.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Create command to call
    cmd = [python, script_path, app_name]
    cmd.append('--build-type=%s' % build_type)

    # Run
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and pause_on_failed_build:
        print("Press key to close...")

        # Read a char from console
        read_console_char()

    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='build')
    parser.add_argument("APP_PATH", type=str, help=argparse.SUPPRESS)
    parser.add_argument('-t', '--build-type',
        type=str,
        default=DEFAULT_BUILD_TYPE,
        action='store', nargs='?',
        choices=['Release', 'Debug'],
        help="Build type, default: {0}".format(DEFAULT_BUILD_TYPE))

    if not sys.platform.startswith('linux'):
        parser.add_argument("-np", "--no-pause", 
            action="store_true",
            help="Don't pause afterwards on failed build")
    args = parser.parse_args()

    pause = False
    if not sys.platform.startswith('linux') and not args.no_pause:
        pause = True
    exit_code = build_app_by_dir(args.APP_PATH, args.build_type, pause)

    # Expose exit code
    sys.exit(exit_code)
