#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import find_project, get_camelcase_project_name, add_module_to_project_json, get_cmake_path, get_python_path

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_MISSING_PROJECT = 2
ERROR_EXISTING_MODULE = 3

def create_project_module(project_name, update_project_json, generate_solution, show_solution):
    # Ensure project exists
    project_path = find_project(project_name, True) 
    if project_path is None:
        print("Error: can't find project with name '%s'" % project_name)
        sys.exit(ERROR_MISSING_PROJECT)

    # Load camelcase project name from project.json
    module_name = get_camelcase_project_name(project_name)

    # Set our paths
    module_path = os.path.join(project_path, 'module')    
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir, os.pardir, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
    user_module_path = os.path.abspath(os.path.join(nap_root, 'modules', 'mod_%s' % module_name.lower()))
    duplicate_module_path = os.path.abspath(os.path.join(nap_root, 'system_modules', 'mod_%s' % module_name.lower()))

    # Ensure project doesn't already have module
    if os.path.exists(module_path):
        print("Error: '%s' already has a module" % project_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for existing module with same name
    if os.path.exists(user_module_path):
        print("Error: User module with name %s already exists" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for existing NAP module with same name
    if os.path.exists(duplicate_module_path):
        print("Error: NAP module exists with same name '%s'" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    print("Creating project module mod_%s for project %s in %s" % (module_name.lower(), module_name, module_path))

    # Create module from template
    cmake = get_cmake_path()
    cmd = [cmake, 
           '-DMODULE_NAME_PASCALCASE=%s' % module_name, 
           '-DPROJECT_MODULE=1', 
           '-DPROJECT_MODULE_PROJECT_PATH=%s' % project_path,
           '-P', 'module_creator.cmake'
           ]
    if call(cmd, cwd=cmake_template_dir) != 0:
        print("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    if update_project_json:
        # Update project.json
        add_module_to_project_json(project_name, 'mod_%s' % module_name.lower())

        # Solution regeneration
        if generate_solution:
            print("Module created")
            print("Regenerating solution")        

            # Determine our Python interpreter location
            python = get_python_path()

            cmd = [python, './tools/buildsystem/common/regenerate_app_by_name.py', project_name]
            if not show_solution and not sys.platform.startswith('linux'):
                cmd.append('--no-show')
            if call(cmd, cwd=nap_root) != 0:
                print("Solution generation failed")
                sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)    

    print("Project module created in %s" % os.path.relpath(module_path))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str,
                        help="The project name")
    parser.add_argument("-nu", "--no-update-project", action="store_true",
                        help="Don't update the project.json")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't regenerate the solution for the updated project")       
    if not sys.platform.startswith('linux'):    
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the regenerated solution")

    args = parser.parse_args()

    project_name = args.PROJECT_NAME.lower()

    update_project_json = not args.no_update_project
    regenerate_project = update_project_json and not args.no_generate
    show_solution = not sys.platform.startswith('linux') and not args.no_show
    exit_code = create_project_module(project_name, update_project_json, regenerate_project, show_solution)
    sys.exit(exit_code)
