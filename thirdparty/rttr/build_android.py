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

WORKING_DIR = '.'
BUILD_TYPES = ('Release', 'Debug')
ANDROID_ABIS = ('arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64')
ANDROID_PLATFORM = 'android-19'
    
ERROR_MISSING_ANDROID_NDK = 1

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

def build(android_ndk_root):
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
            build_dir_for_type = "build_android_%s_%s" % (abi, build_type.lower())

            cmake_command = ['cmake', 
                             '-H.', 
                             '-B%s' % build_dir_for_type, 
                             '-DCMAKE_TOOLCHAIN_FILE=%s' % toolchain_file,
                             '-DANDROID_NDK=%s' % ndk_root,
                             '-DANDROID_ABI=%s' % abi,
                             '-DANDROID_PLATFORM=%s' % ANDROID_PLATFORM,
                             '-DCMAKE_BUILD_TYPE=%s' % build_type,
                             '-DBUILD_STATIC=OFF',
                             '-DBUILD_RTTR_DYNAMIC=ON',
                             '-DBUILD_UNIT_TESTS=OFF',
                             '-DBUILD_BENCHMARKS=OFF',
                             '-DBUILD_EXAMPLES=OFF',
                             '-DBUILD_PACKAGE=OFF',
                             '-DBUILD_DOCUMENTATION=OFF',
                             '-DBUILD_INSTALLER=OFF'
                             ]

            # Configure
            call(WORKING_DIR, cmake_command)

            # Build
            call(WORKING_DIR, ['cmake', '--build', build_dir_for_type, '--', '-j', str(cpu_count())])

            # Install to
            install_dir = os.path.join('.', 'android', 'install', 'bin', build_type, abi)
            if not os.path.exists(install_dir):
                os.makedirs(install_dir)
            lib_filename = 'librttr_core.so' if build_type is 'Release' else 'librttr_core_d.so'
            library_path = os.path.join('.', build_dir_for_type, 'bin', lib_filename)
            shutil.copyfile(library_path, os.path.join(install_dir, lib_filename))

    # Install includes
    # TODO fairly hacky copying of includes from the Linux install path to accommodate
    #      there (seeming to?) be no install phase in the Android toolchain
    include_src_dir = os.path.join('.', 'linux', 'install', 'include')
    include_install_dir = os.path.join('android', 'install', 'include')
    if os.path.exists(include_install_dir):
        shutil.rmtree(include_install_dir)
    shutil.copytree(include_src_dir, include_install_dir)

    # Install license/docs
    shutil.copyfile('README.md', os.path.join('android', 'install', 'README.md'))
    shutil.copyfile('LICENSE.txt', os.path.join('android', 'install', 'LICENSE.txt'))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--android-ndk-root", 
                        type=str,
                        help="The path to NDK to use for the Android build")

    args = parser.parse_args()

    build(args.android_ndk_root)
