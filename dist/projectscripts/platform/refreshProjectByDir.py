#!/usr/bin/env python

import argparse
import os
from subprocess import call
import sys 
import termios
import tty

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

    project_name = os.path.basename(args.PROJECT_PATH)
    nap_root = os.path.abspath(os.path.join(args.PROJECT_PATH, os.pardir, os.pardir))

    # If we're on Windows or macOS and we're generating a solution for the first time show the generated solution
    refresh_args = ''
    if sys.platform == 'darwin':
        if os.path.exists(os.path.join(args.PROJECT_PATH, 'xcode')):
            refresh_args = '--dont-show'
    elif sys.platform == 'win32':
        if os.path.exists(os.path.join(args.PROJECT_PATH, 'msvc64')):
            refresh_args = '--dont-show'

    call(["/usr/bin/env python %s/tools/refreshProject.py %s %s" % (nap_root, project_name, refresh_args)], shell=True)

    print("Press key to close...")

    # Read a char from console
    getchr()