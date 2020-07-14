#!/usr/bin/env python
import argparse
import datetime
import os
from multiprocessing import cpu_count
import shutil
import stat
import subprocess
import sys
from sys import platform
from enum import Enum

class CrossCompileTarget(Enum):
    ANDROID = 1
    IOS = 2

WORKING_DIR = '.'
BUILD_DIR = 'packaging_build'
LIB_DIR = 'packaging_lib'
BIN_DIR = 'packaging_bin'
PACKAGING_DIR = 'packaging_staging'
ARCHIVING_DIR = 'archiving'
APPS_SOURCE_DIR = 'apps'
APPS_DEST_DIR = 'projects'
BUILDINFO_FILE = 'dist/cmake/build_info.json'
BUILD_TYPES = ('Release', 'Debug')
APPLY_PERMISSIONS_BATCHFILE = 'apply_executable_permissions.cmd'

DEFAULT_ANDROID_ABIS = ('arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64')
ANDROID_PLATFORM = 'android-19'
    
ERROR_PACKAGE_EXISTS = 1
ERROR_INVALID_VERSION = 2
ERROR_MISSING_ANDROID_NDK = 3
ERROR_COULD_NOT_REMOVE_DIRECTORY = 4
ERROR_BAD_INPUT = 5
ERROR_SOURCE_ARCHIVE_GIT_NOT_CLEAN = 6
ERROR_SOURCE_ARCHIVE_EXISTING = 7

def call(cwd, cmd, capture_output=False, exception_on_nonzero=True):
    """Execute command in provided working directory"""

    # print('dir: %s' % cwd)
    # print('cmd: %s' % cmd)
    if capture_output:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        proc = subprocess.Popen(cmd, cwd=cwd)
    (out, err) = proc.communicate()
    if type(out) is bytes:
        out = out.decode('ascii', 'ignore')
        err = err.decode('ascii', 'ignore')
    if err:
        raise Exception(err)
    return out, err

def package(zip_release, 
            include_debug_symbols,
            include_docs, 
            include_apps, 
            single_app_to_include, 
            clean, 
            include_timestamp_in_name, 
            build_label, 
            build_projects,
            archive_source, 
            archive_source_zipped,
            archive_source_only,
            overwrite, 
            android_build, 
            android_ndk_root, 
            android_abis,
            android_enable_python):
    """Package a NAP platform release - main entry point"""

    nap_root = get_nap_root()
    os.chdir(nap_root)
    build_label_out = build_label if not build_label is None else ''

    # Define cross compilation target, if relevant 
    cross_compile_target = None
    if android_build:
        cross_compile_target = CrossCompileTarget.ANDROID

    # Add timestamp and git revision for build info
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    (git_revision, _) = call(WORKING_DIR, ['git', 'rev-parse', 'HEAD'], True)
    git_revision = git_revision.strip()
    (package_basename, source_archive_basename) = build_package_basename(timestamp if include_timestamp_in_name else None, build_label, cross_compile_target)

    # Ensure we won't overwrite any existing package
    if not archive_source_only and not overwrite:
        check_for_existing_package(package_basename, zip_release)
    
    if archive_source:
        # Verify our repo is clean if we want to do a source archive
        verify_clean_git_repo()

        # Check we're not going to overwrite an existing source archive
        if not overwrite:
            check_existing_source_archive(source_archive_basename, archive_source_zipped)

    if not archive_source_only:
        print("Packaging...")

        # Remove old packaging path if it exists
        if os.path.exists(PACKAGING_DIR):
            remove_directory_exit_on_failure(PACKAGING_DIR, 'old packaging staging area')
        os.makedirs(PACKAGING_DIR)

        # Setup ABIs for Android
        if android_abis is None:
            android_abis = DEFAULT_ANDROID_ABIS
        else:
            android_abis = android_abis.split(',')

        # Clean build if requested
        if clean:
            clean_the_build(cross_compile_target, android_abis)

        # Do the packaging
        if android_build:
            package_path = package_for_android(package_basename, 
                                               timestamp, 
                                               git_revision, 
                                               build_label_out, 
                                               overwrite, 
                                               zip_release, 
                                               android_abis, 
                                               android_ndk_root, 
                                               android_enable_python
                                               )
        elif platform.startswith('linux'):    
            package_path = package_for_linux(package_basename, 
                                             timestamp, 
                                             git_revision, 
                                             build_label_out, 
                                             overwrite, 
                                             include_apps, 
                                             single_app_to_include, 
                                             include_docs, 
                                             zip_release, 
                                             include_debug_symbols,
                                             build_projects
                                             )
        elif platform == 'darwin':
            package_path = package_for_macos(package_basename, 
                                             timestamp, 
                                             git_revision, 
                                             build_label_out, 
                                             overwrite, 
                                             include_apps, 
                                             single_app_to_include, 
                                             include_docs, 
                                             zip_release, 
                                             include_debug_symbols,
                                             build_projects
                                             )
        else:
            package_path = package_for_win64(package_basename, 
                                             timestamp, 
                                             git_revision, 
                                             build_label_out, 
                                             overwrite, 
                                             include_apps, 
                                             single_app_to_include, 
                                             include_docs, 
                                             zip_release, 
                                             include_debug_symbols,
                                             build_projects
                                             )

    # Archive source
    if archive_source:
        create_source_archive(source_archive_basename, archive_source_zipped, build_label_out, timestamp, git_revision, overwrite)

        # Make main package path visible
        if not archive_source_only:
            print("Release packaged to %s" % package_path)

