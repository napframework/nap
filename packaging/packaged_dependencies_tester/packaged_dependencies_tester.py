#!/usr/bin/env python

"""This script does some crude dependencies testing on a NAP framework release. See the README."""

import argparse
import copy
import datetime
import json
from multiprocessing import cpu_count
import os
import re
from subprocess import call, Popen, PIPE
import shutil
import signal
import sys
import time

# How long to wait for the process to run. This should be long enough that we're sure
# it will have completed initialisation.
WAIT_SECONDS_FOR_PROCESS_HEALTH = 3

# Name for project created from template
TEMPLATE_APP_NAME = 'TemplateProject'

# Build type for build projects
PROJECT_BUILD_TYPE = 'Release'

# Directory to iterate for testing
DEFAULT_TESTING_PROJECTS_DIR = 'demos'

# JSON report filename
REPORT_FILENAME = 'report.json'

# List of locations on a Ubuntu system where we're happy to find system libraries. Restricting
# to these paths helps us identify libraries being source from strange locations, hand installed libs.
# TODO Handle other architectures.. eventually
LINUX_ACCEPTED_SYSTEM_LIB_PATHS = ['/usr/lib/x86_64-linux-gnu/', '/lib/x86_64-linux-gnu']

# List of libraries we accept being sourced from the system paths defined above. Notes:
# - These currently support Ubuntu 18.04/18.10 and are likely to require minor tweaks for new versions
# - If/when we support more distros and architectures we should either break these lists out by 
#   architecture and distro+version or restrict dependencies testing to one distro
# - Developed and tested against Nvidia open source and proprietary drivers plus Intel i965, other
#   hardware may require additions
# - Developed against x.org, Wayland will need additions
# - Regular expressions supported
# - This is somewhat a proof of concept and is by nature fairly brittle. Let's see how it goes.
LINUX_BASE_ACCEPTED_SYSTEM_LIBS = [
    'i965_dri',
    r'ld-[0-9]+\.[0-9]+',
    'libasound',
    'libasound_module_pcm_pulse',
    'libasyncns',
    'libbsd',
    r'libc-[0-9]+\.[0-9]+',
    'libdbus-1',
    r'libdl-[0-9]+\.[0-9]+',
    'libdrm',
    'libdrm_amdgpu',
    'libdrm_intel',
    'libdrm_nouveau',
    'libdrm_radeon',
    'libedit',
    r'libelf-[0-9]+\.[0-9]+',
    'libexpat',
    'libffi',
    'libFLAC',
    'libgcc_s',
    'libgcrypt',
    'libGL',
    'libglapi',
    'libGLX',
    'libGLX_mesa',
    'libGLX_nvidia',
    'libGLdispatch',
    'libgpg-error',
    'libjack',
    r'libLLVM-[0-9]+',
    'liblz4',
    'liblzma',
    r'libm-[0-9]+\.[0-9]+',
    r'libnsl-[0-9]+\.[0-9]+',
    r'libnss_compat-[0-9]+\.[0-9]+',
    r'libnss_files-[0-9]+\.[0-9]+',
    r'libnss_nis-[0-9]+\.[0-9]+',
    'nouveau_dri',
    'libnvidia-glcore',
    'libnvidia-tls',
    'libogg',
    'libpciaccess',
    r'libpthread-[0-9]+\.[0-9]+',
    'libpulse',
    r'libpulsecommon-[0-9]+\.[0-9]+',
    r'libpython[0-9]+\.[0-9]+m',
    r'libresolv-[0-9]+\.[0-9]+',
    r'librt-[0-9]+\.[0-9]+',
    'libsndfile',
    'libsensors',
    r'libstdc\+\+',
    'libsystemd',
    'libtinfo',
    r'libutil-[0-9]+\.[0-9]+',
    'libvorbis',
    'libvorbisenc',
    'libwrap',
    'libX11',
    'libX11-xcb',
    'libXau',
    'libxcb',
    'libxcb-dri2',
    'libxcb-dri3',
    'libxcb-glx',
    'libxcb-present',
    'libxcb-sync',
    'libXdamage',
    'libXdmcp',
    'libXext',
    'libXfixes',
    'libxshmfence',
    'libXxf86vm',
    'libz'
]

# Extra Linux system libs we accept being used, for Napkin only
LINUX_NAPKIN_ACCEPTED_SYSTEM_LIBS = [
    'libfontconfig',
    'libfreetype',
    r'libglib-[0-9]+\.[0-9]+',
    r'libgthread-[0-9]+\.[0-9]+',
    'libpcre',
    'libpng16',
    'libudev',
    r'libusb-[0-9]+\.[0-9]+',
    'libuuid',
    'libX11-xcb',
    'libXcursor',
    'libXfixes',
    'libXi',
    'libXrender'
]

# List of locations on a macOS system where we're happy to find system libraries. As with Ubuntu, 
# above, restricting to these paths helps us identify libraries being source from strange 
# locations. However on macOS, unlike on Linux, we trust any libraries we find within these folders.
# The logic here is that on macOS you're typically going to install custom locations via Brew or
# MacPorts etc and are less likely to end up within these system paths. If that logic doesn't hold
# up we'll need to define a list like above for Linux, or use another approach.
MACOS_ACCEPTED_SYSTEM_LIB_PATHS = ['/usr/lib/', 
                                   '/System/Library/Frameworks/', 
                                   '/System/Library/PrivateFrameworks/', 
                                   '/System/Library/Extensions/', 
                                   '/System/Library/Components/'
                                   ]

# Quicker iteration when debugging this script
SCRIPT_DEBUG_ONE_PROJECT_ONLY = False

def call_capturing_output(cmd, shell=True):
    """Run specified command, capturing output

    Parameters
    ----------
    cmd : str
        Command to run
    shell : bool
        Whether to run in a shell

    Returns
    -------
    returncode : int
        Return code from process
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """

    p = Popen(cmd, shell=shell, stdout=PIPE, stderr=PIPE)
    (stdout, stderr) = p.communicate()
    if type(stdout) == bytes:
        stdout = stdout.decode('utf8')
        stderr = stderr.decode('utf8')

    return (p.returncode, stdout, stderr)

def get_packaged_project_output_path(project_name, pre_files, post_files):
    """Determine the name of the directory containing the newly packaged project

    Parameters
    ----------
    project_name : str
        Name of project
    pre_files : list of str
        List of directory contents before the packaging ran
    pre_files : list of str
        List of directory contents after the packaging ran

    Returns
    -------
    str or None
        Output path
    """

    new_files = list(set(post_files) - set(pre_files))
    for f in new_files:
        if f.lower().startswith(project_name):
            return f

    print("Error: get_packaged_project_output_path() sees no difference")
    return None

