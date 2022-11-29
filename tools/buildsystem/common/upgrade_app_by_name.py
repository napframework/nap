#!/usr/bin/env python3
import argparse
import sys
import os
from subprocess import call

from nap_shared import find_project, get_python_path

# Exit codes
ERROR_MISSING_PROJECT = 1

def upgrade_project(project_name):
    # Find the project
    project_path = find_project(project_name)
    if project_path is None:
        return ERROR_MISSING_PROJECT

    script_path = os.path.join(os.path.dirname(__file__), 'project_and_module_updater.py')

    python = get_python_path()
    cmd = [python, script_path, 'UPGRADE_PROJECT', project_path] 
    return call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str, help="The project to upgrade")
    args = parser.parse_args()
    exit_code = upgrade_project(args.PROJECT_NAME)
    sys.exit(exit_code)
