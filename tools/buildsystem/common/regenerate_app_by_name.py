#!/usr/bin/env python3

import argparse
import sys
import os
from subprocess import Popen, call

from nap_shared import find_app, get_cmake_path, get_build_context, get_nap_root, get_python_path, BuildType, Platform, get_default_build_dir_name, get_default_generator

# Exit codes
ERROR_MISSING_MODULE = 1
ERROR_INVALID_BUILD_TYPE = 2
ERROR_CONFIGURE_FAILURE = 3

def cmake_reconfigure_app_framework_release(search_app_name, build_type, show_solution):
    # Find the app
    (app_path, app_name) = find_app(search_app_name)
    if app_path is None:
        return ERROR_MISSING_MODULE

    # Create cmake command
    build_dir = get_default_build_dir_name()
    cmake = get_cmake_path()
    cmd = [cmake, '-H.', '-B%s' % build_dir, '-G%s' % get_default_generator()]

    # Add build config if selected or default
    if build_type:
        cmd.append('-DCMAKE_BUILD_TYPE=%s' % build_type)

    # Create dir if it doesn't exist (Windows only)
    if Platform.get() == Platform.Windows:
        full_build_dir = os.path.join(app_path, build_dir)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

    # Run solution generation -> bail on failure
    exit_code = call(cmd, cwd=app_path)
    if exit_code != 0:
        return ERROR_CONFIGURE_FAILURE

    # Show generated solution on supported platforms
    if show_solution and Platform.get() == Platform.Windows:
        msvc_solution_path = os.path.join(app_path, build_dir, '%s.sln' % app_name)
        call(r'explorer /select,"%s"' % msvc_solution_path)
    elif show_solution and Platform.get() == Platform.macOS:
        xcode_solution_path = os.path.join(app_path, build_dir, '%s.xcodeproj' % app_name)
        call(["open", "-R", xcode_solution_path])

    print("Solution generated in %s" % os.path.relpath(os.path.join(app_path, build_dir)))
    return 0

def cmake_reconfigure_app_source(build_type):
    nap_root = get_nap_root()
    script_path = os.path.join(nap_root, 'tools', 'buildsystem', 'generate_solution', 'generate_solution.py')

    # Determine our Python interpreter location
    python = get_python_path()

    # Build our command
    cmd = [python, script_path]
    if build_type:
        cmd.append('-t')
        cmd.append(build_type)
    return call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("APP_NAME", type=str, help="The app to regenerate")    
    parser.add_argument('-t', '--build-type',
        type=str,
        default=None,
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type for single solution generators such as Makefile, default: {0}".format(BuildType.get_default()))
    parser.add_argument("-ns", "--no-show", 
        action="store_true",
        help="Don't show the generated solution")

    # Parse arguments
    args = parser.parse_args()

    # Get build type for Linux
    build_type = args.build_type
    if Platform.get() == Platform.Linux and build_type is None:
        build_type = BuildType.get_default()

    build_ctx = get_build_context()
    if build_ctx == 'framework_release':
        exit_code = cmake_reconfigure_app_framework_release(args.APP_NAME, build_type, not args.no_show)
    else:
        exit_code = cmake_reconfigure_app_source(build_type)

    sys.exit(exit_code)