def is_windows():
    """Is this Windows?

    Returns
    -------
    bool
        Success
    """

    return sys.platform.startswith('win')

def run_process_then_stop(cmd, accepted_shared_libs_path=None, testing_napkin=False, wait_time_seconds=WAIT_SECONDS_FOR_PROCESS_HEALTH):
    """Run specified command and after the specified number of seconds check that the process is
       still running before closing it

    Parameters
    ----------
    cmd : str
        Command to run
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from. 
        Typically NAP framework for build programs running from framework, or the packaged app for 
        single apps.
    testing_napkin : bool
        Whether testing Napkin
    wait_time_seconds : str
        Number of seconds to wait before checking the process is still running and terminating it

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """

    # Launch the app
    my_env = os.environ.copy()
    # For shared libraries tracking on macOS
    if sys.platform == 'darwin':
        my_env['DYLD_PRINT_LIBRARIES'] = '1'
    p = Popen(cmd, stdout=PIPE, stderr=PIPE, env=my_env)

    # Wait for the app to initialise
    time.sleep(wait_time_seconds)

    if sys.platform.startswith('linux'):
        unexpected_libraries = linux_check_for_unexpected_library_use(p.pid, accepted_shared_libs_path, testing_napkin)

    # Check and make sure the app's still running
    p.poll()
    if p.returncode != None:
        print("  Error: Process already done?")
        (stdout, stderr) = p.communicate()
        if type(stdout) == bytes:
            stdout = stdout.decode('utf8')
            stderr = stderr.decode('utf8')
            
        if sys.platform == 'darwin':
            unexpected_libraries = macos_check_for_unexpected_library_use(stderr, accepted_shared_libs_path, testing_napkin)
        elif sys.platform == 'win32':
            unexpected_libraries = []            
        return (False, stdout, stderr, unexpected_libraries)

    # Send SIGTERM and wait a moment to close
    p.terminate()
    time.sleep(1)
    p.poll()

    # If the app hasn't exited, brute close with kill signal
    while p.returncode is None:
        time.sleep(1)
        p.poll()
        print("  Failed to close on terminate, sending kill signal")
        try:
            p.kill()
        except OSError:
            pass

    # Pull the output and return success
    (stdout, stderr) = p.communicate()
    if type(stdout) == bytes:
        stdout = stdout.decode('utf8')
        stderr = stderr.decode('utf8')    

    if sys.platform == 'darwin':
        unexpected_libraries = macos_check_for_unexpected_library_use(stderr, accepted_shared_libs_path, testing_napkin)
    elif sys.platform == 'win32':
        unexpected_libraries = []

    return (True, stdout, stderr, unexpected_libraries)

def linux_check_for_unexpected_library_use(pid, accepted_shared_libs_path, testing_napkin):
    """Check whether the specified NAP process is using unexpected libraries on Linux

    Parameters
    ----------
    pid : str
        Command to run
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from. 
        Typically NAP framework for build programs running from framework, or the packaged app for 
        single apps.
    testing_napkin : bool
        Whether testing Napkin

    Returns
    -------
    list
        List of paths to unexpected libraries used
    """

    # Launch the app
    p = Popen(['/usr/bin/lsof', '-X', '-p', str(pid)], stdout=PIPE, stderr=PIPE)

    # Check and make sure the app's still running
    (stdout, stderr) = p.communicate()
    if type(stdout) == bytes:
        stdout = stdout.decode('utf8')
        stderr = stderr.decode('utf8')

    unexpected_libs = []
    for line in stdout.split('\n'):
        chunks = line.split()
        if len(chunks) < 5:
            continue
        if chunks[3] == 'mem' and chunks[4] == 'REG':
            path = ' '.join(chunks[8:])
            if linux_file_is_shared_lib(path):
                if not shared_lib_accepted(path, accepted_shared_libs_path, testing_napkin):
                    unexpected_libs.append(path)

    return unexpected_libs

def macos_check_for_unexpected_library_use(stdout, accepted_shared_libs_path, testing_napkin):
    """Check whether the a NAP process has used unexpected libraries on macOS

    Parameters
    ----------
    stdout : str
        STDOUT from the process
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from. 
        Typically NAP framework for build programs running from framework, or the packaged app for 
        single apps.
    testing_napkin : bool
        Whether testing Napkin

    Returns
    -------
    list
        List of paths to unexpected libraries used
    """

    unexpected_libs = []
    # Iterate STDOUT lines for shared libraries loaded and logged via DYLD_PRINT_LIBRARIES env. var
    for line in stdout.split('\n'):
        if line.startswith('dyld: loaded:'):
            # Parse library path
            lib = ':'.join(line.split(':')[2:]).strip()

            # Get absolute path
            libs_abs_path = os.path.abspath(lib)

            # Check if library is accepted
            if not shared_lib_accepted(libs_abs_path, accepted_shared_libs_path, testing_napkin):
                unexpected_libs.append(libs_abs_path) 

    return unexpected_libs

def linux_file_is_shared_lib(file_path):
    """Check whether a file is a shared library on Linux

    Parameters
    ----------
    file_path : str
        Path to file

    Returns
    -------
    bool
        Whether a shared library
    """

    # Use file util to check file type
    p = Popen(['/usr/bin/file', file_path], stdout=PIPE, stderr=PIPE)

    # Get output
    (stdout, stderr) = p.communicate()
    if type(stdout) == bytes:
        stdout = stdout.decode('utf8')
        stderr = stderr.decode('utf8')

    # Check if ELF binary
    chunks = stdout.split(':')[1].split(',')
    return chunks[0].strip().startswith('ELF ')