def remove_directory_exit_on_failure(path, use):
    try:
        shutil.rmtree(path)
    except OSError as e:
        print("Error: Could not remove directory '%s' (%s): %s" % (path, use, e))
        sys.exit(ERROR_COULD_NOT_REMOVE_DIRECTORY)

def clean_the_build(cross_compile_target, android_abis):
    """Clean the build"""

    print("Cleaning...")

    if not cross_compile_target is None:
        if cross_compile_target == CrossCompileTarget.ANDROID:
            # Iterate ABIs
            for abi in android_abis:
                # Iterate build types
                for build_type in BUILD_TYPES:
                    build_dir_for_type = "%s_Android_%s_%s" % (BUILD_DIR, abi, build_type.lower())
                    if os.path.exists(build_dir_for_type):
                        print("Clean removing %s" % os.path.abspath(build_dir_for_type))
                        remove_directory_exit_on_failure(build_dir_for_type, 'build dir for %s' % build_type.lower())
    elif platform.startswith('linux'):    
        for build_type in BUILD_TYPES:
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
        if platform.startswith('linux'):
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


def package_for_linux(package_basename, timestamp, git_revision, build_label, overwrite, include_apps, single_app_to_include, include_docs, zip_release, include_debug_symbols, build_projects):
    """Package NAP platform release for Linux"""

    for build_type in BUILD_TYPES:
        build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())
        call(WORKING_DIR, [get_cmake_path(), 
                           '-H.', 
                           '-B%s' % build_dir_for_type, 
                           '-DNAP_PACKAGED_BUILD=1',
                           '-DCMAKE_BUILD_TYPE=%s' % build_type,
                           '-DINCLUDE_DOCS=%s' % int(include_docs),
                           '-DPACKAGE_NAIVI_APPS=%s' % int(include_apps),
                           '-DBUILD_TIMESTAMP=%s' % timestamp,
                           '-DBUILD_GIT_REVISION=%s' % git_revision,
                           '-DBUILD_LABEL=%s' % build_label,
                           '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                           '-DBUILD_PROJECTS=%s' % int(build_projects)
                           ])

        d = '%s/%s' % (WORKING_DIR, build_dir_for_type)
        call(d, ['make', 'all', 'install', '-j%s' % cpu_count()])

    # Remove all Naivi apps but the requested one
    if include_apps and not single_app_to_include is None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        return archive_framework_to_linux_tar_bz2(package_basename)
    else:
        return archive_to_timestamped_dir(package_basename)

def package_for_macos(package_basename, timestamp, git_revision, build_label, overwrite, include_apps, single_app_to_include, include_docs, zip_release, include_debug_symbols, build_projects):
    """Package NAP platform release for macOS"""

    # Generate project
    call(WORKING_DIR, [get_cmake_path(), 
                       '-H.', 
                       '-B%s' % BUILD_DIR, 
                       '-G', 'Xcode',
                       '-DNAP_PACKAGED_BUILD=1',                 
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DPACKAGE_NAIVI_APPS=%s' % int(include_apps),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                       '-DBUILD_PROJECTS=%s' % int(build_projects)
                       ])

    # Build & install to packaging dir
    d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
    for build_type in BUILD_TYPES:
        call(d, ['xcodebuild', '-configuration', build_type, '-target', 'install', '-jobs', str(cpu_count())])

    # Remove unwanted files (eg. .DS_Store)
    call(PACKAGING_DIR, ['find', '.', '-name', '.DS_Store', '-type', 'f', '-delete'])

    # Remove all Naivi apps but the requested one
    if include_apps and not single_app_to_include is None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        return archive_framework_to_macos_zip(package_basename)
    else:
        return archive_to_timestamped_dir(package_basename)

