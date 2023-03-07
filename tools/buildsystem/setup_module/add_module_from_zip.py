#!/usr/bin/env python3

import argparse
import os
import shutil
from subprocess import run, PIPE
import sys
import zipfile

script_dir = os.path.dirname(__file__)
nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir))
sys.path.append(os.path.join(nap_root, 'tools', 'buildsystem', 'common'))
from nap_shared import eprint, get_nap_root, read_yes_no
from setup_module import ModuleInitialiser

class InvalidModuleArchive(Exception):
    pass

class ArchiveHandler():

    MODULES_DIR_NAME = 'modules'

    def __init__(self, interactive, force_overwrite_module, deploy_demo, run_demo, force_overwrite_demo):
        self.__modules_dir = os.path.join(get_nap_root(), self.MODULES_DIR_NAME)

        self.__interactive = interactive
        self.__force_overwrite_module = force_overwrite_module
        self.__deploy_demo = deploy_demo
        self.__run_demo = run_demo
        self.__force_overwrite_demo = force_overwrite_demo

    def add_module_from_archive(self, archive_path):
        self.__ensure_modules_dir()
        if not os.path.exists(archive_path):
            eprint(f"Error: Can't find module at {archive_path}")
            return False
        if os.path.isdir(archive_path) or not zipfile.is_zipfile(archive_path):
            eprint(f"Error: {archive_path} is not a module .zip file")
            return False

        success = False
        with zipfile.ZipFile(archive_path, mode='r') as archive:
            try:
                module_name = ArchiveHandler.read_module_id_from_archive(archive, archive_path)
            except InvalidModuleArchive as e:
                eprint("Error:", e)
                return False

            module_path = os.path.join(self.__modules_dir, module_name)
            if os.path.exists(module_path):
                if not self.__force_overwrite_module:
                    if not self.__interactive or not read_yes_no(f"Module already exists at {module_path}, overwrite?"):
                        eprint(f"Error: Module exists at {module_path}")
                        return False
                shutil.rmtree(module_path)
            print(f"Extracting {archive_path} to {self.__modules_dir}")
            if os.name == 'posix':
                # Use Info-ZIP to preserver symlinks on *nix
                abs_archive = os.path.abspath(archive_path)
                cmd = f'unzip {abs_archive}'
                p = run(cmd, shell=True, cwd=self.__modules_dir, stdout=PIPE)
            else:
                archive.extractall(path=self.__modules_dir)


            initialiser = ModuleInitialiser(self.__interactive,
                                            self.__deploy_demo,
                                            self.__run_demo,
                                            self.__force_overwrite_demo
                                            )
            success = initialiser.setup_module_by_dir(module_path)
        return success

    @staticmethod
    def read_module_id_from_archive(archive, archive_path):
        top = {item.split('/')[0] for item in archive.namelist() if not item.startswith('.')}
        if len(top) == 0:
            raise InvalidModuleArchive(f"{archive_path} doesn't appear to contain a module (empty)")
        elif len(top) > 1:
            raise InvalidModuleArchive(f"{archive_path} doesn't appear to contain a module (more than one top level directory)")
        return list(top)[0]

    def __ensure_modules_dir(self):
        if not os.path.exists(self.__modules_dir):
            print("Creating modules dir")
            os.mkdir(self.__modules_dir)

def yes_no_to_bool(val):
    ret = None
    if val == 'yes':
        ret = True
    elif val == 'no':
        ret = False
    return ret

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("ARCHIVE", type=str, help="The module archive")
    parser.add_argument("--interactive", choices=['yes', 'no'], default='yes', help="Disable interactive prompts")
    parser.add_argument("--force-overwrite-module", choices=['yes', 'no'], help="Force overwriting of any existing module")
    parser.add_argument("--deploy-demo", choices=['yes', 'no'], help="Deploy a demo, if it exists")
    parser.add_argument("--run-demo", choices=['yes', 'no'], help="Run (and deploy) a demo, if it exists")
    parser.add_argument("--force-overwrite-demo", choices=['yes', 'no'], help="Force overwriting of any existing demo")
    args = parser.parse_args()

    handler = ArchiveHandler(yes_no_to_bool(args.interactive),
                                 yes_no_to_bool(args.force_overwrite_module),
                                 yes_no_to_bool(args.deploy_demo),
                                 yes_no_to_bool(args.run_demo),
                                 yes_no_to_bool(args.force_overwrite_demo)
                                 )

    if not handler.add_module_from_archive(args.ARCHIVE):
        sys.exit(1)
