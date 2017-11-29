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

    # Check for existing project with same name
    if os.path.exists(project_path):
        print("Error: Project with name %s already exists" % project_name)
        sys.exit(ERROR_EXISTING_PROJECT)

    # Check for existing example with same name
    dupe_project_path = os.path.abspath(os.path.join(nap_root, 'examples/%s' % project_name.lower()))
    if os.path.exists(dupe_project_path):
        print("Error: Example exists with same name '%s'" % project_name)
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
    if call(nap_root, cmd):
        print("Solution generated")
    else:
        print("Solution generation failed")
        sys.exit(ERROR_SOLUTION_GENERATION_FAILURE)
