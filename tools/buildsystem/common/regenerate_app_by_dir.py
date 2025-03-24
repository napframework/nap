#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys

from nap_shared import read_console_char, get_python_path, BuildType, Platform

def regenerate_app_by_dir(app_path, suppress_showing_solution, build_type, pause_on_failure):
    app_name = os.path.basename(app_path.strip('\\'))
    nap_root = os.path.abspath(os.path.join(app_path, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'common', 'regenerate_app_by_name.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Create cmd to call
    cmd = [python, script_path, app_name, '-t', build_type]

    # If we don't want to show the solution and we weren't not on Linux specify that
    if suppress_showing_solution:
        cmd.append('--no-show')
    exit_code = call(cmd)

    # Pause to display output in case we're running from Windows Explorer / macOS Finder
    if exit_code != 0 and pause_on_failure:
        print("Press key to close...")
        read_console_char()

    return exit_code

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='regenerate')
    parser.add_argument("APP_PATH", type=str, help=argparse.SUPPRESS)

    parser.add_argument('-t', '--build-type',
        type=str,
        default=BuildType.get_default(),
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type for single solution generators such as Makefile, default: {0}".format(BuildType.get_default()))
    parser.add_argument("-ns", "--no-show", 
        action="store_true",
        help="Don't show the generated solution")
    parser.add_argument("-np", "--no-pause", 
        action="store_true",
        help="Don't pause afterwards on failed generation")
    args = parser.parse_args()

    # Regenerate app
    exit_code = regenerate_app_by_dir(args.APP_PATH, args.no_show, args.build_type, not args.no_pause)

    # Expose exit code
    sys.exit(exit_code)
