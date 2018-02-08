#!/usr/bin/python

import argparse
import json
import os
import sys

PROJECT_INFO_FILENAME = 'project.json'
PROJECT_INFO_CMAKE_CACHE_FILENAME = 'cached_project_json.cmake'

def update_project_info_to_cmake(project_name):
    project_path = find_project(project_name)
    if project_path is None:
        return False

    output_filename = os.path.join(project_path, PROJECT_INFO_CMAKE_CACHE_FILENAME)

    # If any existing output file exists remove it.  This ensure that CMake will fail if our JSON parsing etc fails.
    if os.path.exists(output_filename):
        os.remove(output_filename)

    with open(os.path.join(project_path, PROJECT_INFO_FILENAME)) as json_file:
        json_dict = json.load(json_file)
        if not 'modules' in json_dict:
            print("Missing element 'modules' in %s" % PROJECT_INFO_FILENAME)
            return False

        if not type(json_dict['modules']) is list:
            print("Element 'modules' in %s is not an array" % PROJECT_INFO_FILENAME)
            return False

        nap_modules = ' '.join(json_dict['modules'])
        print("Built modules: %s" % nap_modules)

    with open(output_filename, 'w') as out_file:
        # out_file.write("project(%s)\n" % project_name)
        out_file.write("set(NAP_MODULES %s)\n" % nap_modules)

# TODO share with refreshProject
def find_project(project_name):
    script_path = os.path.realpath(__file__)
    # TODO clean up, use absolute path
    nap_root = os.path.join(os.path.dirname(script_path), '../..')

    projects_root = os.path.join(nap_root, 'projects')
    project_path = os.path.join(projects_root, project_name)
    examples_root = os.path.join(nap_root, 'examples')
    example_path = os.path.join(examples_root, project_name)
    demos_root = os.path.join(nap_root, 'demos')
    demo_path = os.path.join(demos_root, project_name)

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


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('PROJECT_NAME')
    args = parser.parse_args()

    # print("Got project name: %s" % args.PROJECT_NAME)
    if not update_project_info_to_cmake(args.PROJECT_NAME):
        sys.exit(1)
