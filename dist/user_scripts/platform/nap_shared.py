from collections import OrderedDict
import json
import os
from subprocess import Popen
import sys

PROJECT_INFO_FILENAME = 'project.json'
MODULE_INFO_FILENAME = 'module.json'

# Keys for entries in project.json and module.json
CFG_KEY_DEPENDENCIES = 'Dependencies'
CFG_KEY_MODULES = 'Modules'

# Run command, raising exception on failure
def call_except_on_failure(cwd, cmd):
    # print('dir: %s' % cwd)
    # print('cmd: %s' % cmd)
    proc = Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out

# Locate module specified by name
def find_module(module_name):
    nap_root = get_nap_root()

    # Create module dir name
    module_dir_name = module_name.lower()
    if not module_dir_name.startswith("mod_"):
        module_dir_name = "mod_%s" % module_dir_name

    modules_root = os.path.join(nap_root, 'user_modules')
    module_path = os.path.join(modules_root, module_dir_name)

    # Does it exist?
    if os.path.exists(module_path):
        cmake_path = os.path.join(module_path, 'CMakeLists.txt')
        if os.path.exists(cmake_path):
            print("Found module %s at %s" % (module_name, module_path))
            return module_path
        else:
            print("Module %s at %s does not contain CMakeLists.txt and can't be regenerated" % (module_name, module_path))
            return None
    else:
        print("Couldn't find module with name '%s'" % module_name)
        return None

# Locate project specified by name
def find_project(project_name, silent_failure=False, silent_success=False):
    nap_root = get_nap_root()

    project_dir_name = project_name.lower()

    for project_type in ['projects', 'examples', 'demos']:
        project_search_path = os.path.join(nap_root, project_type, project_dir_name)
        if os.path.exists(project_search_path):
            if not silent_success:
                print("Found %s at %s" % (project_type[:-1], project_search_path))
            return project_search_path

    if not silent_failure:
        print("Couldn't find project, demo or example with name '%s'" % project_name)
    return None

# Super basic pascal case validation of name
def validate_pascalcase_name(module_name):
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

# Pause for input
def read_console_char():
    if sys.platform == 'win32':
        from msvcrt import getch
        getch()
    else:
        import tty, termios
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            return sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)

# Get camelcase project name
def get_camelcase_project_name(project_name):
    project_path = find_project(project_name, True, True)
    if project_path is None:
        print("Error: couldn't find project '%s'" % project_name)
        return None

    project_info_path = os.path.join(project_path, PROJECT_INFO_FILENAME)
    with open(project_info_path) as json_file:
        json_dict = json.load(json_file)
        if not 'Title' in json_dict:
            print("Missing element 'Title' in %s" % project_info_path)
            return None

        project_name = json_dict['Title']
    return project_name

# Add module to project.json
def add_module_to_project_json(project_name, full_module_name):
    project_path = find_project(project_name, True, True)
    if project_path is None:
        print("Error: couldn't find project '%s'" % project_name)
        return False

    project_info_path = os.path.join(project_path, PROJECT_INFO_FILENAME)

    with open(project_info_path) as json_file:
        json_dict = json.load(json_file, object_pairs_hook=OrderedDict)

        if not 'RequiredModules' in json_dict:
            print("Missing element 'RequiredModules' in %s" % project_info_path)
            return False

        if not type(json_dict['RequiredModules']) is list:
            print("Element 'RequiredModules' in %s is not an array" % PROJECT_INFO_FILENAME)
            return False

    if not full_module_name in json_dict['RequiredModules']:
        json_dict['RequiredModules'].append(full_module_name)

        with open(project_info_path, 'w') as json_file:
            json.dump(json_dict, json_file, indent=4)

    return True

# Get absolute path to NAP root
def get_nap_root():
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    framework_release_context_known_path = os.path.join(os.path.dirname(script_path), script_to_nap_root, 'modules')
    if os.path.exists(framework_release_context_known_path):
        return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))
    else:
        script_to_nap_root = os.path.join(os.pardir, os.pardir, os.pardir)
        return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))
    
# Fetch the path to the CMake binary, providing for future providing of CMake via included thirdparty
def get_cmake_path():
    nap_root = get_nap_root()
    return os.path.join(nap_root, 'thirdparty', 'cmake', 'bin', 'cmake')

# Fetch deep module dependencies for a project
def get_full_project_module_requirements(framework_root, project_name, project_path):
    with open(os.path.join(project_path, PROJECT_INFO_FILENAME)) as json_file:
        json_dict = json.load(json_file)
        modules = []
        for module_name in json_dict['RequiredModules']:
            if not isinstance(module_name, str):
                module_name = module_name.encode('utf8')
            modules.append(module_name)
        new_modules = modules
    
    while len(new_modules) > 0:
        loop_modules = []
        for search_module in new_modules:
            found_module_path = None
            if search_module == 'mod_{}'.format(project_name):
                found_module_path = os.path.join(project_path, 'module')
            else:
                for module_source in ('modules', 'user_modules'):
                    check_path = os.path.join(framework_root, module_source, search_module)
                    if os.path.exists(check_path):
                        found_module_path = check_path
                        break

            if not found_module_path is None:
                with open(os.path.join(found_module_path, 'module.json')) as json_file:
                    json_dict = json.load(json_file)
                    for module_name in json_dict['RequiredModules']:
                        if module_name not in modules and module_name not in loop_modules:
                            if not isinstance(module_name, str):
                                module_name = module_name.encode('utf8')
                            loop_modules.append(module_name)
            else:
                print("{} not found".format(search_module))
        new_modules = loop_modules
        modules.extend(new_modules)

    return modules

def get_python_path():
    """Determine Python interpreter location"""
    nap_root = get_nap_root()
    if sys.platform == 'win32':
        python = os.path.join(nap_root, 'thirdparty', 'python', 'python')
    else:
        python = os.path.join(nap_root, 'thirdparty', 'python', 'bin', 'python3')
    return python
