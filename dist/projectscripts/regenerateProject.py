#!/usr/bin/python

import sys
import os
import subprocess

if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
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

def update_project(project_name):
    # print project_name
    project_path = find_project(project_name)
    if project_path is None:
        return

    if sys.platform in ["linux", "linux2"]:
        call(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR])
    elif sys.platform == 'darwin':
        call(project_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])
    else:
        # create dir if it doesn't exist
        if not os.path.exists(BUILD_DIR):
            os.makedirs(BUILD_DIR)

        # generate prject
        call(WORKING_DIR, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

if __name__ == '__main__':
    # TODO update, use option parser or similar
    if len(sys.argv) < 2:
        print 'usage: python updateProject.py PROJECT_NAME'
        sys.exit(1)

    project_name = sys.argv[1]
    update_project(project_name)
