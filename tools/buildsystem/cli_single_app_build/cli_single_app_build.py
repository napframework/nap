#!/usr/bin/env python3
import argparse
import os
import subprocess
from platform import machine
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil

LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'Xcode'
MSVC_BUILD_DIR = 'msvc64'
THIRDPARTY = 'thirdparty'

DEFAULT_BUILD_TYPE = 'Release'

ERROR_CANT_LOCATE_NAP = 1
ERROR_CONFIGURE = 2
ERROR_CANT_LOCATE_PROJECT = 3

class SingleProjectBuilder:

    def __init__(self):
        pass

    def call(self, cwd, cmd, shell=False):
#       print('Dir: %s' % cwd)
#       print('Command: %s' % ' '.join(cmd))
        proc = subprocess.Popen(cmd, cwd=cwd, shell=shell)
        proc.communicate()
        return proc.returncode

    def get_cmake_path(self):
        """Fetch the path to the CMake binary"""

        if self.__source_context:
            cmake_thirdparty_root = os.path.join(self.__nap_root, os.pardir, THIRDPARTY, 'cmake')
            if platform.startswith('linux'):
                arch = machine()
                if arch == 'x86_64':
                    return os.path.join(cmake_thirdparty_root, 'linux', 'x86_64', 'bin', 'cmake')
                elif arch == 'aarch64':
                    return os.path.join(cmake_thirdparty_root, 'linux', 'arm64', 'bin', 'cmake')
                else:
                    return os.path.join(cmake_thirdparty_root, 'linux', 'armhf', 'bin', 'cmake')
            elif platform == 'darwin':
                return os.path.join(cmake_thirdparty_root, 'macos', 'x86_64', 'bin', 'cmake')
            else:
                return os.path.join(cmake_thirdparty_root, 'msvc', 'x86_64', 'bin', 'cmake')
        else:
            cmake_thirdparty_root = os.path.join(self.__nap_root, THIRDPARTY, 'cmake')
            return os.path.join(cmake_thirdparty_root, 'bin', 'cmake')

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
            if sys.platform == 'win32':
                self.__python = os.path.join(self.__nap_root, 'thirdparty', 'python', 'python')
            else:
                self.__python = os.path.join(self.__nap_root, 'thirdparty', 'python', 'bin', 'python3')

    def build(self, project_name, build_type):
        if not build_type is None:
            build_type = build_type.lower().capitalize()

        # Determine environment, adapt for Source and Framework Release contexts
        self.determine_environment()

        # Build
        if self.__source_context:
            self.build_source_context_project(project_name, build_type)
        else:
            self.build_packaged_framework_project(project_name, build_type)

    def build_source_context_project(self, project_name, build_type):
        if build_type is None:
            build_type = DEFAULT_BUILD_TYPE
        else:
            build_type = build_type.capitalize()

        # Late import to handle different operating contexts
        sys.path.append(os.path.join(self.__nap_root, 'tools', 'buildsystem', 'common'))
        from nap_shared import find_app
        project_path = find_app(project_name, False, True)
        if project_path is None:
            print("Error: Can't find project %s" % project_name)
            sys.exit(ERROR_CANT_LOCATE_PROJECT)

        build_dir = None
        if platform.startswith('linux'):
            build_dir = LINUX_BUILD_DIR
        elif platform == 'darwin':
            build_dir = MACOS_BUILD_DIR
        else:
            build_dir = MSVC_BUILD_DIR
        build_dir = os.path.join(self.__nap_root, build_dir)

        # Generate solution
        if platform.startswith('linux'):
            rc = self.call(self.__nap_root, ['./generate_solution.sh', '--build-path=%s' % build_dir, '-t', build_type.lower()])
        elif platform == 'darwin':
            rc = self.call(self.__nap_root, ['./generate_solution.sh', '--build-path=%s' % build_dir])
        else:
            rc = self.call(self.__nap_root, ['generate_solution.bat', '--build-path=%s' % build_dir], True)

        if rc != 0:
            print("Error: Failed to configure project")
            sys.exit(ERROR_CONFIGURE)

        if platform.startswith('linux'):
            # Linux
            self.call(build_dir, ['make', project_name, '-j%s' % cpu_count()])
        elif platform == 'darwin':
            # macOS
            self.call(build_dir, ['xcodebuild', '-project', 'NAP.xcodeproj', '-target', project_name, '-configuration', build_type])
        else:
            # Windows
            cmake = self.get_cmake_path()
            self.call(self.__nap_root, [cmake, '--build', build_dir, '--target', project_name, '--config', build_type])

    def build_packaged_framework_project(self, project_name, build_type):
        # Late import to handle different operating contexts
        sys.path.append(os.path.join(self.__nap_root, 'tools', 'buildsystem', 'common'))
        from nap_shared import find_app
        project_path = find_app(project_name, False, True)
        if project_path is None:
            print("Error: Can't find project %s" % project_name)
            sys.exit(ERROR_CANT_LOCATE_PROJECT)

        build_dir = None
        if platform.startswith('linux'):
            build_dir = LINUX_BUILD_DIR
        elif platform == 'darwin':
            build_dir = MACOS_BUILD_DIR.lower()
        else:
            build_dir = MSVC_BUILD_DIR
        build_dir = os.path.join(project_path, build_dir)

        # Only explicitly regenerate the solution if it doesn't already exist or a build type has
        # been specified. Provides quicker CLI build times if regeneration not required while 
        # ensuring that the right build type will be generated for Napkin on Linux.
        generate_solution = build_type != None or not os.path.exists(build_dir) 

        if build_type is None:
            build_type = DEFAULT_BUILD_TYPE
        else:
            build_type = build_type.capitalize()

        if generate_solution:
            cmd = [self.__python, './tools/buildsystem/common/regenerate_app_by_name.py', project_name]
            if sys.platform.startswith('linux'):
                cmd.append(build_type)        
            else:
                cmd.append('--no-show')        
            if self.call(self.__nap_root, cmd) != 0:
                print("Error: Solution generation failed")
                sys.exit(ERROR_CONFIGURE)

        if platform.startswith('linux'):
            # Linux
            self.call(build_dir, ['make', project_name, '-j%s' % cpu_count()])
        elif platform == 'darwin':
            # macOS
            self.call(build_dir, ['xcodebuild', '-project', '%s.xcodeproj' % project_name, '-configuration', build_type])
        else:
            # Windows
            cmake = self.get_cmake_path()
            self.call(self.__nap_root, [cmake, '--build', build_dir, '--target', project_name, '--config', build_type], True)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str, help="The project name")
    parser.add_argument('-t', '--build-type', type=str.lower, default=None,
            choices=['release', 'debug'], help="Build type (default=%s)" % DEFAULT_BUILD_TYPE.lower())
    args = parser.parse_args()

    b = SingleProjectBuilder()
    b.build(args.PROJECT_NAME, args.build_type)
