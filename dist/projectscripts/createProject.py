#!/usr/bin/python
import argparse
import os
import sys
from subprocess import call

from platform.NAPShared import find_project, validate_camelcase_name

# Default modules if none are specified
DEFAULT_MODULE_LIST = "mod_naprender,mod_napmath,mod_napinput,mod_napsdlinput,mod_napsdlwindow,mod_napapp,mod_napscene,mod_napimgui"

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_PROJECT = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

def create_project(project_name, module_list, generate_solution):
    print("Creating project %s" % project_name)

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.join(script_path, os.pardir)

    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake/projectCreator'))
    project_path = os.path.abspath(os.path.join(nap_root, 'projects/%s' % project_name.lower()))

    # Check for duplicate project
    if not find_project(project_name, True) is None:
        print("Error: demo, example or project exists with same name '%s'" % project_name)
        return ERROR_EXISTING_PROJECT

    # Create project from template
    input_module_list = module_list.lower().replace(',', ';')
    cmd = ['cmake', '-DPROJECT_NAME_CAMELCASE=%s' % project_name, '-DMODULE_LIST=%s' % input_module_list, '-P', 'projectCreator.cmake']
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
            python = 'python'
                
        cmd = [python, './tools/refreshProject.py', project_name]
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
    parser.add_argument('MODULE_LIST_CSV', nargs='?', default=DEFAULT_MODULE_LIST,
                        help="List of modules to user, otherwise default list used: %s" % DEFAULT_MODULE_LIST)    
    parser.add_argument("-dg", "--dont-generate", action="store_true",
                        help="Don't generate the solution for the created project")       
    args = parser.parse_args()

    project_name = args.CAMEL_CASE_PROJECT_NAME

    # Validate project name is camelcase, only includes valid characters
    if not validate_camelcase_name(project_name):
        print("Error: Please specify project name in CamelCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    # TODO validate project name only has includes valid characters

    module_list = args.MODULE_LIST_CSV
    if module_list == DEFAULT_MODULE_LIST:
        print("No module list specified, using default list: %s" % DEFAULT_MODULE_LIST)

    # TODO validate module list is CSV, no invalid characters

    exit_code = create_project(project_name, module_list, not args.dont_generate)
    sys.exit(exit_code)