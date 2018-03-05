#!/usr/bin/env python

import argparse
import os
from subprocess import call
import sys 

if sys.platform == 'win32':
    from msvcrt import getch
else:
    def getch():
        import tty, termios
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            return sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)            

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_PATH", type=str, help="Path to the project")
    args = parser.parse_args()

    project_name = os.path.basename(args.PROJECT_PATH.strip('\\'))
    nap_root = os.path.abspath(os.path.join(args.PROJECT_PATH, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'refreshProject.py')

    # If we're on Windows or macOS and we're generating a solution for the first time show the generated solution
    show_solution = False
    if sys.platform == 'darwin':
        if os.path.exists(os.path.join(args.PROJECT_PATH, 'xcode')):
            show_solution = True
    elif sys.platform == 'win32':
        if os.path.exists(os.path.join(args.PROJECT_PATH, 'msvc64')):
            show_solution = True

    # Determine our Python interpreter location
    if sys.platform == 'win32':
        python = os.path.join(nap_root, 'thirdparty', 'python', 'python')
    else:
        python = 'python'

    if show_solution:
        call([python, script_path, project_name,  '--no-show'])        
    else:
        call([python, script_path, project_name])

    print("Press key to close...")

    # Read a char from console
    getch()