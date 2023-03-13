#!/usr/bin/env python3
"""Project.info and module.info conversion script from old to new format.

TODO: Update this docstring to account for versioning

This script will either
    - take an app and convert its json descriptors to the latest format
    - or take the NAP root directory and convert all apps/modules to the latest format

See the main() function below.

"""

import argparse
import json
import logging
import os
import sys
from collections import OrderedDict
from subprocess import call

# TODO Move out to wrapper scripts PYTHONPATH?
script_dir = os.path.dirname(__file__)
if not os.path.exists(os.path.join(script_dir, 'nap_shared.py')):
    nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir))
    sys.path.append(os.path.join(nap_root, 'tools', 'buildsystem', 'common'))
from nap_shared import get_cmake_path, get_nap_root, get_build_context, eprint

LOG = logging.getLogger(os.path.basename(os.path.dirname(__file__)))

CURRENT_CMAKE_VERSION = '3.18.4'

def _convert_module_ref(module_name):
    return {
        'Type': 'nap::ModuleInfo',
        'mID': module_name,
        'ModuleName': module_name,
    }

def _convert_service_config(directory):
    filename = os.path.join(directory, 'config.json')
    if not os.path.exists(filename):
        return

    with open(filename, 'r') as fp:
        service_config = json.load(fp)

    for obj in service_config.get('Objects', []):
        yield obj

    os.remove(filename)

def _load_json(directory, filename):
    if not os.path.exists(directory):
        raise IOError('Directory not found: %s' % directory)

    filepath = os.path.join(directory, filename)
    if not os.path.exists(filepath):
        eprint('File not found: %s' % filepath)
        return None, None

    with open(filepath, 'r') as fp:
        data = json.load(fp, object_pairs_hook=OrderedDict)

    return filepath, data

def _find_data_file(root_dir):
    proj_name = os.path.basename(root_dir)
    assert proj_name, 'Expected root dir to be in a app directory: %s' % root_dir
    data_dir = os.path.join(root_dir, 'data')

    # check for data dir first
    if not os.path.exists(data_dir):
        print('Expected data dir: %s' % data_dir)
        return

    # attempt to find common files
    guesses = (
        os.path.join(data_dir, '%s.json' % proj_name),
        os.path.join(data_dir, 'data.json'),
    )
    found_path = next((g for g in guesses if os.path.exists(g)), None)
    if found_path is None:
        return None
    else:
        return os.path.relpath(found_path, root_dir)

def _get_current_cmake_version_setter():
    return('cmake_minimum_required(VERSION {})'.format(CURRENT_CMAKE_VERSION))

def convert_module(directory):
    convert_module_info(directory)

    # Determine if it's an app module
    app_module = os.path.basename(directory) == 'module'

    # Update CMakeLists.txt in module root if in Framework Release context
    update_module_cmake(directory, app_module)

def update_module_cmake(directory, app_module):
    file_path = os.path.join(directory, 'CMakeLists.txt')
    # Allow for precompiled modules in Framework Release which don't have a CMakeLists.txt
    if not os.path.exists(file_path):
        return
    with open(file_path) as f:
        contents = f.read()

    if get_build_context() == 'framework_release':
        update_framework_release_module_cmake(directory, app_module, contents)
    else:
        update_source_module_cmake(directory, app_module, contents)

def update_source_module_cmake(directory, app_module, contents):
    # Detect if needs update
    needs_update = False

    # Check Unix library 'lib' prefix stripping for v0.4
    cmake_version_setter = _get_current_cmake_version_setter()
    if not 'PROPERTIES PREFIX ""' in contents:
        needs_update = True
    if not cmake_version_setter in contents:
        needs_update = True
    if not needs_update:
        print("Module at %s doesn't need CMake update" % directory)
        return

    print("Upgrading module CMake at %s" % directory)

    if not cmake_version_setter in contents:
        index = contents.find('cmake_minimum_required')
        if index == -1:
            contents = '{}\n{}'.format(cmake_version_setter, contents)
        else:
            closing_index = contents.find(')', index)
            contents = '{}{}{}'.format(contents[:index], cmake_version_setter, contents[closing_index+1:])

    file_path = os.path.join(directory, 'CMakeLists.txt')
    with open(file_path, 'w') as f:
        f.write(contents)

def update_framework_release_module_cmake(directory, app_module, contents):
    # Detect if needs update
    needs_update = False
    # Check for app definition relocation for v0.4
    if not 'dist_shared_crossplatform.cmake' in contents:
        needs_update = True
    if not _get_current_cmake_version_setter() in contents:
        needs_update = True
    if not needs_update:
        print("Module at %s doesn't need CMake update" % directory)
        return

    cmake = get_cmake_path()
    nap_root = get_nap_root()

    # Create module from template
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'module_creator'))
    if not os.path.exists(cmake_template_dir):
        cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'dist', 'cmake', 'native', 'module_creator'))

    print("Upgrading module CMake at %s" % directory)
    cmd = [cmake,
           '-DMODULE_CMAKE_OUTPATH=%s' % os.path.join(directory, 'CMakeLists.txt'),
           '%s' % '-DAPP_MODULE=1' if app_module else '',
           '-DCMAKE_ONLY=1',
           '-P', os.path.join(cmake_template_dir, 'module_creator.cmake')
           ]
    if call(cmd) != 0:
        eprint("CMake upgrade at %s failed" % directory)

