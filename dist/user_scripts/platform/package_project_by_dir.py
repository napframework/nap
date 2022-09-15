#!/usr/bin/env python3

import argparse
import os
from subprocess import call
import sys

from nap_shared import read_console_char, get_python_path

def package_project_by_dir(project_path, include_napkin, zip_package, show, bundle, codesign, pause_on_package):
    project_name = os.path.basename(project_path.strip('\\'))
    nap_root = os.path.abspath(os.path.join(project_path, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'platform', 'package_project_by_name.py')

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
    if bundle:
        cmd.append('--bundle')
        if not codesign is None:
            cmd.append('--codesign')
            cmd.append(codesign)
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
    if sys.platform.startswith('darwin'):
        parser.add_argument("-b", "--bundle", action="store_true",
                            help="Create macos app bundle")
        parser.add_argument("-cs", "--codesign", type=str, required=False,
                            help="Codesign app bundle")
    if not sys.platform.startswith('linux'):
        parser.add_argument("-np", "--no-pause", action="store_true",
                            help="Don't pause afterwards")
    args = parser.parse_args()

    pause_on_package = False
    if not sys.platform.startswith('linux') and not args.no_pause:
        pause_on_package = True
    bundle = False
    codesign = None
    if sys.platform.startswith('darwin'):
        if args.bundle:
            bundle = True
            codesign = args.codesign
            if args.no_zip:
                print("Warning: --no-zip argument will be ignored because MacOS app bundle output was chosen.")
        else:
            if not args.codesign is None:
                print("Warning: --codesign argument will be ignored because MacOS app bundle was not chosen.")
    exit_code = package_project_by_dir(args.PROJECT_PATH, not args.no_napkin, not args.no_zip, not args.no_show, bundle, codesign, pause_on_package)

    # Expose exit code
    sys.exit(exit_code)
