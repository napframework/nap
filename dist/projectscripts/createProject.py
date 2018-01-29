#!/usr/bin/python
import os
import sys
from subprocess import Popen

# Default modules if none are specified
DEFAULT_MODULE_LIST = "mod_naprender,mod_napmath,mod_napinput,mod_napsdlinput,mod_napsdlwindow,mod_napapp,mod_napscene,mod_napimgui"

# Exit codes
ERROR_INVALID_INPUT = 1
ERROR_EXISTING_PROJECT = 2
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
        print("Usage: %s CAMEL_CASE_PROJECT_NAME [MODULE_LIST_CSV]" % sys.argv[0])
        print("\n  eg. %s MyProjectName" % sys.argv[0])
        print("\nIf no module list if specified the following default applies: %s" % DEFAULT_MODULE_LIST)
        sys.exit(ERROR_INVALID_INPUT)

    project_name = sys.argv[1]

    if not validate_camelcase_name(project_name):
        print("Error: Please specify project name in CamelCase (ie. with an uppercase letter for each word, starting with the first word)")
        sys.exit(ERROR_INVALID_INPUT)

    print("Creating project %s" % project_name)

    if len(sys.argv) == 3:
        module_list = sys.argv[2]
    else:
        print("No module list specified, using default list: %s" % DEFAULT_MODULE_LIST)
        module_list = DEFAULT_MODULE_LIST

    # TODO validate project name is camelcase, only includes valid characters
    # TODO validate module list is CSV, no invalid characters

    # Set our paths
    script_path = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_path, os.pardir))
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake/projectCreator'))
    project_path = os.path.abspath(os.path.join(nap_root, 'projects/%s' % project_name.lower()))

    for project_type in ['projects', 'examples', 'demos']:
        dupe_project_path = os.path.abspath(os.path.join(nap_root, '%s/%s' % (project_type, project_name.lower())))
        if os.path.exists(dupe_project_path):
            print("Error: %s exists with same name '%s'" % (project_type[:-1], project_name))
            sys.exit(ERROR_EXISTING_PROJECT)

    # Create project from template
    input_module_list = module_list.lower().replace(',',';')
    cmd = ['cmake', '-DPROJECT_NAME_CAMELCASE=%s' % project_name, '-DMODULE_LIST=%s' % input_module_list, '-P', 'projectCreator.cmake']
    if call(cmake_template_dir, cmd):
        print("Project created, generating solution")
    else:
        print("Project creation failed")
        sys.exit(ERROR_CMAKE_CREATION_FAILURE)

    # Solution generation
    cmd = ['python', './tools/regenerateProject.py', project_name.lower()]
    if not call(nap_root, cmd):
        print("Solution generation failed")
        sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)