def convert_module_info(directory):
    """Find a moduleinfo file in the specified directory, convert to new format and write to same file"""
    filepath, mod_info_json = _load_json(directory, 'module.json')
    if not filepath:
        return

    if any(t not in mod_info_json for t in ('Type', 'mID')):
        print('Converting: %s' % filepath)

        new_mod_info_json = OrderedDict((
            ('Type', 'nap::ModuleInfo'),
            ('mID', 'ModuleInfo'),
            ('RequiredModules', mod_info_json.get('dependencies', [])),
        ))

    else:
        new_mod_info_json = mod_info_json

    with open(filepath, 'w') as fp:
        json.dump(new_mod_info_json, fp, indent=4)

def convert_app_info(directory):
    """Find a projectinfo file in the specified directory, convert to new format and write to same file.
    This will also attempt to merge and convert any existing service configurations into the same file
    and delete the original config.json
    """
    filepath, proj_info_json = _load_json(directory, 'app.json')
    if not proj_info_json:
        return

    # Has this file been converted yet?
    if any(t not in proj_info_json for t in ('Type', 'mID')):
        print('Converting: %s' % filepath)

        new_proj_info_json = OrderedDict((
            ('Type', 'nap::ProjectInfo'),
            ('mID', 'ProjectInfo'),
            ('Title', proj_info_json.get('title')),
            ('Version', proj_info_json.get('version')),
            ('RequiredModules', proj_info_json.get('modules', [])),
            # TODO ServiceConfig changes disabled until resolved
            #('ServiceConfig', list(_convert_service_config(directory))),
            ('ServiceConfig', ''),
        ))

    else:
        new_proj_info_json = proj_info_json

    # Ensure some values
    new_proj_info_json.setdefault('PathMapping', 'cache/path_mapping.json')
    new_proj_info_json.setdefault('Data', _find_data_file(directory) or '')

    with open(filepath, 'w') as fp:
        json.dump(new_proj_info_json, fp, indent=4)

def convert_app(app_dir):
    convert_app_info(app_dir)

    module_dir = os.path.join(app_dir, 'module')
    if os.path.exists(module_dir):
        convert_module(module_dir)

    # Update CMakeLists.txt in app root if in Framework Release context
    update_app_cmake(app_dir)

def update_app_cmake(directory):
    file_path = os.path.join(directory, 'CMakeLists.txt')
    try:
        with open(file_path) as f:
            contents = f.read()
    except FileNotFoundError:
        return

    # Detect if needs update
    needs_update = False
    # Check for app definition relocation for v0.4
    if not 'dist_shared_crossplatform.cmake' in contents:
        needs_update = True
    if not _get_current_cmake_version_setter() in contents:
        needs_update = True
    if not needs_update:
        print("App at %s doesn't need CMake update" % directory)
        return

    cmake = get_cmake_path()
    nap_root = get_nap_root()

    # Create module from template
    cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'cmake', 'app_creator'))
    if not os.path.exists(cmake_template_dir):
        cmake_template_dir = os.path.abspath(os.path.join(nap_root, 'dist', 'cmake', 'native', 'app_creator'))

    print("Upgrading app CMake at %s" % directory)
    cmd = [cmake,
           '-DPROJECT_DIR=%s' % directory,
           '-DCMAKE_ONLY=1',
           '-P', os.path.join(cmake_template_dir, 'app_creator.cmake')
           ]
    if call(cmd) != 0:
        eprint("CMake upgrade at %s failed" % directory)

def convert_all(root_directory):
    print('Convert projectinfo and moduleinfo files in NAP directory: %s' % root_directory)
    project_dirs = [
        'apps',
        'demos',
        'projects',
        'test',
    ]
    for p in project_dirs:
        parent_dir = os.path.join(root_directory, p)
        if not os.path.exists(parent_dir):
            continue
        for d in os.listdir(parent_dir):
            if not d.startswith('.'):
                directory = os.path.join(parent_dir, d)
                convert_app(directory)

    module_dirs = [
        'system_modules',
        'modules',
        'user_modules',
    ]
    for p in module_dirs:
        parent_dir = os.path.join(root_directory, p)
        if not os.path.exists(parent_dir):
            continue
        for d in os.listdir(parent_dir):
            if not d.startswith('.'):
                directory = os.path.join(parent_dir, d)
                convert_module(directory)

def convert_app_wrapper(args):
    directory = os.path.abspath(args.APP_PATH)
    assert os.path.exists(directory), 'Directory does not exist at: %s' % directory
    convert_app(directory)

def convert_module_wrapper(args):
    directory = os.path.abspath(args.MODULE_PATH)
    assert os.path.exists(directory), 'Directory does not exist at: %s' % directory
    convert_module(directory)

def convert_all_wrapper(args):
    nap_root_dir = get_nap_root()
    verify_path = os.path.join(nap_root_dir, 'system_modules')
    assert os.path.exists(verify_path), 'NAP root dir not found at: %s' % nap_root_dir
    convert_all(nap_root_dir)

def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(metavar='command', help='UPGRADE_APP, UPGRADE_MODULE, or UPGRADE_ALL')

    app_subparser = subparsers.add_parser('UPGRADE_APP')
    app_subparser.add_argument('APP_PATH', type=str, help='Path to app to upgrade')
    app_subparser.set_defaults(func=convert_app_wrapper)

    module_subparser = subparsers.add_parser('UPGRADE_MODULE')
    module_subparser.add_argument('MODULE_PATH', type=str, help='Path to module to upgrade')
    module_subparser.set_defaults(func=convert_module_wrapper)

    all_subparser = subparsers.add_parser('UPGRADE_ALL')
    all_subparser.set_defaults(func=convert_all_wrapper)

    args = parser.parse_args()
    if not hasattr(args, 'func'):
        parser.print_help(sys.stderr)
        sys.exit(1)

    args.func(args)

if __name__ == '__main__':
    main()
