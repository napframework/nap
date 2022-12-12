#!/usr/bin/env python3

import argparse
import json
import os
import sys

from nap_shared import get_platform_name, eprint

class ModuleInfoParser():

    MODULE_INFO_FILENAME = 'module.json'
    MODULE_INFO_CMAKE_CACHE_FILENAME = 'cached_module_json.cmake'

    def __init__(self, nap_root, path_mapping_file, arch, path_mapping_element):
        self.__nap_root = nap_root
        self.__arch = arch
        self.__path_mapping_element = path_mapping_element

        # Check our path mapping exists
        if not os.path.exists(path_mapping_file):
            raise Exception(f"Path mapping missing at {path_mapping_file}")
        self.__get_path_relationship_from_path_mapping(path_mapping_file)

        self.__full_nap_modules = []
        self.__rpaths = []
        self.__new_dependencies = []

    def __find_module(self, module_name):
        module_path = None
        # Check for NAP module in framework release
        if os.path.exists(os.path.join(self.__nap_root, 'system_modules', module_name)):
            module_path = os.path.join(self.__nap_root, 'system_modules', module_name)
        # Check for user module
        elif os.path.exists(os.path.join(self.__nap_root, 'modules', module_name)):
            module_path = os.path.join(self.__nap_root, 'modules', module_name)

        if module_path is None:
            raise Exception(f"Couldn't find module {module_name}")
        return module_path

    def __find_new_dependencies_for_modules(self):
        output_dependencies = []
        for module_name in self.__new_dependencies:
            module_path = self.__find_module(module_name)
            (module_dependencies, module_rpaths) = self.__read_single_module_dependencies(module_path)
            for found_module_name in module_dependencies:
                if found_module_name not in self.__full_nap_modules and found_module_name not in output_dependencies:
                    output_dependencies.append(found_module_name)
            for rpath in module_rpaths:
                if rpath not in self.__rpaths:
                    self.__rpaths.append(rpath)

        self.__new_dependencies = output_dependencies

    def __read_single_module_dependencies(self, module_path):
        # Read in the JSON
        module_json_path = os.path.join(module_path, self.MODULE_INFO_FILENAME)
        with open(module_json_path) as json_file:
            json_dict = json.load(json_file)
            if not 'RequiredModules' in json_dict:
                raise Exception(f"Missing element 'RequiredModules' in {module_json_path}")

            if not type(json_dict['RequiredModules']) is list:
                raise Exception(f"Element 'RequiredModules' in {self.MODULE_INFO_FILENAME} is not an array")

            rpaths = []
            if 'LibrarySearchPaths' in json_dict:
                platform = get_platform_name()
                if not type(json_dict['LibrarySearchPaths']) is dict:
                    raise Exception(f"Element 'LibrarySearchPaths' in {self.MODULE_INFO_FILENAME} is not a dict")

                if platform in json_dict['LibrarySearchPaths']:
                    if not type(json_dict['LibrarySearchPaths'][platform]) is list:
                        raise Exception(f"Element 'LibrarySearchPaths' > '{platform}' in {self.MODULE_INFO_FILENAME} is not a dict")
                    rpaths.extend(json_dict['LibrarySearchPaths'][platform])

                    # Make RPATH substitutions
                    for i in range(len(rpaths)):
                        rpaths[i] = self.__substitute_in_path_mapping(rpaths[i], module_path)

            return (json_dict['RequiredModules'], rpaths)

    def __get_path_relationship_from_path_mapping(self, path_mapping_path):
        # Read in the JSON
        with open(path_mapping_path) as json_file:
            json_dict = json.load(json_file)
            if not self.__path_mapping_element in json_dict:
                raise Exception(f"Missing element '{self.__path_mapping_element}' in {path_mapping_path}")
            self.__exe_to_root_mapping = json_dict[self.__path_mapping_element]

    def __substitute_in_path_mapping(self, path, module_dir):
        module_relative_to_root = os.path.relpath(module_dir, start=self.__nap_root)
        path = path.replace('{MODULE_DIR}', f'{self.__exe_to_root_mapping}/{module_relative_to_root}')
        path = path.replace('{ROOT}', self.__exe_to_root_mapping)
        path = path.replace('{BUILD_ARCH}', self.__arch)
        return path

    def run_from_module_json(self, module_path):
        # Check our module exists
        if not os.path.exists(module_path):
            raise Exception(f"Module missing at {module_path}")

        self.__output_filename = os.path.join(module_path, self.MODULE_INFO_CMAKE_CACHE_FILENAME)

        # Read in the JSON
        (top_level_modules, self.__rpaths) = self.__read_single_module_dependencies(module_path)

        self.__run(top_level_modules)

    def run_from_module_list(self, module_list, output_dir):
        self.__output_filename = os.path.join(output_dir, self.MODULE_INFO_CMAKE_CACHE_FILENAME)
        module_list = module_list.split(";")
        self.__new_dependencies = module_list
        self.__run(module_list)

    def __run(self, initial_modules):
        # If any existing output file exists remove it.  This ensure that CMake will fail if our JSON parsing etc fails.
        if os.path.exists(self.__output_filename):
            os.remove(self.__output_filename)

        self.__new_dependencies = initial_modules

        # Read full, deep, module dependencies
        while len(self.__new_dependencies) > 0:
            self.__full_nap_modules.extend(self.__new_dependencies)
            self.__find_new_dependencies_for_modules()

        # Write out
        with open(self.__output_filename, 'w') as out_file:
            out_file.write("# Don't edit this file\n")
            out_file.write("#\n# It was auto generated by CMake from module.json which should be edited instead\n\n")
            out_file.write("set(DEPENDENT_NAP_MODULES %s)\n" % ' '.join(initial_modules))
            out_file.write("set(DEEP_DEPENDENT_NAP_MODULES %s)\n" % ' '.join(self.__full_nap_modules))
            out_file.write("set(DEEP_DEPENDENT_RPATHS %s)\n" % ' '.join(self.__rpaths))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('NAP_ROOT')
    parser.add_argument('PATH_MAPPING_FILE')
    parser.add_argument('BUILD_ARCH')
    parser.add_argument('-p', '--path-mapping-element', type=str, default='ProjectExeToRoot')
    # TODO Update Python to 3.7+ and set required=True here
    subparsers = parser.add_subparsers(dest='COMMAND')

    parse_json_parser = subparsers.add_parser("parse_json")
    parse_json_parser.add_argument('MODULE_PATH')

    module_list_parser = subparsers.add_parser("process_module_list")
    module_list_parser.add_argument('MODULES')
    module_list_parser.add_argument('OUTPUT_DIR')

    args = parser.parse_args()

    # Workaround for Python v3.6 (see above)
    if args.COMMAND is None:
        eprint("Error: COMMAND is required")
        sys.exit(1)

    try:
        parser = ModuleInfoParser(args.NAP_ROOT,
                                  args.PATH_MAPPING_FILE,
                                  args.BUILD_ARCH,
                                  args.path_mapping_element
                                  )
        if args.COMMAND == 'parse_json':
            parser.run_from_module_json(args.MODULE_PATH)
        else:
            parser.run_from_module_list(args.MODULES, args.OUTPUT_DIR)
    except Exception as e:
        eprint(e)
        sys.exit(1)
