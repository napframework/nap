#!/usr/bin/env python
from msvcrt import getch
import os
import subprocess
import sys
import webbrowser

REQUIRED_WINDOWS_VERSION = '10.0'
VS_2015_INSTALLED_REG_KEY = 'HKEY_CLASSES_ROOT\VisualStudio.DTE.14.0'
VS_2015_VERSION_REG_QUERY = 'HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\DevDiv\\vs\\Servicing\\14.0\\devenv /v UpdateVersion'
REQUIRED_VS_2015_PATCH_VERSION = 25420
CMAKE_MIN_VERSION = (3, 5)

def call(cmd, provide_exit_code=False):
    """Execute command and return stdout"""

    # print("Command: %s" % cmd)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    if provide_exit_code:
        return proc.returncode
    else:
        return str(out.strip())

def log_test_success(test_item, success):
    """Log a test success"""

    print("Checking %s: %s" % (test_item, 'PASS' if success else 'FAIL'))

def read_yes_no(question):
    """Read a yes/no answer for a question"""

    yes = ('yes','y', 'ye', '')
    no = ('no','n')
     
    while True:
        prompt = question + ' [Y/n] '
        if sys.version_info >= (3, 0):
            choice = input(prompt)
        else:
            choice = raw_input(prompt)
        choice = choice.lower().strip()
        if choice in yes:
           return True
        elif choice in no:
           return False
        else:
           print("Please respond with 'yes' or 'no'\n")

def check_windows_version():
    """Check if the we're running Windows 10"""

    ver_output = call('ver').strip()
    version = ver_output.split()[-1]
    # version = '3.11'
    windows_version_ok = version.startswith(REQUIRED_WINDOWS_VERSION + '.')
    log_test_success('for Windows 10', windows_version_ok)
    return windows_version_ok    

def check_visual_studio_2015_installed():
    """Check if Visual Studio 2015 is installed"""

    return_code = call('reg query "%s"' % VS_2015_INSTALLED_REG_KEY, True)

    # return_code = 1
    visual_studio_2015_installed = return_code == 0
    log_test_success('for Visual Studio 2015', visual_studio_2015_installed)
    return visual_studio_2015_installed    
    
def check_visual_studio_2015_is_update3():
    """Check if Visual Studio 2015 version is Update 3"""

    ver_output = call('reg query %s' % VS_2015_VERSION_REG_QUERY)
    version = ver_output.strip("'").split()[-1]
    # version = '14.0.0'
    (_, minor, patch) = version.split('.')
    version_ok = int(minor) == 0 and int(patch) >= REQUIRED_VS_2015_PATCH_VERSION
    log_test_success('Visual Studio 2015 is Update 3', version_ok)
    return version_ok

def handle_missing_vs2015_update3(have_earlier_vs_2015):
    """If we don't have Visual Studio 2015 Update 3 help install it"""

    # Show different help depending on whether they already have an older version installed.
    if have_earlier_vs_2015:        
        print("\nVisual Studio 2015 is installed but is an older version.  Update 3 is required and can be installed within VS or downloaded from https://www.visualstudio.com/vs/older-downloads/.")
    else:
        print("\nVisual Studio 2015 Update 3 is required and Community Edition can be downloaded for free from https://www.visualstudio.com/vs/older-downloads/.")

    # Offer to open download page
    open_vs_download = read_yes_no("Open download page?")
    if open_vs_download:
        webbrowser.open('https://www.visualstudio.com/vs/older-downloads/')
        print("\nPlease re-run checkBuildEnvironment after update has completed and you have rebooted.")
    else:
        # Provide more specific instruction if they aren't downloading now
        print("\nPlease re-run checkBuildEnvironment after you have installed Visual Studio 2015 Update 3 and rebooted.")        

def check_cmake():
    """Check if the CMake >= 3.5 is installed"""

    # Check if installed
    cmake_ok = False
    return_code = call('cmake', True)
    cmake_installed = return_code == 0

    # Check version >= 3.5
    if cmake_installed:
        cmake_version = call('cmake --version')
        chunks = cmake_version.split("\n")[0].strip().split()
        (major, minor, patch) = chunks[2].split('.')
        cmake_ok = int(major) == CMAKE_MIN_VERSION[0] and int(minor) >= CMAKE_MIN_VERSION[1]

    # cmake_ok = False
    log_test_success('CMake >= 3.5', cmake_ok)
    return cmake_ok

def handle_missing_cmake():
    """Assist with CMake installation"""

    print("\nCMake version 3.5 or higher is required and can be downloaded from https://cmake.org/download/.  It needs to be available in the PATH environment variable.")
    open_cmake_download = read_yes_no("Open download page?")
    if open_cmake_download:
        webbrowser.open('https://cmake.org/download/')

    # We'll need to run in a new shell with updated environment variables for CMake to be available on the command line,
    # so there's no point in attempting to loop through the tests again.
    print("\nRe-run checkBuildEnvironment once CMake is installed and added to the PATH.")

def check_build_environment():
    """Check whether Windows build environment appears ready for NAP"""

    # Check Windows version
    windows_version_ok = check_windows_version()

    # Check if Visual Studio 2015 is installed
    have_vs_2015 = check_visual_studio_2015_installed()

    # Check if Visual Studio 2015 Update 3 is installed
    have_vs_2015_update3 = have_vs_2015 and check_visual_studio_2015_is_update3()

    # Check CMake
    cmake_ok = check_cmake()

    print("")

    # If everything looks good log and exit
    if windows_version_ok and have_vs_2015_update3 and cmake_ok:
        print("Your build environment appears to be ready for NAP!")
        return

    print("Some issues were encountered:")

    # Warn about wrong Windows version
    if not windows_version_ok:
        print("\nWarning: This version of NAP is supported on Windows 10.  Other Windows versions may work but are unsupported.")

    # If we don't have Visual Studio 2015 Update 3 help install it
    if not have_vs_2015_update3:
        handle_missing_vs2015_update3(have_vs_2015)
        return

    # If we don't CMake >= 3.5 provide help for installing it
    if not cmake_ok:
        handle_missing_cmake()

if __name__ == '__main__':
    check_build_environment()
    print("\nPress key to close...")
    getch()
