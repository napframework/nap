import os
from subprocess import Popen
import sys

def call_except_on_failure(cwd, cmd):
    # print('dir: %s' % cwd)
    # print('cmd: %s' % cmd)
    proc = Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out

def find_module(module_name):
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    nap_root = os.path.join(os.path.dirname(script_path), script_to_nap_root)

    module_dir_name = module_name.lower()
    if not module_dir_name.startswith("mod_"):
        module_dir_name = "mod_%s" % module_dir_name

    modules_root = os.path.join(nap_root, 'usermodules')
    module_path = os.path.join(modules_root, module_dir_name)

    if os.path.exists(module_path):
        cmake_path = os.path.join(module_path, 'CMakeLists.txt')
        if os.path.exists(cmake_path):
            print("Found module %s at %s" % (module_name, module_path))
            return module_path
        else:
            print("Module %s at %s does not contain CMakeLists.txt and can't be refreshed" % (module_name, module_path))
            return None
    else:
        print("Couldn't find module with name '%s'" % module_name)
        return None

def find_project(project_name, silent_failure=False):
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    nap_root = os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))

    project_dir_name = project_name.lower()

    for project_type in ['projects', 'examples', 'demos']:
        project_search_path = os.path.join(nap_root, project_type, project_dir_name)
        if os.path.exists(project_search_path):
            print("Found %s at %s" % (project_type[:-1], project_search_path))
            return project_search_path

    if not silent_failure:
        print("Couldn't find project, demo or example with name '%s'" % project_name)
    return None

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

# Pause for input
def read_console_char():
    if sys.platform == 'win32':
        from msvcrt import getch
    else:
        def getch():
            import tty, termios
            fd = sys.stdin.fileno()
            old = termios.tcgetattr(fd)
            try:
                tty.setraw(fd)
                return sys.stdin.read(1)
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old)

