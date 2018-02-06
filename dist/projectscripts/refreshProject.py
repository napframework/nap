#!/usr/bin/python

import argparse
import sys
import os
import subprocess

if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
elif sys.platform == 'win32':
    BUILD_DIR = 'msvc64'
else:
    BUILD_DIR = 'build'

def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out

# TODO share with projectInfoParseToCMake
def find_project(project_name):
    script_path = os.path.realpath(__file__)
    # TODO clean up, use absolute path
    nap_root = os.path.join(os.path.dirname(script_path), '..')

    project_dir_name = project_name.lower()
    projects_root = os.path.join(nap_root, 'projects')
    project_path = os.path.join(projects_root, project_dir_name)
    examples_root = os.path.join(nap_root, 'examples')
    example_path = os.path.join(examples_root, project_dir_name)
    demos_root = os.path.join(nap_root, 'demos')
    demo_path = os.path.join(demos_root, project_dir_name)

    if os.path.exists(project_path):
        print("Found project %s at %s" % (project_name, project_path))
        return project_path
    elif os.path.exists(example_path):
        print("Found example %s at %s" % (project_name, example_path))
        return example_path
    elif os.path.exists(demo_path):
        print("Found demo %s at %s" % (project_name, demo_path))
        return demo_path
    else:
        print("Couldn't find project or example with name '%s'" % project_name)
        return None

def update_project(project_name, build_type, show_solution):
    # print project_name
    project_path = find_project(project_name)
    if project_path is None:
        return

    if sys.platform in ["linux", "linux2"]:
        call(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type])

        # Show in Nautilus?
        # Seems a bit pointless if we're not opening it in an IDE
        # if show_solution:
        #     subprocess.call(["nautilus -s %s > /dev/null 2>&1 &" % BUILD_DIR], shell=True)

    elif sys.platform == 'darwin':
        call(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])

        # Show in Finder
        if show_solution:
            xcode_solution_path = os.path.join(project_path, BUILD_DIR, '%s.xcodeproj' % project_name)
            subprocess.call(["open", "-R", xcode_solution_path])
    else:
        # create dir if it doesn't exist
        full_build_dir = os.path.join(project_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        # generate prject
        call(project_path, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

        # Show in Explorer
        if show_solution:
            msvc_solution_path = os.path.join(project_path, BUILD_DIR, '%s.sln' % project_name)
            subprocess.Popen(r'explorer /select,"%s"' % msvc_solution_path)

    print("Solution generated in %s" % os.path.relpath(os.path.join(project_path, BUILD_DIR)))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str,
                        help="The project to refresh")
    if sys.platform in ["linux", "linux2"]:
        parser.add_argument('BUILD_TYPE', nargs='?', default='Debug')
    if not sys.platform in ["linux", "linux2"]:
        parser.add_argument("-ds", "--dontshow", action="store_true",
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
        show_solution = not args.dontshow

    update_project(args.PROJECT_NAME, build_type, show_solution)