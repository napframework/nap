#!/usr/bin/env python
import json
import os
from subprocess import call, Popen, PIPE
import sys

# Copy any /opt or /usr/local dylibs on the executables to lib
# # For every dylib in lib
#     1. Copy any dylibs linked from /opt
#     2. Change any @rpath links in dylibs to point to local directory @loader_path
#     3. Change any all dylibs to have their paths as @rpath/libname.dylib
# On executable make all dylibs using @rpath, /opt or /usr/local point to @loader_path/lib

PACKAGED_CONFIG = 'Release'
PROJECT_INFO_FILE = 'project.json'
EXTERNAL_PATHS = ['/opt', '/usr/local']

ERROR_MISSING_INPUT = 1
ERROR_BAD_PACKAGING_PATH = 2

# TODO share with projectInfoParseToCMake
def find_project(project_name):
    script_path = os.path.realpath(__file__)
    # TODO clean up, use absolute path
    nap_root = os.path.join(os.path.dirname(script_path), '..')

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
        print("Couldn't find project or example with name '%s'" % project_name)
        return None

def copy_local_object_linked_local_dylibs(object_path, dest_path):
    cmd = "otool -L %s" % object_path
    # print cmd
    p = Popen([cmd], shell=True, stdout=PIPE, stderr=PIPE)
    (output, _) = p.communicate()

    copy_count = 0

    for line in output.split("\n")[2:]:
        if any(x in line for x in EXTERNAL_PATHS):
            chunks = line.strip().split()
            filename = os.path.basename(chunks[0])
            if not os.path.exists("%s/%s" % (dest_path, filename)):
                print("Copying %s" % chunks[0])
                call(["cp %s %s" % (chunks[0], dest_path)], shell=True)
                call(["chmod +w %s/%s" % (dest_path, filename)], shell=True)
                copy_count += 1
    return copy_count

def copy_local_object_linked_local_dylibs_for_all_files_in_dir(path, dest_path):
    copy_count = 0
    for filename in os.listdir(path):
        copy_count += copy_local_object_linked_local_dylibs("%s/%s" % (path, filename), dest_path)
    return copy_count

def update_single_dylib_self_path(dylib_path, dest_prefix):

    new_path = "%s/%s" % (dest_prefix.rstrip('/'), os.path.basename(dylib_path))
    cmd = "install_name_tool -id \"%s\" %s" % (new_path, dylib_path)
    call([cmd], shell=True)

def update_all_dylibs_self_paths_in_dir(directory, dest_prefix):
    if not os.path.exists(directory):
        print "'update_all_dylibs_self_paths_in_dir not working on %s which doesn't exist" % directory
        return

    for filename in os.listdir(directory):
        update_single_dylib_self_path("%s/%s" % (directory, filename), dest_prefix)

def update_external_dylib_paths_for_single_object(filename, new_prefix, replace_system_paths, replace_rpath):
    cmd = "otool -L %s" % filename
    # print cmd
    p = Popen([cmd], shell=True, stdout=PIPE, stderr=PIPE)
    (output, _) = p.communicate()

    paths_to_replace = []
    if replace_system_paths:
        paths_to_replace.extend(EXTERNAL_PATHS)
    if replace_rpath:
        paths_to_replace.append('@rpath')

    for line in output.split("\n")[2:]:
        # Skip empty lines
        if line.strip() == '':
            continue

        chunks = line.strip().split()
        # Check for any of our replacement paths or a local path
        if any(x in line for x in paths_to_replace) or not '/' in chunks[0]:
            dylib_filename = os.path.basename(chunks[0])

            update_single_link(chunks[0], '%s/%s' % (new_prefix, dylib_filename), filename)

def update_single_link(search, replace, target):
    cmd = "install_name_tool -change %s %s %s" % (search, replace, target)
    print "Upating link to %s in %s" % (search, target)
    call([cmd], shell=True)

def update_external_dylib_paths_for_all_dylibs_in_dir(directory, new_prefix, replace_system_paths):
    if not os.path.exists(directory):
        print "update_external_dylib_paths_for_all_dylibs_in_dir not working on %s which doesn't exist" % directory
        return

    for filename in os.listdir(directory):
        update_external_dylib_paths_for_single_object("%s/%s" % (directory, filename), new_prefix, replace_system_paths, False)


def run(packaging_path):
    os.chdir(packaging_path)
    project_path = os.path.abspath(os.path.join(packaging_path, os.pardir))
    executable_name = os.path.basename(project_path)
    print("Project name: %s" % executable_name)

    executable_relative_path = os.path.join(project_path, executable_name)
    # Ensure we can find our project executable
    if not os.path.exists(executable_name):
        print("Couldn't find executable %s in %s" % (executable_name, project_path))
        return False

    print("Project binary found at %s" % executable_name)

    nap_root = os.path.abspath('../../..')

    # Copy all external dylib dependencies on the executable
    copy_local_object_linked_local_dylibs(executable_name, 'lib')

    # Iterate over all dylibs in the lib folder copying any external dependencies until
    # we add no new dylibs
    copied_in_pass = -1
    while copied_in_pass != 0:
        copied_in_pass = copy_local_object_linked_local_dylibs_for_all_files_in_dir('lib', 'lib')

    # Update our location for each dylib to @rpath
    update_all_dylibs_self_paths_in_dir('lib', '@rpath')

    # Update our external links in all of our dylibs
    update_external_dylib_paths_for_all_dylibs_in_dir('lib', '@loader_path', True)

    # Update our external links in our binary
    update_external_dylib_paths_for_single_object(executable_name, '@rpath', True, False)

if __name__ == '__main__':
    # TODO use argparse

    if len(sys.argv) != 2:
        print("Usage: %s PROJECT_NAME PACKAGING_PATH" % sys.argv[0])
        sys.exit(ERROR_MISSING_INPUT)

    packaging_path = sys.argv[1]
    if not os.path.exists(packaging_path):
        print("Error: Packaging path %s does not exist" % packaging_path)
        sys.exit(ERROR_BAD_PACKAGING_PATH)

    if not os.path.isdir(packaging_path):
        print("Error: Packaging path %s is not a directory")
        sys.exit(ERROR_BAD_PACKAGING_PATH)

    print("Packaging project from %s" % packaging_path)
    run(packaging_path)