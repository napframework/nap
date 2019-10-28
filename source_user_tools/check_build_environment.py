#!/usr/bin/env python
import os
from subprocess import call
from sys import platform
import sys

def check_environment():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_dir, os.pardir))
    if platform.startswith('linux'):
        script_path = os.path.join(nap_root, 'dist/linux/check_build_environment/check_build_environment_worker.py')
        call('%s %s %s' % (sys.executable, script_path, '--source'), shell=True)
    elif platform == 'darwin':
        script_path = os.path.join(nap_root, 'dist/macos/check_build_environment/check_build_environment')
        call('%s %s %s' % (sys.executable, script_path, '--source'), shell=True)
    else:
        script_path = os.path.join(nap_root, 'dist/win64/check_build_environment/check_build_environment.bat')
        call('%s %s' % (script_path, '--source'), shell=True)
        
if __name__ == '__main__':
    check_environment()
