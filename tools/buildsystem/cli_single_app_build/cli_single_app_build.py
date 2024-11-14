#!/usr/bin/env python3
import argparse
import os
import subprocess
from multiprocessing import cpu_count
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_python_path, get_default_build_dir_name, Platform, BuildType, get_system_generator

ERROR_CANT_LOCATE_NAP = 1
ERROR_CONFIGURE = 2
ERROR_CANT_LOCATE_APP = 3

class SingleAppBuilder:

    def __init__(self):
        pass

    def call(self, cwd, cmd, shell=False):
        proc = subprocess.Popen(cmd, cwd=cwd, shell=shell)
        proc.communicate()
        return proc.returncode

    def determine_environment(self):
        """Verify environment and populate paths"""
        script_path = os.path.dirname(os.path.realpath(__file__))
        script_to_nap_root = os.path.join(os.pardir, os.pardir, os.pardir)

        # Check for Source context
        source_root_cmakelists = os.path.join(script_path, script_to_nap_root, 'CMakeLists.txt')
        if os.path.exists(source_root_cmakelists):
            self.__nap_root = os.path.abspath(os.path.join(script_path, script_to_nap_root))
            self.__source_context = True
        else:
            # Verify in Framework Release context
            package_modules_dir = os.path.join(script_path, script_to_nap_root, 'system_modules')
            if not os.path.exists(package_modules_dir):
                print("Error: Can't locate NAP root")
                sys.exit(ERROR_CANT_LOCATE_NAP)
            self.__nap_root = os.path.abspath(os.path.join(script_path, script_to_nap_root))
            self.__source_context = False

            # Determine Python interpreter location
            self.__python = get_python_path()

    def build(self, app_name, build_type):

        # Determine environment, adapt for Source and Framework Release contexts
        self.determine_environment()

        # Build
        if self.__source_context:
            self.build_source_context_app(app_name, build_type)
        else:
            self.build_packaged_framework_app(app_name, build_type)

    def build_source_context_app(self, app_name, build_type):
        # Late import to handle different operating contexts
        sys.path.append(os.path.join(self.__nap_root, 'tools', 'buildsystem', 'common'))
        from nap_shared import find_app
        (app_path, app_name) = find_app(app_name, False, True)
        if app_path is None:
            print("Error: Can't find app %s" % app_name)
            sys.exit(ERROR_CANT_LOCATE_APP)

        # Create solution generation cmd
        build_dir = os.path.join(self.__nap_root, get_default_build_dir_name())
        gen_cmd = ['./generate_solution.%s' % ('bat' if Platform.get() == Platform.Windows else 'sh'), '--build-path=%s' % build_dir]

        # Add build type if generator is single
        if get_system_generator().is_single():
            gen_cmd.extend(['-t', build_type])

        # Generate solution
        if self.call(self.__nap_root, gen_cmd) != 0:
            print("Error: Failed to configure app")
            sys.exit(ERROR_CONFIGURE)

        # Build solution
        build_cmd = [get_cmake_path(), '--build', build_dir, '--target', app_name, '--config', build_type, '-j', str(cpu_count())]
        if not get_system_generator().is_single():
            build_cmd.extend(['--config', 'build_type'])
        self.call(self.__nap_root, build_cmd)

    def build_packaged_framework_app(self, app_name, build_type):
        # Late import to handle different operating contexts
        sys.path.append(os.path.join(self.__nap_root, 'tools', 'buildsystem', 'common'))
        from nap_shared import find_app
        (app_path, app_name) = find_app(app_name, False, True)
        if app_path is None:
            print("Error: Can't find app %s" % app_name)
            sys.exit(ERROR_CANT_LOCATE_APP)

        # Only explicitly regenerate the solution if it doesn't already exist or a build type has
        # been specified. Provides quicker CLI build times if regeneration not required while
        # ensuring that the right build type will be generated for Napkin on Linux.
        build_dir = os.path.join(app_path, get_default_build_dir_name())
        generate_solution = build_type != None or not os.path.exists(build_dir)

        if generate_solution:
            cmd = [self.__python, './tools/buildsystem/common/regenerate_app_by_name.py',
                    app_name, '-t', build_type, '--no-show']
            if self.call(self.__nap_root, cmd) != 0:
                print("Error: Solution generation failed")
                sys.exit(ERROR_CONFIGURE)

        # Build solution
        build_cmd = [get_cmake_path(), '--build', build_dir, '--target', app_name, '-j', str(cpu_count())]
        if not get_system_generator().is_single():
            build_cmd.extend(['--config', 'build_type'])
        self.call(self.__nap_root, build_cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument("APP_NAME", 
        type=str, 
        help="The app name")

    parser.add_argument('-t', '--build-type',
        type=str,
        default=BuildType.get_default(),
        action='store', nargs='?',
        choices=BuildType.to_list(),
        help="Build type, default: {0}".format(BuildType.get_default()))
    args = parser.parse_args()

    b = SingleAppBuilder()
    b.build(args.APP_NAME, args.build_type)