def package_for_win64(package_basename, timestamp, git_revision, build_label, overwrite, include_apps, single_app_to_include, include_docs, zip_release, include_debug_symbols, build_projects):
    """Package NAP platform release for Windows"""

    # Create build dir if it doesn't exist
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    # Generate project
    call(WORKING_DIR, [get_cmake_path(), 
                       '-H.', 
                       '-B%s' % BUILD_DIR, 
                       '-G', 'Visual Studio 14 2015 Win64',
                       '-DNAP_PACKAGED_BUILD=1',
                       '-DPYBIND11_PYTHON_VERSION=3.5',
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DPACKAGE_NAIVI_APPS=%s' % int(include_apps),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                       '-DBUILD_PROJECTS=%s' % int(build_projects)
                       ])

    # Build & install to packaging dir
    for build_type in BUILD_TYPES:
        call(WORKING_DIR, [get_cmake_path(), '--build', BUILD_DIR, '--target', 'install', '--config', build_type])

    # Remove all Naivi apps but the requested one
    if include_apps and not single_app_to_include is None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        return archive_framework_to_win64_zip(package_basename)
    else:
        return archive_to_timestamped_dir(package_basename)

def package_for_android(package_basename, timestamp, git_revision, build_label, overwrite, zip_release, android_abis, android_ndk_root, android_enable_python):
    """Cross compile NAP and package platform release for Android"""      

    # Let's check we have an NDK path
    ndk_root = None
    if not android_ndk_root is None:
        ndk_root = android_ndk_root
    elif 'ANDROID_NDK_ROOT' in os.environ:
        ndk_root = os.environ['ANDROID_NDK_ROOT']
    else:
        print("Error: Couldn't find Android NDK. Set with --android-ndk-root or in environment variable ANDROID_NDK_ROOT")
        sys.exit(ERROR_MISSING_ANDROID_NDK)

    # Verify NDK looks crudely sane
    toolchain_file = os.path.join(ndk_root, 'build', 'cmake', 'android.toolchain.cmake')
    if not os.path.exists(toolchain_file):
        print("Error: Android NDK path '%s' does not appear to be valid (couldn't find build/cmake/android.toolchain.cmake')" % ndk_root)
        sys.exit(ERROR_MISSING_ANDROID_NDK)

    # Iterate ABIs
    for abi in android_abis:
        # Iterate build types
        for build_type in BUILD_TYPES:
            build_dir_for_type = "%s_Android_%s_%s" % (BUILD_DIR, abi, build_type.lower())

            # Build CMake command with varying platform options
            # TODO test and support Linux?
            cmake_command = [get_cmake_path(), 
                             '-H.', 
                             '-B%s' % build_dir_for_type, 
                             '-DCMAKE_TOOLCHAIN_FILE=%s' % toolchain_file,
                             '-DANDROID_NDK=%s' % ndk_root,
                             '-DANDROID_ABI=%s' % abi,
                             '-DANDROID_PLATFORM=%s' % ANDROID_PLATFORM,
                             '-DNAP_PACKAGED_BUILD=1',
                             '-DCMAKE_BUILD_TYPE=%s' % build_type,
                             '-DINCLUDE_DOCS=0' 
                             '-DPACKAGE_NAIVI_APPS=0'
                             '-DBUILD_TIMESTAMP=%s' % timestamp,
                             '-DBUILD_GIT_REVISION=%s' % git_revision,
                             '-DBUILD_LABEL=%s' % build_label
                             ]

            # Set Ninja generator on Windows
            if platform.startswith('win'):
                cmake_command.append('-GNinja')

            # Enable Python if requested
            if android_enable_python:
                cmake_command.append('-DANDROID_PYTHON=1')

            # Generate project
            call(WORKING_DIR, cmake_command)

            # Build
            build_command = [get_cmake_path(), '--build', build_dir_for_type, '--target', 'install']
            # TODO ANDROID ensure all cores are being utilised on Win64/Ninja
            if not platform.startswith('win'):
                build_command.extend(['-j', str(cpu_count())])
            call(WORKING_DIR, build_command)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        if platform.startswith('linux'):
            # TODO I feel like any Android builds should go to zip, supporting easy extraction on any platform.
            #      Linux is currently creating a bz2 tarball.
            package_path = archive_framework_to_linux_tar_bz2(package_basename)
        elif platform == 'darwin':
            package_path = archive_framework_to_macos_zip(package_basename)
        else:
            package_path = archive_framework_to_win64_zip(package_basename)
    else:
        package_path = archive_to_timestamped_dir(package_basename)
    return package_path

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
            
