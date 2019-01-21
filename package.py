#!/usr/bin/env python
import argparse
import datetime
import json
import os
from multiprocessing import cpu_count
import shutil
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
BUILDINFO_FILE = 'dist/cmake/build_info.json'
BUILD_TYPES = ('Release', 'Debug')

ANDROID_ABIS = ('arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64')
# ANDROID_ABIS = ('armeabi-v7a',)
# ANDROID_ABIS = ('arm64-v8a',)
ANDROID_PLATFORM = 'android-19'
    
ERROR_PACKAGE_EXISTS = 1
ERROR_INVALID_VERSION = 2
ERROR_MISSING_ANDROID_NDK = 3

def call(cwd, cmd, capture_output=False, exception_on_nonzero=True):
    """Execute command in provided working directory"""

    # print('dir: %s' % cwd)
    # print('cmd: %s' % cmd)
    if capture_output:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        proc = subprocess.Popen(cmd, cwd=cwd)
    (out, err) = proc.communicate()
    if exception_on_nonzero and proc.returncode != 0:
        print("Bailing for non zero returncode")
        raise Exception(proc.returncode)
    return (out, err)

def package(zip_release, include_docs, include_apps, clean, include_timestamp_in_name, overwrite, android_build, android_ndk_root):
    """Package a NAP platform release - main entry point"""

    os.chdir(os.path.dirname(os.path.realpath(__file__)))

    # Define cross compilation target, if relevant 
    cross_compile_target = None
    if android_build:
        cross_compile_target = CrossCompileTarget.ANDROID

    # Add timestamp and git revision for build info
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    (git_revision, _) = call(WORKING_DIR, ['git', 'rev-parse', 'HEAD'], True)
    git_revision = git_revision.decode('ascii', 'ignore').strip()
    package_basename = build_package_basename(include_timestamp_in_name, timestamp, cross_compile_target)

    # Ensure we won't overwrite any existing package
    if not overwrite:
        check_for_existing_package(package_basename, zip_release)

    print("Packaging...")

    # Remove old packaging path if it exists
    if os.path.exists(PACKAGING_DIR):
        shutil.rmtree(PACKAGING_DIR, True)
    os.makedirs(PACKAGING_DIR)

    # Clean build if requested
    if clean:
        clean_the_build(cross_compile_target)

    # Do the packaging
    if android_build:
        package_for_android(package_basename, timestamp, git_revision, overwrite, zip_release, android_ndk_root)
    elif platform.startswith('linux'):    
        package_for_linux(package_basename, timestamp, git_revision, overwrite, include_apps, include_docs, zip_release)
    elif platform == 'darwin':
        package_for_macos(package_basename, timestamp, git_revision, overwrite, include_apps, include_docs, zip_release)
    else:
        package_for_win64(package_basename, timestamp, git_revision, overwrite, include_apps, include_docs, zip_release)

def clean_the_build(cross_compile_target):
    """Clean the build"""

    print("Cleaning...")

    if not cross_compile_target is None:
        if cross_compile_target == CrossCompileTarget.ANDROID:
            # Iterate ABIs
            for abi in ANDROID_ABIS:
                # Iterate build types
                for build_type in BUILD_TYPES:
                    build_dir_for_type = "%s_Android_%s_%s" % (BUILD_DIR, abi, build_type.lower())
                    if os.path.exists(build_dir_for_type):
                        print("Clean removing %s" % os.path.abspath(build_dir_for_type))
                        shutil.rmtree(build_dir_for_type, True)
    elif platform.startswith('linux'):    
        for build_type in BUILD_TYPES:
            build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())
            if os.path.exists(build_dir_for_type):
                print("Clean removing %s" % os.path.abspath(build_dir_for_type))
                shutil.rmtree(build_dir_for_type, True)
    else:
        if os.path.exists(BUILD_DIR):
            print("Clean removing %s" % os.path.abspath(BUILD_DIR))
            shutil.rmtree(BUILD_DIR, True)

    if os.path.exists(LIB_DIR):
        print("Clean removing %s" % os.path.abspath(LIB_DIR))
        shutil.rmtree(LIB_DIR, True)
    if os.path.exists(BIN_DIR):
        print("Clean removing %s" % os.path.abspath(BIN_DIR))
        shutil.rmtree(BIN_DIR, True)                  

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
                shutil.rmtree(package_path, True)
        else:
            print("Error: %s already exists" % package_path)
            sys.exit(ERROR_PACKAGE_EXISTS)

