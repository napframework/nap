#!/usr/bin/env python3
from distutils.version import LooseVersion
from msvcrt import getch
import os
import shutil
import subprocess
import sys
import webbrowser

REQUIRED_WINDOWS_VERSION = '10.0'
VS_INSTALLED_REG_KEY = 'HKEY_CLASSES_ROOT\\VisualStudio.DTE.16.0'
REQUIRED_VS_VERSION = "2019"
REQUIRED_QT_VERSION = '5.15.2'
VS_VERSION_REG_QUERY = 'HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\DevDiv\\vs\\Servicing\\16.0\\devenv /v UpdateVersion'
REQUIRED_PATCH_VERSION = 25420

def call(cmd, provide_exit_code=False):
    """Execute command and return stdout"""

    # print("Command: %s" % cmd)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    if provide_exit_code:
        return proc.returncode
    else:
        return str(out.strip())

def call_with_returncode(cmd):
    """Execute command and return stdout and returncode"""

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    return (out.strip().decode('utf-8'), proc.returncode)

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
    windows_version_ok = version.startswith(REQUIRED_WINDOWS_VERSION + '.')
    log_test_success('for Windows {0}'.format(REQUIRED_WINDOWS_VERSION), windows_version_ok)
    return windows_version_ok    

def check_visual_studio_installed():
    """Check if Visual Studio is installed"""

    return_code = call('reg query "%s"' % VS_INSTALLED_REG_KEY, True)

    visual_studio_installed = return_code == 0
    log_test_success('for Visual Studio {0}'.format(REQUIRED_VS_VERSION), visual_studio_installed)
    return visual_studio_installed    
    
def check_visual_studio_has_update():
    """Check Visual Studio version"""

    ver_output = call('reg query %s' % VS_VERSION_REG_QUERY)
    version = ver_output.strip("'").split()[-1]
    (_, minor, patch) = version.split('.')
    version_ok = int(minor) == 0 and int(patch) >= REQUIRED_PATCH_VERSION
    log_test_success('Visual Studio Update', version_ok)
    return version_ok

def handle_missing_vs():
    """If we don't have Visual Studio, help install it"""

    # Show different help depending on whether they already have an older version installed.
    print("\nVisual Studio {0} is required. The Community Edition can be downloaded for free from https://www.visualstudio.com".format(REQUIRED_VS_VERSION))

    # Offer to open download page
    open_vs_download = read_yes_no("Open download page?")
    if open_vs_download:
        webbrowser.open('https://www.visualstudio.com')
    print("\nPlease re-run check_build_environment after you have installed Visual Studio {0}".format(REQUIRED_VS_VERSION))        

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
    nap_root = get_nap_root()
    qt_checker_path = os.path.join(nap_root, 'cmake', 'qt_checker')
    temp_build_dir = os.path.join(qt_checker_path, 'project_temp')
    if os.path.exists(temp_build_dir):
        shutil.rmtree(temp_build_dir)
    
    # Run Qt version checking logic, parsing output
    thirdparty_dir = os.path.join(nap_root, 'thirdparty')
    cmake = os.path.join(thirdparty_dir, 'cmake', 'msvc', 'x86_64', 'bin', 'cmake')
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
        print("\nThis version of NAP requires Qt, preferably v%s. Once Qt v%s has been downloaded it should be pointed to with the environment variable QT_DIR, eg. QT_DIR=\"C:\\Qt%s\\%s\\msvc2019_64\"." % (REQUIRED_QT_VERSION, REQUIRED_QT_VERSION, REQUIRED_QT_VERSION, REQUIRED_QT_VERSION))
    else:
        print("\nThis version of NAP requires Qt v%s, however you appear to have v%s. Other versions may work but are not supported." % (REQUIRED_QT_VERSION, qt_found_version))    
    
def get_nap_root():
    """Get framework root directory"""
    script_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(script_dir, "../../../.."))        

def check_build_environment(against_source):
    """Check whether Windows build environment appears ready for NAP"""

    # Check Windows version
    windows_version_ok = check_windows_version()

    # Check if Visual Studio is installed
    have_vs = check_visual_studio_installed()

    if against_source:
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
    if against_source and (not qt_env_var_ok or not qt_version_ok):
        extra_source_requirements_ok = False 

    # If everything looks good log and exit
    if windows_version_ok and have_vs and extra_source_requirements_ok:
        print("Your build environment appears to be ready for NAP!")
        return

    print("Some issues were encountered:")

    # Warn about wrong Windows version
    if not windows_version_ok:
        print("\nWarning: This version of NAP is supported on Windows {0}. Other Windows versions may work but are unsupported.".format(REQUIRED_WINDOWS_VERSION))

    # If we don't have Visual help install it
    if not have_vs:
        handle_missing_vs()

    # Show Qt help
    if against_source:
        if (not qt_env_var_ok or not qt_version_ok):
            log_qt_help(qt_env_var_ok, qt_found_version)
            print("\nRe-run check_build_environment once you have made the required changes.")
            
if __name__ == '__main__':
    against_source = len(sys.argv) > 1 and sys.argv[1] == '--source'
    check_build_environment(against_source)
    print("\nPress key to close...")
    getch()
