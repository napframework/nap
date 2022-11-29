#!/usr/bin/env python3
import argparse
import os
import re
import sys
from subprocess import call

from nap_shared import validate_pascalcase_name, get_cmake_path, get_python_path, eprint, add_to_solution_info, get_build_context

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_MODULE = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

def create_module(module_name, generate_solution):
    print("Creating module %s in modules/mod_%s" % (module_name, module_name.lower()))

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir, os.pardir, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
    module_path = os.path.abspath(os.path.join(nap_root, 'modules', 'mod_%s' % module_name.lower()))
    duplicate_module_path = os.path.abspath(os.path.join(nap_root, 'modules', 'mod_%s' % module_name.lower()))

    # Check for existing module with same name
    if os.path.exists(module_path):
        eprint("Error: Module with name %s already exists" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for existing NAP module with same name
    if os.path.exists(duplicate_module_path):
        eprint("Error: NAP module exists with same name '%s'" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Create module from template
    cmake = get_cmake_path()    
    cmd = [cmake, '-DMODULE_NAME_PASCALCASE=%s' % module_name, '-P', 'module_creator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        eprint("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    if get_build_context() == 'source':
        print("Adding module to solution info")
        add_to_solution_info(f'modules/mod_{module_name.lower()}')

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
    parser.add_argument("PASCAL_CASE_MODULE_NAME", type=str,
                        help="The module name, in pascal case (eg. MyModuleName)")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created module")       
    args = parser.parse_args()

    module_name = args.PASCAL_CASE_MODULE_NAME

    # If module name is prefixed with mod_ remove it
    if module_name.startswith('mod_'):
        module_name = module_name[4:]

    # Validate module name only includes valid characters.  For now we're only allowing ASCII alphabet, numeric, underscore and dash.
    if re.match(r'^[A-Za-z0-9_-]+$', module_name) == None:
        eprint("Error: Please specify module name in PascalCase (ie. with an uppercase letter for each word, starting with the first word) without any special characters")
        sys.exit(ERROR_INVALID_INPUT)

    # Validate module name is pascal case, only includes valid characters
    if not validate_pascalcase_name(module_name):
        eprint("Error: Please specify module name in PascalCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    exit_code = create_module(module_name, not args.no_generate)
    sys.exit(exit_code)
