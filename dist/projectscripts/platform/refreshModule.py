#!/usr/bin/python
import argparse
import sys
import os
from subprocess import Popen

from NAPShared import find_module, call_except_on_failure

# Exit codes
ERROR_MISSING_PROJECT = 1
ERROR_INVALID_PROJECT_JSON = 2

# Platform-specific build directories
if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
elif sys.platform == 'win32':
    BUILD_DIR = 'msvc64'
else:
    BUILD_DIR = 'build'

def update_module(module_name, build_type):
    # Find the module
    module_path = find_module(module_name)
    if module_path is None:
        return ERROR_MISSING_PROJECT

    if sys.platform.startswith('linux'):
        call_except_on_failure(module_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type])
    elif sys.platform == 'darwin':
        call_except_on_failure(module_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])
    else:
        # Create dir if it doesn't exist
        full_build_dir = os.path.join(module_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        call_except_on_failure(module_path, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

    print("Solution generated in %s" % os.path.relpath(os.path.join(module_path, BUILD_DIR)))
    return(0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str, help="The module to refresh")
    # Linux: if we've specified a build type let's grab that, otherwise default to debug
    if sys.platform.startswith('linux'):
        parser.add_argument('BUILD_TYPE', nargs='?', default='Debug')
    args = parser.parse_args()

    if sys.platform.startswith('linux'):
        build_type = args.BUILD_TYPE
        print("Using build type '%s'" % build_type)
    else:
        build_type = None

    exit_code = update_module(args.MODULE_NAME, build_type)
    sys.exit(exit_code)