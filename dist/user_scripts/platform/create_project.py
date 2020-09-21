#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import find_project, validate_pascalcase_name, add_module_to_project_json, get_cmake_path, get_python_path

# Default modules if none are specified
DEFAULT_MODULE_LIST = "mod_napapp,mod_napaudio,mod_napimgui"

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_PROJECT = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4
ERROR_CMAKE_MODULE_CREATION_FAILURE = 5

def create_project(project_name, module_list, with_module, generate_solution, show_solution):
    print("Creating project %s" % project_name)

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.join(script_path, os.pardir, os.pardir)

    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'project_creator'))
    project_path = os.path.abspath(os.path.join(nap_root, 'projects', '%s' % project_name.lower()))

    # Check for duplicate project
    if not find_project(project_name, True) is None:
        print("Error: demo, example or project exists with same name '%s'" % project_name)
        return ERROR_EXISTING_PROJECT

    # Create project from template
    input_module_list = module_list.lower().replace(',', ';')
    cmake = get_cmake_path()
    cmd = [cmake, '-DPROJECT_NAME_PASCALCASE=%s' % project_name, '-DMODULE_LIST=%s' % input_module_list, '-P', 'project_creator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        print("Project creation failed")
        return ERROR_CMAKE_CREATION_FAILURE

    # Add project module on request
    if with_module:
        # Create module from template
        cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
        cmd = [cmake, 
               '-DMODULE_NAME_PASCALCASE=%s' % project_name, 
               '-DPROJECT_MODULE=1', 
               '-DPROJECT_MODULE_PROJECT_PATH=%s' % project_path,
               '-P', 'module_creator.cmake'
               ]
        if call(cmd, cwd=cmake_template_dir) != 0:
            print("Project module creation failed")
            sys.exit(ERROR_CMAKE_MODULE_CREATION_FAILURE)

        # Update project.json
        add_module_to_project_json(project_name, 'mod_%s' % project_name.lower())

    # Solution generation
    if generate_solution:
        print("Project created")
        print("Generating solution")

        # Determine our Python interpreter location
        python = get_python_path()

        cmd = [python, './tools/platform/regenerate_project_by_name.py', project_name]
        if not show_solution and not sys.platform.startswith('linux'):
            cmd.append('--no-show')        
        if call(cmd, cwd=nap_root) != 0:
            print("Solution generation failed")
            return ERROR_SOLUTION_GENERATION_FAILURE
    else:
        print("Project created in %s" % os.path.relpath(project_path))

    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PASCAL_CASE_PROJECT_NAME", type=str,
                        help="The project name, in pascal case (eg. MyProjectName)")
    parser.add_argument("-m", "--with-module", action="store_true",
                        help="Include a project module")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created project")       
    if not sys.platform.startswith('linux'):    
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")
    args = parser.parse_args()

    project_name = args.PASCAL_CASE_PROJECT_NAME

    # Validate module name only includes valid characters.  For now we're only allowing ASCII alphabet, numeric, underscore and dash.
    if re.match(r'^[A-Za-z0-9_-]+$', project_name) == None:
        print("Error: Please specify project name in PascalCase (ie. with an uppercase letter for each word, starting with the first word) without any special characters")
        sys.exit(ERROR_INVALID_INPUT)

    # Validate project name is pascal case, only includes valid characters
    if not validate_pascalcase_name(project_name):
        print("Error: Please specify project name in PascalCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    show_solution = not sys.platform.startswith('linux') and not args.no_show
    exit_code = create_project(project_name, DEFAULT_MODULE_LIST, args.with_module, not args.no_generate, show_solution)
    sys.exit(exit_code)