def shared_lib_accepted(file_path, accepted_shared_libs_path, testing_napkin):
    """Check whether a shared library is expected for the executed NAP process

    Parameters
    ----------
    file_path : str
        Path to the shared library
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from. 
        Typically NAP framework for build programs running from framework, or the packaged app for 
        single apps.
    testing_napkin : bool
        Whether testing Napkin

    Returns
    -------
    bool
        Whether the library is accepted
    """

    # If we're running something inside the framework path and it's using libs from within there 
    # we're happy with that. Same goes for a packaged app and the app path.
    if not accepted_shared_libs_path is None:
        if file_path.startswith(accepted_shared_libs_path):
            return True

    filename = os.path.basename(file_path)
    in_system_path = False
    accepted = False    

    # Check if it's within the system libs paths
    if sys.platform.startswith('linux'):
        for system_path in LINUX_ACCEPTED_SYSTEM_LIB_PATHS:
            if file_path.startswith(system_path):
                in_system_path = True

                if not ".so" in filename:
                    print("Error: Unhandled Linux library due to lacking .so: %s" % path)
                    return False

                # Get short library name used for verification
                filename_parts = filename.split(".so")
                short_lib_name = filename_parts[0]

                # Verify against system library list
                accepted = linux_system_library_accepted(short_lib_name, testing_napkin)
                break
    else:
        for system_path in MACOS_ACCEPTED_SYSTEM_LIB_PATHS:
            if file_path.startswith(system_path):
                # on macOS if it's within one of our system paths accept the library. See notes 
                # above with MACOS_ACCEPTED_SYSTEM_LIB_PATHS definition.
                in_system_path = True
                accepted = True
                break

    if in_system_path:
        if accepted:
            return True
        else:
            print("Error: unexpected system library encountered: %s" % file_path)
            return False
    else:
        print("Error: library found outside of system path: %s" % file_path)
        return False

def linux_system_library_accepted(short_lib_name, testing_napkin):
    """Check whether a single shared system library is expected for the executed NAP process on 
       Linux

    Parameters
    ----------
    short_lib_name : str
        The library name (with .so and everything after stripped)
    testing_napkin : bool
        Whether testing Napkin

    Returns
    -------
    bool
        Whether the library is accepted
    """

    # Build list of accepted library names
    all_accepted_libs = LINUX_BASE_ACCEPTED_SYSTEM_LIBS
    if testing_napkin:
        all_accepted_libs.extend(LINUX_NAPKIN_ACCEPTED_SYSTEM_LIBS)
    
    # Verify the library name against the list
    for check_lib in all_accepted_libs:
        expr = "^" + check_lib + "$"
        if re.match(expr, short_lib_name):
            return True

    return False

def regenerate_cwd_project():
    """Configure project in current working directory

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """

    print("- Regenerating...")

    # Build command
    if sys.platform.startswith('linux'):
        cmd = './regenerate %s' % PROJECT_BUILD_TYPE
    else:
        cmd = '%s -ns -np' % os.path.join('.', 'regenerate')

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    if success:
        print("  Done.")
    else:
        print("  Error: Couldn't regenerate, return code: %s" % returncode)
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)
    
    return (success, stdout, stderr)

def build_cwd_project(project_name):
    """Build project in current working directory, excluding Napkin

    Parameters
    ----------
    project_name : str
        Name of project

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """

    print("- Building...")

    # Build build command
    if sys.platform.startswith('darwin'):
        os.chdir('xcode')
        cmd = 'xcodebuild -configuration %s -jobs %s' % (PROJECT_BUILD_TYPE, cpu_count())
        
    elif sys.platform.startswith('linux'):
        os.chdir('build')
        cmd = 'make all . -j%s' % cpu_count()
    else:
        os.chdir('msvc64')
        cmd = 'cmake --build . --target %s --config %s' % (project_name, PROJECT_BUILD_TYPE)

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    if success:
        print("  Done.")              
    else:
        print("  Error: Couldn't build, return code: %s" % returncode)
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)

    os.chdir(os.path.pardir)

    return (success, stdout, stderr)

def package_cwd_project_without_napkin(project_name, root_output_dir, timestamp):
    """Package project in current working directory, excluding Napkin

    Parameters
    ----------
    project_name : str
        Name of project
    root_output_dir : str
        Directory where packaged projects will be moved to
    timestamp : str
        Timestamp of the test run

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """

    print("- Packaging (without Napkin)...")
    pre_files = os.listdir('.')

    # Build command
    cmd = '%s -nz -nn -ns' % os.path.join('.', 'package')
    if not sys.platform.startswith('linux'):
        cmd = '%s -np' % cmd

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    if success:
        # Move package to starting directory
        post_files = os.listdir('.')
        output_path = get_packaged_project_output_path(project_name, pre_files, post_files)
        home_output = os.path.join(root_output_dir, '%s-%s-no_napkin' % (project_name, timestamp))
        os.rename(output_path, home_output)
        print("  Done. Moving to %s." % home_output)
    else:
        print("  Error: Couldn't package project, return code: %s" % returncode)
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)

    return (success, stdout, stderr)

def run_cwd_project(project_name, nap_framework_full_path):
    """Run project from normal build output

    Parameters
    ----------
    project_name : str
        Name of project
    nap_framework_full_path : str
        Absolute path to NAP framework (or None if not relevant for test)

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """
    
    print("- Run from build output...")

    # Find build output path
    build_paths = os.listdir('bin')
    for f in build_paths:
        if PROJECT_BUILD_TYPE.lower() in f.lower():
            build_path = f

    # Build command and run            
    cmd = os.path.abspath(os.path.join(os.getcwd(), 'bin', build_path, project_name))
    (success, stdout, stderr, unexpected_libs) = run_process_then_stop(cmd, nap_framework_full_path)
    if success:
        print("  Done.")
    else:
        print("  Error: Couldn't run from build dir")
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)
        print("  Unexpected libraries: %s" % repr(unexpected_libs))

    return (success, stdout, stderr, unexpected_libs)

def run_packaged_project(root_output_dir, timestamp, project_name):
    """Run packaged project from output directory

    Parameters
    ----------
    root_output_dir : str
        Directory where packaged projects will be moved to
    timestamp : str
        Timestamp of the test run
    project_name : str
        Name of project

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """

    containing_dir = os.path.abspath(os.path.join(root_output_dir, '%s-%s-no_napkin' % (project_name, timestamp)))
    os.chdir(containing_dir)
    print("- Run from package...")

    # Run
    (success, stdout, stderr, unexpected_libs) = run_process_then_stop('./%s' % project_name, containing_dir)
    if success:
        print("  Done.")
    else:
        print("  Error: Couldn't run from package")
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)
        print("  Unexpected libraries: %s" % repr(unexpected_libs))

    return (success, stdout, stderr, unexpected_libs)

