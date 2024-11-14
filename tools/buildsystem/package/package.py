#!/usr/bin/env python3
import argparse
import datetime
import os
from platform import machine
from multiprocessing import cpu_count
import shutil
import stat
import subprocess
import sys
from sys import platform
from enum import Enum

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_nap_root, get_build_arch, BuildType, Platform, get_system_generator

WORKING_DIR = '.'
BUILD_DIR = 'packaging_build'
LIB_DIR = 'packaging_lib'
BIN_DIR = 'packaging_bin'
PACKAGING_DIR = 'packaging_staging'
ARCHIVING_DIR = 'archiving'
APPS_SOURCE_DIR = 'apps'
APPS_DEST_DIR = 'apps'
APPLY_PERMISSIONS_BATCHFILE = 'apply_executable_permissions.cmd'

ERROR_PACKAGE_EXISTS = 1
ERROR_INVALID_VERSION = 2
ERROR_COULD_NOT_REMOVE_DIRECTORY = 4
ERROR_BAD_INPUT = 5
ERROR_SOURCE_ARCHIVE_GIT_NOT_CLEAN = 6
ERROR_SOURCE_ARCHIVE_EXISTING = 7

def call(cwd, cmd, capture_output=False):
    """Execute command in provided working directory"""

    # print('dir: %s' % os.path.abspath(cwd))
    # print('cmd: %s' % cmd)
    if capture_output:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        proc = subprocess.Popen(cmd, cwd=cwd)
    (out, err) = proc.communicate()
    if type(out) is bytes:
        out = out.decode('ascii', 'ignore')
        err = err.decode('ascii', 'ignore')
    if proc.returncode != 0:
        raise Exception("Bailing for non zero returncode: %s", proc.returncode)
    return out, err

def package(zip_release,
            include_debug_symbols,
            include_docs,
            single_app_to_include,
            clean,
            include_timestamp_in_name,
            build_label,
            package_name,
            overwrite,
            additional_dirs,
            enable_python):

    """Package a NAP platform release - main entry point"""
    nap_root = get_nap_root()
    os.chdir(nap_root)
    build_label_out = build_label if not build_label is None else ''

    # Add timestamp and git revision for build info
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')

    # Construct package name if name not given
    package_basename= package_name
    if package_name is None:
        package_basename = build_package_basename(timestamp if include_timestamp_in_name else None, build_label)

    # Ensure we won't overwrite any existing package
    if not overwrite:
        check_for_existing_package(package_basename, zip_release)

    # Remove old packaging path if it exists
    if os.path.exists(PACKAGING_DIR):
        remove_directory_exit_on_failure(PACKAGING_DIR, 'old packaging staging area')
    os.makedirs(PACKAGING_DIR)

    # Clean build if requested
    if clean:
        clean_the_build()

    # Convert additional sub directories to CMake list type
    sub_dirs = ';'.join(additional_dirs)

    # Generate and package
    print("Packaging...")

    git_revision = None
    try:
        (git_revision, _) = call(WORKING_DIR, ['git', 'rev-parse', 'HEAD'], True)
        git_revision = git_revision.strip()
    except Exception as e:
        print("Warning: unable to get git revision")

    if get_system_generator().is_single():
        package_single_stage(
            timestamp,
            git_revision,
            build_label_out,
            include_docs,
            include_debug_symbols,
            sub_dirs,
            enable_python
        )
    else:
        package_multi_stage(
            timestamp,
            git_revision,
            build_label,
            include_docs,
            include_debug_symbols,
            additional_dirs,
            enable_python
        )

    if Platform.get() == Platform.macOS:
        # Remove unwanted files (eg. .DS_Store)
        call(PACKAGING_DIR, ['find', '.', '-name', '.DS_Store', '-type', 'f', '-delete'])

    # Remove all Naivi apps but the requested one
    if single_app_to_include is not None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)

    # Create archive
    if zip_release:
        if Platform.get() == Platform.Linux:
            archive_framework_to_linux_tar_bz2(package_basename)
        if Platform.get() == Platform.macOS:
            archive_framework_to_macos_zip(package_basename)
        if Platform.get() == Platform.Windows:
            archive_framework_to_win64_zip(package_basename)
        raise Exception("Unsupported platform")
    else:
        archive_to_timestamped_dir(package_basename)

