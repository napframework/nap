#!/usr/bin/env python3
import argparse
import sys
import os
from subprocess import call

from nap_shared import find_app, get_python_path, get_nap_root

# Exit codes
ERROR_MISSING_APP = 1

def upgrade_app(app_name):
    # Find the app
    (app_path, _) = find_app(app_name)
    if app_path is None:
        return ERROR_MISSING_APP

    script_path = os.path.join(get_nap_root(), 'tools', 'buildsystem', 'app_and_module_updater', 'app_and_module_updater.py')

    python = get_python_path()
    cmd = [python, script_path, 'UPGRADE_APP', app_path]
    return call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("APP_NAME", type=str, help="The app to upgrade")
    args = parser.parse_args()
    exit_code = upgrade_app(args.APP_NAME)
    sys.exit(exit_code)