def package_for_linux(package_basename, timestamp, git_revision, overwrite, include_apps, include_docs, zip_release):
    """Package NAP platform release for Linux"""

    for build_type in BUILD_TYPES:
        build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())
        call(WORKING_DIR, ['cmake', 
                           '-H.', 
                           '-B%s' % build_dir_for_type, 
                           '-DNAP_PACKAGED_BUILD=1',
                           '-DCMAKE_BUILD_TYPE=%s' % build_type,
                           '-DINCLUDE_DOCS=%s' % int(include_docs),
                           '-DPACKAGE_NAIVI_APPS=%s' % int(include_apps),
                           '-DBUILD_TIMESTAMP=%s' % timestamp,
                           '-DBUILD_GIT_REVISION=%s' % git_revision
                           ])

        d = '%s/%s' % (WORKING_DIR, build_dir_for_type)
        call(d, ['make', 'all', 'install', '-j%s' % cpu_count()])

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        archive_to_linux_tar_bz2(package_basename)
    else:
        archive_to_timestamped_dir(package_basename)

def package_for_macos(package_basename, timestamp, git_revision, overwrite, include_apps, include_docs, zip_release):
    """Package NAP platform release for macOS"""

    # Generate project
    call(WORKING_DIR, ['cmake', 
                       '-H.', 
                       '-B%s' % BUILD_DIR, 
                       '-G', 'Xcode',
                       '-DNAP_PACKAGED_BUILD=1',
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DPACKAGE_NAIVI_APPS=%s' % int(include_apps),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision
                       ])

    # Build & install to packaging dir
    d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
    for build_type in BUILD_TYPES:
        call(d, ['xcodebuild', '-configuration', build_type, '-target', 'install', '-jobs', str(cpu_count())])

    # Remove unwanted files (eg. .DS_Store)
    call(PACKAGING_DIR, ['find', '.', '-name', '.DS_Store', '-type', 'f', '-delete'])

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        archive_to_macos_zip(package_basename)
    else:
        archive_to_timestamped_dir(package_basename)

def package_for_win64(package_basename, timestamp, git_revision, overwrite, include_apps, include_docs, zip_release):
    """Package NAP platform release for Windows"""

    # Create build dir if it doesn't exist
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    # Generate project
    call(WORKING_DIR, ['cmake', 
                       '-H.', 
                       '-B%s' % BUILD_DIR, 
                       '-G', 'Visual Studio 14 2015 Win64',
                       '-DNAP_PACKAGED_BUILD=1',
                       '-DPYBIND11_PYTHON_VERSION=3.5',
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DPACKAGE_NAIVI_APPS=%s' % int(include_apps),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision
                       ])

    # Build & install to packaging dir
    for build_type in BUILD_TYPES:
        call(WORKING_DIR, ['cmake', '--build', BUILD_DIR, '--target', 'install', '--config', build_type])

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)        

    # Create archive
    if zip_release:
        archive_to_win64_zip(package_basename)
    else:
        archive_to_timestamped_dir(package_basename)

def package_for_android(package_basename, timestamp, git_revision, overwrite, zip_release, android_ndk_root):
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
    for abi in ANDROID_ABIS:
        # Iterate build types
        for build_type in BUILD_TYPES:
            build_dir_for_type = "%s_Android_%s_%s" % (BUILD_DIR, abi, build_type.lower())

            # Build CMake command with varying platform options
            # TODO test and support Linux?
            cmake_command = ['cmake', 
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
                             '-DBUILD_GIT_REVISION=%s' % git_revision
                             ]

            # Set Ninja generator on Windows
            if platform.startswith('win'):
                cmake_command.append('-GNinja')

            # Generate project
            call(WORKING_DIR, cmake_command)

            # Build
            build_command = ['cmake', '--build', build_dir_for_type, '--target', 'install']
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
            archive_to_linux_tar_bz2(package_basename)
        elif platform == 'darwin':
            archive_to_macos_zip(package_basename)
        else:
            archive_to_win64_zip(package_basename)
    else:
        archive_to_timestamped_dir(package_basename)