def remove_directory_exit_on_failure(path, use):
    try:
        shutil.rmtree(path)
    except OSError as e:
        print("Error: Could not remove directory '%s' (%s): %s" % (path, use, e))
        sys.exit(ERROR_COULD_NOT_REMOVE_DIRECTORY)

def clean_the_build():
    """Clean the build"""

    print("Cleaning...")
    if Platform.get() == Platform.Linux:
        for build_type in BuildType.to_list():
            build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())
            if os.path.exists(build_dir_for_type):
                print("Clean removing %s" % os.path.abspath(build_dir_for_type))
                remove_directory_exit_on_failure(build_dir_for_type, 'build dir for %s' % build_type.lower())
    else:
        if os.path.exists(BUILD_DIR):
            print("Clean removing %s" % os.path.abspath(BUILD_DIR))
            remove_directory_exit_on_failure(BUILD_DIR, 'build dir')

    if os.path.exists(LIB_DIR):
        print("Clean removing %s" % os.path.abspath(LIB_DIR))
        remove_directory_exit_on_failure(LIB_DIR, 'lib dir')
    if os.path.exists(BIN_DIR):
        print("Clean removing %s" % os.path.abspath(BIN_DIR))
        remove_directory_exit_on_failure(BIN_DIR, 'bin dir')

def check_for_existing_package(package_path, zip_release, remove=False):
    """Ensure we aren't overwriting a previous package, remove if requested"""

    # Add extension if zipping
    if zip_release:
        if Platform.get() == Platform.Linux:
            package_path += '.tar.bz2'
        else:
            package_path += '.zip'

    # Check and fail, or remove if requested
    if os.path.exists(package_path):
        if remove:
            print("Overwriting %s" % os.path.abspath(package_path))
            if zip_release:
                os.remove(package_path)
            else:
                remove_directory_exit_on_failure(package_path, 'overwriting existing package')
        else:
            print("Error: Existing package found: %s" % os.path.abspath(package_path))
            sys.exit(ERROR_PACKAGE_EXISTS)

def package_single_stage(timestamp, git_revision, build_label, include_docs, include_debug_symbols, additional_dirs, enable_python):
    """Package NAP platform release for Linux"""

    for build_type in BuildType.to_list():
        print("\nCurrent packaging config: {0}".format(build_type))
        build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())

        # Create build dir if it doesn't exist
        if Platform.get() == Platform.Windows and not os.path.exists(build_dir_for_type):
            os.makedirs(build_dir_for_type)

        # Generate solution
        call(WORKING_DIR, [get_cmake_path(),
                           '-H.',
                           '-B%s' % build_dir_for_type,
                           '-G%s' % str(get_system_generator()),
                           '-DNAP_PACKAGED_BUILD=1',
                           '-DCMAKE_BUILD_TYPE=%s' % build_type,
                           '-DINCLUDE_DOCS=%s' % int(include_docs),
                           '-DBUILD_TIMESTAMP=%s' % timestamp,
                           '-DBUILD_GIT_REVISION=%s' % git_revision,
                           '-DBUILD_LABEL=%s' % build_label,
                           '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                           '-DADDITIONAL_SUB_DIRECTORIES=%s' % additional_dirs,
                           '-DNAP_ENABLE_PYTHON=%s' % int(enable_python)
                           ])
        # Build
        call(WORKING_DIR, [get_cmake_path(), '--build', build_dir_for_type, '--target', 'install', '-j', str(cpu_count())])

