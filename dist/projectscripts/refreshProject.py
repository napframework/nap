#!/usr/bin/python

import argparse
import sys
import os
from subprocess import Popen, call

from platform.NAPShared import find_project, call_except_on_failure


ERROR_MISSING_MODULE = 1

if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
elif sys.platform == 'win32':
    BUILD_DIR = 'msvc64'
else:
    BUILD_DIR = 'build'

def update_project(project_name, build_type, show_solution):
    project_path = find_project(project_name)
    if project_path is None:
        return ERROR_MISSING_MODULE

    if sys.platform in ["linux", "linux2"]:
        call_except_on_failure(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type])

        # Show in Nautilus?
        # Seems a bit pointless if we're not opening it in an IDE
        # if show_solution:
        #     call(["nautilus -s %s > /dev/null 2>&1 &" % BUILD_DIR], shell=True)

    elif sys.platform == 'darwin':
        call_except_on_failure(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])

        # Show in Finder
        if show_solution:
            xcode_solution_path = os.path.join(project_path, BUILD_DIR, '%s.xcodeproj' % project_name)
            call(["open", "-R", xcode_solution_path])
    else:
        # create dir if it doesn't exist
        full_build_dir = os.path.join(project_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        # generate prject
        call_except_on_failure(project_path, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

        # Show in Explorer
        if show_solution:
            msvc_solution_path = os.path.join(project_path, BUILD_DIR, '%s.sln' % project_name)
            call(r'explorer /select,"%s"' % msvc_solution_path)

    print("Solution generated in %s" % os.path.relpath(os.path.join(project_path, BUILD_DIR)))
    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str,
                        help="The project to refresh")
    if sys.platform in ["linux", "linux2"]:
        parser.add_argument('BUILD_TYPE', nargs='?', default='Debug')
    if not sys.platform in ["linux", "linux2"]:
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")       
    args = parser.parse_args()

    # If we're on Linux and we've specified a build type let's grab that, otherwise
    # default to debug
    if sys.platform in ["linux", "linux2"]:
        build_type = args.BUILD_TYPE
        print("Using build type '%s'" % build_type)
        show_solution = False
    else:
        build_type = None
        show_solution = not args.no_show

    exit_code = update_project(args.PROJECT_NAME, build_type, show_solution)
    sys.exit(exit_code)