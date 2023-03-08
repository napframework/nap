#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import check_for_existing_module, get_cmake_path, get_python_path, eprint, add_to_solution_info, get_solution_info_path, get_build_context

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_MODULE = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

def create_module(module_name, generate_solution):
    prefixed_module_name = f'nap{module_name}'
    prefixed_module_name_lower = prefixed_module_name.lower()

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir, os.pardir, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
    modules_dir = os.path.abspath(os.path.join(nap_root, 'modules'))
    module_path = os.path.abspath(os.path.join(modules_dir, prefixed_module_name))

    if not os.path.exists(modules_dir):
        print("Creating modules directory")
        os.mkdir(modules_dir)

    if check_for_existing_module(prefixed_module_name):
        sys.exit(ERROR_EXISTING_MODULE)

    print(f"Creating module {prefixed_module_name}")

    # Create module from template
    cmake = get_cmake_path()
    cmd = [cmake, '-DUNPREFIXED_MODULE_NAME_INPUTCASE=%s' % module_name, '-P', 'module_creator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        eprint("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    if get_build_context() == 'source':
        module_solution_path = f"modules/{prefixed_module_name}"
        print(f"Adding {module_solution_path} to {get_solution_info_path()}")
        add_to_solution_info(module_solution_path)

    # Solution generation
    if generate_solution:
        print("Generating solution")

        # Determine our Python interpreter location
        python = get_python_path()
        cmd = [python, './tools/buildsystem/common/regenerate_module_by_name.py', prefixed_module_name]
        if call(cmd, cwd=nap_root) != 0:
            eprint("Solution generation failed")
            sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)

    print(f"Successfully created {prefixed_module_name} module at {module_path}")

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str,
                        help="The module name (eg. napMyModuleName)")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created module")
    args = parser.parse_args()

    module_name = args.MODULE_NAME

    # Remove any nap prefix
    if module_name.startswith('nap'):
        module_name = module_name[3:]

    # Validate module name only includes valid characters. For now we're only allowing ASCII alphabet, numeric, underscore and dash.
    if re.match(r'^[A-Za-z0-9_-]+$', module_name) == None:
        eprint(f"Error: Module name '{module_name}' is invalid. Letters (ASCII), numbers, underscores and dashes are accepted.")
        sys.exit(ERROR_INVALID_INPUT)

    exit_code = create_module(module_name, not args.no_generate)
    sys.exit(exit_code)
