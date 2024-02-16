from collections import OrderedDict
import json
import os
from platform import machine
from subprocess import Popen, run, PIPE
import sys

PROJECT_INFO_FILENAME = 'app.json'
MODULE_INFO_FILENAME = 'module.json'
SOLUTION_INFO_FILENAME = 'solution_info.json'

# Keys for entries in app.json and module.json
CFG_KEY_DEPENDENCIES = 'Dependencies'
CFG_KEY_MODULES = 'Modules'

def call_except_on_failure(cwd, cmd):
    """Run command, raising exception on failure"""
    # print('dir: %s' % cwd)
    # print('cmd: %s' % cmd)
    proc = Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out

def find_user_module(module_name):
    """Locate module specified by name"""
    nap_root = get_nap_root()

    # Create module dir name
    if not module_name.startswith('nap'):
        module_name = f'nap{module_name}'

    modules_root = os.path.join(nap_root, 'modules')
    if not os.path.exists(modules_root):
        print("No user modules found")
        return None
    module_path = None
    for d in os.listdir(modules_root):
        if d.lower() == module_name.lower():
            module_path = os.path.join(modules_root, d)
            break

    # Does it exist?
    if not module_path is None and os.path.exists(module_path):
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

def find_app(app_name, silent_failure=False, silent_success=False):
    """Locate app specified by loosely cased name, returning the path and proper identifier"""
    nap_root = get_nap_root()

    for project_type in ['apps', 'examples', 'demos']:
        search_dir = os.path.join(nap_root, project_type)
        if not os.path.isdir(search_dir):
            continue
        for d in os.listdir(search_dir):
            if d.lower() == app_name.lower():
                project_search_path = os.path.join(search_dir, d)
                if not silent_success:
                    print("Found %s at %s" % (project_type[:-1], project_search_path))
                return (project_search_path, d)

    if not silent_failure:
        print("Couldn't find app, demo or example with name '%s'" % app_name)
    return (None, None)

def read_console_char():
    """Pause for input"""
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

def get_camelcase_app_name(app_name):
    """Get camelcase app name"""
    (app_path, _) = find_app(app_name, True, True)
    if app_path is None:
        print("Error: couldn't find app '%s'" % app_name)
        return None

    project_info_path = os.path.join(app_path, PROJECT_INFO_FILENAME)
    with open(project_info_path) as json_file:
        json_dict = json.load(json_file)
        if not 'Title' in json_dict:
            print("Missing element 'Title' in %s" % project_info_path)
            return None

        app_name = json_dict['Title']
    return app_name

def add_module_to_app_json(app_name, full_module_name):
    """Add module to app.json"""
    (app_path, _) = find_app(app_name, True, True)
    if app_path is None:
        print("Error: couldn't find app '%s'" % app_name)
        return False

    project_info_path = os.path.join(app_path, PROJECT_INFO_FILENAME)

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

def get_nap_root():
    """Get absolute path to NAP root"""
    script_path = os.path.realpath(__file__)
    script_to_nap_root = os.path.join(os.pardir, os.pardir)
    framework_release_context_known_path = os.path.join(os.path.dirname(script_path), script_to_nap_root, 'modules')
    if os.path.exists(framework_release_context_known_path):
        return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))
    else:
        script_to_nap_root = os.path.join(os.pardir, os.pardir, os.pardir)
        return os.path.abspath(os.path.join(os.path.dirname(script_path), script_to_nap_root))

def get_cmake_path():
    """Fetch the path to the CMake binary"""
    nap_root = get_nap_root()
    cmake_root = os.path.join(nap_root, 'thirdparty', 'cmake')
    if sys.platform.startswith('linux'):
        cmake = os.path.join(cmake_root, 'linux', get_build_arch(), 'bin', 'cmake')
    elif sys.platform == 'darwin':
        cmake = os.path.join(cmake_root, 'macos', 'universal', 'bin', 'cmake')
    else:
        cmake = os.path.join(cmake_root, 'msvc', 'x86_64', 'bin', 'cmake.exe')
    return cmake

def get_app_full_module_requirements(framework_root, app_name, app_path):
    """Fetch deep module dependencies for an app"""
    with open(os.path.join(app_path, PROJECT_INFO_FILENAME)) as json_file:
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
            app_module_path = os.path.join(app_path, 'module')
            if search_module == f'nap{app_name}' and os.path.exists(app_module_path):
                found_module_path = app_module_path
            else:
                for module_source in ('system_modules', 'modules'):
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
                print("get_app_full_module_requirements: {} not found".format(search_module))
        new_modules = loop_modules
        modules.extend(new_modules)

    return modules

def get_python_path():
    """Determine Python interpreter location"""
    nap_root = get_nap_root()
    python_dir = os.path.join(nap_root, 'thirdparty', 'python')
    if sys.platform.startswith('linux'):
        python = os.path.join(python_dir, 'linux', get_build_arch(), 'bin', 'python3')
    elif sys.platform == 'darwin':
        python = os.path.join(python_dir, 'macos', 'universal', 'bin', 'python3')
    else:
        python = os.path.join(python_dir, 'msvc', 'x86_64', 'python')
    return python