def archive_source_archive_directory(source_directory):
    if platform.startswith('linux'):    
        package_filename_with_ext = create_linux_tar_bz2(source_directory)
        shutil.rmtree(source_directory)
    elif platform == 'darwin':
        package_filename_with_ext = create_macos_zip(source_directory)
        shutil.rmtree(source_directory)
    else:
        (package_filename_with_ext, _) = create_win64_zip(source_directory)
        shutil.rmtree(ARCHIVING_DIR)

    print("Source archived to %s" % os.path.abspath(package_filename_with_ext))

def build_package_basename(timestamp, label, cross_compile_target):
    """Build the name of our package and populate our JSON build info file"""

    # Do the packaging
    if not cross_compile_target is None:
        if cross_compile_target == CrossCompileTarget.ANDROID:
            platform_name = 'Android'
        elif cross_compile_target == CrossCompileTarget.IOS:
            platform_name = 'iOS'
    elif platform.startswith('linux'):
        platform_name = 'Linux'
    elif platform == 'darwin':
        platform_name = 'macOS'
    else:
        platform_name = 'Win64'

    # Fetch version from version.cmake
    (version_unparsed, _) = call(WORKING_DIR, [get_cmake_path(), '-P', 'cmake/version.cmake'], True)
    chunks = version_unparsed.split(':')
    if len(chunks) < 2:
        print("Error passing invalid output from version.cmake: %s" % version_unparsed)
        sys.exit(ERROR_INVALID_VERSION)
    version = chunks[1].strip()

    # Create filename, including timestamp or not as requested
    package_filename = "NAP-%s-%s" % (version, platform_name)
    source_archive_basename = "NAP_SOURCE-%s" % version
    if not timestamp is None:
        package_filename += "-%s" % timestamp
        source_archive_basename += "-%s" % timestamp
    if not label is None:
        package_filename += "-%s" % label
        source_archive_basename += "-%s" % label
    return (package_filename, source_archive_basename)

def verify_clean_git_repo():
    """Verify that the git repository has no local changes"""

    (out, err) = call('.', ['git', 'status', '--porcelain'], True)
    clean = out.strip() == '' and err.strip() == ''
    if not clean:
        print("Error: Git repo needs to be clean to create a source archive")
        sys.exit(ERROR_SOURCE_ARCHIVE_GIT_NOT_CLEAN)

def check_existing_source_archive(source_archive_basename, zip_source_archive, overwrite=False):
    """Verify we're not going to overwrite an existing source archive"""

    # Check staging dir name
    staging_dir = os.path.join(os.path.pardir, source_archive_basename)
    if os.path.exists(staging_dir):
        if overwrite:
            shutil.rmtree(staging_dir)
        else:
            print("Source archive staging dir %s already exists" % staging_dir)
            sys.exit(ERROR_SOURCE_ARCHIVE_EXISTING)

    # Check cwd staging dir name
    if os.path.exists(source_archive_basename):
        if overwrite:
            shutil.rmtree(source_archive_basename)
        else:
            if zip_source_archive:
                print("Source staging dir %s already exists" % source_archive_basename)
            else:
                print("Source archive output dir %s already exists" % source_archive_basename)
            sys.exit(ERROR_SOURCE_ARCHIVE_EXISTING)

    # Check final zipped filename
    output_path = os.path.join(os.path.pardir, source_archive_basename)
    if zip_source_archive:
        if platform.startswith('linux'):
            output_path += '.tar.bz2'
        else:
            output_path += '.zip'
        if os.path.exists(output_path):
            if overwrite:
                shutil.rmtree(output_path)
            else:
                print("Source archive output %s already exists" % output_path)
                sys.exit(ERROR_SOURCE_ARCHIVE_EXISTING)

