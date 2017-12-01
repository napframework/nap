#!/usr/bin/python

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

    projects_root = os.path.join(nap_root, 'projects')
    project_path = os.path.join(projects_root, project_name)
    examples_root = os.path.join(nap_root, 'examples')
    example_path = os.path.join(examples_root, project_name)

    if os.path.exists(project_path):
        print("Found project %s at %s" % (project_name, project_path))
        return project_path
    elif os.path.exists(example_path):
        print("Found example %s at %s" % (project_name, example_path))
        return example_path
    else:
        print("Couldn't find project or example with name '%s'" % project_name)
        return None

def update_project(project_name, build_type):
    # print project_name
    project_path = find_project(project_name)
    if project_path is None:
        return

    if sys.platform in ["linux", "linux2"]:
        call(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type])
    elif sys.platform == 'darwin':
        call(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])
    else:
        # create dir if it doesn't exist
        full_build_dir = os.path.join(project_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        # generate prject
        call(project_path, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

if __name__ == '__main__':
    # TODO update to use argparse

    if len(sys.argv) < 2:
        if sys.platform in ["linux", "linux2"]:
            usage_help = "Usage: %s PROJECT_NAME [BUILD_TYPE]"  % sys.argv[0]
        else:
            usage_help = "Usage: %s PROJECT_NAME" % sys.argv[0]

        print (usage_help)
        sys.exit(1)

    project_name = sys.argv[1]

    # If we're on Linux and we've specified a build type let's grab that, otherwise
    # default to debug
    if sys.platform in ["linux", "linux2"]:
        if len(sys.argv) == 3:
            build_type = sys.argv[2]
        else:
            build_type = 'Debug'
        print("Using build type '%s'" % build_type)
    else:
        build_type = None

    update_project(project_name, build_type)
