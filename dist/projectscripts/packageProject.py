#!/usr/bin/env python

import argparse
import datetime
import json
from multiprocessing import cpu_count
import os
import subprocess
from sys import platform
import sys
import shutil

WORKING_DIR = '.'
PACKAGING_DIR = 'packaging'
ARCHIVING_DIR = 'archiving'
BUILDINFO_FILE = 'cmake/buildInfo.json'
PACKAGED_BUILDINFO_FILE = '%s/cmake/buildinfo.json' % PACKAGING_DIR
PROJECT_INFO_FILE = 'project.json'

ERROR_BAD_INPUT = 1
ERROR_MISSING_PROJECT = 2
ERROR_INVALID_PROJECT_JSON = 3

def call(cwd, cmd, capture_output=False):
    # print('dir: %s' % cwd)
    # print('cmd: %s' % cmd)
    if capture_output:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        proc = subprocess.Popen(cmd, cwd=cwd)
    (out, err) = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return (out, err)

# TODO share with projectInfoParseToCMake
def find_project(project_name):
    script_path = os.path.realpath(__file__)
    nap_root = os.path.abspath(os.path.join(os.path.dirname(script_path), os.pardir))

    project_dir_name = project_name.lower()
    projects_root = os.path.join(nap_root, 'projects')
    project_path = os.path.join(projects_root, project_dir_name)
    examples_root = os.path.join(nap_root, 'examples')
    example_path = os.path.join(examples_root, project_dir_name)
    demos_root = os.path.join(nap_root, 'demos')
    demo_path = os.path.join(demos_root, project_dir_name)

    if os.path.exists(project_path):
        print("Found project %s at %s" % (project_name, project_path))
        return project_path
    elif os.path.exists(example_path):
        print("Found example %s at %s" % (project_name, example_path))
        return example_path
    elif os.path.exists(demo_path):
        print("Found demo %s at %s" % (project_name, demo_path))
        return demo_path
    else:
        print("Error: Couldn't find project or example with name '%s'" % project_name)
        return None

def package_project(project_name, show_created_package):
    project_path = find_project(project_name)
    if project_path is None:
        return ERROR_MISSING_PROJECT

    (project_version, project_full_name) = process_project_info(project_path)
    if project_version is None:
        return ERROR_INVALID_PROJECT_JSON

    print("Packaging %s v%s" % (project_full_name, project_version))

    project_name_lower = project_name.lower()

    # Build directory names
    script_path = os.path.realpath(__file__)
    nap_root = os.path.abspath(os.path.join(os.path.dirname(script_path), os.pardir))
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    local_bin_dir_name = 'bin_package'
    bin_dir = os.path.join(project_path, local_bin_dir_name)
    build_dir_name = os.path.join(project_path, 'build_package')

    # Remove old packaging path if it exists
    if os.path.exists(bin_dir):
        shutil.rmtree(bin_dir, True)
    if os.path.exists(build_dir_name):
        shutil.rmtree(build_dir_name, True)

    if platform in ["linux", "linux2"]:
        print("Linux not yet supported... sorry")
        return 0
        # call(WORKING_DIR, ['cmake', '-H.', '-B%s' % build_dir_name, '-DCMAKE_BUILD_TYPE=%s' % 'Release', '-DPROJECT_PACKAGE_BIN_DIR=%s' % local_bin_dir_name])

        # call(build_dir_name, ['make', 'all', 'install', '-j%s' % cpu_count()])

        # Create archive
        # archive_to_linux_tar_xz(timestamp)

        # Show in Nautilus
        if show_created_package:
            pass
            # TODO implement showing created package in Nautilus
    elif platform == 'darwin':
        # Generate project
        call(WORKING_DIR, ['cmake', '-H%s' % project_path, '-B%s' % build_dir_name, '-G', 'Xcode'])

        # Build & install to packaging dir
        call(build_dir_name, ['xcodebuild', '-configuration', 'Release', '-target', 'install'])

        # Temp: Copy our external dylibs and fix lib paths
        call(bin_dir, ['python', '%s/tools/platform/macOSTempDylibCopyAndPathFix.py' % nap_root, '.'])

        # Create archive
        packaged_to = archive_to_macos_zip(timestamp, bin_dir, project_full_name, project_version)

        # Show in Finder
        if show_created_package:
            subprocess.call(["open", "-R", packaged_to])
    else:
        # Generate project
        call(WORKING_DIR, ['cmake', '-H%s' % project_path, '-B%s' % build_dir_name, '-G', 'Visual Studio 14 2015 Win64', '-DPYBIND11_PYTHON_VERSION=3.5', '-DPROJECT_PACKAGE_BIN_DIR=%s' % local_bin_dir_name])

        # Build & install to packaging dir
        call(build_dir_name, ['cmake', '--build', '.', '--target', project_name_lower, '--config', 'Release'])

        # Create archive
        archive_to_win64_zip(timestamp, bin_dir, project_full_name, project_version)

        # Show in Explorer
        if show_created_package:
            pass
            # TODO implement showing created package in Explorer

    # Cleanup
    shutil.rmtree(build_dir_name, True)

    # Clean exit code
    return 0

