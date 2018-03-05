#!/usr/bin/python
import argparse
import os
import sys
from subprocess import call

from platform.NAPShared import validate_camelcase_name

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_MODULE = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

def create_module(module_name, generate_solution):
    print("Creating module %s in usermodules/mod_%s" % (module_name, module_name.lower()))

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake/moduleCreator'))
    module_path = os.path.abspath(os.path.join(nap_root, 'usermodules/mod_%s' % module_name.lower()))
    duplicate_module_path = os.path.abspath(os.path.join(nap_root, 'modules/mod_%s' % module_name.lower()))

    # Check for existing module with same name
    if os.path.exists(module_path):
        print("Error: Module with name %s already exists" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for existing NAP module with same name
    if os.path.exists(duplicate_module_path):
        print("Error: NAP module exists with same name '%s'" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Create module from template
    cmd = ['cmake', '-DMODULE_NAME_CAMELCASE=%s' % module_name, '-P', 'moduleCreator.cmake']
    if call(cmd, cwd=cmake_template_dir) != 0:
        print("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    # Solution generation
    if generate_solution:
        print("Module created")
        print("Generating solution")        

        # Determine our Python interpreter location
        if sys.platform == 'win32':
            python = os.path.join(nap_root, 'thirdparty', 'python', 'python')
        else:
            python = 'python'

        cmd = [python, './tools/refreshModule.py', module_name.lower()]
        if call(cmd, cwd=nap_root) != 0:
            print("Solution generation failed")
            sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)    
    else:
        print("Module created in %s" % os.path.relpath(module_path))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("CAMEL_CASE_MODULE_NAME", type=str,
                        help="The module name, in camel case (eg. MyModuleName)")
    parser.add_argument("-ng", "--no-generate", action="store_true",
                        help="Don't generate the solution for the created module")       
    args = parser.parse_args()

    module_name = args.CAMEL_CASE_MODULE_NAME

    # Validate module name is camelcase, only includes valid characters
    if not validate_camelcase_name(module_name):
        print("Error: Please specify module name in CamelCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    # TODO validate module name only has includes valid characters

    exit_code = create_module(module_name, not args.no_generate)
    sys.exit(exit_code)