def build_and_package(root_output_dir, timestamp, testing_projects_dir):
    """Configure, build and package all demos

    Parameters
    ----------
    root_output_dir : str
        Directory where packaged projects will be moved to
    timestamp : str
        Timestamp of the test run
    testing_projects_dir : str
        Directory to iterate for testing, by default 'demos'        
        
    Returns
    -------
    dict
        Results
    """

    demos_root_dir = os.getcwd()

    demo_results = {}

    # Iterate demos
    sorted_dirs = os.listdir('.')
    sorted_dirs.sort()
    for demo_name in sorted_dirs:
        demo_path = os.path.join(demos_root_dir, demo_name)
        # Check if path looks sane
        if not os.path.isdir(demo_path) or demo_name.startswith('.'):
            continue

        # If we're iterating our projects directory and we've already got our template project in there
        # from a previous run, don't built it in here as we'll build it separately later
        if demo_name == TEMPLATE_APP_NAME.lower() and testing_projects_dir != DEFAULT_TESTING_PROJECTS_DIR:
            continue

        print("----------------------------")
        print("Demo: %s" % demo_name)
        os.chdir(demo_path)
        demo_results[demo_name] = {}

        # Configure
        (success, stdout, stderr) = regenerate_cwd_project()
        demo_results[demo_name]['generate'] = {}
        demo_results[demo_name]['generate']['success'] = success
        demo_results[demo_name]['generate']['stdout'] = stdout
        demo_results[demo_name]['generate']['stderr'] = stderr
        if not success:
            # Couldn't configure, skip to next demo
            continue

        # Build
        (success, stdout, stderr) = build_cwd_project(demo_name)
        demo_results[demo_name]['build'] = {}
        demo_results[demo_name]['build']['success'] = success
        demo_results[demo_name]['build']['stdout'] = stdout
        demo_results[demo_name]['build']['stderr'] = stderr
        if not success:
            # Couldn't build, skip to next demo
            continue

        # Package
        (success, stdout, stderr) = package_cwd_project_without_napkin(demo_name, root_output_dir, timestamp)
        demo_results[demo_name]['package'] = {}
        demo_results[demo_name]['package']['success'] = success
        demo_results[demo_name]['package']['stdout'] = stdout
        demo_results[demo_name]['package']['stderr'] = stderr

        if SCRIPT_DEBUG_ONE_PROJECT_ONLY:
            break

    os.chdir(os.path.pardir)
    return demo_results

def package_demo_with_napkin(demo_results, root_output_dir, timestamp):
    """Package project in current working directory, including Napkin

    Parameters
    ----------
    demo_results : dict
        Results from demo packaging, assists with identifying healthy project to package Napkin with
    root_output_dir : str
        Directory where packaged projects will be moved to
    timestamp : str
        Timestamp of the test run

    Returns
    -------
    dict
        Results
    """

    napkin_results = {}
    napkin_package_demo = None

    # Iterate demos to find a healthy one to use
    for demo_name, this_demo in sorted(demo_results.items()):
        if 'package' in this_demo and this_demo['package']['success']:
            napkin_package_demo = demo_name
            break

    # Fail if there's no healthy demo
    if napkin_package_demo is None:
        print("Error: no demo found to package Napkin with")
        return {}

    print("Demo: %s" % napkin_package_demo)
    print("- Packaging (with Napkin)...")
    napkin_results['demoPackagedWith'] = napkin_package_demo
    os.chdir(napkin_package_demo)
    pre_files = os.listdir('.')
    
    # Build command
    cmd = '%s -nz -ns' % os.path.join('.', 'package')
    if not sys.platform.startswith('linux'):
        cmd = '%s -np' % cmd

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    napkin_results['packageWithDemo'] = {}
    napkin_results['packageWithDemo']['success'] = success
    napkin_results['packageWithDemo']['stdout'] = stdout
    napkin_results['packageWithDemo']['stderr'] = stderr
    if success:
        # Move package to starting directory
        post_files = os.listdir('.')
        output_path = get_packaged_project_output_path(napkin_package_demo, pre_files, post_files)
        home_output = os.path.join(root_output_dir, '%s-%s-napkin' % (napkin_package_demo, timestamp))
        print("  Done. Moving to %s." % home_output)
        os.rename(output_path, home_output)
    else:
        print("  Error: Couldn't package with Napkin, return code: %s" % returncode)
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)

    os.chdir(os.path.pardir)
    return napkin_results

def create_build_and_package_template_app(root_output_dir, timestamp):
    """Create, configure, build and package project from template

    Parameters
    ----------
    root_output_dir : str
        Directory where packaged projects will be moved to
    timestamp : str
        Timestamp of the test run

    Returns
    -------
    dict
        Results
    """

    prev_cwd = os.getcwd()

    print("Template project")

    # Create dict for results
    template_results = {}

    # Remove any existing project
    project_path = os.path.join('projects', TEMPLATE_APP_NAME.lower())
    if os.path.exists(project_path):
        print("- Pre-existing template project found, removing")
        shutil.rmtree(project_path)

    # Create from template
    print("- Create project from template...")
    pre_files = os.listdir('.')
    cmd = '%s -ng %s' % (os.path.join('.', 'tools', 'create_project'), TEMPLATE_APP_NAME)
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    template_creation_success = returncode == 0
    template_results['create'] = {}
    template_results['create']['success'] = template_creation_success
    template_results['create']['stdout'] = stdout
    template_results['create']['stderr'] = stderr

    if template_creation_success:
        # Creation successful, now configure
        print("  Done.")
        os.chdir(os.path.join('projects', TEMPLATE_APP_NAME.lower()))
        (template_generate_success, stdout, stderr) = regenerate_cwd_project()
        template_results['generate'] = {}
        template_results['generate']['success'] = template_generate_success
        template_results['generate']['stdout'] = stdout
        template_results['generate']['stderr'] = stderr

        if template_generate_success:
            # Configure successful, now build
            (template_build_success, stdout, stderr) = build_cwd_project(TEMPLATE_APP_NAME.lower())
            template_results['build'] = {}
            template_results['build']['success'] = template_build_success
            template_results['build']['stdout'] = stdout
            template_results['build']['stderr'] = stderr

            if template_build_success:
                # Build successful, now package
                (template_package_success, stdout, stderr) = package_cwd_project_without_napkin(TEMPLATE_APP_NAME.lower(), root_output_dir, timestamp)
                template_results['package'] = {}
                template_results['package']['success'] = template_package_success
                template_results['package']['stdout'] = stdout
                template_results['package']['stderr'] = stderr
    else:
        # Couldn't create, print logs
        print("  Error: Couldn't create project from template, return code: %s" % returncode)
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)

    os.chdir(prev_cwd)
    return template_results