# Create build archive to xz tarball on Linux
def archive_to_linux_tar_xz(timestamp, project_name):
    package_filename = build_package_filename_and_build_info(project_name, 'Linux', timestamp)
    shutil.move(PACKAGING_DIR, package_filename)

    package_filename_with_ext = '%s.%s' % (package_filename, 'tar.xz')
    print("Archiving to %s.." % package_filename_with_ext)
    call(WORKING_DIR, ['tar', '-cJvf', package_filename_with_ext, package_filename])

    # Cleanup
    shutil.move(package_filename, PACKAGING_DIR)
    print("Packaged to %s" % package_filename_with_ext)  

# Create build archive to zip on macOS
def archive_to_macos_zip(timestamp, bin_dir, project_full_name, project_version): 
    package_filename = build_package_filename(project_full_name, project_version, 'macOS', timestamp)
    package_filename_with_ext = '%s.%s' % (package_filename, 'zip')

    # Populate build info into the project
    populate_build_info_into_project(bin_dir, timestamp)

    project_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))

    archive_dir = os.path.join(project_dir, package_filename)
    shutil.move(bin_dir, archive_dir)

    # Remove unwanted files (eg. .DS_Store)
    call(archive_dir, ['find', '.', '-name', '.DS_Store', '-type', 'f', '-delete'])

    # Archive
    print("Archiving to %s.." % package_filename_with_ext)
    call(project_dir, ['zip', '-yr', package_filename_with_ext, package_filename])

    # Cleanup
    shutil.rmtree(archive_dir)

    packaged_to_relpath = os.path.relpath(os.path.join(project_dir, package_filename_with_ext))
    print("Packaged to %s" % packaged_to_relpath)
    return packaged_to_relpath

# Create build archive to zip on Win64
def archive_to_win64_zip(timestamp, bin_dir, project_full_name, project_version):
    package_filename = build_package_filename(project_full_name, project_version, 'Win64', timestamp)
    package_filename_with_ext = '%s.%s' % (package_filename, 'zip')

    # Populate build info into the project
    populate_build_info_into_project(bin_dir, timestamp)

    project_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))

    # Create our archive dir, used to create a copy level folder within the archive
    archiving_parent_path = os.path.join(project_dir, ARCHIVING_DIR)
    if os.path.exists(archiving_parent_path):
        shutil.rmtree(archiving_parent_path, True)
    os.makedirs(archiving_parent_path)
    archive_path = os.path.join(archiving_parent_path, package_filename)
    shutil.move(bin_dir, archive_path)

    # Create archive
    print("Archiving to %s.." % package_filename_with_ext)
    shutil.make_archive(os.path.join(project_dir, package_filename), 'zip', archiving_parent_path)

    # Cleanup
    shutil.rmtree(archiving_parent_path)

    print("Packaged to %s" % os.path.relpath(os.path.join(project_dir, package_filename_with_ext)))

# Build the name of our package and populate our JSON build info file
def build_package_filename(project_name, project_version, platform, timestamp):
    package_filename = "%s-%s-%s-%s" % (project_name, project_version, platform, timestamp)
    return package_filename

# Process our JSON project info: Fetch our version and full project name
def process_project_info(project_path):
    with open(os.path.join(project_path, PROJECT_INFO_FILE)) as json_file:
        try:
            project_info = json.load(json_file)
        except ValueError as e:
            print("Invalid project.json: %s" % e)
            return (None, None)

    # TODO validate loaded JSON

    # Read our version
    version = project_info['version']
    full_project_name = project_info['title']

    # Return version for population into package name
    return (version, full_project_name)

# Populate build information into our packaged project JSON
def populate_build_info_into_project(project_package_path, timestamp):
    # Read our project info
    project_info_file = os.path.join(project_package_path, PROJECT_INFO_FILE)
    with open(project_info_file) as json_file:
        project_info = json.load(json_file)

    # Add project timestamp and write back out
    project_info['buildTimestamp'] = timestamp
    with open(project_info_file, 'w') as outfile:
        json.dump(project_info, outfile, sort_keys=True, indent=2)
    
# Main
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("PROJECT_NAME", type=str,
                        help="The project to package")
    parser.add_argument("-ds", "--dontshow", action="store_true",
                        help="Don't show the generated package")
    args = parser.parse_args()

    # Package our build
    sys.exit(package_project(args.PROJECT_NAME, not args.dontshow))
