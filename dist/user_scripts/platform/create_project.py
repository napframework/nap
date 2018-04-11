#!/usr/bin/python
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import find_project, validate_camelcase_name

# Default modules if none are specified
DEFAULT_MODULE_LIST = "mod_naprender,mod_napmath,mod_napinput,mod_napsdlinput,mod_napsdlwindow,mod_napapp,mod_napscene,mod_napimgui,mod_napaudio"

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_PROJECT = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

def create_project(project_name, module_list, generate_solution):
    print("Creating project %s" % project_name)

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.join(script_path, os.pardir, os.pardir)

    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake/project_creator'))
    project_path = os.path.abspath(os.path.join(nap_root, 'projects/%s' % project_name.lower()))

    # Check for duplicate project
    if not find_project(project_name, True) is None:
        print("Error: demo, example or project exists with same name '%s'" % project_name)
        return ERROR_EXISTING_PROJECT

    # Create project from template
    input_module_list = module_list.lower().replace(',', ';')
    cmd = ['cmake', '-DPROJECT_NAME_CAMELCASE=%s' % project_name, '-DMODULE_LIST=%s' % input_module_list, '-P', 'project_creator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        print("Project creation failed")
        return ERROR_CMAKE_CREATION_FAILURE

    # Solution generation
    if generate_solution:
        print("Project created")
        print("Generating solution")

        # Determine our Python interpreter location
        if sys.platform == 'win32':
            python = os.path.join(nap_root, 'thirdparty', 'python', 'python')
        else:
            python = os.path.join(nap_root, 'thirdparty', 'python', 'bin', 'python3')
                
        cmd = [python, './tools/platform/regenerate_project_by_name.py', project_name]
        if call(cmd, cwd=nap_root) != 0:
            print("Solution generation failed")
            return ERROR_SOLUTION_GENERATION_FAILURE
    else:
        print("Project created in %s" % os.path.relpath(project_path))

    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("CAMEL_CASE_PROJECT_NAME", type=str,
                        help="The project name, in camel case (eg. MyProjectName)")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created project")       
    args = parser.parse_args()

    project_name = args.CAMEL_CASE_PROJECT_NAME

    # Validate module name only includes valid characters.  For now we're only allowing ASCII alphabet, numeric, underscore and dash.
    if re.match(r'^[A-Za-z0-9_-]+$', project_name) == None:
        print("Error: Please specify project name in CamelCase (ie. with an uppercase letter for each word, starting with the first word) without any special characters")
        sys.exit(ERROR_INVALID_INPUT)

    # Validate project name is camelcase, only includes valid characters
    if not validate_camelcase_name(project_name):
        print("Error: Please specify project name in CamelCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    exit_code = create_project(project_name, DEFAULT_MODULE_LIST, not args.no_generate)
    sys.exit(exit_code)