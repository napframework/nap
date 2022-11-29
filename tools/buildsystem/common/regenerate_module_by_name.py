#!/usr/bin/env python3
import argparse
import sys
import os
from subprocess import Popen, call

from nap_shared import find_module, get_cmake_path

# Exit codes
ERROR_MISSING_PROJECT = 1
ERROR_INVALID_PROJECT_JSON = 2
ERROR_INVALID_BUILD_TYPE = 3
ERROR_CONFIGURE_FAILURE = 4

# Platform-specific build directories
if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
elif sys.platform == 'win32':
    BUILD_DIR = 'msvc64'
else:
    BUILD_DIR = 'build_dir'

def update_module(module_name, build_type):
    # If module name is prefixed with mod_ remove it
    if module_name.startswith('mod_'):
        module_name = module_name[4:]
        
    # Find the module
    module_path = find_module(module_name)
    if module_path is None:
        return ERROR_MISSING_PROJECT
        
    cmake = get_cmake_path()
    if sys.platform.startswith('linux'):
        exit_code = call([cmake, '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type], cwd=module_path)
    elif sys.platform == 'darwin':
        exit_code = call([cmake, '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'], cwd=module_path)
    else:
        # Create dir if it doesn't exist
        full_build_dir = os.path.join(module_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        exit_code = call([cmake, '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 16 2019'], cwd=module_path)

    if exit_code == 0:
        print("Solution generated in %s" % os.path.relpath(os.path.join(module_path, BUILD_DIR)))
        return 0
    else:
        return ERROR_CONFIGURE_FAILURE

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str, help="The module to regenerate")
    # Linux: if we've specified a build type let's grab that, otherwise default to release
    if sys.platform.startswith('linux'):
        parser.add_argument('BUILD_TYPE', nargs='?', default='Release')
    args = parser.parse_args()

    if sys.platform.startswith('linux'):
        build_type = args.BUILD_TYPE.lower()
        if build_type == 'debug':
            build_type = 'Debug'
        elif build_type == 'release':
            build_type = 'Release'
        else:
            print("Error: Invalid build type '%s' (valid types are release and debug)" % build_type)
            sys.exit(ERROR_INVALID_BUILD_TYPE)

        print("Using build type '%s'" % build_type)
    else:
        build_type = None

    exit_code = update_module(args.MODULE_NAME, build_type)
    sys.exit(exit_code)
