#!/usr/bin/env python3
import argparse
import sys
import os
from subprocess import call

from nap_shared import find_user_module, get_python_path

# Exit codes
ERROR_MISSING_MODULE = 1

def upgrade_module(module_name):
    # If module name isn't prefixed with nap prepend it
    if not module_name.startswith('nap'):
        module_name = f'nap{module_name}'

    # Find the module
    module_path = find_user_module(module_name)
    if module_path is None:
        return ERROR_MISSING_MODULE

    script_path = os.path.join(os.path.dirname(__file__), os.pardir, 'app_and_module_updater', 'app_and_module_updater.py')

    python = get_python_path()
    cmd = [python, script_path, 'UPGRADE_MODULE', module_path]
    return call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str, help="The module to upgrade")
    args = parser.parse_args()
    exit_code = upgrade_module(args.MODULE_NAME)
    sys.exit(exit_code)
