#!/usr/bin/env python
import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil

WORKING_DIR = '.'

def call(cwd, cmd, capture_output=False, exception_on_nonzero=True):
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

# Install dependencies for Linux, checking if we have them first (avoiding an sudo if they're already installed)
def install_dependencies_linux():
    dependencies = [
        'cmake',
        'build-essential',
        'patchelf',
        'doxygen'
    ]

    print("Checking dependencies")
    # Create a list of packages we need to install
    packages_to_install = []
    for d in dependencies:
        if not is_linux_apt_package_installed(d):
            packages_to_install.append(d)

    # Return if all already installed
    if len(packages_to_install) == 0:
        print("All dependencies already installed")
        return

    # Build cmd and install with apt
    print("Installing dependencies via apt: %s" % ' '.join(packages_to_install))
    apt_cmd = ['sudo', 'apt-get', '--assume-yes', 'install']
    apt_cmd.extend(packages_to_install)
    call(WORKING_DIR, apt_cmd)

# Check if we have a package installed with apt
def is_linux_apt_package_installed(package_name):
    (out, err) = call(WORKING_DIR, ['dpkg', '--get-selections', package_name], True)
    if type(err) is bytes:
        err = err.decode('utf-8')    
    installed = not 'no packages' in err
    print("Package %s installed? %s" % (package_name, 'Yes' if installed else 'No'))
    return installed

# Check if brew is installed
def is_osx_brew_installed():
    print("Checking for homebrew")
    try:
        brew_path = call(WORKING_DIR, ['which', 'brew'], True)[0].strip()
        return os.path.exists(brew_path)
    except:
        return False

# Check if we have a package installed with homebrew
def is_osx_brew_package_installed(package_name):
    # This is a bit slow, running brew list once and parsing would be faster.. but we're not going to
    # depend on homebrew anyway.
    (out, err) = call(WORKING_DIR, ['brew', 'list', package_name], True, False)
    if type(err) is bytes:
        err = err.decode('utf-8')
    installed = not 'Error: No such keg' in err
    print("Package %s installed? %s" % (package_name, 'Yes' if installed else 'No'))
    return installed

# Install dependencies for macOS via homebrew, checking if we have them first
def install_dependencies_osx():
    dependencies = [
        'cmake'
    ]

    if not is_osx_brew_installed():
        print("Not installing macOS dependencies as homebrew was not found")
        # Would fail hard here..  but we're not going to depend on homebrew anyway
        return

    print("Checking dependencies")
    # Create a list of packages we need to install
    packages_to_install = []
    for d in dependencies:
        if not is_osx_brew_package_installed(d):
            packages_to_install.append(d)

    # Return if all already installed
    if len(packages_to_install) == 0:
        print("All dependencies already installed")
        return

    print("Installing dependencies")
    for pack in packages_to_install:
        try:
            call(WORKING_DIR, ['brew', 'install', pack])
        except:
            print("Failed installing %s via homebrew" % pack)

def install_dependencies():
    if platform.startswith('linux'):
        install_dependencies_linux()
    elif platform == "darwin":
        install_dependencies_osx()
    elif platform == "win32":
        # Windows...
        print("Not supported on Windows")

# main run
if __name__ == '__main__':
    install_dependencies()