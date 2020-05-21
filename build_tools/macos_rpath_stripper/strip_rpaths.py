#!/usr/bin/env python

import argparse
import os
from subprocess import Popen, PIPE, call
import sys

ERROR_MISSING_OBJECT = 1

def remove_rpaths(object_path, remove_path):
    # Ensure the object exists
    if not os.path.exists(object_path):
        sys.exit(ERROR_MISSING_OBJECT)

    # Use otool to list the existing RPATHs
    cmd = 'otool -l %s' % object_path
    p = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)
    (stdout, stderr) = p.communicate()
    if type(stdout) == bytes:
        stdout = stdout.decode('utf8')
        stderr = stderr.decode('utf8')

    lines = stdout.split('\n')
    i = 0
    # Iterate RPATHs, removing any path that start with the supplied path
    while i < len(lines):
        if lines[i].strip() == 'cmd LC_RPATH':
            i += 2
            lib = 'path'.join(lines[i].strip().split('path')[1:])
            lib = lib.split(' (offset')[0].strip()
            if lib.startswith(remove_path):
                cmd = 'install_name_tool -delete_rpath %s %s' % (lib, object_path)
                print("Removing %s from %s" % (lib, object_path))
                call(cmd, shell=True)
        i += 1

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('OBJECT_PATH', type=str,
                        help="The object to work on")
    parser.add_argument('REMOVE_PATH', type=str,
                        help="The path to remove (including children)")
    args = parser.parse_args()

    remove_rpaths(args.OBJECT_PATH, args.REMOVE_PATH)