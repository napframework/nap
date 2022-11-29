#!/usr/bin/env python3
from distutils.version import LooseVersion
import os
import subprocess
import shutil
import sys
import termios
import tty

REQUIRED_MACOS_VERSION = '10.15'
REQUIRED_MACOS_VERSION_TITLE = 'Catalina'
REQUIRED_QT_VERSION = '5.15.2'

def getch():
    """Read a char from the console; pause"""

    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        return sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)

def wait_for_return(txt):
    """Show text and wait for using pressing return"""
    if sys.version_info >= (3, 0):    
        input(txt)
    else:
        raw_input(txt)

def call(cmd, provide_exit_code=False):
    """Execute command and return stdout"""

    # print("Command: %s" % cmd)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    if provide_exit_code:
        return proc.returncode
    else:
        if type(out) is bytes:
            out = out.decode('utf-8')
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

def check_arch():
    """Check if the we're running on a 64bit machine"""

    arch = call('getconf LONG_BIT')
    arch_ok = arch == '64'
    log_test_success('x86-64 architecture', arch_ok)
    return arch_ok

def check_macos():
    """Check if the we're running a supported macOS version"""

    macos_version = call('sw_vers -productVersion')
    macos_version_ok = macos_version.startswith(REQUIRED_MACOS_VERSION)
    log_test_success('macOS %s' % REQUIRED_MACOS_VERSION_TITLE, macos_version_ok)
    return macos_version_ok    

def check_xcode_installed():
    """Check if the Xcode is installed"""

    xcode_installed = False
    xcode_bundle_id = call('osascript -e "id of application \\"Xcode\\""')
    if xcode_bundle_id != '':
        xcode_installed_path = call('osascript -e "tell application \\"Finder\\" to POSIX path of (get application file id \\"%s\\" as alias)"' % xcode_bundle_id)
        xcode_installed = xcode_installed_path != ''
    log_test_success('Xcode installed', xcode_installed)
    return xcode_installed

def handle_missing_xcode():
    """Assist with Xcode installation"""

    print("\nXcode is not installed.")
    install_xcode = read_yes_no("Open download page?")
    if install_xcode:
        print("Opening Xcode download page...")
        call('open \'https://itunes.apple.com/us/app/xcode/id497799835?mt=12\'')
        wait_for_return("Press return when Xcode has been installed: ")
        re_run_tests = True
    else:
        print("Xcode not installed. Re-run check_build_environment.sh once Xcode is installed.")
        re_run_tests = False
    return re_run_tests

def check_xcode_command_line_tools_installed():
    """Check if the Xcode Command Line Tools are installed"""

    cli_tools_installed = False

    xcode_select_location = call('which xcode-select')
    sane_xcode_select = xcode_select_location != ''

    if sane_xcode_select:
        xcode_select_path = call('xcode-select --print-path')
        if xcode_select_path != '' and os.path.isdir(xcode_select_path):
            cli_tools_installed = True
    log_test_success('Xcode command line tools installed', cli_tools_installed)
    return cli_tools_installed

def handle_missing_xcode_command_line_tools():
    """Assist with Xcode Command Line Tools installation"""

    print("\nXcode Command Line Tools are not installed.")
    install_command_line_tools = read_yes_no("Kick off installation?")
    if install_command_line_tools:
        call('xcode-select --install')
        wait_for_return("Press return once the installation has completed: ")
        re_run_tests = True
    else:
        print("Xcode Command Line Tools not installed. Re-run check_build_environment once they have been setup.")
        re_run_tests = False
    return re_run_tests

def xcode_license_accepted():
    """Attempt to check if the Xcode license has been accepted"""

    license_ok = call('xcrun -find c++', True) == 0
    log_test_success('Xcode license accepted', license_ok)
    return license_ok

def handle_xcode_license_approval():
    """Assist with Xcode unaccepted license"""

    print("\nThe Xcode license does not appear to have been accepted. You can accept it by launching Xcode for the first time.")
    open_xcode = read_yes_no("Launch Xcode for first-time setup?")
    if open_xcode:
        subprocess.call('open -a Xcode', shell=True)
        wait_for_return("When the setup is complete Xcode and press return: ")
        re_run_tests = True
    else:
        print("The Xcode license agreement does not appear to have been accepted. Re-run check_build_environment once you have accepted it.")
        re_run_tests = False
    return re_run_tests

