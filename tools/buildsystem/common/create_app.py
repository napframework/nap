#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import find_app, validate_pascalcase_name, add_module_to_app_json, get_cmake_path, get_python_path, eprint, get_build_context, add_to_solution_info

# Default modules if none are specified
DEFAULT_MODULE_LIST = "mod_napapp,mod_napcameracontrol,mod_napparametergui"

# Modules for project module
APP_MODULE_MODULE_LIST = "mod_naprender,mod_napscene,mod_napparameter"

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_APP = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4
ERROR_CMAKE_MODULE_CREATION_FAILURE = 5

def create_app(app_name, module_list, with_module, generate_solution, show_solution):
    print("Creating app %s" % app_name)

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.join(script_path, os.pardir, os.pardir, os.pardir)

    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'app_creator'))
    project_path = os.path.abspath(os.path.join(nap_root, 'apps', '%s' % app_name.lower()))

    # Check for duplicate app
    if not find_app(app_name, True) is None:
        eprint("Error: demo, example or app exists with same name '%s'" % app_name)
        return ERROR_EXISTING_APP

    # Create app from template
    input_module_list = module_list.lower().replace(',', ';')
    cmake = get_cmake_path()
    cmd = [cmake, '-DPROJECT_NAME_PASCALCASE=%s' % app_name, '-DMODULE_LIST=%s' % input_module_list, '-P', 'app_creator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        eprint("App creation failed")
        return ERROR_CMAKE_CREATION_FAILURE

    # Add app module on request
    if with_module:
        # Create module from template
        cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
        input_module_list = APP_MODULE_MODULE_LIST.lower().replace(',', ';')
        cmd = [cmake, 
               '-DMODULE_NAME_PASCALCASE=%s' % app_name, 
               '-DAPP_MODULE=1', 
               '-DAPP_MODULE_APP_PATH=%s' % project_path,
               '-DAPP_MODULE_MODULE_LIST=%s' % input_module_list,
               '-P', 'module_creator.cmake'
               ]
        if call(cmd, cwd=cmake_template_dir) != 0:
            eprint("Project module creation failed")
            sys.exit(ERROR_CMAKE_MODULE_CREATION_FAILURE)

        # Update project.json
        add_module_to_app_json(app_name, 'mod_%s' % app_name.lower())

    if get_build_context() == 'source':
        print("Adding app to solution info")
        add_to_solution_info(f'apps/{app_name.lower()}')

    # Solution generation
    if generate_solution:
        print("Project created")
        print("Generating solution")

        # Determine our Python interpreter location
        python = get_python_path()

        cmd = [python, './tools/buildsystem/common/regenerate_app_by_name.py', app_name]
        if not show_solution and not sys.platform.startswith('linux'):
            cmd.append('--no-show')        
        if call(cmd, cwd=nap_root) != 0:
            eprint("Solution generation failed")
            return ERROR_SOLUTION_GENERATION_FAILURE
    else:
        print("App created in %s" % os.path.relpath(project_path))

    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PASCAL_CASE_PROJECT_NAME", type=str,
                        help="The project name, in pascal case (eg. MyProjectName)")
    parser.add_argument("-nm", "--no-module", action="store_true",
                        help="Don't include a project module")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created project")       
    if not sys.platform.startswith('linux'):    
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the generated solution")
    args = parser.parse_args()

    app_name = args.PASCAL_CASE_PROJECT_NAME

    # Validate module name only includes valid characters.  For now we're only allowing ASCII alphabet, numeric, underscore and dash.
    if re.match(r'^[A-Za-z0-9_-]+$', app_name) == None:
        eprint("Error: Please specify project name in PascalCase (ie. with an uppercase letter for each word, starting with the first word) without any special characters")
        sys.exit(ERROR_INVALID_INPUT)

    # Validate project name is pascal case, only includes valid characters
    if not validate_pascalcase_name(app_name):
        eprint("Error: Please specify project name in PascalCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    show_solution = not sys.platform.startswith('linux') and not args.no_show
    exit_code = create_app(app_name, DEFAULT_MODULE_LIST, not args.no_module, not args.no_generate, show_solution)
    sys.exit(exit_code)