def strip_client_apps_for_source_archive(staging_dir):
    # Remove apps
    shutil.rmtree(os.path.join(staging_dir, 'apps'))

    # Remove client projects from CMakeLists.txt
    path = os.path.join(staging_dir, 'CMakeLists.txt')
    output = []
    with open(path, 'r') as f:
        lines = f.readlines()
        in_client_targets = False
        for line in lines:
            if 'START_SOURCE_ARCHIVE_REMOVED_SECTION' in line:
                in_client_targets = True
                output.append("# project targets\n")
                output.append("add_subdirectory(projects/example)\n")
            if not in_client_targets:
                output.append(line)
            if 'END_SOURCE_ARCHIVE_REMOVED_SECTION' in line:
                in_client_targets = False

    with open(path, 'w') as f:
        f.writelines(output)

def create_source_archive(source_archive_basename, zip_source_archive, build_label, timestamp, git_revision, overwrite):
    """Create a source archive of the repository with projects removed"""

    print("Creating source archive...")

    # Determine archive name
    staging_dir = os.path.join(os.path.pardir, source_archive_basename)

    # Remove any existing staging/output directories/files
    if overwrite:
        check_existing_source_archive(source_archive_basename, zip_source_archive, True)

    # Copy the repo
    shutil.copytree('.', staging_dir, ignore=shutil.ignore_patterns('packaging_bin', 'packaging_build', 'packaging_lib', 'msvc64'))

    # # Clean any git ignored content
    call(staging_dir, ['git', 'clean', '-dxf'], True)
    # Hard clean the repo, to be sure
    call(staging_dir, ['git', 'reset', '--hard'], True)

    # Remove .git, handling shutil readonly permissions issues on Win64
    if sys.platform == 'win32':
        def del_rw(action, name, exc):
            os.chmod(name, stat.S_IWRITE)
            os.remove(name)
        shutil.rmtree(os.path.join(staging_dir, '.git'), onerror=del_rw)
    else:
        shutil.rmtree(os.path.join(staging_dir, '.git'))

    # Misc. cleanup       
    shutil.rmtree(os.path.join(staging_dir, 'test'))

    # Populate example project
    os.mkdir(os.path.join(staging_dir, 'projects'))
    os.rename(os.path.join(staging_dir, 'apps', 'example'), os.path.join(staging_dir, 'projects', 'example'))

    # Remove client projects
    strip_client_apps_for_source_archive(staging_dir)

    # Create executable bit retaining script if on Windows
    if sys.platform == 'win32':
        create_source_archive_executable_bit_applicator(staging_dir)

    # Populate source_archive.json
    call(WORKING_DIR, [get_cmake_path(), 
                       '-DOUT_PATH=%s' % staging_dir,
                       '-DNAP_ROOT=.',
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-P', 'cmake/source_archive_populate_info.cmake'
                       ])
    
    # Move alongside release
    local_archive_name = os.path.join('.', source_archive_basename)
    if overwrite and os.path.exists(local_archive_name):
        shutil.rmtree(local_archive_name)
    os.rename(staging_dir, local_archive_name)

    # Optionally: compress to file (and cleanup)
    if zip_source_archive:
        archive_source_archive_directory(source_archive_basename)
    else:
        print("Source archived to %s" % os.path.abspath(source_archive_basename))

def create_source_archive_executable_bit_applicator(staging_dir):
    """Create a batch file for retaining source archive file executable bits on Windows"""

    lines = subprocess.check_output('git ls-files --stage', shell=True)
    if type(lines) is bytes:
        lines = lines.decode('ascii', 'ignore')

    batch_filename = os.path.join(staging_dir, APPLY_PERMISSIONS_BATCHFILE)
    with open(batch_filename, 'w') as writer:
        writer.write('@echo off\n')
        for line in lines.split('\n'):
            chunks = line.split()
            if len(chunks) < 4:
                continue
            exe_bit_filepath = chunks[3]
            file_in_staging = os.path.join(staging_dir, *exe_bit_filepath.split('/'))
            if chunks[0] == '100755' and os.path.exists(file_in_staging):
                writer.write('if exist %s (\n' % exe_bit_filepath)
                writer.write('\tgit update-index --chmod=+x %s\n' % exe_bit_filepath)
                writer.write(')\n')

def get_nap_root():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))