def check_for_thirdparty():
    """Check for the thirdparty repository"""
    
    nap_root = get_nap_root()
    thirdparty_ok = os.path.exists(os.path.join(nap_root, os.pardir, 'thirdparty'))
    log_test_success('for third party repository', thirdparty_ok)
    return thirdparty_ok

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
    thirdparty_dir = os.path.join(nap_root, os.pardir, 'thirdparty')
    cmake = os.path.join(thirdparty_dir, 'cmake', 'macos', 'x86_64', 'bin', 'cmake')
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
        print("\nThis version of NAP requires Qt v%s as downloaded directly from Qt. Packaging system versions are not supported. Once Qt v%s has been downloaded it should be pointed to with the environment variable QT_DIR, eg. QT_DIR=\"/Users/username/Qt%s/%s/clang_64/\"." % (REQUIRED_QT_VERSION, REQUIRED_QT_VERSION, REQUIRED_QT_VERSION, REQUIRED_QT_VERSION))
    else:
        print("\nThis version of NAP requires Qt v%s, however you appear to have v%s. Other versions may work but are not supported." % (REQUIRED_QT_VERSION, qt_found_version))    

def check_and_warn_for_potential_packaged_qt():
    """Attempt to detect if the Qt installation might be a packaging system version and warn if so"""
    
    packages_found = False

    have_homebrew = call('which brew') != ''
    if have_homebrew:
        (_, returncode) = call_with_returncode('brew ls --versions qt5')
        if returncode == 0:
            packages_found = True
            print("\nWarning: You appear to have Qt installed via Homebrew. This may cause issues, especially with packaging.")

    have_macports = call('which port') != ''
    if have_macports:
        if call('port installed | grep qt5') != '':
            packages_found = True
            print("\nWarning: You appear to have some Qt packages installed via MacPorts. This may cause issues, especially with packaging.")

    have_fink = call('which fink') != ''
    if have_fink:
        if call('fink list --installed | grep qt5') != '':
            packages_found = True
            print("\nWarning: You appear to have some Qt packages installed via Fink. This may cause issues, especially with packaging.")

    if not packages_found and (have_homebrew or have_macports):
        packaging_systems = []
        if have_homebrew:
            packaging_systems.append('Homebrew')
        if have_macports:
            packaging_systems.append('MacPorts')
        if have_fink:
            packaging_systems.append('Fink')
        print("\nWarning: A third party package manager (%s) was found on your system. Be mindful of interactions with this and the NAP packaging process." % " & ".join(packaging_systems))
    
def get_nap_root():
    """Get framework root directory"""
    script_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir, os.pardir))

def check_build_environment(against_source):
    """Check whether macOS build environment appears ready for NAP"""

    # Check if 64bit
    arch_ok = check_arch()
    if not arch_ok:
        print("\nNAP only supports x86-64 systems. Not continuing checks.")
        sys.exit(1)

    # Check macOS version
    macos_version_ok = check_macos()

    # Check if Xcode installed (without triggering installation)
    xcode_installed = check_xcode_installed()

    # Check if Xcode command line tools are installed
    xcode_cli_tools_installed = check_xcode_command_line_tools_installed()

    # Check if the Xcode license has been accepted
    xcode_license_ok = xcode_installed and xcode_cli_tools_installed and xcode_license_accepted()

    if against_source:
        # Check for thirdparty repo
        thirdparty_ok = check_for_thirdparty()

        if not thirdparty_ok:
            print("\nThe third party repository ('thirdparty') needs to be cloned alongside the main repository.")
            print("\nNot continuing checks. Re-run this script after cloning.")
            sys.exit(1)

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
    if against_source and (not thirdparty_ok or not qt_env_var_ok or not qt_version_ok):
        extra_source_requirements_ok = False 

    # If everything looks good log and exit
    if macos_version_ok \
      and xcode_installed \
      and xcode_cli_tools_installed \
      and extra_source_requirements_ok:
        print("Your build environment appears to be ready for NAP!")

        if against_source:
            check_and_warn_for_potential_packaged_qt()

        return False

    print("Some issues were encountered:")

    # Warn about wrong macOS version
    if not macos_version_ok:
        print("\nWarning: This version of NAP is supported on macOS %s (%s). Other macOS versions may work but are unsupported." 
              % (REQUIRED_MACOS_VERSION_TITLE, REQUIRED_MACOS_VERSION))

    # If Xcode isn't installed assist in installation
    if not xcode_installed:
        return handle_missing_xcode()

    # If we don't have Xcode Command Line Tools help install it
    if not xcode_cli_tools_installed:
        return handle_missing_xcode_command_line_tools()

    # If they don't seem to have accepted the Xcode license agreement offer to show it
    if not xcode_license_ok:
        return handle_xcode_license_approval()

    # Show Qt help
    if against_source:
        if (not qt_env_var_ok or not qt_version_ok):
            log_qt_help(qt_env_var_ok, qt_found_version)
            
        if qt_env_var_ok:
            check_and_warn_for_potential_packaged_qt()

        if (not qt_env_var_ok or not qt_version_ok):
            print("\nRe-run check_build_environment once you have made the required changes.")
            return False

    # We might fall through here if the only issue they have is their macOS version
    return False

if __name__ == '__main__':
    against_source = len(sys.argv) > 1 and sys.argv[1] == '--source'

    # Re-run our test sequence as need be as dependencies are installed
    while check_build_environment(against_source):
        print("Re-running checks")

    print("\nPress key to close...")
    getch()
