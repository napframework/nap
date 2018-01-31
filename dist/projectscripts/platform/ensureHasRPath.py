#!/usr/bin/env python
import os
from subprocess import Popen, PIPE
import sys

ERROR_MISSING_INPUT = 1
ERROR_BAD_FILE_PATH = 2

# Add the RPATH, returning a clean exit code to CMake by ignoring any pre-existing duplicate RPATH
def add_rpath(rpath, file_path):
    p = Popen(['/usr/bin/install_name_tool', '-add_rpath', rpath ,file_path], stdout=PIPE, stderr=PIPE)
    p.wait()

if __name__ == '__main__':
    # TODO use argparse

    if len(sys.argv) != 3:
        print("Usage: %s FILE RPATH" % sys.argv[0])
        sys.exit(ERROR_MISSING_INPUT)

    file_path = sys.argv[1]
    if not os.path.exists(file_path):
        print("Error: File %s does not exist" % file_path)
        sys.exit(ERROR_BAD_FILE_PATH)

    add_rpath(sys.argv[2], file_path)