#!/usr/bin/env python3

import argparse
import sys
import os
from subprocess import Popen, call

from nap_shared import find_project, get_cmake_path

# Exit codes
ERROR_MISSING_MODULE = 1
ERROR_INVALID_BUILD_TYPE = 2
ERROR_CONFIGURE_FAILURE = 3

# Platform-specific build directories
if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
elif sys.platform == 'win32':
    BUILD_DIR = 'msvc64'
else:
    BUILD_DIR = 'build_dir'

def cmake_reconfigure_project(project_name, build_type, show_solution):
    # Find the project
    project_path = find_project(project_name)
    if project_path is None:
        return ERROR_MISSING_MODULE

    cmake = get_cmake_path()
    if sys.platform.startswith('linux'):    
        exit_code = call([cmake, '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type], cwd=project_path)

        # Show in Nautilus?
        # Seems a bit pointless if we're not opening it in an IDE from the file browser
        # if show_solution:
        #     call(["nautilus -s %s > /dev/null 2>&1 &" % BUILD_DIR], shell=True)

    elif sys.platform == 'darwin':
        osx_arch = '-DCMAKE_OSX_ARCHITECTURES=x86_64'
        exit_code = call([cmake, '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode', osx_arch], cwd=project_path)

        # Show in Finder
        if exit_code == 0 and show_solution:
            xcode_solution_path = os.path.join(project_path, BUILD_DIR, '%s.xcodeproj' % project_name)
            call(["open", "-R", xcode_solution_path])
    else:
        # Create dir if it doesn't exist
        full_build_dir = os.path.join(project_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        # Generate project
        exit_code = call([cmake, '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 16 2019', '-DPYBIND11_PYTHON_VERSION=3.5'], cwd=project_path)

        # Show in Explorer
        if exit_code == 0 and show_solution:
            msvc_solution_path = os.path.join(project_path, BUILD_DIR, '%s.sln' % project_name)
            call(r'explorer /select,"%s"' % msvc_solution_path)

    if exit_code == 0:
        print("Solution generated in %s" % os.path.relpath(os.path.join(project_path, BUILD_DIR)))
        return 0
    else:
        return(ERROR_CONFIGURE_FAILURE)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str,
                        help="The project to regenerate")
    if sys.platform.startswith('linux'):    
        parser.add_argument('BUILD_TYPE', nargs='?', default='Debug')
    else:
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")       
    args = parser.parse_args()

    # If we're on Linux and we've specified a build type let's grab that, otherwise
    # default to debug
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
        show_solution = False
    else:
        build_type = None
        show_solution = not args.no_show

    exit_code = cmake_reconfigure_project(args.PROJECT_NAME, build_type, show_solution)
    sys.exit(exit_code)
