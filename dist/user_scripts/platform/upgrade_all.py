#!/usr/bin/python
import argparse
import sys
import os
from subprocess import call

from nap_shared import get_python_path

def upgrade_all():
    script_path = os.path.join(os.path.dirname(__file__), 'project_and_module_updater.py')
    python = get_python_path()
    cmd = [python, script_path, 'UPGRADE_ALL'] 
    return call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    args = parser.parse_args()
    exit_code = upgrade_all()
    sys.exit(exit_code)