def package_multi_stage(timestamp, git_revision, build_label, include_docs, include_debug_symbols, additional_dirs, enable_python):
    """Package NAP platform release for Windows"""

    # Create build dir if it doesn't exist
    if Platform.get() == Platform.Windows and not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    # Generate solution
    call(WORKING_DIR, [get_cmake_path(),
                       '-H.',
                       '-B%s' % BUILD_DIR,
                       '-G%s' % str(get_system_generator()),
                       '-DNAP_PACKAGED_BUILD=1',
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                       '-DADDITIONAL_SUB_DIRECTORIES=%s' % additional_dirs,
                       '-DNAP_ENABLE_PYTHON=%s' % int(enable_python)
                       ])

    # Build & install to packaging dir
    for build_type in BuildType.to_list():
        print("\nCurrent packaging config: {0}".format(build_type))
        call(WORKING_DIR, [get_cmake_path(), '--build', BUILD_DIR, '--target', 'install', '--config', build_type, '-j', str(cpu_count())])

def archive_framework_to_linux_tar_bz2(package_basename):
    """Create build archive to bzipped tarball on Linux"""

    shutil.move(PACKAGING_DIR, package_basename)

    package_filename_with_ext = create_linux_tar_bz2(package_basename)

    # Cleanup
    shutil.move(package_basename, PACKAGING_DIR)

    full_out_path = os.path.abspath(package_filename_with_ext)
    print("Packaged to %s" % full_out_path)
    return full_out_path

def create_linux_tar_bz2(source_directory):
    """Create a bzipped tarball for the provided directory on Linux"""

    archive_filename_with_ext = '%s.%s' % (source_directory, 'tar.bz2')
    print("Archiving to %s ..." % os.path.abspath(archive_filename_with_ext))
    call(WORKING_DIR, ['tar', '-cjvf', archive_filename_with_ext, source_directory])
    return archive_filename_with_ext

def archive_framework_to_macos_zip(package_basename):
    """Create build archive to zip on macOS"""

    shutil.move(PACKAGING_DIR, package_basename)

    # Archive
    package_filename_with_ext = create_macos_zip(package_basename)

    # Cleanup
    shutil.move(package_basename, PACKAGING_DIR)

    full_out_path = os.path.abspath(package_filename_with_ext)
    print("Packaged to %s" % full_out_path)
    return full_out_path

def create_macos_zip(source_directory):
    """Create a zip for the provided directory on macOS"""

    archive_filename_with_ext = '%s.%s' % (source_directory, 'zip')
    print("Archiving to %s ..." % os.path.abspath(archive_filename_with_ext))
    call(WORKING_DIR, ['zip', '-yr', archive_filename_with_ext, source_directory])
    return archive_filename_with_ext

def archive_framework_to_win64_zip(package_basename):
    """Create build archive to zip on Win64"""

    # Rename our packaging dir to match the release
    shutil.move(PACKAGING_DIR, package_basename)

    (archive_filename_with_ext, archive_path) = create_win64_zip(package_basename)

    # Cleanup
    shutil.move(archive_path, PACKAGING_DIR)
    shutil.rmtree(ARCHIVING_DIR)

    full_out_path = os.path.abspath(archive_filename_with_ext)
    print("Packaged to %s" % full_out_path)
    return(full_out_path)

def create_win64_zip(source_directory):
    """Create a zip for the provided directory on Win64"""

    archive_filename_with_ext = '%s.%s' % (source_directory, 'zip')

    # Create our archive dir, used to create a copy level folder within the archive
    if os.path.exists(ARCHIVING_DIR):
        shutil.rmtree(ARCHIVING_DIR, True)
    os.makedirs(ARCHIVING_DIR)
    archive_path = os.path.join(ARCHIVING_DIR, source_directory)
    shutil.move(source_directory, archive_path)

    # Create archive
    print("Archiving to %s ..." % os.path.abspath(archive_filename_with_ext))
    shutil.make_archive(source_directory, 'zip', ARCHIVING_DIR)

    # Cleanup
    return archive_filename_with_ext, archive_path

def archive_to_timestamped_dir(package_basename):
    """Copy our packaged dir to a timestamped dir"""

    shutil.move(PACKAGING_DIR, package_basename)

    full_out_path = os.path.abspath(package_basename)
    print("Packaged to directory %s" % full_out_path)
    return(full_out_path)

