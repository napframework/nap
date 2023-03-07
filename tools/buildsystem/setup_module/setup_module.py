#!/usr/bin/env python3

import argparse
import glob
import os
from pathlib import PurePath
import shutil
from subprocess import run
import sys

script_dir = os.path.dirname(__file__)
nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir))
sys.path.append(os.path.join(nap_root, 'tools', 'buildsystem', 'common'))
from nap_shared import add_to_solution_info, ensure_set_executable, find_user_module, eprint, get_build_arch, \
        get_build_context, get_nap_root, read_yes_no

class ModuleInitialiser():

    MODULES_DIR_NAME = 'modules'
    TOOLS_DIR_NAME = 'tools'
    # Currently deploying to apps so that any deployed demos in Source context aren't packaged into the
    # Framework Release
    DEMO_DEST_DIR = 'apps'
    APPS_DIR = 'apps'

    def __init__(self, interactive, deploy_demo, run_demo, force_overwrite_demo):
        self.__nap_root = get_nap_root()
        self.__modules_dir = os.path.join(self.__nap_root, self.MODULES_DIR_NAME)
        self.__modules_dir_relpath = os.path.join('.', os.path.relpath(self.__modules_dir))
        self.__module_cmakelists = os.path.join(self.__nap_root, 'cmake', 'module_creator', 'template', 'CMakeLists.txt')
        self.__tools_dir = os.path.join(self.__nap_root, self.TOOLS_DIR_NAME)

        self.__interactive = interactive
        if deploy_demo in (None, False) and run_demo:
            deploy_demo = True
        if deploy_demo is None and not interactive:
            print("Defaulting to deploying any demo (for non-interactive")
            deploy_demo = True
        self.__deploy_demo = deploy_demo
        if run_demo is None and not interactive:
            print("Defaulting to not running any demo (for non-interactive")
            run_demo = False
        self.__run_demo = run_demo
        self.__force_overwrite_demo = force_overwrite_demo

    def setup_module_by_dir(self, module_path):
        if not os.path.exists(module_path):
            eprint(f"Error: Can't find module at {module_path}")
            return False
        if not os.path.isdir(module_path):
            eprint(f"Error: {module_path} is not a directory")
            return False

        module_purepath = PurePath(os.path.abspath(module_path))
        if str(module_purepath.parent) != self.__modules_dir:
            eprint(f"Error: {module_path} is not a user module directory under {self.__modules_dir_relpath}")
            return False

        # Deploy module CMakeLists.txt
        print("Copying module CMakeLists.txt")
        shutil.copy(self.__module_cmakelists, module_path)

        # Add module to solution info in Source context
        if get_build_context() == 'source':
            print("Adding module to solution info")
            self.__add_path_to_solution_info(module_path)

        # Deploy module dir shortcuts
        self.__deploy_shortcuts('module_dir_shortcuts', module_path)

        # Check and install demo
        return self.__process_demo(module_path)

    def setup_module_by_name(self, module_name):
        module_path = os.path.join(self.__modules_dir, module_name)
        if not os.path.exists(module_path):
            eprint(f"Error: Can't find {module_name} at {module_path}")
            return False
        return self.setup_module_by_dir(module_path)

    def __add_path_to_solution_info(self, new_path):
        rel_path = os.path.relpath(new_path, self.__nap_root)
        if os.name != 'posix':
            rel_path = PurePath(rel_path).as_posix()
        add_to_solution_info(rel_path)

    def __process_demo(self, module_path):
        demo_wrapper_dir = os.path.join(module_path, 'demo')
        success = True
        if os.path.exists(demo_wrapper_dir):
            demos = [name for name in os.listdir(demo_wrapper_dir) if os.path.isdir(os.path.join(demo_wrapper_dir, name)) and not name.startswith('.')]
            if len(demos) > 0:
                if self.__deploy_demo is None:
                    self.__deploy_demo = read_yes_no(f"Deploy demo?")
                if self.__deploy_demo:
                    demo_app_id = demos[0]
                    if len(demos) > 1:
                        print(f"There appears to be more than one demo included with the module. {demo_app_id} will be deployed.")
                    demo_dir = os.path.join(demo_wrapper_dir, demo_app_id)
                    success = self.__install_demo(demo_app_id, demo_dir)
        return success

    def __install_demo(self, demo_app_id, demo_dir):
        demo_dest_dir = os.path.join(self.__nap_root, self.DEMO_DEST_DIR, demo_app_id)

        # Avoid clashing with existing app
        if self.DEMO_DEST_DIR != self.APPS_DIR:
            apps_duplicate_dir = os.path.join(self.__nap_root, self.APPS_DIR, demo_app_id)
            if os.path.exists(apps_duplicate_dir):
                eprint("Error: An app exists with the same name")
                return False

        # Handle existing demo
        if os.path.exists(demo_dest_dir):
            if not self.__force_overwrite_demo:
                demo_relpath = os.path.relpath(demo_dest_dir, self.__nap_root)
                if not self.__interactive or not read_yes_no(f"Demo already exists at {demo_relpath}, overwrite?"):
                    eprint(f"Error: Demo exists at {demo_dest_dir}")
                    return False
            shutil.rmtree(demo_dest_dir)

        # Copy demo
        print(f"Deploying demo {demo_app_id}")
        shutil.copytree(demo_dir, demo_dest_dir)

        # Deploy demo/app CMakeLists.txt
        app_cmakelists = os.path.join(self.__nap_root, 'cmake', 'app_creator', 'template', 'CMakeLists.txt')
        print("Copying demo CMakeLists.txt")
        shutil.copy(app_cmakelists, demo_dest_dir)

        # Deploy demo/app module CMakeLists.txt
        demo_module_dir = os.path.join(demo_dest_dir, 'module')
        if os.path.exists(demo_module_dir):
            print("Copying demo module CMakeLists.txt")
            shutil.copy(self.__module_cmakelists, demo_module_dir)

        # Add demo to solution info in Source context
        if get_build_context() == 'source':
            print("Adding demo to solution info")
            self.__add_path_to_solution_info(demo_dest_dir)

        # Deploy app dir shortcuts
        self.__deploy_shortcuts('app_dir_shortcuts', demo_dest_dir)

        # Build and run?
        if self.__run_demo is None:
            # Due to argument processing in the constructor we know we're not running interactively
            self.__run_demo = read_yes_no(f"Build and run demo?", 'n')
        if self.__run_demo:
            if not self.__build_and_launch_demo(demo_app_id):
                return False
        return True

    def __build_and_launch_demo(self, demo_app_id):
        # Remove any previous binary ensuring post-build steps are run, deploying the path mapping
        binary_path = self.__build_binary_path(demo_app_id)
        if os.path.exists(binary_path):
            os.remove(binary_path)

        print(f"Building {demo_app_id}")
        build_script = os.path.join(self.__tools_dir, 'build_app') + ModuleInitialiser.__get_platform_scriptextension()
        cmd = f'{build_script} {demo_app_id}'
        p = run(cmd, shell=True)
        if p.returncode != 0:
            eprint(f"Error: {demo_app_id} failed to build")
            return False

        print("Launching demo")
        run(binary_path, shell=True)
        return True

    def __build_binary_path(self, app_name, release_build=True):
        build_config = ModuleInitialiser.__get_build_config(release_build)
        if get_build_context() == 'source':
            path = os.path.join(self.__nap_root, 'bin', build_config, app_name)
        else:
            path = os.path.join(self.__nap_root, self.DEMO_DEST_DIR, app_name, 'bin', build_config, app_name)
        if sys.platform == 'win32':
            path += '.exe'
        return path

    def __deploy_shortcuts(self, shortcut_dir_name, dest_path):
        shortcuts_dir = os.path.join(self.__tools_dir,
                                     'buildsystem',
                                     shortcut_dir_name,
                                     'unix' if os.name == 'posix' else 'win64',
                                     '*'
                                     )
        for file in glob.glob(shortcuts_dir):
            shutil.copy(file, dest_path)
            if os.name == 'posix':
                dest_file = os.path.join(dest_path, PurePath(file).name)
                ensure_set_executable(dest_file)

    @staticmethod
    def __get_platform_scriptextension():
        return '' if sys.platform == 'win32' else '.sh'

    @staticmethod
    def __get_build_config(release_build=True):
        build_type = 'Release' if release_build else 'Debug'
        arch = get_build_arch()
        return f'{build_type}-{arch}'

def yes_no_to_bool(val):
    ret = None
    if val == 'yes':
        ret = True
    elif val == 'no':
        ret = False
    return ret

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("MODULE_NAME", type=str, help="The module name")
    parser.add_argument("--interactive", choices=['yes', 'no'], default='yes', help="Disable interactive prompts")
    parser.add_argument("--deploy-demo", choices=['yes', 'no'], help="Deploy a demo, if it exists")
    parser.add_argument("--run-demo", choices=['yes', 'no'], help="Run (and deploy) a demo, if it exists")
    parser.add_argument("--force-overwrite-demo", choices=['yes', 'no'], help="Force overwriting of any existing demo")
    args = parser.parse_args()

    initialiser = ModuleInitialiser(yes_no_to_bool(args.interactive),
                                    yes_no_to_bool(args.deploy_demo),
                                    yes_no_to_bool(args.run_demo),
                                    yes_no_to_bool(args.force_overwrite_demo)
                                    )

    if not initialiser.setup_module_by_name(args.MODULE_NAME):
        sys.exit(1)
