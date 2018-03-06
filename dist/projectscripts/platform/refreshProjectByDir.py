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
    if not sys.platform.startswith('linux'):    
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")       
        parser.add_argument("-np", "--no-pause", action="store_true",
                            help="Don't pause afterwards")
    args = parser.parse_args()

    project_name = os.path.basename(args.PROJECT_PATH.strip('\\'))
    nap_root = os.path.abspath(os.path.join(args.PROJECT_PATH, os.pardir, os.pardir))
    script_path = os.path.join(nap_root, 'tools', 'platform', 'refreshProjectByName.py')

    # If we're on Windows or macOS and we're generating a solution for the first time show the generated solution
    show_solution = sys.platform in ('win32', 'darwin') and not args.no_show

    # TODO Discuss re-enabling logic which only shows the generated solution if it appears to be being generated for the first time.  
    #      Remove if not keeping.
    # if sys.platform == 'darwin':
    #     if os.path.exists(os.path.join(args.PROJECT_PATH, 'xcode')):
    #         show_solution = True
    # elif sys.platform == 'win32':
    #     if os.path.exists(os.path.join(args.PROJECT_PATH, 'msvc64')):
    #         show_solution = True

    # Determine our Python interpreter location
    if sys.platform == 'win32':
        python = os.path.join(nap_root, 'thirdparty', 'python', 'python')
    else:
        python = os.path.join(nap_root, 'thirdparty', 'python', 'bin', 'python3')

    cmd = [python, script_path, project_name] 
    if not show_solution and not sys.platform.startswith('linux'):
        cmd.append('--no-show')
    call(cmd)

    # Pause to display output in case we're running from a file manager on Explorer / Finder
    # TODO Ideally work out if we're running from a terminal and don't ever pause if we are
    # TODO Discuss the possibility to only pause for input if we've hit an issue
    # TODO If we think it's a common use case that people are running this from a file manager in Linux remove the Linux criteria
    if not sys.platform.startswith('linux') and not args.no_pause:
        print("Press key to close...")

        # Read a char from console
        getch()