def run_build_directory_demos(demo_results):
    """Run demos from normal build output

    Parameters
    ----------
    demo_results : dict
        Results from demo building
    """

    demos_root_dir = os.getcwd()

    for demo_name, this_demo in sorted(demo_results.items()):
        print("Demo: %s" % demo_name)
        os.chdir(os.path.join(demos_root_dir, demo_name))

        # If demo didn't build skip it
        if not 'build' in this_demo or not this_demo['build']['success']:
            continue

        # Run
        (success, stdout, stderr, unexpected_libs) = run_cwd_project(demo_name, os.path.abspath(os.path.join(demos_root_dir, os.pardir)))
        this_demo['runFromBuildOutput'] = {}
        this_demo['runFromBuildOutput']['success'] = success
        this_demo['runFromBuildOutput']['stdout'] = stdout
        this_demo['runFromBuildOutput']['stderr'] = stderr
        this_demo['runFromBuildOutput']['unexpectedLibraries'] = unexpected_libs

        print("----------------------------")

    os.chdir(demos_root_dir)

def run_build_directory_template_project(template_results, nap_framework_full_path):
    """Run template project from the normal build output

    Parameters
    ----------
    template_results : dict
        Results for template project
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    projects_dir = os.getcwd()

    # Change to template project directory
    os.chdir(TEMPLATE_APP_NAME.lower())

    # Run
    (success, stdout, stderr, unexpected_libs) = run_cwd_project(TEMPLATE_APP_NAME.lower(), nap_framework_full_path)
    template_results['runFromBuildOutput'] = {}
    template_results['runFromBuildOutput']['success'] = success
    template_results['runFromBuildOutput']['stdout'] = stdout
    template_results['runFromBuildOutput']['stderr'] = stderr
    template_results['runFromBuildOutput']['unexpectedLibraries'] = unexpected_libs

    os.chdir(projects_dir)

def run_build_directory_napkin(demo_results, napkin_results, nap_framework_full_path):
    """Run Napkin from the normal build output

    Parameters
    ----------
    demo_results : dict
        Results for demos
    napkin_results : dict
        Results for Napkin
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    napkin_results['runFromBuildOutput'] = {}    

    # Iterate demos to find a healthy one to test from
    napkin_run_demo = None
    for demo_name, this_demo in sorted(demo_results.items()):
        if 'build' in this_demo and this_demo['build']['success']:
            napkin_run_demo = demo_name
            break

    # Fail if there's no healthy demo to run against
    if napkin_run_demo is None:
        print("Error: no demo found to run Napkin from")
        napkin_results['runFromBuildOutput']['success'] = False
        return

    os.chdir(napkin_run_demo)

    print("- Run Napkin from build output...")
    cwd = os.getcwd()

    # Locate the directory for the build output
    build_paths = os.listdir('bin')
    for f in build_paths:
        if PROJECT_BUILD_TYPE.lower() in f.lower():
            build_path = f

    # Change directory and run
    os.chdir(os.path.join('bin', build_path))
    (success, stdout, stderr, unexpected_libs) = run_process_then_stop('./napkin', nap_framework_full_path, True)
    napkin_results['runFromBuildOutput']['success'] = success
    napkin_results['runFromBuildOutput']['stdout'] = stdout
    napkin_results['runFromBuildOutput']['stderr'] = stderr
    napkin_results['runFromBuildOutput']['unexpectedLibraries'] = unexpected_libs

    if success:
        print("  Done.")
    else:
        print("  Error: Running Napkin from build directory failed")
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)
        print("  Unexpected libraries: %s" % repr(unexpected_libs))        

    os.chdir(os.path.join(cwd, os.path.pardir))
    print("----------------------------")

def run_napkin_from_packaged_demo(napkin_results, root_output_dir, timestamp):
    """Run Napkin from packaged output

    Parameters
    ----------
    napkin_results : dict
        Results for Napkin
    root_output_dir : str
        Directory where packaged projects were moved to
    timestamp : str
        Timestamp of the test run     
    """

    cwd = os.getcwd()

    # Get the name of the demo that Napkin was packaged with
    demo_name = napkin_results['demoPackagedWith']
    containing_dir = os.path.abspath(os.path.join(root_output_dir, '%s-%s-napkin' % (demo_name, timestamp)))
    os.chdir(containing_dir)

    # Run demo from packaged project
    print("- Run Napkin from package...")
    (success, stdout, stderr, unexpected_libs) = run_process_then_stop('./napkin', containing_dir, True)

    napkin_results['runFromPackagedOutput'] = {}
    napkin_results['runFromPackagedOutput']['success'] = success
    napkin_results['runFromPackagedOutput']['stdout'] = stdout
    napkin_results['runFromPackagedOutput']['stderr'] = stderr
    napkin_results['runFromPackagedOutput']['unexpectedLibraries'] = unexpected_libs

    os.chdir(cwd)
    if success:
        print("  Done.")
    else:
        print("  Error: Napkin from package failed to run")
        print("  STDOUT: %s" % stdout)
        print("  STDERR: %s" % stderr)
        print("  Unexpected libraries: %s" % repr(unexpected_libs))        

def run_packaged_demos(demo_results, root_output_dir, timestamp):
    """Run demos from packaged output

    Parameters
    ----------
    demo_results : dict
        Results for demos
    root_output_dir : str
        Directory where packaged projects were moved to
    timestamp : str
        Timestamp of the test run     
    """

    prev_cwd = os.getcwd()

    # Iterate demos
    for demo_name, this_demo in sorted(demo_results.items()):
        # Only run if it was packaged successfully
        if 'package' in this_demo and this_demo['package']['success']:
            print("Demo: %s" % demo_name)

            (success, stdout, stderr, unexpected_libs) = run_packaged_project(root_output_dir, timestamp, demo_name)
            this_demo['runFromPackagedOutput'] = {}
            this_demo['runFromPackagedOutput']['success'] = success
            this_demo['runFromPackagedOutput']['stdout'] = stdout
            this_demo['runFromPackagedOutput']['stderr'] = stderr
            this_demo['runFromPackagedOutput']['unexpectedLibraries'] = unexpected_libs

            print("----------------------------")        

    os.chdir(prev_cwd)

def run_packaged_template_project(template_results, root_output_dir, timestamp):
    """Run template project from packaged output

    Parameters
    ----------
    template_results : dict
        Results for template project
    root_output_dir : str
        Directory where packaged projects were moved to
    timestamp : str
        Timestamp of the test run     
    """

    (success, stdout, stderr, unexpected_libs) = run_packaged_project(root_output_dir, timestamp, TEMPLATE_APP_NAME.lower())
    template_results['runFromPackagedOutput'] = {}
    template_results['runFromPackagedOutput']['success'] = success
    template_results['runFromPackagedOutput']['stdout'] = stdout
    template_results['runFromPackagedOutput']['stderr'] = stderr
    template_results['runFromPackagedOutput']['unexpectedLibraries'] = unexpected_libs

