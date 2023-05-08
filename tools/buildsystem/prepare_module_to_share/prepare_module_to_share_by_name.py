#!/usr/bin/env python3
import argparse
import sys
import os
from subprocess import call

script_dir = os.path.dirname(__file__)
nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir))
sys.path.append(os.path.join(nap_root, 'tools', 'buildsystem', 'common'))
from nap_shared import find_user_module, get_cmake_path, get_build_context, get_nap_root, get_python_path

def prepare_module(module_name, overwrite):
    nap_root = get_nap_root()
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'prepare_module_to_share', 'prepare_module_to_share_by_dir.py')

    # If module name isn't prefixed with nap prepend it
    if not module_name.startswith('nap'):
        module_name = f'nap{module_name}'

    # Find the module
    module_path = find_user_module(module_name)
    if module_path is None:
        return False

    # Determine our Python interpreter location
    python = get_python_path()

    # Build our command
    cmd = [python, script_path, module_path]
    if overwrite:
        cmd.append('--overwrite')
    return call(cmd) == 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str, help="The module name")
    parser.add_argument('--overwrite', action='store_true', help="Overwrite any existing output directory")
    args = parser.parse_args()

    if not prepare_module(args.MODULE_NAME, args.overwrite):
        sys.exit(1)
