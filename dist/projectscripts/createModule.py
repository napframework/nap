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

if __name__ == '__main__':
    # Simple input parsing
    # TODO Switch to argparse?
    # TODO Add option to not generate solution?
    if len(sys.argv) == 1:
        print("Usage: %s CAMEL_CASE_MODULE_NAME" % sys.argv[0])
        print("\n  eg. %s MyModuleName" % sys.argv[0])
        sys.exit(ERROR_INVALID_INPUT)

    module_name = sys.argv[1]
    print("Creating module %s in modules/mod_%s" % (module_name, module_name.lower()))

    # TODO validate module name is camelcase, only includes valid characters
    # TODO validate module list is CSV, no invalid characters

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake/moduleCreator'))
    module_path = os.path.abspath(os.path.join(nap_root, 'modules/mod_%s' % module_name.lower()))

    # Check for existing module with same name
    if os.path.exists(module_path):
        print("Error: Module with name %s already exists" % module_name)
        sys.exit(ERROR_EXISTING_MODULE)

    # Check for NAP module example with same name
    nap_module_path = os.path.abspath(os.path.join(nap_root, 'modules/mod_nap%s' % module_name.lower()))
    if os.path.exists(nap_module_path):
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
    cmd = ['python', './tools/regenerateModule.py', module_name.lower()]
    if call(nap_root, cmd):
        print("Solution generated")
    else:
        print("Solution generation failed")
        sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)