def cleanup_packaged_apps(demo_results, template_results, napkin_results, root_output_dir, timestamp, warnings):
    """Delete packaged projects created during testing

    Parameters
    ----------
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template project
    napkin_results : dict
        Results for Napkin
    root_output_dir : str
        Directory where packaged projects were moved to
    timestamp : str
        Timestamp of the test run     
    warnings : list of str
        Any warnings generated throughout the testing
    """

    # Remove bulk of packaged apps
    for demo_name, this_demo in demo_results.items():
        if 'package' in this_demo and this_demo['package']['success']:
            containing_dir = os.path.join(root_output_dir, '%s-%s-no_napkin' % (demo_name, timestamp))
            try:
                shutil.rmtree(containing_dir)
            except OSError:
                warning = "Couldn't remove packaged project at %s during cleanup" % containing_dir
                print("  Warning: %s" % warning)
                warnings.append(warning)

    # Remove packaged app with Napkin
    if 'packageWithDemo' in napkin_results and napkin_results['packageWithDemo']['success']:
        containing_dir = os.path.join(root_output_dir, '%s-%s-napkin' % (napkin_results['demoPackagedWith'], timestamp))
        try:
            shutil.rmtree(containing_dir)
        except OSError:
            warning = "Couldn't remove packaged project at %s during cleanup" % containing_dir
            print("  Warning: %s" % warning)       
            warnings.append(warning)

    # Remove packaged template project
    if 'package' in template_results and template_results['package']['success']:
        containing_dir = os.path.join(root_output_dir, '%s-%s-no_napkin' % (TEMPLATE_APP_NAME.lower(), timestamp))

        try:
            shutil.rmtree(containing_dir)
        except OSError:
            warning = "Couldn't remove packaged project at %s during cleanup" % containing_dir
            print("  Warning: %s" % warning)       
            warnings.append(warning)

def determine_run_success(demo_results, template_results, napkin_results):
    """Was the whole suite successful?

    Parameters
    ----------
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template project
    napkin_results : dict
        Results for Napkin

    Returns
    -------
    bool
        Success of entire run
    """

    # Check demos for failure
    for demo_name, this_demo in sorted(demo_results.items()):
        if not 'generate' in this_demo or not this_demo['generate']['success']:
            return False
        if not 'build' in this_demo or not this_demo['build']['success']:
            return False
        if not 'package' in this_demo or not this_demo['package']['success']:
            return False
        if not 'runFromBuildOutput' in this_demo:
            return False
        elif not this_demo['runFromBuildOutput']['success'] or len(this_demo['runFromBuildOutput']['unexpectedLibraries']) > 0:
            return False
        if not 'runFromPackagedOutput' in this_demo:
            return False
        elif not this_demo['runFromPackagedOutput']['success'] or len(this_demo['runFromPackagedOutput']['unexpectedLibraries']) > 0:
            return False

    # Check template project for failure
    if not 'create' in template_results or not template_results['create']['success']:
        return False
    if not 'generate' in template_results or not template_results['generate']['success']:
        return False
    if not 'build' in template_results or not template_results['build']['success']:
        return False
    if not 'package' in template_results or not template_results['package']['success']:
        return False
    if not 'runFromBuildOutput' in template_results:
        return False
    elif not template_results['runFromBuildOutput']['success'] or len(template_results['runFromBuildOutput']['unexpectedLibraries']) > 0:
        return False
    if not 'runFromPackagedOutput' in template_results:
        return False
    elif not template_results['runFromPackagedOutput']['success'] or len(template_results['runFromPackagedOutput']['unexpectedLibraries']) > 0:
        return False

    # Check Napkin results for failure
    if not 'packageWithDemo' in napkin_results or not napkin_results['packageWithDemo']['success']:
        return False
    if not 'runFromBuildOutput' in napkin_results:
        return False
    elif not napkin_results['runFromBuildOutput']['success'] or len(napkin_results['runFromBuildOutput']['unexpectedLibraries']) > 0:
        return False
    if not 'runFromPackagedOutput' in napkin_results:
        return False
    elif not napkin_results['runFromPackagedOutput']['success'] or len(napkin_results['runFromPackagedOutput']['unexpectedLibraries']) > 0:
        return False

    return True

def dict_entry_to_success(dict_in, phase):
    """Log of a summary of the test run

    Parameters
    ----------
    dict_in : dict
        Results for single demo
    phase : str
        Test phase

    Returns
    -------
    str
        'PASS' or 'FAIL'
    """

    if not phase in dict_in:
        return 'FAIL'
    return 'PASS' if dict_in[phase]['success'] else 'FAIL'

def dict_entry_to_libs_success(dict_in, phase):
    """Log of a summary of the test run based on having no unexpected library usage

    Parameters
    ----------
    dict_in : dict
        Results for single demo
    phase : str
        Test phase

    Returns
    -------
    str
        'PASS' or 'FAIL'
    """

    if not phase in dict_in or not 'unexpectedLibraries' in dict_in[phase]:
        return 'FAIL'
    return 'PASS' if len(dict_in[phase]['unexpectedLibraries']) == 0 else 'FAIL'

def log_summary(demo_results, template_results, napkin_results):
    """Log of a summary of the test run

    Parameters
    ----------
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template project
    napkin_results : dict
        Results for Napkin
    """

    for demo_name, this_demo in sorted(demo_results.items()):
        print("Demo: " + demo_name)
        print("- Generate: %s" % dict_entry_to_success(this_demo, 'generate'))
        print("- Build: %s" % dict_entry_to_success(this_demo, 'build'))
        print("- Package: %s" % dict_entry_to_success(this_demo, 'package'))
        print("- Run from build output: %s" % dict_entry_to_success(this_demo, 'runFromBuildOutput'))
        print("- Run from build output, libs. check: %s" % dict_entry_to_libs_success(this_demo, 'runFromBuildOutput'))
        print("- Run from packaged output: %s" % dict_entry_to_success(this_demo, 'runFromPackagedOutput'))
        print("- Run from packaged output, libs. check: %s" % dict_entry_to_libs_success(this_demo, 'runFromPackagedOutput'))
        print("----------------------------")        

    print("Template project")
    print("- Create: %s" % dict_entry_to_success(template_results, 'create'))
    print("- Generate: %s" % dict_entry_to_success(template_results, 'generate'))
    print("- Build: %s" % dict_entry_to_success(template_results, 'build'))
    print("- Package: %s" % dict_entry_to_success(template_results, 'package'))
    print("- Run from build output: %s" % dict_entry_to_success(template_results, 'runFromBuildOutput'))
    print("- Run from build output, libs. check: %s" % dict_entry_to_libs_success(template_results, 'runFromBuildOutput'))
    print("- Run from packaged output: %s" % dict_entry_to_success(template_results, 'runFromPackagedOutput'))
    print("- Run from packaged output, libs. check: %s" % dict_entry_to_libs_success(template_results, 'runFromPackagedOutput'))
    print("----------------------------")

    print("Napkin")
    print("- Package with demo: %s" % dict_entry_to_success(napkin_results, 'packageWithDemo'))
    print("- Run from build output: %s" % dict_entry_to_success(napkin_results, 'runFromBuildOutput'))
    print("- Run from build output, libs. check: %s" % dict_entry_to_libs_success(napkin_results, 'runFromBuildOutput'))
    print("- Run from packaged output: %s" % dict_entry_to_success(napkin_results, 'runFromPackagedOutput'))
    print("- Run from packaged output, libs. check: %s" % dict_entry_to_libs_success(napkin_results, 'runFromPackagedOutput'))
    if 'packaged' in napkin_results and napkin_results['packaged']['success']:
        print("  (was packaged with demo '%s')" % napkin_results['demoPackagedWith'])
    print("----------------------------")

