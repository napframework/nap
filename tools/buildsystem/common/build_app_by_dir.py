#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys

from nap_shared import read_console_char, get_python_path, get_nap_root, BuildType

def build_app_by_dir(app_path, build_type, pause_on_failed_build):
    app_name = os.path.basename(os.path.abspath(app_path.strip('\\')))
    nap_root = get_nap_root()
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'cli_single_app_build', 'cli_single_app_build.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Create and run command to call
    cmd = [python, script_path, app_name, '-t', build_type]
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer
    if exit_code != 0 and pause_on_failed_build:
        print("Press key to close...")
        read_console_char()

    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='build')
    parser.add_argument("APP_PATH", type=str, help=argparse.SUPPRESS)
    parser.add_argument('-t', '--build-type',
        type=str,
        default=BuildType.get_default(),
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type, default: {0}".format(BuildType.get_default()))

    parser.add_argument("-np", "--no-pause", 
        action="store_true",
        help="Don't pause afterwards on failed build")

    args = parser.parse_args()
    exit_code = build_app_by_dir(args.APP_PATH, args.build_type, not args.no_pause)

    # Expose exit code
    sys.exit(exit_code)
