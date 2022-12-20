#!/usr/bin/env python3

import argparse
import filecmp
import json
import os
from pathlib import PurePath
import shutil
from subprocess import run, PIPE
import sys

script_dir = os.path.dirname(__file__)
nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir))
sys.path.append(os.path.join(nap_root, 'tools', 'buildsystem', 'common'))
from nap_shared import eprint, get_nap_root

MODULE_JSON_FILE = 'module.json'
DEMO_APP_JSON_KEY = 'DemoApp'
DEMO_SEARCH_DIRS = ('apps', 'demos')
SYSTEM_MODULES_DIR = 'system_modules'

def prepare_module(module_dir, overwrite):
    # Check git installed/available
    if not check_git_installed():
        eprint("Error: This script depends on Git")
        return False

    # Check path exists, is dir and has module.json
    module_json_path = os.path.join(module_dir, MODULE_JSON_FILE)
    if not os.path.isdir(module_dir) or not os.path.isfile(module_json_path):
        eprint(f"Error: {module_dir} does not appear to be a valid module directory")
        return False

    # Check not a system module
    if PurePath(module_dir).parent == SYSTEM_MODULES_DIR:
        eprint(f"Error: {module_dir} is a system module")
        return False

    module_name = PurePath(module_dir).name
    nap_root = get_nap_root()
    print(f"Processing module {module_name}")

    # Read module.json to get any demo name
    demo_app_name = None
    with open(module_json_path, 'r') as json_file:
        try:
            json_dict = json.load(json_file)
        except Exception as e:
            eprint(f"Error: Failed to parse {module_json_path}:", e)
            return False
        if DEMO_APP_JSON_KEY in json_dict:
            demo_app_val = json_dict[DEMO_APP_JSON_KEY]
            if type(demo_app_val) is str and demo_app_val.strip() != '':
                demo_app_name = demo_app_val.strip()

    # If a demo is specified ensure it exists
    demo_dir = None
    if demo_app_name:
        (demo_dir, success) = get_non_local_demo_dir(demo_app_name, module_dir, nap_root)
        if not success:
            return False

    # Prepare destination
    # TODO add version?
    # TODO allow custom path specification as argument?
    dest_path = os.path.abspath(os.path.join(nap_root, os.pardir, module_name))
    if os.path.exists(dest_path):
        if not overwrite:
            eprint(f"Error: {dest_path} exists and not overwriting")
            return False
        shutil.rmtree(dest_path)

    # Copy tree to location alongside NAP root
    shutil.copytree(module_dir, dest_path)

    # Copy any (non-local) demo into the destination module tree
    if demo_dir:
        dest_demo_dir = os.path.join(dest_path, 'demo')
        shutil.copytree(module_dir, dest_demo_dir)

    # Clean
    clean_module(dest_path, nap_root)

    # Log path
    print(f"Prepared to {dest_path}")
    return True

def run_cmd(cmd, show_output=False):
    if show_output:
        p = run(cmd, shell=True)
    else:
        p = run(cmd, shell=True, stdout=PIPE, stderr=PIPE)
    return p.returncode == 0

def check_git_installed():
    cmd = 'git --version'
    return run_cmd(cmd)

def get_non_local_demo_dir(demo_app_name, module_dir, nap_root):
    """Get the directory of the demo to include with the module

    If the demo is found under the local module directory that location is preferred and the
    directory isn't returned as it will be included with the main module directory copy.
    """

    print(f"Demo: {demo_app_name}")
    demo_found_locally = False
    for search_dir in DEMO_SEARCH_DIRS:
        check_dir = os.path.join(nap_root, search_dir, demo_app_name)
        if os.path.exists(check_dir):
            demo_dir = check_dir
            break
    demo_dir_under_module = os.path.join(module_dir, 'demo', demo_app_name)
    if os.path.exists(demo_dir_under_module):
        demo_found_locally = True
        if not demo_dir is None:
            reldir = os.path.relpath(demo_dir)
            print(f"Demo was found at {reldir} however {demo_dir_under_module} will be used instead")
            # We're found the demo locally next to the module, don't return any other found path
            demo_dir = None
        else:
            demo_dir = demo_dir_under_module
    if demo_dir is None and not demo_found_locally:
        eprint(f"Error: {demo_app_name} could not be found")
        return (None, False)
    return (demo_dir, True)

def clean_module(dest_path, nap_root):
    # If .gitignore missing copy from template
    dest_gitignore = os.path.join(dest_path, '.gitignore')
    template_gitignore = os.path.relpath(os.path.join(nap_root, 'cmake', 'module_creator', 'template', '.gitignore'))
    if os.path.exists(dest_gitignore):
        # Log a note if the gitignore has diverged from our template version
        if not filecmp.cmp(dest_gitignore, template_gitignore):
            print(f"Existing module .gitignore found and leaving as is. Please ensure this contains the entries in {template_gitignore}")
    else:
        shutil.copy(template_gitignore, dest_path)

    # Git init if no .git exists
    os.chdir(dest_path)
    git_initialised = False
    if not os.path.exists('.git'):
        if not run_cmd('git init'):
            eprint("Error: Failed to create temporary git repo")
            return False
        git_initialised = True

    # Apply .gitignore
    if not run_cmd('git clean -fdX', show_output=True):
        eprint("Error: Failed to apply .gitignore")
        return False

    # Remove any .git if it was added
    if git_initialised:
        shutil.rmtree('.git')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('MODULE_DIR', type=str, help="The directory the module")
    parser.add_argument('--overwrite', action='store_true', help="Overwrite any existing output directory")
    args = parser.parse_args()

    if not prepare_module(args.MODULE_DIR, args.overwrite):
        sys.exit(1)