def dump_json_report(starting_dir,
                     timestamp,
                     formatted_duration,
                     nap_framework_full_path,
                     run_success,
                     demo_results,
                     template_results,
                     napkin_results,
                     always_include_logs,
                     warnings):
    """Create a JSON report for the test run, to REPORT_FILENAME

    Parameters
    ----------
    starting_dir : str
        The working directory the test run is called from (where the report will be created)
    timestamp : str
        Timestamp of the test run
    formatted_duration : str
        The duration of the test run
    nap_framework_full_path : str
        The absolute path to the NAP framework
    run_success : str
        Whether the full test suite run was successful
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template project
    napkin_results : dict
        Results for Napkin
    always_include_logs : bool
        Whether to force inclusion logs for all processes into report, not just on failure
    warnings : list of str
        Any warnings generated throughout the testing
    """
    
    report = {}

    # Include summary details for whole test run
    report['run'] = {}
    report['run']['success'] = run_success    
    report['run']['duration'] = formatted_duration
    report['run']['startTime'] = timestamp
    report['run']['frameworkPath'] = nap_framework_full_path
    report['run']['warnings'] = warnings

    # Pull in build info
    with open(os.path.join(nap_framework_full_path, 'cmake', 'build_info.json'), 'r') as build_data:
        report['run']['frameworkBuildInfo'] = json.load(build_data)

    # Add demo results
    demo_results = copy.deepcopy(demo_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for demo_name, demo in sorted(demo_results.items()):
            for phase in ('generate', 'build', 'package', 'runFromBuildOutput', 'runFromPackagedOutput'):
                if phase in demo and demo[phase]['success']:
                    del(demo[phase]['stdout'])
                    del(demo[phase]['stderr'])
    report['demos'] = demo_results

    # Add template project results
    template_results = copy.deepcopy(template_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for phase in ('create', 'generate', 'build', 'package', 'runFromBuildOutput', 'runFromPackagedOutput'):
            if phase in template_results and template_results[phase]['success']:
                del(template_results[phase]['stdout'])
                del(template_results[phase]['stderr'])
    report['templateProject'] = template_results

    # Add Napkin results
    napkin_results = copy.deepcopy(napkin_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for phase in ('packageWithDemo', 'runFromBuildOutput', 'runFromPackagedOutput'):
            if phase in napkin_results and napkin_results[phase]['success']:
                del(napkin_results[phase]['stdout'])
                del(napkin_results[phase]['stderr'])
    report['napkin'] = napkin_results

    # Write report
    with open(os.path.join(starting_dir, REPORT_FILENAME), 'w') as f:
        f.write(json.dumps(report, indent=4, sort_keys=True))


def rename_qt_dir(warnings):
    """Attempt to rename the Qt library, if one is pointed to in environment variable QT_DIR

    Parameters
    ----------
    warnings : list of str
        Any warnings generated throughout the testing

    Returns
    -------
    str or None
        The path to the top level Qt directory
    """

    qt_top_level_path = None

    # Check for environment var
    if 'QT_DIR' in os.environ and os.path.exists(os.environ['QT_DIR']):
        # Find top-level Qt folder
        qt_top_level_path = os.path.abspath(os.environ['QT_DIR'])
        found = False
        while not found:
            if os.path.basename(os.path.normpath(qt_top_level_path)).lower().startswith('qt'):
                found = True
            else:
                qt_top_level_path = os.path.abspath(os.path.join(qt_top_level_path, os.path.pardir))
                if qt_top_level_path == '/':
                    break
        
        if found:
            try:
                print("- Renaming Qt directory")
                os.rename(qt_top_level_path, '%s-rename' % qt_top_level_path)
            except OSError as e:
                print("Couldn't rename %s: %s" % (qt_top_level_path, e))
                qt_top_level_path = None
        else:
            print("  Warning: Couldn't determine top-level Qt path to rename")
            warnings.append("Couldn't rename Qt due to inability to determine top-level Qt path")                
            qt_top_level_path = None

    return qt_top_level_path


def perform_test_run(nap_framework_path, testing_projects_dir, create_json_report, force_log_reporting, rename_framework, rename_qt):
    """Main entry point to the testing

    Parameters
    ----------
    nap_framework_path : str
        Command line provided path to NAP framework to test
    testing_projects_dir : str
        Directory to iterate for testing, by default 'demos'
    create_json_report : bool
        Whether to create a report
    force_log_reporting : bool
        Whether to force inclusion logs for all processes into report, not just on failure
    rename_framework : bool
        Whether to rename the NAP framework directory when testing packaged projects
    rename_qt : bool
        Whether to attempt to rename any Qt library pointed to via environment variable QT_DIR when testing packaged projects

    Returns
    -------
    bool
        Success of entire run
    """

    # Check to see if the specified path exists
    if not os.path.exists(nap_framework_path):
        print("Error: %s doesn't exist" % nap_framework_path)
        return False

    starting_dir = os.getcwd()
    root_output_dir = os.path.abspath('.')
    nap_framework_full_path = os.path.abspath(nap_framework_path)
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    duration_start_time = time.time()
    warnings = []

    # Check to see if our framework path looks valid
    if not os.path.exists(os.path.join(nap_framework_full_path, 'cmake', 'build_info.json')):
        print("Error: %s doesn't look like a valid extracted NAP framework" % nap_framework_path)
        return False

    # Warn if an existing template is about to be overwritten
    if os.path.exists(os.path.join(nap_framework_full_path, 'projects', TEMPLATE_APP_NAME.lower())):
        print("Warning: Template project already exists at %s and will be replaced" % os.path.join(nap_framework_path, 'projects', TEMPLATE_APP_NAME.lower()))

    # Warn about not doing framework and Qt renames on *nix
    if not is_windows() and not rename_framework:
        warning = "Not renaming NAP framework may result in missing dependencies not being detected"
        print("Warning: %s" % warning)
        warnings.append(warning)
    if not is_windows() and not rename_qt:
        warning = "Not renaming Qt may result in missing dependencies not being detected"
        print("Warning: %s" % warning)
        warnings.append(warning)

    os.chdir(os.path.join(nap_framework_full_path, testing_projects_dir))

    # Configure, build and package all demos
    print("============ Phase #1 - Building and packaging demos ============")
    demo_results = build_and_package(root_output_dir, timestamp, testing_projects_dir)

    # Package a demo with Napkin
    print("============ Phase #2 - Packaging demo with Napkin ============")
    napkin_results = package_demo_with_napkin(demo_results, root_output_dir, timestamp)

    # Create, configure, build and package a project from template
    print("============ Phase #3 - Creating, building and packaging project from template ============")
    os.chdir(nap_framework_full_path)
    template_results = create_build_and_package_template_app(root_output_dir, timestamp)
    os.chdir(os.path.join(nap_framework_full_path, testing_projects_dir))

    # Run all demos from normal build output
    print("============ Phase #4 - Running demos from build output directory ============")
    run_build_directory_demos(demo_results)

    # Run template project from normal build output
    print("============ Phase #5 - Running template project from build output directory ============")
    if 'build' in template_results and template_results['build']['success']:
        os.chdir(os.path.join(nap_framework_full_path, 'projects'))
        run_build_directory_template_project(template_results, nap_framework_full_path)
        os.chdir(os.path.join(nap_framework_full_path, testing_projects_dir))
    else:
        print("Skipping due to build failure")

    # Rename Qt (to avoid dependencies being sourced from there)
    if rename_qt:
        qt_top_level_path = rename_qt_dir(warnings)

    # Run Napkin from normal build output
    print("============ Phase #6 - Running Napkin from build output directory ============")
    run_build_directory_napkin(demo_results, napkin_results, nap_framework_full_path)

    os.chdir(starting_dir)

    print("============ Phase #7 - Running Napkin from packaged app ============")

    # Rename NAP framework (to avoid dependencies being sourced from there)
    if rename_framework:
        print("- Renaming NAP framework")
        os.rename(nap_framework_full_path, '%s-rename' % nap_framework_full_path)

    # Run Napkin from packaged project
    if 'packageWithDemo' in napkin_results and napkin_results['packageWithDemo']['success']:
        run_napkin_from_packaged_demo(napkin_results, root_output_dir, timestamp)
    else:
        print("Skipping due to package failure")

    # Run all demos from packaged projects
    print("============ Phase #8 - Running packaged demos ============")
    run_packaged_demos(demo_results, root_output_dir, timestamp)

    # Run template project from packaged projects
    print("============ Phase #9 - Running packaged template project ============")
    if 'package' in template_results and template_results['package']['success']:
        run_packaged_template_project(template_results, root_output_dir, timestamp)
    else:
        print("Skipping due to package failure")

    os.chdir(starting_dir)

    # Cleanup   
    print("============ Phase #10 - Clean up ============")
    cleanup_packaged_apps(demo_results, template_results, napkin_results, root_output_dir, timestamp, warnings)

    # Revert NAP framework rename
    if rename_framework:
        print("- Renaming NAP framework back")
        os.rename('%s-rename' % nap_framework_full_path, nap_framework_full_path)

    # Revert Qt rename
    if rename_qt and not qt_top_level_path is None:
        print("- Renaming Qt directory back")        
        os.rename('%s-rename' % qt_top_level_path, qt_top_level_path)        

    # Determine run duration
    (minutes, seconds) = divmod(time.time() - duration_start_time, 60)
    formatted_duration = '{:0>2}m{:0>2}s'.format(int(minutes), int(seconds))

    # Determine run success
    run_success = determine_run_success(demo_results, template_results, napkin_results)
    
    # Report
    if create_json_report:
        print("============ Phase #11 - Creating JSON report ============")
        dump_json_report(starting_dir,
            timestamp,
            formatted_duration,
            nap_framework_full_path,
            run_success,
            demo_results,
            template_results,
            napkin_results,
            force_log_reporting,
            warnings)

    # Log summary
    print("============ Summary ============")        
    log_summary(demo_results, template_results, napkin_results)

    # Final success log
    if create_json_report:
        print("Report: %s" % REPORT_FILENAME)
    print("Duration: %s" % formatted_duration)
    if run_success and len(warnings) == 0:
        print("%s passed all tests" % nap_framework_path)
    elif run_success and len(warnings) > 0:
        print("%s passed all tests, with warnings" % nap_framework_path)
        print("Warnings:")
        for warning in warnings:
            print("- %s" % warning)
    else:
        print("Error: %s has issues" % nap_framework_path)

    return run_success

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('NAP_FRAMEWORK_PATH', type=str,
                        help="The framework path to test against")

    parser.add_argument('--testing-projects-dir', type=str,
                        default=DEFAULT_TESTING_PROJECTS_DIR,
                        action='store', nargs='?',
                        help="Directory to test on (default %s)" % DEFAULT_TESTING_PROJECTS_DIR)
    parser.add_argument('-nj', '--no-json-report', action='store_true',
                        help="Don't create a JSON report to %s" % REPORT_FILENAME)
    parser.add_argument('-fl', '--force-log-reporting', action='store_true',
                        help="If reporting to JSON, include STDOUT and STDERR even if there has been no issue")
    if not is_windows():
        parser.add_argument('-nrf', '--no-rename-framework', action='store_true',
                            help="Don't rename the NAP framework while testing packaged projects")
        parser.add_argument('-nrq', '--no-rename-qt', action='store_true',
                            help="Don't attempt to rename the Qt library dir pointed to by QT_DIR while testing packaged projects")

    args = parser.parse_args()

    # Don't do any NAP framework / Qt renaming on Windows
    if is_windows():
        args.no_rename_framework = True
        args.no_rename_qt = True

    success = perform_test_run(args.NAP_FRAMEWORK_PATH, args.testing_projects_dir, not args.no_json_report, args.force_log_reporting, not args.no_rename_framework, not args.no_rename_qt)
    sys.exit(not success)