def get_platform_name():
    """Get display-friendly platform name"""
    if sys.platform.startswith('linux'):
        return 'Linux'
    elif sys.platform == 'darwin':
        return 'macOS'
    else:
        return 'Windows'

def eprint(*args, **kwargs):
    """Print to stderr"""
    print(*args, file=sys.stderr, **kwargs)

def get_build_context():
    """Fetch build context"""
    script_path = os.path.dirname(os.path.realpath(__file__))
    script_to_nap_root = os.path.join(os.pardir, os.pardir, os.pardir)

    # Check for Source context
    source_root_cmakelists = os.path.join(script_path, script_to_nap_root, 'CMakeLists.txt')
    if os.path.exists(source_root_cmakelists):
        return 'source'
    else:
        return 'framework_release'

def get_solution_info_path():
    return os.path.join(get_nap_root(), SOLUTION_INFO_FILENAME)

def add_to_solution_info(new_path):
    """Add new path to solution_info.json in Source context"""
    if get_build_context() != 'source':
        return

    filepath = get_solution_info_path()
    if os.path.exists(filepath):
        with open(filepath, 'r') as fp:
            data = json.load(fp, object_pairs_hook=OrderedDict)
    else:
        data = OrderedDict((
            ('Type', 'nap::SolutionInfo'),
            ('mID', 'SolutionInfo'),
            ('AdditionalTargets', []),
        ))
    if not 'AdditionalTargets' in data:
        data['AdditionalTargets'] = []

    if not new_path in data['AdditionalTargets']:
        data['AdditionalTargets'].append(new_path)

    data['AdditionalTargets'] = sorted(list(set(data['AdditionalTargets'])))

    with open(filepath, 'w') as fp:
        json.dump(data, fp, indent=4)

def read_yes_no(question, default='yes'):
    """Read a yes/no answer for a question"""
    yes = ['yes', 'y', 'ye']
    no = ['no', 'n']

    if default is None:
        default = ''
    if default.lower() in yes:
        yes.append('')
        options = 'Y/n'
    elif default.lower() in no:
        no.append('')
        options = 'y/N'
    else:
        options = 'y/n'

    while True:
        prompt = f"{question} [{options}] "
        choice = input(prompt)
        choice = choice.lower().strip()
        if choice in yes:
           return True
        elif choice in no:
           return False
        else:
           print("Please respond with 'y' or 'n'\n")

def get_build_arch():
    """Fetch build architecture as used by NAP"""
    machine_arch = machine()
    if machine_arch.lower() in ('x86_64', 'amd64'):
        nap_arch = 'x86_64'
    elif machine_arch == 'aarch64':
        p = run('getconf LONG_BIT', shell=True, universal_newlines=True, stdout=PIPE, stderr=PIPE)
        if p.stdout.strip() == '64':
            nap_arch = 'arm64'
        else:
            nap_arch = 'armhf'
    else:
        nap_arch = 'armhf'
    return nap_arch

def ensure_set_executable(filepath):
    """Ensure file has executable bit set on POSIX"""
    if not os.name == 'posix':
        return
    if os.path.exists(filepath) and not os.access(filepath, os.X_OK):
        cmd = f'chmod +x {filepath}'
        run(cmd, shell=True)

def check_for_existing_module(module_name):
    """Check for any existing module with the specified name"""
    nap_root = get_nap_root()
    module_name_lower = module_name.lower()

    # Check for existing module with same name
    user_module_dir = os.path.abspath(os.path.join(nap_root, 'modules'))
    try:
        names = [d.lower() for d in os.listdir(user_module_dir)]
    except FileNotFoundError:
        names = []
    if module_name_lower in names:
        eprint(f"Error: Module with name {module_name} already exists")
        return True

    # Check for existing system module with same name
    system_module_dir = os.path.abspath(os.path.join(nap_root, 'system_modules'))
    names = [d.lower() for d in os.listdir(system_module_dir)]
    if module_name_lower in names:
        eprint(f"Error: System module with name {module_name} already exists")
        return True

    # Check for app modules
    def check_for_app_modules_under_dir(search_dir):
        for app in os.listdir(search_dir):
            if f'nap{app.lower()}' == module_name_lower:
                app_module_dir = os.path.join(search_dir, app, 'module')
                if os.path.exists(app_module_dir):
                    eprint(f"Error: App module with name {module_name} already exists")
                    return True
        return False

    for dirname in ('demos', 'apps', 'test'):
        check_dir = os.path.abspath(os.path.join(nap_root, dirname))
        if not os.path.isdir(check_dir):
            continue
        if check_for_app_modules_under_dir(check_dir):
            return True

    return False
