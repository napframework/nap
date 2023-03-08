#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import add_module_to_app_json, check_for_existing_module, find_app, get_camelcase_app_name, get_cmake_path, get_python_path

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_MISSING_APP = 2
ERROR_EXISTING_MODULE = 3

def create_app_module(search_app_name, update_app_json, generate_solution, show_solution):
    # Ensure app exists
    (app_path, app_name) = find_app(search_app_name, True)
    if app_path is None:
        print("Error: can't find app with name '%s'" % search_app_name)
        sys.exit(ERROR_MISSING_APP)

    # Load camelcase app name from app.json
    app_name = get_camelcase_app_name(app_name)
    prefixed_module_name = f'nap{app_name}'

    # Set our paths
    module_path = os.path.join(app_path, 'module')
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir, os.pardir, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
    user_module_path = os.path.abspath(os.path.join(nap_root, 'modules', prefixed_module_name))
    duplicate_module_path = os.path.abspath(os.path.join(nap_root, 'system_modules', prefixed_module_name))

    # Ensure app doesn't already have module
    if os.path.exists(module_path):
        print("Error: '%s' already has a module" % app_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for existing module which would clash
    if check_for_existing_module(prefixed_module_name):
        sys.exit(ERROR_EXISTING_MODULE)

    print("Creating app module %s for app %s in %s" % (prefixed_module_name, app_name, module_path))

    # Create module from template
    cmake = get_cmake_path()
    cmd = [cmake,
           '-DUNPREFIXED_MODULE_NAME_INPUTCASE=%s' % app_name,
           '-DAPP_MODULE=1',
           '-DAPP_MODULE_APP_PATH=%s' % app_path,
           '-P', 'module_creator.cmake'
           ]
    if call(cmd, cwd=cmake_template_dir) != 0:
        print("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    if update_app_json:
        # Update app.json
        add_module_to_app_json(app_name, prefixed_module_name)

        # Solution regeneration
        if generate_solution:
            print("Module created")
            print("Regenerating solution")

            # Determine our Python interpreter location
            python = get_python_path()

            cmd = [python, './tools/buildsystem/common/regenerate_app_by_name.py', app_name]
            if not show_solution and not sys.platform.startswith('linux'):
                cmd.append('--no-show')
            if call(cmd, cwd=nap_root) != 0:
                print("Solution generation failed")
                sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)

    print(f"Successfully created {prefixed_module_name} at {module_path}")

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("APP_NAME", type=str,
                        help="The app name")
    parser.add_argument("-nu", "--no-update-app", action="store_true",
                        help="Don't update the app.json")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't regenerate the solution for the updated app")
    if not sys.platform.startswith('linux'):
        parser.add_argument("-ns", "--no-show", action="store_true",
                            help="Don't show the regenerated solution")

    args = parser.parse_args()

    app_name = args.APP_NAME

    update_app_json = not args.no_update_app
    regenerate_app = update_app_json and not args.no_generate
    show_solution = not sys.platform.startswith('linux') and not args.no_show
    exit_code = create_app_module(app_name, update_app_json, regenerate_app, show_solution)
    sys.exit(exit_code)
