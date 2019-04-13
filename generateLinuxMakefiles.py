#!/usr/bin/env python
import argparse
from subprocess import call
import sys

DEFAULT_BUILD_TYPE = 'Debug'
BUILD_DIR = 'build'

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-bt', '--build-type', type=str,
                        default=DEFAULT_BUILD_TYPE,
                        action='store', nargs='?',
                        choices=['release', 'debug'])
    args = parser.parse_args()

    call(['cmake -H. -B%s -DCMAKE_BUILD_TYPE=%s' % (BUILD_DIR, args.build_type.lower().capitalize())], shell=True)