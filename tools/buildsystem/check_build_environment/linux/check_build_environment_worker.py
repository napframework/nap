#!/usr/bin/env python3
from distutils.version import LooseVersion
import json
import os
from platform import machine
import subprocess
import shutil
import sys

REQUIRED_UBUNTU_VERSIONS = ['20.04','22.04','24.04']
REQUIRED_QT_VERSION = '5.15.2'
REQUIRED_RASPBIAN_VERSION = '11'
SUPPORTED_ARCHITECTURES = ('x86_64', 'arm64', 'armhf')

def call(cmd):
    """Execute command and return stdout"""

    # print("Command: %s" % cmd)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    return out.strip().decode('utf-8')

def call_with_returncode(cmd):
    """Execute command and return stdout and returncode"""

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    return (out.strip().decode('utf-8'), proc.returncode)

def log_test_success(test_item, success):
    """Log a test success"""

    print("Checking %s: %s" % (test_item, 'PASS' if success else 'FAIL'))

def get_build_arch():
    """Fetch build architecture as used by NAP"""
    machine_arch = machine()
    if machine_arch.lower() in ('x86_64', 'amd64'):
        nap_arch = 'x86_64'
    elif machine_arch == 'aarch64':
        p = subprocess.run('getconf LONG_BIT', shell=True, universal_newlines=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if p.stdout.strip() == '64':
            nap_arch = 'arm64'
        else:
            nap_arch = 'armhf'
    else:
        nap_arch = 'armhf'
    return nap_arch

def check_arch():
    """Check if the machine matches the package arch"""

    expected_arch = None
    build_info_file = os.path.join(get_nap_root(), 'cmake', 'build_info.json')
    with open(build_info_file, 'r') as f:
        data = json.load(f)
        expected_arch = data['architecture']

    arch = get_build_arch()
    arch_ok = arch == expected_arch
    log_test_success('matched architecture', arch_ok)
    return (arch_ok, expected_arch, arch)

def check_distribution():
    """Check if the we're running our supported distro for the arch"""

    arch = get_build_arch()
    distribution = call('lsb_release -id | grep ID')
    distributor_id = distribution.split(':')[1].strip().lower()
    if arch == 'armhf':
        distribution_ok = distributor_id == 'raspbian'
        log_test_success('Raspbian distribution', distribution_ok)
    else:
        distribution_ok = distributor_id == 'ubuntu'
        log_test_success('Ubuntu distribution', distribution_ok)

    return distribution_ok

def check_distribution_version():
    """Check if the we're running our supported distribution version"""

    arch = get_build_arch()
    release = call('lsb_release -r')
    release = release.split(':')[1].strip()
    if arch == 'armhf':
        release_ok = release == REQUIRED_RASPBIAN_VERSION
        log_test_success('Raspbian %s' % REQUIRED_RASPBIAN_VERSION, release_ok)
    else:
        release_ok = release in REQUIRED_UBUNTU_VERSIONS
        log_test_success('Ubuntu %s' % release, release_ok)
    return release_ok

def apt_package_installed(package_name):
    """Check if a package is installed via apt"""

    list_output = call('dpkg -l %s | grep %s' % (package_name, package_name))
    return list_output.startswith('ii')

def check_compiler():
    """Check that c++ is setup for GCC"""

    alternatives_output = call('update-alternatives --query c++ | grep Value')
    gcc_ok = alternatives_output == '' or '/g++' in alternatives_output

    log_test_success('C++ is GCC', gcc_ok)
    return gcc_ok

def check_qt_env_var():
    """Check Qt env. var. for source user"""

    qt_env_var_ok = 'QT_DIR' in os.environ
    log_test_success('Qt environment variable', qt_env_var_ok)
    return qt_env_var_ok

def check_qt_version():
    """Check Qt version for source user"""

    qt_found_version = None
    qt_version_ok = False

    # Remove temporary directory for CMake project files to go into if it exists
    nap_root = get_source_nap_root()
    qt_checker_path = os.path.join(nap_root, 'cmake', 'qt_checker')
    temp_build_dir = os.path.join(qt_checker_path, 'project_temp')
    if os.path.exists(temp_build_dir):
        shutil.rmtree(temp_build_dir)

    # Run Qt version checking logic, parsing output
    thirdparty_dir = os.path.join(nap_root, 'thirdparty')
    arch = get_build_arch()
    cmake = os.path.join(thirdparty_dir, 'cmake', 'linux', 'x86_64', 'bin', 'cmake')

    (out, returncode) = call_with_returncode(' '.join((cmake, qt_checker_path, '-B', temp_build_dir)))
    if returncode == 0:
        lines = out.split('\n')
        for line in lines:
            if 'Found Qt' in line:
                chunks = line.strip().split('Found Qt')
                if len(chunks) > 1:
                    qt_found_version = chunks[-1].strip()
                break

    # OK is version matching required version
    if not qt_found_version is None:
        qt_version_ok = LooseVersion(qt_found_version) == LooseVersion(REQUIRED_QT_VERSION)

    # Cleanup
    if os.path.exists(temp_build_dir):
        shutil.rmtree(temp_build_dir)

    log_test_success('Qt version v%s' % REQUIRED_QT_VERSION, qt_version_ok)
    return (qt_version_ok, qt_found_version)

def log_qt_help(qt_env_var_ok, qt_found_version):
    """Log help for source user Qt problems"""

    if not qt_env_var_ok:
        print("\nThis version of NAP requires Qt v%s as downloaded directly from Qt. Distribution versions are not supported. Once Qt v%s has been downloaded it should be pointed to with the environment variable QT_DIR, eg. QT_DIR=\"/home/username/Qt%s/%s/gcc_64\"." % (REQUIRED_QT_VERSION, REQUIRED_QT_VERSION, REQUIRED_QT_VERSION, REQUIRED_QT_VERSION))
    else:
        print("\nThis version of NAP requires Qt v%s, however you appear to have v%s. Other versions may work but are not supported." % (REQUIRED_QT_VERSION, qt_found_version))

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

def get_nap_root():
    """Get framework root directory"""
    script_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir))