def archive_to_linux_tar_bz2(package_basename):
    """Create build archive to bzipped tarball on Linux"""

    shutil.move(PACKAGING_DIR, package_basename)

    package_filename_with_ext = '%s.%s' % (package_basename, 'tar.bz2')
    print("Archiving to %s ..." % os.path.abspath(package_filename_with_ext))
    call(WORKING_DIR, ['tar', '-cjvf', package_filename_with_ext, package_basename])

    # Cleanup
    shutil.move(package_basename, PACKAGING_DIR)
    print("Packaged to %s" % os.path.abspath(package_filename_with_ext))

def archive_to_macos_zip(package_basename):
    """Create build archive to zip on macOS"""

    shutil.move(PACKAGING_DIR, package_basename)

    # Archive
    package_filename_with_ext = '%s.%s' % (package_basename, 'zip')
    print("Archiving to %s ..." % os.path.abspath(package_filename_with_ext))
    call(WORKING_DIR, ['zip', '-yr', package_filename_with_ext, package_basename])

    # Cleanup
    shutil.move(package_basename, PACKAGING_DIR)
    print("Packaged to %s" % os.path.abspath(package_filename_with_ext))    

def archive_to_win64_zip(package_basename):
    """Create build archive to zip on Win64"""

    package_filename_with_ext = '%s.%s' % (package_basename, 'zip')

    # Rename our packaging dir to match the release
    shutil.move(PACKAGING_DIR, package_basename)

    # Create our archive dir, used to create a copy level folder within the archive
    if os.path.exists(ARCHIVING_DIR):
        shutil.rmtree(ARCHIVING_DIR, True)
    os.makedirs(ARCHIVING_DIR)
    archive_path = os.path.join(ARCHIVING_DIR, package_basename)
    shutil.move(package_basename, archive_path)

    # Create archive
    print("Archiving to %s ..." % os.path.abspath(package_filename_with_ext))
    shutil.make_archive(package_basename, 'zip', ARCHIVING_DIR)

    # Cleanup
    shutil.move(archive_path, PACKAGING_DIR)
    shutil.rmtree(ARCHIVING_DIR)

    print("Packaged to %s" % os.path.abspath(package_filename_with_ext))  

def archive_to_timestamped_dir(package_basename):
    """Copy our packaged dir to a timestamped dir"""

    shutil.move(PACKAGING_DIR, package_basename)

    print("Packaged to directory %s" % os.path.abspath(package_basename))

def build_package_basename(include_timestamp_in_name, timestamp, cross_compile_target):
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
    (version_unparsed, _) = call(WORKING_DIR, ['cmake', '-P', 'cmake/version.cmake'], True)
    chunks = version_unparsed.decode('ascii', 'ignore').split(':')
    if len(chunks) < 2:
        print("Error passing invalid output from version.cmake: %s" % version_unparsed)
        sys.exit(ERROR_INVALID_VERSION)
    version = chunks[1].strip()

    # Create filename, including timestamp or not as requested
    package_filename = "NAP-%s-%s" % (version, platform_name)
    if include_timestamp_in_name:
        package_filename += "-%s" % timestamp
    return package_filename
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-nz", "--no-zip", action="store_true",
                        help="Don't zip the release, package to a directory")
    parser.add_argument("-nt", "--no-timestamp", action="store_true",
                        help="Don't include timestamp in the release archive and folder name, for final releases")
    parser.add_argument("-c", "--clean", action="store_true",
                        help="Clean build")
    parser.add_argument("-a", "--include-apps", action="store_true",
                        help="Include Naivi apps, packaging them as projects")
    parser.add_argument("-o", "--overwrite", action="store_true",
                        help="Overwrite any existing package")
    parser.add_argument("--include-docs", action="store_true",
                        help="Include documentation")
    parser.add_argument("--android", action="store_true",
                        help="Build for Android")
    parser.add_argument("--android-ndk-root", 
                        type=str,
                        help="The path to NDK to use for the Android build")

    args = parser.parse_args()

    # Package our build
    package(not args.no_zip, args.include_docs, args.include_apps, args.clean, not args.no_timestamp, args.overwrite, args.android, args.android_ndk_root)