def remove_all_apps_but_specified(single_app_to_include):
    """Remove all internal Naivi apps but the specified one"""
    for app_name in os.listdir(os.path.join(PACKAGING_DIR, APPS_DEST_DIR)):
        if not app_name == single_app_to_include:
            path = os.path.join(PACKAGING_DIR, APPS_DEST_DIR, app_name)
            remove_directory_exit_on_failure(path, 'unwanted internal app')
def build_package_basename(timestamp, label):
    """Build the name of our package and populate our JSON build info file"""

    # Do the packaging
    if Platform.get() == Platform.Linux:
        platform_name = 'Linux'
    elif Platform.get() == Platform.macOS:
        platform_name = 'macOS'
    elif Platform.get() == Platform.Windows:
        platform_name = 'Win64'
    else:
        print("Error: Unsupported platform")
        sys.exit(ERROR_BAD_INPUT)

    # Fetch version from version.cmake
    (version_unparsed, _) = call(WORKING_DIR, [get_cmake_path(), '-P', 'cmake/version.cmake'], True)
    chunks = version_unparsed.split(':')
    if len(chunks) < 2:
        print("Error passing invalid output from version.cmake: %s" % version_unparsed)
        sys.exit(ERROR_INVALID_VERSION)
    version = chunks[1].strip()

    # Fetch architecture
    arch = get_architecture()

    # Create filename, including timestamp or not as requested
    package_filename = "NAP-%s-%s-%s" % (version, platform_name, arch)
    if not timestamp is None:
        package_filename += "-%s" % timestamp
    if not label is None:
        package_filename += "-%s" % label
    return package_filename

def get_architecture():
    """Retrieve architecture identifier"""
    if Platform.get() == Platform.Linux:
        v = get_build_arch()
    else:
        v = 'x86_64'
    return v

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    labelling_group = parser.add_argument_group('Labelling')
    labelling_group.add_argument("-nt", "--no-timestamp", action="store_true",
                        help="Don't include timestamp in the release archive and folder name, for final releases")
    labelling_group.add_argument("-l", "--label", type=str,
                        help="An optional suffix for the package")
    labelling_group.add_argument("-n", "--name", type=str,
                        help="Overrides the package name. NAP timestamp, version and label information is excluded")

    core_group = parser.add_argument_group('Core Behaviour')
    core_group.add_argument("-nz", "--no-zip", action="store_true",
                        help="Don't zip the release, package to a directory")
    core_group.add_argument("-ds", "--include-debug-symbols", action="store_true",
                        help="Include debug symbols")
    core_group.add_argument("-o", "--overwrite", action="store_true",
                        help="Overwrite any existing framework or source package")
    core_group.add_argument("-c", "--clean", action="store_true",
                        help="Clean build")
    core_group.add_argument('-p', '--enable-python', action="store_true",
                       help="Enable python integration using pybind (deprecated)")

    nap_apps_group = parser.add_argument_group('Applications')
    nap_apps_group.add_argument("-sna", "--include-single-app", type=str,
                        help="Include only a single application with the given name.")
    nap_apps_group.add_argument("-d", "--additional_dirs", nargs='+', type=str, default=[],
                        help="List of additional sub directories to add to the build")

    unsupported_group = parser.add_argument_group('Unsupported')
    unsupported_group.add_argument("--include-docs", action="store_true",
                        help="Include documentation")
    args = parser.parse_args()

    # If we're packaging a single Naivi app make sure it exists
    if args.include_single_app:
        path = os.path.join(get_nap_root(), APPS_SOURCE_DIR, args.include_single_app)
        if not os.path.exists(path):
            print("Error: Can't package single Naivi app '%s' as it doesn't exist" % args.include_single_app)
            sys.exit(ERROR_BAD_INPUT)

    # Package our build
    package(not args.no_zip,
            args.include_debug_symbols,
            args.include_docs,
            args.include_single_app,
            args.clean,
            not args.no_timestamp,
            args.label,
            args.name,
            args.overwrite,
            args.additional_dirs,
            args.enable_python)