def get_source_nap_root():
    """Get source framework root directory"""
    script_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(script_dir, "../../../.."))

def check_and_warn_for_potential_system_qt():
    """Attempt to detect if the Qt installation might be a system version and warn if so"""

    running_apt_distro = call('which apt') != ''
    if running_apt_distro:
        (_, returncode) = call_with_returncode("dpkg -l qtdeclarative5-dev")
        if returncode == 0:
            print("\nWarning: You appear to have Qt development packages installed via your system packaging system. This may cause issues, especially with packaging.")

    qt_dir = os.environ['QT_DIR']
    qt_dir = os.path.abspath(qt_dir)
    if qt_dir.startswith('/usr/') and not qt_dir.startswith('/usr/local'):
        print("\nWarning: Your Qt appears to be located in a system directory. Ensure that you are using the distribution directly from Qt instead of your system packaged version.")

def check_build_environment(against_source):
    """Check whether Linux build environment appears ready for NAP"""

    arch = get_build_arch()

    if against_source:
        # Check if supported arch
        supported_arch = arch in SUPPORTED_ARCHITECTURES
        log_test_success('supported architecture', supported_arch)
        if not supported_arch:
            print("\nNAP only currently supports %s systems under Linux".format(','.join(SUPPORTED_ARCHITECTURES)))
            sys.exit(1)
    else:
        # Check if arch matches build
        (arch_ok, expected_arch, system_arch) = check_arch()
        if not arch_ok:
            print("\nNAP only supports %s systems and yours is %s. Not continuing checks.".format(expected_arch, system_arch))
            sys.exit(1)

    # Check distribution
    distribution_ok = check_distribution()
    if not distribution_ok:
        if arch == 'armhf':
            print("\nThis version of NAP supports Raspbian Linux (%s). Other distributions may work but are unsupported." % REQUIRED_RASPBIAN_VERSION)
        else:
            print("\nThis version of NAP supports Ubuntu Linux (%s). Other distributions may work but are unsupported." % REQUIRED_UBUNTU_VERSION)
        print("Hint for the adventurous: On Debian-based distros we depend on build-essential, patchelf and when working against Source libglu1-mesa-dev (for Qt)")
        print("\nNot continuing checks.")
        sys.exit(1)

    # Check distribution version
    distribution_version_ok = check_distribution_version()

    # Check package build-essential installed
    build_essential_installed = apt_package_installed('build-essential')
    log_test_success('for build-essential package', build_essential_installed)

    # Check package libxcb-xinerama, brings in xcb dependencies for running napkin
    libxcb_installed = apt_package_installed('libxcb-xinerama0')
    log_test_success('for libxcb-xinerama package', libxcb_installed)

    # Check package libjack0 - required by portaudio
    libjack_installed = apt_package_installed('libjack0')
    log_test_success('for libjack0 package', libjack_installed)

    # Check C++ is GCC
    compiler_ok = check_compiler()

    # Check package patchelf installed
    patchelf_installed = apt_package_installed('patchelf')
    log_test_success('for patchelf package', patchelf_installed)

    # Check mesa vulkan drivers installed
    mesa_vulkan_installed = apt_package_installed('mesa-vulkan-drivers')
    log_test_success('for mesa vulkan drivers', mesa_vulkan_installed)

    if against_source:

        # Check package libglu1-mesa-dev installed, currently brings in
        # dependencies needed by Qt when working against source
        glut_installed = apt_package_installed('libglu1-mesa-dev')
        log_test_success('for libglu1-mesa-dev package (needed by Qt for Napkin)', glut_installed)

        # Check for Qt
        qt_env_var_ok = check_qt_env_var()

        # Check Qt version
        if qt_env_var_ok:
            (qt_version_ok, qt_found_version) = check_qt_version()
        else:
            qt_version_ok = False
            qt_found_version = None

    print("")

    # If we're running for source users check source requirements are met
    extra_source_requirements_ok = True
    if against_source and (not qt_env_var_ok or not qt_version_ok or not glut_installed):
        extra_source_requirements_ok = False

    # If everything looks good log and exit
    if distribution_version_ok \
      and build_essential_installed \
      and patchelf_installed \
      and compiler_ok \
      and extra_source_requirements_ok:
        print("Your build environment appears to be ready for NAP!")

        if against_source:
            check_and_warn_for_potential_system_qt()

        return False

    print("Some issues were encountered:")

    # Warn about wrong Ubuntu version
    if not distribution_version_ok:
        if arch == 'armhf':
            print("\nWarning: This version of NAP is supported on Raspbian %s. Other Linux configurations may work but are unsupported." % REQUIRED_RASPBIAN_VERSION)
        else:
            print("\nWarning: This version of NAP is supported on Ubuntu %s. Other Linux configurations may work but are unsupported." % REQUIRED_UBUNTU_VERSION)

    if not compiler_ok:
        print("\nYour C++ compiler is not currently set to GCC. This release of NAP only currently supports GCC.")
        return False

    # Build apt packages to offer to install
    packages_to_install = []
    if not build_essential_installed:
        packages_to_install.append('build-essential')
    if not patchelf_installed:
        packages_to_install.append('patchelf')
    if against_source and not glut_installed:
        packages_to_install.append('libglu1-mesa-dev')
    if not mesa_vulkan_installed:
        packages_to_install.append('mesa-vulkan-drivers')
    if not libxcb_installed:
        packages_to_install.append('libxcb-xinerama0')
    if not libjack_installed:
        packages_to_install.append('libjack0')

    if len(packages_to_install) > 0:
        package_str = ' '.join(packages_to_install)
        print("\nThe following package/s are required and are not installed: %s" % package_str)
        installation_approved = read_yes_no("Kick off installation via apt?")
        if installation_approved:
            subprocess.call('sudo apt-get install %s' % package_str, shell=True)
            return True
        else:
            print("Re-run check_build_environment once you have installed the requirements.")
            return False

    # Show Qt help
    if against_source :
        if (not qt_env_var_ok or not qt_version_ok):
            log_qt_help(qt_env_var_ok, qt_found_version)

        if qt_env_var_ok:
            check_and_warn_for_potential_system_qt()

        if (not qt_env_var_ok or not qt_version_ok):
            print("\nRe-run check_build_environment once you have made the required changes.")
            return False

if __name__ == '__main__':
    against_source = len(sys.argv) > 1 and sys.argv[1] == '--source'
    while check_build_environment(against_source):
         print("\nRe-running checks...\n")
