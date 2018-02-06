#!/usr/bin/python
import os
import sys
from subprocess import Popen

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_MODULE = 2
ERROR_CMAKE_CREATION_FAILURE = 3
ERROR_SOLUTION_GENERATION_FAILURE = 4

# Execute process in working directory, returns clean exit value
def call(cwd, cmd):
    p = Popen(cmd, cwd=cwd)
    p.wait()
    return p.returncode == 0

def validate_camelcase_name(module_name):
    # Check we're not a single char
    if len(module_name) < 2:
        return False

    # Check our first character is uppercase
    if (not module_name[0].isalpha()) or module_name[0].islower():
        return False

    # Check we're not all uppercase
    if module_name.isupper():
        return False

    return True

if __name__ == '__main__':
    # Simple input parsing
    # TODO Switch to argparse?
    # TODO Add option to not generate solution?
    if len(sys.argv) == 1:
        print("Usage: %s CAMEL_CASE_MODULE_NAME" % sys.argv[0])
        print("\n  eg. %s MyModuleName" % sys.argv[0])
        sys.exit(ERROR_INVALID_INPUT)

    module_name = sys.argv[1]

    if not validate_camelcase_name(module_name):
        print("Error: Please specify module name in CamelCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    print("Creating module %s in usermodules/mod_%s" % (module_name, module_name.lower()))

    # TODO validate module name is camelcase, only includes valid characters
    # TODO validate module list is CSV, no invalid characters

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
    if call(cmake_template_dir, cmd):
        print("Module created, generating solution")
    else:
        print("Module creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    # Solution generation
    cmd = ['python', './tools/refreshModule.py', module_name.lower()]
    if not call(nap_root, cmd):
        print("Solution generation failed")
        sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)