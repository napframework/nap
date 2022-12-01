#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import get_cmake_path, get_python_path, eprint, add_to_solution_info, get_build_context

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_MODULE = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

def create_module(module_name, generate_solution):
    prefixed_module_name = f'nap{module_name}'
    prefixed_module_name_lower = prefixed_module_name.lower()
    print("Creating module %s in modules/%s" % (module_name, prefixed_module_name))


    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir, os.pardir, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
    module_path = os.path.abspath(os.path.join(nap_root, 'modules', prefixed_module_name_lower))
    duplicate_module_path = os.path.abspath(os.path.join(nap_root, 'modules', prefixed_module_name_lower))

    # Check for existing module with same name
    if os.path.exists(module_path):
        eprint("Error: Module with name %s already exists" % prefixed_module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for existing NAP module with same name
    if os.path.exists(duplicate_module_path):
        eprint("Error: NAP module exists with same name '%s'" % prefixed_module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Create module from template
    cmake = get_cmake_path()
    cmd = [cmake, '-DUNPREFIXED_MODULE_NAME_INPUTCASE=%s' % module_name, '-P', 'module_creator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        eprint("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    if get_build_context() == 'source':
        print("Adding module to solution info")
        add_to_solution_info(f'modules/{prefixed_module_name_lower}')

    # Solution generation
    if generate_solution:
        print("Module created")
        print("Generating solution")        

        # Determine our Python interpreter location
        python = get_python_path()
        cmd = [python, './tools/buildsystem/common/regenerate_module_by_name.py', module_name.lower()]
        if call(cmd, cwd=nap_root) != 0:
            eprint("Solution generation failed")
            sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)    
    else:
        print("Module created in %s" % os.path.relpath(module_path))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str,
                        help="The module name (eg. MyModuleName)")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created module")       
    args = parser.parse_args()

    module_name = args.MODULE_NAME

    # Validate module name only includes valid characters. For now we're only allowing ASCII alphabet, numeric, underscore and dash.
    if re.match(r'^[A-Za-z0-9_-]+$', module_name) == None:
        eprint("Error: Module name includes invalid characters. Letters (ASCII), numbers, underscores and dashes are accepted.")
        sys.exit(ERROR_INVALID_INPUT)

    exit_code = create_module(module_name, not args.no_generate)
    sys.exit(exit_code)