def get_cmake_path():
    """Fetch the path to the CMake binary"""

    nap_root = get_nap_root()
    cmake_thirdparty_root = os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake')
    if platform.startswith('linux'):
        return os.path.join(cmake_thirdparty_root, 'linux', 'install', 'bin', 'cmake')
    elif platform == 'darwin':
        return os.path.join(cmake_thirdparty_root, 'osx', 'install', 'bin', 'cmake')
    else:
        return os.path.join(cmake_thirdparty_root, 'msvc', 'install', 'bin', 'cmake.exe')
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    labelling_group = parser.add_argument_group('Labelling')
    labelling_group.add_argument("-nt", "--no-timestamp", action="store_true",
                        help="Don't include timestamp in the release archive and folder name, for final releases")
    labelling_group.add_argument("-l", "--label", type=str,
                        help="An optional suffix for the package")    

    core_group = parser.add_argument_group('Core Behaviour')
    core_group.add_argument("-nz", "--no-zip", action="store_true",
                        help="Don't zip the release, package to a directory")
    core_group.add_argument("-ds", "--include-debug-symbols", action="store_true",
                        help="Include debug symbols")
    core_group.add_argument("-o", "--overwrite", action="store_true",
                    help="Overwrite any existing framework or source package")
    core_group.add_argument("-c", "--clean", action="store_true",
                        help="Clean build")
    core_group.add_argument("--build-projects", action="store_true",
                        help="Build projects while packaging (does not include in package)")

    source_archive_group = parser.add_argument_group('Source Archiving')
    source_archive_group.add_argument("-as", "--archive-source", action="store_true",
                        help="Create a source archive")        
    source_archive_group.add_argument("-asz", "--source-archive-zipped", action="store_true",
                        help="Zip the created source archive")        
    source_archive_group.add_argument("-aso", "--source-archive-only", action="store_true",
                        help="Only create a source archive")        

    naivi_apps_group = parser.add_argument_group('Naivi Apps')
    naivi_apps_group.add_argument("-a", "--include-apps", action="store_true",
                        help="Include Naivi apps, packaging them as projects")
    naivi_apps_group.add_argument("-sna", "--include-single-naivi-app",
                        type=str,
                        help="A single Naivi app to include, packaged as project. Conflicts with --include-apps.")

    android_group = parser.add_argument_group('Android')
    android_group.add_argument("--android", action="store_true",
                        help="Build for Android")
    android_group.add_argument("--android-ndk-root", 
                        type=str,
                        help="The path to NDK to use for the Android build")
    android_group.add_argument("--android-abis", 
                        type=str,
                        help="Restrict our package to the provided comma-separated ABI/s for Android")
    android_group.add_argument("--android-enable-python", action="store_true",
                        help="Enable Python for Android")

    unsupported_group = parser.add_argument_group('Unsupported')
    unsupported_group.add_argument("--include-docs", action="store_true",
                        help="Include documentation")

    args = parser.parse_args()

    # Make sure we're not trying to package all Naivi apps and just a single one
    if args.include_apps and not args.include_single_naivi_app is None:
        print("Error: Can't include a single Naivi app and include all Naivi apps..")
        sys.exit(ERROR_BAD_INPUT)

    # If we're packaging a single Naivi app make sure it exists
    if args.include_single_naivi_app:
        path = os.path.join(get_nap_root(), APPS_SOURCE_DIR, args.include_single_naivi_app)
        if not os.path.exists(path):
            print("Error: Can't package single Naivi app '%s' as it doesn't exist" % args.include_single_naivi_app)
            sys.exit(ERROR_BAD_INPUT)

    # It doesn't make sense to zip a source archive that we're not creating
    if args.source_archive_zipped and not args.archive_source:
        print("Error: Can't zip a source archive that you're not creating")
        sys.exit(ERROR_BAD_INPUT)

    if args.source_archive_only and (args.no_zip 
                                     or args.include_docs 
                                     or args.include_apps
                                     or args.include_single_naivi_app
                                     or args.clean
                                     or args.include_apps
                                     or args.android
                                     or args.android_ndk_root
                                     or args.android_abis
                                     or args.android_enable_python
                                     or args.include_debug_symbols
                                     or args.build_projects
                                     ):
        print("Error: You have specified options that don't make sense if only creating a source archive")
        sys.exit(ERROR_BAD_INPUT)

    # Package our build
    package(not args.no_zip,
            args.include_debug_symbols, 
            args.include_docs, 
            args.include_apps or not args.include_single_naivi_app is None, 
            args.include_single_naivi_app,
            args.clean, 
            not args.no_timestamp, 
            args.label,
            args.build_projects,
            args.archive_source or args.source_archive_only, 
            args.source_archive_zipped,
            args.source_archive_only,            
            args.overwrite, 
            args.android, 
            args.android_ndk_root,
            args.android_abis,
            args.android_enable_python)
