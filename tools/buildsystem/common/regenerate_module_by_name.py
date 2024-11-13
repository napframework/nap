#!/usr/bin/env python3
import argparse
import sys
import os
from subprocess import Popen, call

from nap_shared import find_user_module, get_cmake_path, get_build_context, get_nap_root, get_python_path, BuildType, Platform, get_default_build_dir_name, get_default_generator

# Exit codes
ERROR_MISSING_APP = 1
ERROR_INVALID_APP_JSON = 2
ERROR_INVALID_BUILD_TYPE = 3
ERROR_CONFIGURE_FAILURE = 4

def update_module_framework_release(module_name, build_type):
    # If module name isn't prefixed with nap prepend it
    if not module_name.startswith('nap'):
        module_name = f'nap{module_name}'

    # Find the module
    module_path = find_user_module(module_name)
    if module_path is None:
        return ERROR_MISSING_APP

    # Cmake generate command
    build_dir = get_default_build_dir_name()
    cmd = '%s -H. -B%s -G\"%s\"' % (get_cmake_path(), 
        get_default_build_dir_name(), 
        get_default_generator())

    # Add build config if selected or default
    if build_type:
        cmd += ' -DCMAKE_BUILD_TYPE=%s' % build_type

    # Generate solution for individual platforms
    exit_code = call(cmd, cwd=module_path)
    if exit_code != 0:
        return ERROR_CONFIGURE_FAILURE

    print("Solution generated in %s" % os.path.relpath(os.path.join(module_path, build_dir)))
    return exit_code

def update_module_source(build_type):
    nap_root = get_nap_root()
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'generate_solution', 'generate_solution.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Build our command
    cmd = [python, script_path]
    if build_type:
        cmd.append('-t')
        cmd.append(build_type)

    # Call solution generation
    return call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    
    parser.add_argument("MODULE_NAME", type=str, help="The module to regenerate")
    parser.add_argument('-t', '--build-type',
        type=str,
        default=None,
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type for single solution generators such as Makefile, default: {0}".format(BuildType.get_default()))

    # Select system default build type if build_type is not provided
    # On linux the default is make -> single generator, on windows it's visual studio -> multi-generator
    args = parser.parse_args()
    build_type = args.build_type
    if Platform.get() == Platform.Linux and build_type is None:
        build_type = BuildType.get_default()

    if get_build_context() == 'framework_release':
        exit_code = update_module_framework_release(args.MODULE_NAME, build_type)
    else:
        exit_code = update_module_source(build_type)

    sys.exit(exit_code)
