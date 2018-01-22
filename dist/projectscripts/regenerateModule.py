#!/usr/bin/python

import sys
import os
import subprocess

if sys.platform == 'darwin':
    BUILD_DIR = 'xcode'
elif sys.platform == 'win32':
    BUILD_DIR = 'msvc64'
else:
    BUILD_DIR = 'build'

def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out

def find_module(module_name):
    script_path = os.path.realpath(__file__)
    # TODO clean up, use absolute path
    nap_root = os.path.join(os.path.dirname(script_path), '..')

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
        # TODO temporarily? disabled as it seems too restrictive
        # elif module_dir_name.startswith('mod_nap'):
        #     print("Module %s at %s is a NAP module and can't be regenerated" % (module_name, module_path))
        #     return None
        else:
            print("Module %s at %s does not contain CMakeLists.txt and can't be regenerated" % (module_name, module_path))
            return None
    else:
        print("Couldn't find module with name '%s'" % module_name)
        return None

def update_module(module_name, build_type):
    module_path = find_module(module_name)
    if module_path is None:
        return

    if sys.platform in ["linux", "linux2"]:
        call(module_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-DCMAKE_BUILD_TYPE=%s' % build_type])
    elif sys.platform == 'darwin':
        call(module_path, ['cmake', '-H.', '-B%s' % BUILD_DIR, '-G', 'Xcode'])
    else:
        # create dir if it doesn't exist
        full_build_dir = os.path.join(module_path, BUILD_DIR)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)

        # generate prject
        call(module_path, ['cmake', '-H.','-B%s' % BUILD_DIR,'-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5'])

    print("Solution generated in %s" % os.path.relpath(os.path.join(module_path, BUILD_DIR)))

if __name__ == '__main__':
    # TODO update to use argparse

    if len(sys.argv) < 2:
        if sys.platform in ["linux", "linux2"]:
            print("Usage: %s MODULE_NAME [BUILD_TYPE]"  % sys.argv[0])
        else:
            print("Usage: %s MODULE_NAME" % sys.argv[0])
        print("\n  eg. %s MyModuleName" % sys.argv[0])
        sys.exit(1)

    module_name = sys.argv[1]

    # If we're on Linux and we've specified a build type let's grab that, otherwise
    # default to debug
    if sys.platform in ["linux", "linux2"]:
        if len(sys.argv) == 3:
            build_type = sys.argv[2]
        else:
            build_type = 'Debug'
        print("Using build type '%s'" % build_type)
    else:
        build_type = None

    update_module(module_name, build_type)
