#!/usr/bin/env python
"""Project.info and module.info conversion script from old to new format.

TODO: Update this docstring to account for versioning

This script will either
    - take a project and convert its json descriptors to the latest format
    - or take the NAP root directory and convert all projects/modules to the latest format

See the main() function below.

"""

import argparse
import json
import logging
import os
import sys
from collections import OrderedDict

LOG = logging.getLogger(os.path.basename(os.path.dirname(__file__)))


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
        print('File not found: %s' % filepath)
        return None, None

    with open(filepath, 'r') as fp:
        data = json.load(fp, object_pairs_hook=OrderedDict)

    return filepath, data


def _find_data_file(root_dir):
    proj_name = os.path.basename(root_dir)
    assert proj_name, 'Expected root dir to be in a project directory: %s' % root_dir
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


def convert_module(directory):
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


def convert_project_info(directory):
    """Find a projectinfo file in the specified directory, convert to new format and write to same file.
    This will also attempt to merge and convert any existing service configurations into the same file
    and delete the original config.json
    """
    filepath, proj_info_json = _load_json(directory, 'project.json')
    if not proj_info_json:
        return

    # has this file been converted yet?
    if any(t not in proj_info_json for t in ('Type', 'mID')):
        print('Converting: %s' % filepath)

        new_proj_info_json = OrderedDict((
            ('Type', 'nap::ProjectInfo'),
            ('mID', 'ProjectInfo'),
            ('Title', proj_info_json.get('title')),
            ('Version', proj_info_json.get('version')),
            ('RequiredModules', proj_info_json.get('modules', [])),
            ('ServiceConfig', list(_convert_service_config(directory))),
        ))

    else:
        new_proj_info_json = proj_info_json

    # -- ensure some values
    new_proj_info_json.setdefault('PathMapping', 'cache/path_mapping.json')
    new_proj_info_json.setdefault('Data', _find_data_file(directory) or '')

    with open(filepath, 'w') as fp:
        json.dump(new_proj_info_json, fp, indent=4)


def convert_project(project_dir):
    convert_project_info(project_dir)

    module_dir = os.path.join(project_dir, 'module')
    if os.path.exists(module_dir):
        convert_module_info(module_dir)
        convert_module(module_dir)


def convert_repository(root_directory):
    print('Convert project and moduleinfo files in nap repository: %s' % root_directory)
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
            directory = os.path.join(parent_dir, d)
            convert_project(directory)

    module_dirs = [
        'modules'
    ]
    for p in module_dirs:
        parent_dir = os.path.join(root_directory, p)
        for d in os.listdir(parent_dir):
            directory = os.path.join(parent_dir, d)
            convert_module(directory)


def convert_project_wrapper(args):
    convert_project(args.PROJECT_PATH)

def convert_module_wrapper(args):
    convert_module(args.MODULE_PATH)

def convert_repository_wrapper(args):
    nap_root_dir = os.path.realpath('%s/../../' % os.path.dirname(__file__))
    verify_path = os.path.join(nap_root_dir, 'modules')
    assert os.path.exists(verify_path), 'NAP root dir not found at: %s' % nap_root_dir
    convert_repository(nap_root_dir)

def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(metavar='COMMAND', help='UPGRADE_PROJECT, UPGRADE_MODULE, or UPGRADE_REPO')

    project_subparser = subparsers.add_parser('UPGRADE_PROJECT')
    project_subparser.add_argument('PROJECT_PATH', type=str, help='Path to project to upgrade')
    project_subparser.set_defaults(func=convert_project_wrapper)

    module_subparser = subparsers.add_parser('UPGRADE_MODULE')
    module_subparser.add_argument('MODULE_PATH', type=str, help='Path to module to upgrade')
    module_subparser.set_defaults(func=convert_module_wrapper)

    repo_subparser = subparsers.add_parser('UPGRADE_REPO')
    repo_subparser.set_defaults(func=convert_repository_wrapper)

    args = parser.parse_args()
    args.func(args)

if __name__ == '__main__':
    main()
