#!/usr/bin/env python3

"""This script does some crude dependencies testing on a NAP framework release. See the README."""

from __future__ import print_function
import argparse
import copy
import datetime
import json
from multiprocessing import cpu_count
import os
from platform import machine
import re
from subprocess import call, Popen, PIPE, check_output, TimeoutExpired, run
import shlex
import shutil
import signal
import sys
import time
import sys

# How long to wait for the process to run. This should be long enough that we're sure
# it will have completed initialisation.
WAIT_SECONDS_FOR_PROCESS_HEALTH = 6
if sys.platform.startswith('linux') and not machine() == 'x86_64':
    WAIT_SECONDS_FOR_PROCESS_HEALTH = 30

# Name for app created from template
TEMPLATE_APP_NAME = 'TemplateApp'

# Build type for build apps
APP_BUILD_TYPE = 'Release'

# Directory to iterate for testing
DEFAULT_TESTING_APPS_DIR = 'demos'

# Directory containing modules, to verify they all get dependency tested
MODULES_DIR = 'system_modules'

# Main app structure filename
APP_FILENAME = 'app.json'

# JSON report filename
REPORT_FILENAME = 'report.json'

# Process success exit code
SUCCESS_EXIT_CODE = 0

# Seconds to wait for a Napkin load app and exit with expected exit code
NAPKIN_SECONDS_WAIT_FOR_PROCESS = 30
if sys.platform.startswith('linux') and not machine() == 'x86_64':
    NAPKIN_SECONDS_WAIT_FOR_PROCESS = 40

# Build directory names
LINUX_BUILD_DIR = 'build'
MACOS_BUILD_DIR = 'xcode'
MSVC_BUILD_DIR = 'msvc64'

# List of locations on a Ubuntu system where we're happy to find system libraries. Restricting
# to these paths helps us identify libraries being source from strange locations, hand installed libs.
# TODO Handle other architectures.. eventually
LINUX_ACCEPTED_SYSTEM_LIB_PATHS = []
if sys.platform.startswith('linux'):
    arch = machine()
    if arch == 'aarch64':
        p = run('getconf LONG_BIT', shell=True, text=True, capture_output=True)
        if p.stdout.strip() == '64':
            arch = 'arm64'
        else:
            arch = 'armhf'
    if arch == 'x86_64':
        LINUX_ACCEPTED_SYSTEM_LIB_PATHS.extend(['/usr/lib/x86_64-linux-gnu/', 
            '/lib/x86_64-linux-gnu', 
            '/usr/lib/mesa-diverted/x86_64-linux-gnu/'])
    elif arch in ('arm64', 'armhf'):
        LINUX_ACCEPTED_SYSTEM_LIB_PATHS.append('/usr/lib/aarch64-linux-gnu/')
        LINUX_ACCEPTED_SYSTEM_LIB_PATHS.extend(['/usr/lib/arm-linux-gnueabihf/',
            '/opt/vc/lib/'])

# List of libraries we accept being sourced from the system paths defined above. Notes:
# - These currently support Ubuntu 20.04/20.10 and are likely to require minor tweaks for new versions
# - If/when we support more distros and architectures we should either break these lists out by 
#   architecture and distro+version or restrict dependencies testing to one distro
# - Developed and tested against Nvidia open source and proprietary drivers plus Intel i965, other
#   hardware may require additions
# - Developed against x.org, Wayland will need additions
# - Regular expressions supported
# - This is somewhat a proof of concept and is by nature fairly brittle. Let's see how it goes.
LINUX_BASE_ACCEPTED_SYSTEM_LIBS = [
    'i965_dri',
    'iris_dri',
   r'ld-[0-9]+\.[0-9]+',
    'ld-linux-x86-64',
    'libFLAC',
    'libICE',
    'libGL',
    'libGLX',
    'libGLX_mesa',
    'libGLX_nvidia',
    'libGLdispatch',
    'libLLVM-[0-9]+',
    'libOpenCL',
    'libSM',
    'libVkLayer_MESA_device_select',
    'libX11',
    'libX11-xcb',
    'libXau',
    'libXcursor',
    'libXdamage',
    'libXdmcp',
    'libXext',
    'libXfixes',
    'libXi',
    'libXinerama',
    'libXrandr',
    'libXrender',
    'libXss',
    'libXtst',
    'libXxf86vm',
    'libaom',
    'libapparmor',
    'libarmmem-v7l',
    'libasound',
    'libasound_module_pcm_a52',
    'libasound_module_pcm_jack',
    'libasound_module_pcm_oss',
    'libasound_module_pcm_pulse',
    'libasound_module_pcm_speex',
    'libasound_module_pcm_upmix',
    'libasound_module_pcm_usb_stream',
    'libasound_module_pcm_vdownmix',
    'libasound_module_rate_lavrate',
    'libasound_module_rate_samplerate',
    'libasound_module_rate_speexrate',
    'libasyncns',
    'libatomic',
    'libavcodec',
    'libavresample',
    'libavutil',
    'libbcm_host',
    'libblkid',
    'libbrotlicommon',
    'libbrotlidec',
    'libbsd',
    'libbz2',
    'libc',
   r'libc-[0-9]+\.[0-9]+',
    'libcairo',
    'libcairo-gobject',
    'libcap',
    'libcodec2',
    'libcom_err',
   r'libcroco-[0-9]+\.[0-9]+',
    'libcuda',
    'libdatrie',
    'libdav1d',
   r'libdb-[0-9]+\.[0-9]+',
    'libdbus-1',
    'libdecor-0',
    'libdl',
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
    'libfontconfig',
    'libfreetype',
    'libfribidi',
    'libgbm',
    'libgcc_s',
    'libgcrypt',
   r'libgdk_pixbuf-[0-9]+\.[0-9]+',
   r'libgio-[0-9]+\.[0-9]+',
    'libglapi',
    'libglib-2.0',
   r'libglib-[0-9]+\.[0-9]+',
   r'libgmodule-[0-9]+\.[0-9]+',
   r'libgobject-[0-9]+\.[0-9]+',
    'libgomp',
    'libgpg-error',
    'libgraphite2',
    'libgsm',
    'libgssapi_krb5',
   r'libgthread-[0-9]+\.[0-9]+',
    'libharfbuzz',
    'libicudata',
    'libicui18n',
    'libicuuc',
    'libjack',
    'libjpeg',
    'libk5crypto',
    'libkeyutils',
    'libkrb5',
    'libkrb5support',
    'liblz4',
    'liblzma',
    'libm',
   r'libm-[0-9]+\.[0-9]+',
    'libmd',
    'libmfx',
    'libmount',
    'libmp3lame',
    'libmmal_core',
    'libmmal_util',
    'libmmal_vc_client',
   r'libmvec-[0-9]+\.[0-9]+',
    'libnsl',
   r'libnsl-[0-9]+\.[0-9]+',
   r'libnss_compat-[0-9]+\.[0-9]+',
    'libnss_compat',
   r'libnss_files-[0-9]+\.[0-9]+',
    'libnss_nis',
   r'libnss_nis-[0-9]+\.[0-9]+',
    'libnuma',
    'libnvidia-cbl',
    'libnvidia-compiler',
    'libnvidia-fatbinaryloader',
    'libnvidia-glcore',
    'libnvidia-glvkspirv',
    'libnvidia-opencl',
    'libnvidia-rtcore',
    'libnvidia-tls',
    'libogg',
    'libopenjp2',
    'libopus',
   r'libpango-[0-9]+\.[0-9]+',
   r'libpangocairo-[0-9]+\.[0-9]+',
   r'libpangoft2-[0-9]+\.[0-9]+',
    'libpciaccess',
    'libpcre',
    'libpcre2-8',
    'libpixman-1',
    'libpng16',
    'libpthread',
   r'libpthread-[0-9]+\.[0-9]+',
    'libpulse',
   r'libpulsecommon-[0-9]+\.[0-9]+',
   r'libpython[0-9]+\.[0-9]+m',
   r'libresolv-[0-9]+\.[0-9]+',
    'libresolv',
    'librsvg-2',
    'librt',
   r'librt-[0-9]+\.[0-9]+',
    'libsamplerate',
    'libselinux',
    'libsensors',
    'libshine',
    'libsnappy',
    'libsndfile',
    'libsoxr',
    'libspeex',
    'libspeexdsp',
   r'libstdc\+\+',
    'libswresample',
    'libsystemd',
    'libthai',
    'libtheoradec',
    'libtheoraenc',
    'libtinfo',
    'libtirpc',
    'libtwolame',
    'libudev',
   r'libusb-[0-9]+\.[0-9]+',
   r'libutil-[0-9]+\.[0-9]+',
    'libutil',
    'libuuid',
    'libva',
    'libva-drm',
    'libva-x11',
    'libvchiq_arm',
    'libvcos',
    'libvcsm',
    'libvdpau',
    'libvorbis',
    'libvorbisenc',
    'libvpx',
    'libvulkan_broadcom',
    'libvulkan_intel',
    'libvulkan_lvp',
    'libvulkan_radeon',
    'libwavpack',
    'libwayland-client',
    'libwayland-cursor',
    'libwayland-egl',
    'libwayland-server',
    'libwebp',
    'libwebpmux',
    'libwrap',
    'libx264',
    'libx265',
    'libxcb',
    'libxcb-dri2',
    'libxcb-dri3',
    'libxcb-glx',
    'libxcb-icccm',
    'libxcb-image',
    'libxcb-keysyms',
    'libxcb-present',
    'libxcb-randr',
    'libxcb-render',
    'libxcb-render-util',
    'libxcb-shape',
    'libxcb-shm',
    'libxcb-sync',
    'libxcb-util',
    'libxcb-xinerama',
    'libxcb-xkb',
    'libxcb-xfixes',
    'libxkbcommon',
    'libxkbcommon-x11',
    'libxml2',
    'libxshmfence',
    'libxvidcore',
    'libz',
    'libz3',
    'libzstd',
    'libzvbi',
    'nouveau_dri',
    'radeonsi_dri',
    'sun4i-drm_dri',
    'vc4_dri',
    'libvulkan_virtio',
    'libvulkan_freedreno.so',
    'ld-linux-aarch64'
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
                                   '/System/Library/Components/',
                                   '/Library/CoreMediaIO/'
                                   ]

# Quicker iteration when debugging this script
SCRIPT_DEBUG_ONE_APP_ONLY = False

# Whether to treat unexpected libs as an error
# TODO Temporary global until upcoming small restructure
TREAT_UNEXPECTED_LIBS_AS_ERROR = True

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def get_platform_scriptpath(script):
    if sys.platform == 'win32':
        return f'{script}'
    else:
        return os.path.join('.', f'{script}.sh')

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

def get_packaged_app_output_path(app_name, pre_files, post_files):
    """Determine the name of the directory containing the newly packaged app

    Parameters
    ----------
    app_name : str
        Name of app
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
        if f.lower().startswith(app_name.lower()):
            return f

    eprint("Error: get_packaged_app_output_path() sees no difference")
    return None

def is_linux():
    """Is this Linux?

    Returns
    -------
    bool
        Success
    """

    return sys.platform.startswith('linux')

def is_windows():
    """Is this Windows?

    Returns
    -------
    bool
        Success
    """

    return sys.platform.startswith('win')

def is_linux_root():
    """Are we running as a root account on Linux?

    Returns
    -------
    bool
        Success
    """
    if not is_linux():
        return False

    return os.geteuid() == 0

def launch_pulseaudio():
    """Launch pulseaudio. Used for when running as root on Linux (which is necessary to test 
       websocket functionality)."""
    cmd = 'pulseaudio -D --disallow-exit=1--exit-idle-time=-1> /dev/null 2>&1'
    Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)

def kill_pulseaudio():
    """Kill pulseaudio. Used for when running as root on Linux (which is necessary to test 
       websocket functionality)."""
    call_capturing_output('pulseaudio -k')

def force_quit(process):
    """Force quit given process by sending kill signal"""
    while process.returncode is None:
        try:
            process.kill()
        except OSError:
            pass
        time.sleep(1);
        process.poll();

def run_process_then_stop(cmd, accepted_shared_libs_path=None, testing_napkin=False, expect_early_closure=False, success_exit_code=0, wait_for_seconds=WAIT_SECONDS_FOR_PROCESS_HEALTH):
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
    expect_early_closure : bool
        Whether process having closed before we kill it is OK
    success_exit_code : int
        Process exit code representing success when closing
    wait_for_seconds : int
        Seconds to wait before determining run success


    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    unexpected_libraries : list of str
        List of unexpected libraries in use (Unix only)
    returncode : int
        Process exit code
    """

    # Launch the app
    my_env = os.environ.copy()
    # For shared libraries tracking on macOS
    if sys.platform == 'darwin':
        my_env['DYLD_PRINT_LIBRARIES'] = '1'
    # Split command on Unix
    if sys.platform != 'win32':
        cmd = shlex.split(cmd)

    # If running Napkin on Windows don't capture STDOUT when running from a packaged app after 
    # issues seen 20/08/2020 with Napkin effectively locking up and returning an error code when 
    # opening apps with either a service config or using the video module while piping STDOUT. 
    # Hopefully temporary.
    if testing_napkin and sys.platform == 'win32' and '..\\%s' % APP_FILENAME in cmd:
        p = Popen(cmd, stderr=PIPE, env=my_env)
    else:
        p = Popen(cmd, stdout=PIPE, stderr=PIPE, env=my_env)

    # Wait for the app to initialise
    waited_time = 0
    while waited_time < wait_for_seconds and p.returncode is None:
        time.sleep(0.5)
        waited_time += 0.5
        p.poll()


    # Track success
    success = False
    unexpected_libraries = []

    # Process running
    if p.poll() == None:

        # Get unexpected libs
        if is_linux():
            unexpected_libraries = linux_check_for_unexpected_library_use(p.pid, accepted_shared_libs_path)

        # Send SIGTERM and wait a moment to close. Use force quit if wait timer expires.
        try:
            p.terminate()
            p.wait(10)
            success = not expect_early_closure
        except TimeoutExpired:
            print("  Failed to close on terminate, sending kill signal")
            force_quit(p)
            pass
    # Process done
    else:
        # Exit code must be 'success_exit_code'. 
        # If an application crashed or failed on initialization the exit code will not be 'success_exit_code'.
        success = p.returncode == success_exit_code

    # Gather info from stream
    (stdout, stderr) = p.communicate()
    if type(stdout) == bytes:
        stdout = stdout.decode('utf8')
    if type(stderr) == bytes:
        stderr = stderr.decode('utf8')    
        
    # Check for unexpected libraries
    if sys.platform == 'darwin':
        unexpected_libraries = macos_check_for_unexpected_library_use(stderr, accepted_shared_libs_path)

    # Done
    return (success, stdout, stderr, unexpected_libraries, p.returncode)

def linux_check_for_unexpected_library_use(pid, accepted_shared_libs_path):
    """Check whether the specified NAP process is using unexpected libraries on Linux

    Parameters
    ----------
    pid : str
        Command to run
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from.
        Typically NAP framework for build programs running from framework, or the packaged app for
        single apps.

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
                if not shared_lib_accepted(path, accepted_shared_libs_path):
                    unexpected_libs.append(path)

    return unexpected_libs

def macos_check_for_unexpected_library_use(stdout, accepted_shared_libs_path):
    """Check whether the a NAP process has used unexpected libraries on macOS

    Parameters
    ----------
    stdout : str
        STDOUT from the process
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from.
        Typically NAP framework for build programs running from framework, or the packaged app for
        single apps.

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
            if not shared_lib_accepted(libs_abs_path, accepted_shared_libs_path):
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

def shared_lib_accepted(file_path, accepted_shared_libs_path):
    """Check whether a shared library is expected for the executed NAP process

    Parameters
    ----------
    file_path : str
        Path to the shared library
    accepted_shared_libs_path : str
        Absolute path to directory which we're happy to see any shared libraries source from.
        Typically NAP framework for build programs running from framework, or the packaged app for
        single apps.

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
    if is_linux():
        for system_path in LINUX_ACCEPTED_SYSTEM_LIB_PATHS:
            if file_path.startswith(system_path):
                in_system_path = True

                if not ".so" in filename:
                    eprint("Error: Unhandled Linux library due to lacking .so: %s" % path)
                    return False

                # Get short library name used for verification
                filename_parts = filename.split(".so")
                short_lib_name = filename_parts[0]

                # Verify against system library list
                accepted = linux_system_library_accepted(short_lib_name)
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
            level = "Error" if TREAT_UNEXPECTED_LIBS_AS_ERROR else "Warning"
            eprint("%s: unexpected system library encountered: %s" % (level, file_path))
            return False
    else:
        eprint("Error: library found outside of system path: %s" % file_path)
        return False

def linux_system_library_accepted(short_lib_name):
    """Check whether a single shared system library is expected for the executed NAP process on
       Linux

    Parameters
    ----------
    short_lib_name : str
        The library name (with .so and everything after stripped)

    Returns
    -------
    bool
        Whether the library is accepted
    """

    # Build list of accepted library names
    all_accepted_libs = LINUX_BASE_ACCEPTED_SYSTEM_LIBS

    # Verify the library name against the list
    for check_lib in all_accepted_libs:
        expr = "^" + check_lib + "$"
        if re.match(expr, short_lib_name):
            return True

    return False

def regenerate_cwd_app(build_type=APP_BUILD_TYPE):
    """Configure app in current working directory

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
    script = get_platform_scriptpath('regenerate')
    if is_linux():
        cmd = f'{script} %s' % build_type
    else:
        cmd = f'{script} -ns -np'

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    if success:
        print("  Done.")
    else:
        eprint("  Error: Couldn't regenerate, return code: %s" % returncode)
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)

    return (success, stdout, stderr)

def build_cwd_app(app_name, build_type=APP_BUILD_TYPE):
    """Build app in current working directory, excluding Napkin

    Parameters
    ----------
    app_name : str
        Name of app

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
        os.chdir(MACOS_BUILD_DIR)
        cmd = 'xcodebuild -configuration %s -jobs %s' % (build_type, cpu_count())
    elif is_linux():
        os.chdir(LINUX_BUILD_DIR)
        cmd = 'make all . -j%s' % cpu_count()
    else:
        os.chdir(MSVC_BUILD_DIR)
        cmd = '..\\..\\..\\thirdparty\\cmake\\msvc\\x86_64\\bin\\cmake --build . --target %s --config %s' % (app_name, build_type)

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    if success:
        print("  Done.")
    else:
        eprint("  Error: Couldn't build, return code: %s" % returncode)
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)

    os.chdir(os.path.pardir)

    return (success, stdout, stderr)

def package_cwd_app_with_napkin(app_name, root_output_dir, timestamp):
    """Package app in current working directory, excluding Napkin

    Parameters
    ----------
    app_name : str
        Name of app
    root_output_dir : str
        Directory where packaged apps will be moved to
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

    print("- Packaging (with Napkin)...")
    pre_files = os.listdir('.')

    # Build command
    script = get_platform_scriptpath('package')
    cmd = f'{script} -nz -ns'
    if not is_linux():
        cmd = '%s -np' % cmd

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    if success:
        # Move package to starting directory
        post_files = os.listdir('.')
        output_path = get_packaged_app_output_path(app_name, pre_files, post_files)
        home_output = os.path.join(root_output_dir, '%s-%s-napkin' % (app_name, timestamp))
        os.rename(output_path, home_output)
        nap_framework_full_path = os.path.join(os.getcwd(), os.pardir, os.pardir)
        patch_audio_service_configuration('.', home_output, app_name, nap_framework_full_path)
        print("  Done. Moving to %s." % home_output)
    else:
        eprint("  Error: Couldn't package app, return code: %s" % returncode)
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)

    return (success, stdout, stderr)

def run_cwd_app(app_name, nap_framework_full_path, build_type=APP_BUILD_TYPE):
    """Run app from normal build output

    Parameters
    ----------
    app_name : str
        Name of app
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
    unexpected_libraries : list of str
        List of unexpected libraries in use (Unix only)
    returncode : int
        Process exit code
    """

    print("- Run from build output...")

    # Find build output path
    build_paths = os.listdir('bin')
    for f in build_paths:
        if build_type.lower() in f.lower():
            build_path = f

    # Build command and run
    folder = os.path.abspath(os.path.join(os.getcwd(), 'bin', build_path))
    patch_audio_service_configuration(os.getcwd(), os.getcwd(), app_name, nap_framework_full_path)
    cmd = os.path.join(folder, app_name)
    (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop(cmd, nap_framework_full_path)
    if success:
        print("  Done.")
    else:
        eprint("  Error: Couldn't run from build dir")
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)
        eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
        eprint("  Exit code: %s" % return_code)

    return (success, stdout, stderr, unexpected_libs, return_code)

def run_packaged_app(results, root_output_dir, timestamp, app_name, has_napkin=True):
    """Run packaged app from output directory

    Parameters
    ----------
    results : dict
        Results for the app
    root_output_dir : str
        Directory where packaged apps will be moved to
    timestamp : str
        Timestamp of the test run
    app_name : str
        Name of app
    has_napkin : bool
        Whether the app was packaged with Napkin
    """

    suffix = 'napkin' if has_napkin else 'no_napkin'
    containing_dir = os.path.abspath(os.path.join(root_output_dir, '%s-%s-%s' % (app_name, timestamp, suffix)))
    os.chdir(containing_dir)
    print("- Run from package...")

    # Run
    (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop('./%s' % app_name, containing_dir)
    if success:
        print("  Done.")
    else:
        eprint("  Error: Couldn't run from package")
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)
        eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
        eprint("  Exit code: %s" % return_code)

    results['runFromPackagedOutput'] = {}
    results['runFromPackagedOutput']['success'] = success
    results['runFromPackagedOutput']['stdout'] = stdout
    results['runFromPackagedOutput']['stderr'] = stderr
    results['runFromPackagedOutput']['unexpectedLibraries'] = unexpected_libs
    if not success:
        results['runFromPackagedOutput']['exitCode'] = return_code

def build_and_package(root_output_dir, timestamp, testing_apps_dir, apps_to_exclude):
    """Configure, build and package all demos

    Parameters
    ----------
    root_output_dir : str
        Directory where packaged apps will be moved to
    timestamp : str
        Timestamp of the test run
    testing_apps_dir : str
        Directory to iterate for testing, by default 'demos'
    apps_to_exclude: str
        apps to exclude

    Returns
    -------
    dict
        Results
    """

    demos_root_dir = os.getcwd()
    demo_results = {}
    excluded_apps = apps_to_exclude.split()
    for app in excluded_apps:
        print("Excluding app: {0}".format(app))

    # Iterate demos
    sorted_dirs = os.listdir('.')
    sorted_dirs.sort()
    for demo_name in sorted_dirs:
        demo_path = os.path.join(demos_root_dir, demo_name)
        # Check if path looks sane
        if not os.path.isdir(demo_path) or demo_name.startswith('.'):
            continue

        # If we're iterating our apps directory and we've already got our template app in there
        # from a previous run, don't built it in here as we'll build it separately later
        if demo_name == TEMPLATE_APP_NAME and testing_apps_dir != DEFAULT_TESTING_APPS_DIR:
            continue

        # If the app is excluded, skip
        if demo_name in excluded_apps:
            continue

        print("----------------------------")
        print("Demo: %s" % demo_name)
        os.chdir(demo_path)
        demo_results[demo_name] = {}

        # Configure
        (success, stdout, stderr) = regenerate_cwd_app()
        demo_results[demo_name]['generate'] = {}
        demo_results[demo_name]['generate']['success'] = success
        demo_results[demo_name]['generate']['stdout'] = stdout
        demo_results[demo_name]['generate']['stderr'] = stderr
        if not success:
            # Couldn't configure, skip to next demo
            continue

        # Build
        (success, stdout, stderr) = build_cwd_app(demo_name)
        demo_results[demo_name]['build'] = {}
        demo_results[demo_name]['build']['success'] = success
        demo_results[demo_name]['build']['stdout'] = stdout
        demo_results[demo_name]['build']['stderr'] = stderr
        if not success:
            # Couldn't build, skip to next demo
            continue

        # Package
        (success, stdout, stderr) = package_cwd_app_with_napkin(demo_name, root_output_dir, timestamp)
        demo_results[demo_name]['package'] = {}
        demo_results[demo_name]['package']['success'] = success
        demo_results[demo_name]['package']['stdout'] = stdout
        demo_results[demo_name]['package']['stderr'] = stderr

        if SCRIPT_DEBUG_ONE_APP_ONLY:
            break

    os.chdir(os.path.pardir)
    return demo_results

def build_other_build_type_demo(build_type, misc_results):
    """Build a demo using the build type not used elsewhere

    Parameters
    ----------
    build_type : str
        Build type to use for test

    Returns
    -------
    dict
        Results
    """

    demos_root_dir = os.getcwd()

    other_build_type_results = {}

    # Iterate demos
    sorted_dirs = os.listdir('.')
    sorted_dirs.sort()
    test_demo = None
    for demo_name in sorted_dirs:
        demo_path = os.path.join(demos_root_dir, demo_name)
        # Check if path looks sane
        if not os.path.isdir(demo_path) or demo_name.startswith('.'):
            continue

        test_demo = demo_name

        # Prefer a demo with a module
        module_path = os.path.join(demo_path, 'module')
        if os.path.isdir(module_path):
            break

    print("Demo: %s" % test_demo)
    other_build_type_results['demoName'] = test_demo
    other_build_type_results['buildType'] = build_type

    os.chdir(demo_path)

    # Configure
    (success, stdout, stderr) = regenerate_cwd_app(build_type)
    other_build_type_results['generate'] = {}
    other_build_type_results['generate']['success'] = success
    other_build_type_results['generate']['stdout'] = stdout
    other_build_type_results['generate']['stderr'] = stderr

    # Build
    if success:
        (success, stdout, stderr) = build_cwd_app(test_demo, build_type)
        other_build_type_results['build'] = {}
        other_build_type_results['build']['success'] = success
        other_build_type_results['build']['stdout'] = stdout
        other_build_type_results['build']['stderr'] = stderr

    misc_results['otherBuildType'] = other_build_type_results
    os.chdir(os.path.pardir)
    return other_build_type_results

def package_demo_without_napkin(demo_results, root_output_dir, timestamp):
    """Package app in current working directory, without Napkin

    Parameters
    ----------
    demo_results : dict
        Results from demo packaging, assists with identifying healthy app to package Napkin with
    root_output_dir : str
        Directory where packaged apps will be moved to
    timestamp : str
        Timestamp of the test run

    Returns
    -------
    dict
        Results
    """

    napkin_package_demo = None

    # Iterate demos to find a healthy one to use
    for demo_name, this_demo in sorted(demo_results.items()):
        if 'package' in this_demo and this_demo['package']['success']:
            napkin_package_demo = demo_name
            break

    # Fail if there's no healthy demo
    if napkin_package_demo is None:
        eprint("Error: no demo found to package Napkin with")
        return {}

    print("Demo: %s" % napkin_package_demo)
    print("- Packaging (without Napkin)...")
    results = {}
    results['name'] = napkin_package_demo
    os.chdir(napkin_package_demo)
    pre_files = os.listdir('.')

    # Build command
    script = get_platform_scriptpath('package')
    cmd = f'{script} -nn -nz -ns'
    if not is_linux():
        cmd = f'{cmd} -np'

    # Run
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    success = returncode == 0
    results['package'] = {}
    results['package']['success'] = success
    results['package']['stdout'] = stdout
    results['package']['stderr'] = stderr
    if success:
        # Move package to starting directory
        post_files = os.listdir('.')
        output_path = get_packaged_app_output_path(napkin_package_demo, pre_files, post_files)
        home_output = os.path.join(root_output_dir, '%s-%s-no_napkin' % (napkin_package_demo, timestamp))
        nap_framework_full_path = os.path.join(os.getcwd(), os.pardir, os.pardir)
        patch_audio_service_configuration('.', output_path, napkin_package_demo, nap_framework_full_path)
        print("  Done. Moving to %s." % home_output)
        os.rename(output_path, home_output)
    else:
        eprint("  Error: Couldn't package with Napkin, return code: %s" % returncode)
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)

    os.chdir(os.path.pardir)
    misc_results = {'packagedWithoutNapkin': results}
    return misc_results

def create_build_and_package_template_app(root_output_dir, timestamp):
    """Create, configure, build and package app from template

    Parameters
    ----------
    root_output_dir : str
        Directory where packaged apps will be moved to
    timestamp : str
        Timestamp of the test run

    Returns
    -------
    dict
        Results
    """

    prev_cwd = os.getcwd()

    print("Template app")

    # Create dict for results
    template_results = {}

    # Remove any existing app
    app_path = os.path.join('apps', TEMPLATE_APP_NAME)
    if os.path.exists(app_path):
        print("- Pre-existing template app found, removing")
        shutil.rmtree(app_path)

    # Create from template
    print("- Create app from template...")
    pre_files = os.listdir('.')
    script = get_platform_scriptpath(os.path.join('tools', 'create_app'))
    cmd = f'{script} -ng {TEMPLATE_APP_NAME}'
    (returncode, stdout, stderr) = call_capturing_output(cmd)
    template_creation_success = returncode == 0
    template_results['create'] = {}
    template_results['create']['success'] = template_creation_success
    template_results['create']['stdout'] = stdout
    template_results['create']['stderr'] = stderr

    if template_creation_success:
        # Creation successful, now configure
        print("  Done.")
        os.chdir(os.path.join('apps', TEMPLATE_APP_NAME))
        (template_generate_success, stdout, stderr) = regenerate_cwd_app()
        template_results['generate'] = {}
        template_results['generate']['success'] = template_generate_success
        template_results['generate']['stdout'] = stdout
        template_results['generate']['stderr'] = stderr

        if template_generate_success:
            # Configure successful, now build
            (template_build_success, stdout, stderr) = build_cwd_app(TEMPLATE_APP_NAME)
            template_results['build'] = {}
            template_results['build']['success'] = template_build_success
            template_results['build']['stdout'] = stdout
            template_results['build']['stderr'] = stderr

            if template_build_success:
                # Build successful, now package
                (template_package_success, stdout, stderr) = package_cwd_app_with_napkin(TEMPLATE_APP_NAME, root_output_dir, timestamp)
                template_results['package'] = {}
                template_results['package']['success'] = template_package_success
                template_results['package']['stdout'] = stdout
                template_results['package']['stderr'] = stderr
    else:
        # Couldn't create, print logs
        eprint("  Error: Couldn't create app from template, return code: %s" % returncode)
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)

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
        (success, stdout, stderr, unexpected_libs, return_code) = run_cwd_app(demo_name, os.path.abspath(os.path.join(demos_root_dir, os.pardir)))
        this_demo['runFromBuildOutput'] = {}
        this_demo['runFromBuildOutput']['success'] = success
        this_demo['runFromBuildOutput']['stdout'] = stdout
        this_demo['runFromBuildOutput']['stderr'] = stderr
        this_demo['runFromBuildOutput']['unexpectedLibraries'] = unexpected_libs
        if not success:
            this_demo['runFromBuildOutput']['exitCode'] = return_code

        print("----------------------------")

    os.chdir(demos_root_dir)

def run_other_build_type_demo(results, build_type):
    """Run demo using the build type not used elsewhere

    Parameters
    ----------
    results : dict
        Results from demo building
    """

    demos_root_dir = os.getcwd()

    demo_name = results['demoName']
    print("Demo: %s" % demo_name)
    os.chdir(os.path.join(demos_root_dir, demo_name))

    # Run
    (success, stdout, stderr, unexpected_libs, return_code) = run_cwd_app(demo_name, os.path.abspath(os.path.join(demos_root_dir, os.pardir)), build_type)
    results['runFromBuildOutput'] = {}
    results['runFromBuildOutput']['success'] = success
    results['runFromBuildOutput']['stdout'] = stdout
    results['runFromBuildOutput']['stderr'] = stderr
    results['runFromBuildOutput']['unexpectedLibraries'] = unexpected_libs
    if not success:
        results['runFromBuildOutput']['exitCode'] = return_code

    print("----------------------------")

    os.chdir(demos_root_dir)

def run_build_directory_template_app(template_results, nap_framework_full_path):
    """Run template app from the normal build output

    Parameters
    ----------
    template_results : dict
        Results for template app
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    apps_dir = os.getcwd()

    # Change to template app directory
    os.chdir(TEMPLATE_APP_NAME)

    # Run
    (success, stdout, stderr, unexpected_libs, return_code) = run_cwd_app(TEMPLATE_APP_NAME, nap_framework_full_path)
    template_results['runFromBuildOutput'] = {}
    template_results['runFromBuildOutput']['success'] = success
    template_results['runFromBuildOutput']['stdout'] = stdout
    template_results['runFromBuildOutput']['stderr'] = stderr
    template_results['runFromBuildOutput']['unexpectedLibraries'] = unexpected_libs
    if not success:
        template_results['runFromBuildOutput']['exitCode'] = return_code

    os.chdir(apps_dir)

def open_napkin_from_framework_release_without_app(napkin_results, nap_framework_full_path):
    """Open Napkin the framework release without opening a app

    Parameters
    ----------
    napkin_results : dict
        Results for Napkin
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    napkin_results['runFromFrameworkRelease'] = {}
    prev_wd = os.getcwd()

    # Change directory and run
    os.chdir(os.path.join(nap_framework_full_path, 'tools', 'napkin'))
    (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop('./napkin --no-project-reopen',
                                                                                    nap_framework_full_path,
                                                                                    True)

    napkin_results['runFromFrameworkRelease']['success'] = success
    napkin_results['runFromFrameworkRelease']['stdout'] = stdout
    napkin_results['runFromFrameworkRelease']['stderr'] = stderr
    napkin_results['runFromFrameworkRelease']['unexpectedLibraries'] = unexpected_libs
    if not success:
        napkin_results['runFromFrameworkRelease']['exitCode'] = return_code

    if success:
        print("  Done.")
    else:
        eprint("  Error: Running Napkin from Framework Release without app failed")
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)
        eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
        eprint("  Exit code: %s" % return_code)

    os.chdir(prev_wd)

def open_apps_in_napkin_from_framework_release(demo_results, nap_framework_full_path):
    """Open demos in Napkin from the framework release

    Parameters
    ----------
    demo_results : dict
        Results for demos
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    demos_root_dir = os.getcwd()
    os.chdir(os.path.join(nap_framework_full_path, 'tools', 'napkin'))
    for demo_name, this_demo in sorted(demo_results.items()):
        # If demo didn't build skip it
        if not 'build' in this_demo or not this_demo['build']['success']:
            continue

        print("Demo: %s" % demo_name)
        print("- Open with Napkin from framework release...")

        # Run
        demo_app_json = os.path.join(demos_root_dir, demo_name, APP_FILENAME)
        (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop('./napkin -p %s --exit-after-load' % demo_app_json,
                                                                                        nap_framework_full_path,
                                                                                        True,
                                                                                        True,
                                                                                        SUCCESS_EXIT_CODE,
                                                                                        NAPKIN_SECONDS_WAIT_FOR_PROCESS)

        this_demo['openWithNapkinBuildOutput'] = {}
        this_demo['openWithNapkinBuildOutput']['success'] = success
        this_demo['openWithNapkinBuildOutput']['stdout'] = stdout
        this_demo['openWithNapkinBuildOutput']['stderr'] = stderr
        this_demo['openWithNapkinBuildOutput']['unexpectedLibraries'] = unexpected_libs

        if success:
            print("  Done.")
        else:
            eprint("  Error: Failed to open app")
            eprint("  STDOUT: %s" % stdout)
            eprint("  STDERR: %s" % stderr)
            eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
            eprint("  Exit code: %s" % return_code)

        print("----------------------------")

    os.chdir(demos_root_dir)


def open_template_app_in_napkin_from_framework_release(template_results, nap_framework_full_path):
    """Open template app in Napkin from framework release

    Parameters
    ----------
    tempate_results : dict
        Results for template app
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    prev_wd = os.getcwd()

    # Template app
    print("Template app")
    print("- Open with Napkin from framework release...")
    os.chdir(os.path.join(nap_framework_full_path, 'tools', 'napkin'))
    if 'build' in template_results and template_results['build']['success']:
        template_app_json = os.path.join(nap_framework_full_path, 'apps', TEMPLATE_APP_NAME, APP_FILENAME)
        (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop('./napkin -p %s --exit-after-load' % template_app_json,
                                                                                        nap_framework_full_path,
                                                                                        True,
                                                                                        True,
                                                                                        SUCCESS_EXIT_CODE,
                                                                                        NAPKIN_SECONDS_WAIT_FOR_PROCESS)

        template_results['openWithNapkinBuildOutput'] = {}
        template_results['openWithNapkinBuildOutput']['success'] = success
        template_results['openWithNapkinBuildOutput']['stdout'] = stdout
        template_results['openWithNapkinBuildOutput']['stderr'] = stderr
        template_results['openWithNapkinBuildOutput']['unexpectedLibraries'] = unexpected_libs

        if success:
            print("  Done.")
        else:
            eprint("  Error: Failed to open app")
            eprint("  STDOUT: %s" % stdout)
            eprint("  STDERR: %s" % stderr)
            eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
            eprint("  Exit code: %s" % return_code)
    else:
        print("  Skipping due to build failure")

    os.chdir(prev_wd)

def open_napkin_from_packaged_app(demo_results, napkin_results, root_output_dir, timestamp):
    """Run Napkin from packaged app without opening app

    Parameters
    ----------
    demo_results : dict
        Results for demos
    napkin_results : dict
        Results for Napkin
    root_output_dir : str
        Directory where packaged apps were moved to
    timestamp : str
        Timestamp of the test run
    """

    # Get the name of the demo that Napkin was packaged with
    app_name = None
    for demo_name, this_demo in demo_results.items():
        if 'package' in this_demo and this_demo['package']['success']:
            app_name = demo_name
            break

    if app_name is None:
        print("  Failed to find packaged app to test against")
        return

    containing_dir = os.path.abspath(os.path.join(root_output_dir, '%s-%s-napkin' % (app_name, timestamp)))
    os.chdir(os.path.join(containing_dir, 'napkin'))

    # Run demo from packaged app
    print("- Run Napkin from packaged app...")
    demo_app_json = os.path.join(os.pardir, APP_FILENAME)
    (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop('./napkin --no-project-reopen',
                                                                                    os.path.abspath(os.pardir),
                                                                                    True)

    napkin_results['runFromPackagedOutput'] = {}
    napkin_results['runFromPackagedOutput']['success'] = success
    napkin_results['runFromPackagedOutput']['stdout'] = stdout
    napkin_results['runFromPackagedOutput']['stderr'] = stderr
    napkin_results['runFromPackagedOutput']['unexpectedLibraries'] = unexpected_libs
    if not success:
        napkin_results['runFromPackagedOutput']['exitCode'] = return_code

    if success:
        print("  Done.")
    else:
        eprint("  Error: Napkin from package failed to run")
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)
        eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
        eprint("  Exit code: %s" % return_code)

def open_app_in_napkin_from_packaged_app(results, app_name, root_output_dir, timestamp):
    """Open app from Napkin in packaged app

    Parameters
    ----------
    results: dict
        Results for results
    app_name : str
        Name of app
    root_output_dir : str
        Directory where packaged apps were moved to
    timestamp : str
        Timestamp of the test run
    """

    # Get the name of the demo that Napkin was packaged with
    containing_dir = os.path.abspath(os.path.join(root_output_dir, '%s-%s-napkin' % (app_name, timestamp)))
    os.chdir(os.path.join(containing_dir, 'napkin'))

    # Run demo from packaged app
    print("- Open app with Napkin from packaged app...")
    demo_app_json = os.path.join(os.pardir, APP_FILENAME)
    (success, stdout, stderr, unexpected_libs, return_code) = run_process_then_stop('./napkin -p %s --exit-after-load' % demo_app_json,
                                                                                    os.path.abspath(os.pardir),
                                                                                    True,
                                                                                    True,
                                                                                    SUCCESS_EXIT_CODE,
                                                                                    NAPKIN_SECONDS_WAIT_FOR_PROCESS)

    results['openWithNapkinPackagedApp'] = {}
    results['openWithNapkinPackagedApp']['success'] = success
    results['openWithNapkinPackagedApp']['stdout'] = stdout
    results['openWithNapkinPackagedApp']['stderr'] = stderr
    results['openWithNapkinPackagedApp']['unexpectedLibraries'] = unexpected_libs

    if success:
        print("  Done.")
    else:
        eprint("  Error: Napkin from package failed to open app")
        eprint("  STDOUT: %s" % stdout)
        eprint("  STDERR: %s" % stderr)
        eprint("  Unexpected libraries: %s" % repr(unexpected_libs))
        eprint("  Exit code: %s" % return_code)

    print("----------------------------")

def open_apps_in_napkin_from_packaged_apps(demo_results, root_output_dir, timestamp):
    """Open apps in Napkin from packaged app

    Parameters
    ----------
    demo_results : dict
        Results for demos
    root_output_dir : str
        Directory where packaged apps were moved to
    timestamp : str
        Timestamp of the test run
    """

    cwd = os.getcwd()

    for demo_name, this_demo in sorted(demo_results.items()):
        # If demo didn't package skip it
        if not 'package' in this_demo or not this_demo['package']['success']:
            os.chdir(cwd)
            continue

        print("Demo: %s" % demo_name)
        open_app_in_napkin_from_packaged_app(demo_results[demo_name], demo_name, root_output_dir, timestamp)
        os.chdir(cwd)

def open_template_app_in_napkin_from_packaged_app(template_results, root_output_dir, timestamp):
    """Open template app in Napkin from packaged app

    Parameters
    ----------
    tempate_results : dict
        Results for template app
    root_output_dir : str
        Directory where packaged apps were moved to
    timestamp : str
        Timestamp of the test run
    """

    if not 'package' in template_results or not template_results['package']['success']:
        print("  Skipping due to packaging failure")
        return

    cwd = os.getcwd()
    open_app_in_napkin_from_packaged_app(template_results, TEMPLATE_APP_NAME, root_output_dir, timestamp)
    os.chdir(cwd)


def run_packaged_demos(demo_results, root_output_dir, timestamp):
    """Run demos from packaged output

    Parameters
    ----------
    demo_results : dict
        Results for demos
    root_output_dir : str
        Directory where packaged apps were moved to
    timestamp : str
        Timestamp of the test run
    """

    prev_cwd = os.getcwd()

    # Iterate demos
    for demo_name, this_demo in sorted(demo_results.items()):
        # Only run if it was packaged successfully
        if 'package' in this_demo and this_demo['package']['success']:
            print("Demo: %s" % demo_name)
            run_packaged_app(this_demo, root_output_dir, timestamp, demo_name)
            print("----------------------------")

    os.chdir(prev_cwd)

def cleanup_packaged_apps(demo_results, template_results, napkin_results, misc_results, root_output_dir, timestamp, warnings):
    """Delete packaged apps created during testing

    Parameters
    ----------
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template app
    napkin_results : dict
        Results for Napkin
    misc_results: dict
        Misc. smaller results
    root_output_dir : str
        Directory where packaged apps were moved to
    timestamp : str
        Timestamp of the test run
    warnings : list of str
        Any warnings generated throughout the testing
    """

    # Remove bulk of packaged apps
    for demo_name, this_demo in demo_results.items():
        if 'package' in this_demo and this_demo['package']['success']:
            containing_dir = os.path.join(root_output_dir, '%s-%s-napkin' % (demo_name, timestamp))
            try:
                shutil.rmtree(containing_dir)
            except OSError:
                warning = "Couldn't remove packaged app at %s during cleanup" % containing_dir
                print("  Warning: %s" % warning)
                warnings.append(warning)

    # Remove packaged app without Napkin
    results = {} if not 'packagedWithoutNapkin' in misc_results else misc_results['packagedWithoutNapkin']
    if 'package' in results and results['package']['success']:
        containing_dir = os.path.join(root_output_dir, '%s-%s-no_napkin' % (results['name'], timestamp))
        try:
            shutil.rmtree(containing_dir)
        except OSError:
            warning = "Couldn't remove packaged app at %s during cleanup" % containing_dir
            print("  Warning: %s" % warning)
            warnings.append(warning)

    # Remove packaged template app
    if 'package' in template_results and template_results['package']['success']:
        containing_dir = os.path.join(root_output_dir, '%s-%s-napkin' % (TEMPLATE_APP_NAME, timestamp))

        try:
            shutil.rmtree(containing_dir)
        except OSError:
            warning = "Couldn't remove packaged app at %s during cleanup" % containing_dir
            print("  Warning: %s" % warning)
            warnings.append(warning)

def determine_run_success(demo_results, template_results, napkin_results, misc_results, fail_on_unexpected_libs):
    """Was the whole suite successful?

    Parameters
    ----------
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template app
    napkin_results : dict
        Results for Napkin
    misc_results : dict
        Misc. smaller results
    fail_on_unexpected_libs : bool
        Whether to fail the test run if unexpected libraries are encountered

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
        elif not this_demo['runFromBuildOutput']['success']:
            return False
        elif fail_on_unexpected_libs and len(this_demo['runFromBuildOutput']['unexpectedLibraries']) > 0:
            return False
        if not 'runFromPackagedOutput' in this_demo:
            return False
        elif not this_demo['runFromPackagedOutput']['success']:
            return False
        elif fail_on_unexpected_libs and len(this_demo['runFromPackagedOutput']['unexpectedLibraries']) > 0:
            return False
        if not 'openWithNapkinBuildOutput' in this_demo or not this_demo['openWithNapkinBuildOutput']['success']:
            return False
        if not 'openWithNapkinPackagedApp' in this_demo or not this_demo['openWithNapkinPackagedApp']['success']:
            return False

    # Check template app for failure
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
    elif not template_results['runFromBuildOutput']['success']:
        return False
    elif fail_on_unexpected_libs and len(template_results['runFromBuildOutput']['unexpectedLibraries']) > 0:
        return False
    if not 'runFromPackagedOutput' in template_results:
        return False
    elif not template_results['runFromPackagedOutput']['success']:
        return False
    elif fail_on_unexpected_libs and len(template_results['runFromPackagedOutput']['unexpectedLibraries']) > 0:
        return False
    if not 'openWithNapkinBuildOutput' in template_results or not template_results['openWithNapkinBuildOutput']['success']:
         return False
    if not 'openWithNapkinPackagedApp' in template_results or not template_results['openWithNapkinPackagedApp']['success']:
         return False

    # Check other build type demo for failure
    other_build_type_results = misc_results['otherBuildType']
    if not 'generate' in other_build_type_results or not other_build_type_results['generate']['success']:
        return False
    if not 'build' in other_build_type_results or not other_build_type_results['build']['success']:
        return False
    if not 'runFromBuildOutput' in other_build_type_results:
        return False
    elif not other_build_type_results['runFromBuildOutput']['success']:
        return False
    elif fail_on_unexpected_libs and len(other_build_type_results['runFromBuildOutput']['unexpectedLibraries']) > 0:
        return False

    # Check demo packaged without napkin
    results = {} if not 'packagedWithoutNapkin' in misc_results else misc_results['packagedWithoutNapkin']
    if not 'package' in results or not results['package']['success']:
        return False
    if not 'runFromPackagedOutput' in results:
        return False
    elif not results['runFromPackagedOutput']['success']:
        return False
    elif fail_on_unexpected_libs and len(results['runFromPackagedOutput']['unexpectedLibraries']) > 0:
        return False

    # Check Napkin results for failure
    if not 'runFromFrameworkRelease' in napkin_results:
        return False
    elif not napkin_results['runFromFrameworkRelease']['success']:
        return False
    elif fail_on_unexpected_libs and len(napkin_results['runFromFrameworkRelease']['unexpectedLibraries']) > 0:
        return False
    if not 'runFromPackagedOutput' in napkin_results:
        return False
    elif not napkin_results['runFromPackagedOutput']['success']:
        return False
    elif fail_on_unexpected_libs and len(napkin_results['runFromPackagedOutput']['unexpectedLibraries']) > 0:
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

def log_single_app_summary(dict_in, log_packaging_result=True, log_napkin_result=False):
    """Log summary of testing for single app

    Parameters
    ----------
    dict_in: dict
        Results for single app
    log_packaging_result: bool
        Whether to log packaging results
    log_napkin_result: bool
        Whether to log Napkin run results
    """
    print("- Generate: %s" % dict_entry_to_success(dict_in, 'generate'))
    print("- Build: %s" % dict_entry_to_success(dict_in, 'build'))
    if log_packaging_result:
        print("- Package: %s" % dict_entry_to_success(dict_in, 'package'))
    success = dict_entry_to_success(dict_in, 'runFromBuildOutput')
    print("- Run from build output: %s" % success)
    if success == 'PASS' and not sys.platform == 'win32':
        print("- Run from build output, libs. check: %s" % dict_entry_to_libs_success(dict_in, 'runFromBuildOutput'))
    if log_packaging_result:
        success = dict_entry_to_success(dict_in, 'runFromPackagedOutput')
        print("- Run from packaged output: %s" % success)
        if success == 'PASS' and not sys.platform == 'win32':
            print("- Run from packaged output, libs. check: %s" % dict_entry_to_libs_success(dict_in, 'runFromPackagedOutput'))
    if log_napkin_result:
        print("- Open with Napkin (from framework release): %s" % dict_entry_to_success(dict_in, 'openWithNapkinBuildOutput'))
        print("- Open with Napkin (from packaged app): %s" % dict_entry_to_success(dict_in, 'openWithNapkinPackagedApp'))

def log_summary(demo_results, template_results, napkin_results, misc_results):
    """Log of a summary of the test run

    Parameters
    ----------
    demo_results : dict
        Results for demos
    template_results : dict
        Results for template app
    napkin_results : dict
        Results for Napkin
    misc_results: dict
        Misc. smaller results
    """

    for demo_name, this_demo in sorted(demo_results.items()):
        print("Demo: " + demo_name)
        log_single_app_summary(this_demo, True, True)
        print("----------------------------")

    print("Template app")
    log_single_app_summary(template_results, True, True)
    print("----------------------------")

    other_build_type_results = misc_results['otherBuildType']
    if other_build_type_results:
        print("%s build (other results are %s)" % (other_build_type_results['buildType'], APP_BUILD_TYPE.lower()))
        log_single_app_summary(other_build_type_results, False)
        print("  (was with demo '%s')" % other_build_type_results['demoName'])
    else:
        print("Other build type testing")
        print("- App selection: FAIL")
        print("  (Common cause: the testing looks for a demo with a module to use)")
    print("----------------------------")

    print("Demo packaged without Napkin")
    results = {} if not 'packagedWithoutNapkin' in misc_results else misc_results['packagedWithoutNapkin']
    print("- Package: %s" % dict_entry_to_success(results, 'package'))
    if 'name' in results:
        print("  (was with demo '%s')" % results['name'])
    success = dict_entry_to_success(results, 'runFromPackagedOutput')
    print("- Run from packaged output: %s" % success)
    if success == 'PASS':
        print("- Run from packaged output, libs. check: %s" % dict_entry_to_libs_success(results, 'runFromPackagedOutput'))
    print("----------------------------")

    print("Napkin")
    success = dict_entry_to_success(napkin_results, 'runFromFrameworkRelease')
    print("- Run from framework release without app: %s" % success)
    if success == 'PASS' and not sys.platform == 'win32':
        print("- Run from framework release, libs. check: %s" % dict_entry_to_libs_success(napkin_results, 'runFromFrameworkRelease'))
    success = dict_entry_to_success(napkin_results, 'runFromPackagedOutput')
    print("- Run from packaged output without app: %s" % success)
    if success == 'PASS' and not sys.platform == 'win32':
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
                     misc_results,
                     always_include_logs,
                     warnings,
                     excluded_apps):
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
        Results for template app
    napkin_results : dict
        Results for Napkin
    misc_results: dict
        Misc. smaller results
    always_include_logs : bool
        Whether to force inclusion logs for all processes into report, not just on failure
    warnings : list of str
        Any warnings generated throughout the testing
    excluded_apps: str
        Apps that are excluded
    """

    report = {}

    # Include summary details for whole test run
    report['run'] = {}
    report['run']['success'] = run_success
    report['run']['duration'] = formatted_duration
    report['run']['startTime'] = timestamp
    report['run']['frameworkPath'] = nap_framework_full_path
    report['run']['warnings'] = warnings
    report['run']['excluded'] = excluded_apps

    # Pull in build info
    with open(os.path.join(nap_framework_full_path, 'cmake', 'build_info.json'), 'r') as build_data:
        report['run']['frameworkBuildInfo'] = json.load(build_data)

    # Add demo results
    demo_results = copy.deepcopy(demo_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for demo_name, demo in sorted(demo_results.items()):
            for phase in ('generate', 'build', 'package', 'runFromBuildOutput', 'runFromPackagedOutput', 'openWithNapkinBuildOutput', 'openWithNapkinPackagedApp'):
                if phase in demo and demo[phase]['success']:
                    del(demo[phase]['stdout'])
                    del(demo[phase]['stderr'])
    report['demos'] = demo_results

    # Add template app results
    template_results = copy.deepcopy(template_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for phase in ('create', 'generate', 'build', 'package', 'runFromBuildOutput', 'runFromPackagedOutput', 'openWithNapkinBuildOutput', 'openWithNapkinPackagedApp'):
            if phase in template_results and template_results[phase]['success']:
                del(template_results[phase]['stdout'])
                del(template_results[phase]['stderr'])
    report['templateApp'] = template_results

    # Add Napkin results
    napkin_results = copy.deepcopy(napkin_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for phase in ('runFromBuildOutput', 'runFromPackagedOutput', 'runFromBuildOutputOtherBuildType', 'runFromFrameworkRelease'):
            if phase in napkin_results and napkin_results[phase]['success']:
                del(napkin_results[phase]['stdout'])
                del(napkin_results[phase]['stderr'])
    report['napkin'] = napkin_results

    # Add Misc. results
    misc_results = copy.deepcopy(misc_results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        results = {} if not 'packagedWithoutNapkin' in misc_results else misc_results['packagedWithoutNapkin']
        for phase in ('package', 'runFromPackagedOutput'):
            if phase in results and results[phase]['success']:
                del(results[phase]['stdout'])
                del(results[phase]['stderr'])
        results = {} if not 'otherBuildType' in misc_results else misc_results['otherBuildType']
        for phase in ('generate', 'build',  'runFromBuildOutput'):
            if phase in results and results[phase]['success']:
                del(results[phase]['stdout'])
                del(results[phase]['stderr'])
    report['misc'] = misc_results

    # Write report
    with open(os.path.join(starting_dir, REPORT_FILENAME), 'w') as f:
        f.write(json.dumps(report, indent=4, sort_keys=True))

    print("  Done.")

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
                print("* Renaming Qt directory")
                os.rename(qt_top_level_path, '%s-rename' % qt_top_level_path)
            except OSError as e:
                print("Couldn't rename %s: %s" % (qt_top_level_path, e))
                qt_top_level_path = None
        else:
            print("  Warning: Couldn't determine top-level Qt path to rename")
            warnings.append("Couldn't rename Qt due to inability to determine top-level Qt path")
            qt_top_level_path = None

    return qt_top_level_path

def patch_audio_service_configuration(app_dir, output_dir, app_name, nap_framework_full_path):
    """Patches audio service configuration to have zero input channels on any app
    using napaudio

    Parameters
    ----------
    app_dir: str
        Path to app to patch
    output_dir: str
        Directory for patched app.json and config.json
    app_name : str
        Name of app
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    modules = get_app_full_module_requirements(nap_framework_full_path, app_name, app_dir)
    if not 'napaudio' in modules:
        return

    # Create or patch the config.json
    config_filename = 'config.json'
    config_path = os.path.join(output_dir, config_filename)
    loaded_config = False
    if os.path.exists(config_path):
        with open(config_path, 'r') as f:
            config = json.load(f)
        if 'Objects' in config:
            loaded_config = True

    if not loaded_config:
        config = {'Objects':[]}

    for obj in list(config['Objects']):
        if obj['Type'] == 'nap::audio::AudioServiceConfiguration':
            config['Objects'].remove(obj)

    new_obj = {
        'Type': 'nap::audio::AudioServiceConfiguration',
        'mID': 'AudioServiceConfiguration',
        'SampleRate' : 44100,
        'OutputChannelCount' : 2,
        'AllowChannelCountFailure': 'True',
        'DisableInput': 'True'
    }
    config['Objects'].append(new_obj)

    with open(config_path, 'w') as f:
        f.write(json.dumps(config, indent=4))

    # Update the app.json
    # TODO Cater for ProjectInfos that already have a ServiceConfig entry
    #      set, potentially with another filename
    app_info_path = os.path.join(output_dir, APP_FILENAME)
    app_info = None
    if os.path.exists(app_info_path):
        with open(app_info_path, 'r') as f:
            app_info = json.load(f)

    if not app_info is None:
        app_info['ServiceConfig'] = config_filename
        with open(app_info_path, 'w') as f:
            f.write(json.dumps(app_info, indent=4))

def get_modules_used_in_all_apps(nap_framework_full_path, testing_apps_dir):
    """Fetch a list of all modules in use within the demos in the release.

    Parameters
    ----------
    nap_framework_full_path : str
        Absolute path to NAP framework
    testing_apps_dir : str
        Directory to iterate for testing, by default 'demos'
    """

    test_apps_dir = os.path.join(nap_framework_full_path, testing_apps_dir)
    dirs = os.listdir(test_apps_dir)
    modules = []
    for app_name in dirs:
        if app_name.startswith('.'):
            continue
        app_dir = os.path.join(test_apps_dir, app_name)
        modules.extend(get_app_full_module_requirements(nap_framework_full_path, app_name, app_dir))
    unique_used_modules = list(set(modules))
    return unique_used_modules

def get_modules_in_release(nap_framework_full_path):
    """Fetch a list of (non app) modules included in a release.

    Parameters
    ----------
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    modules_dir = os.path.join(nap_framework_full_path, MODULES_DIR)
    modules_in_release = os.listdir(modules_dir)
    modules_in_release.sort()
    return modules_in_release

def create_fake_apps_for_modules_without_demos(nap_framework_full_path, testing_apps_dir, warnings):
    """Creates fake apps for modules which aren't tested in any of the demos.
    At least provides some basic dependency testing.

    Parameters
    ----------
    nap_framework_full_path : str
        Absolute path to NAP framework
    testing_apps_dir : str
        Directory to iterate for testing, by default 'demos'
    warnings : list of str
        Any warnings generated throughout the testing
    """

    prev_wd = os.getcwd()

    # Fetch all the (non app) modules included in the release
    modules_in_release = get_modules_in_release(nap_framework_full_path)

    # Get the modules already in use in demos
    unique_used_modules = get_modules_used_in_all_apps(nap_framework_full_path, testing_apps_dir)

    # Determine the untested modules
    difference = list(set(modules_in_release) - set(unique_used_modules))
    difference.sort()
    print("Creating fake apps for modules without demos: %s" % ', '.join(difference))

    os.chdir(nap_framework_full_path)

    for module in difference:
        # Build a app name
        processed_name = module.replace('_', '').title()
        app_name = 'FakeDemo%s' % processed_name
        created_app_path = os.path.join('apps', app_name)
        dest_app_path = os.path.join(testing_apps_dir, app_name)

        # Remove if it already exists
        for path in (created_app_path, dest_app_path):
            if os.path.exists(path):
                warning = "App %s seems to already exists and will be replaced" % path
                print(warning)
                warnings.append(warning)
                shutil.rmtree(path)

        # Generate the app
        script = get_platform_scriptpath(os.path.join('tools', 'create_app'))
        cmd = f'{script} -ng {app_name}'
        (returncode, stdout, stderr) = call_capturing_output(cmd)
        template_creation_success = returncode == 0

        if template_creation_success:
            # Patch app.json
            app_info_path = os.path.join(created_app_path, APP_FILENAME)
            app_info = None
            if os.path.exists(app_info_path):
                with open(app_info_path, 'r') as f:
                    app_info = json.load(f)
            if not app_info is None:
                if 'napaudio' in app_info['RequiredModules']:
                    app_info['RequiredModules'].remove('napaudio')
                app_info['RequiredModules'].append(module)

                with open(app_info_path, 'w') as f:
                    f.write(json.dumps(app_info, indent=4))

            # Move the app alongside the other demos so they get automatically tested
            shutil.move(created_app_path, testing_apps_dir)
        else:
            warning = "Failed to create fake demo for module %s" % module
            print("Warning: %s" % warning)
            warnings.append(warning)

    os.chdir(prev_wd)

def perform_test_run(nap_framework_path,
                     testing_apps_dir,
                     create_json_report,
                     force_log_reporting,
                     rename_framework,
                     rename_qt,
                     create_fake_apps,
                     excluded_apps,
                     fail_on_unexpected_libs):
    """Main entry point to the testing

    Parameters
    ----------
    nap_framework_path : str
        Command line provided path to NAP framework to test
    testing_apps_dir : str
        Directory to iterate for testing, by default 'demos'
    create_json_report : bool
        Whether to create a report
    force_log_reporting : bool
        Whether to force inclusion logs for all processes into report, not just on failure
    rename_framework : bool
        Whether to rename the NAP framework directory when testing packaged apps
    rename_qt : bool
        Whether to attempt to rename any Qt library pointed to via environment variable QT_DIR when testing packaged apps
    create_fake_apps : bool
        Whether to create fake apps for modules that aren't represented in any demos
    fail_on_unexpected_libs : bool
        Whether to fail the test run if unexpected libraries are encountered

    Returns
    -------
    bool
        Success of entire run
    """

    starting_dir = os.getcwd()
    root_output_dir = os.path.abspath('.')
    nap_framework_full_path = os.path.abspath(nap_framework_path)
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    duration_start_time = time.time()
    warnings = []
    phase = 0
    # TODO Temporary global until upcoming small restructure
    global TREAT_UNEXPECTED_LIBS_AS_ERROR
    TREAT_UNEXPECTED_LIBS_AS_ERROR = fail_on_unexpected_libs

    # Check to see if our framework path looks valid
    if not os.path.exists(os.path.join(nap_framework_full_path, 'cmake', 'build_info.json')):
        eprint("Error: %s doesn't look like a valid extracted NAP framework" % nap_framework_path)
        return False

    # Warn if an existing template is about to be overwritten
    if os.path.exists(os.path.join(nap_framework_full_path, 'apps', TEMPLATE_APP_NAME)):
        print("Warning: Template app already exists at %s and will be replaced" % os.path.join(nap_framework_path, 'apps', TEMPLATE_APP_NAME))

    # Warn about not doing framework and Qt renames on *nix
    if not is_windows() and not rename_framework:
        warning = "Not renaming NAP framework may result in missing dependencies not being detected"
        print("Warning: %s" % warning)
        warnings.append(warning)
    if not is_windows() and not rename_qt:
        warning = "Not renaming Qt may result in missing dependencies not being detected"
        print("Warning: %s" % warning)
        warnings.append(warning)

    # Warn about Qt not being found for renaming
    if not is_windows() and rename_qt:
        if 'QT_DIR' in os.environ:
            if not os.path.exists(os.environ['QT_DIR']):
                warning = "Qt does not exist at path pointed to by QT_DIR env. variable. Not renaming Qt may result in missing dependencies not being detected."
                print("Warning: %s" % warning)
                warnings.append(warning)
        else:
            warning = "Env. variable QT_DIR not defined. Not renaming Qt may result in missing dependencies not being detected."
            print("Warning: %s" % warning)
            warnings.append(warning)

    # Make any needed fake dependencies apps
    if create_fake_apps:
        print("============ Phase #%s - Dummy app creation ============" % phase)
        create_fake_apps_for_modules_without_demos(nap_framework_full_path, testing_apps_dir, warnings)

    os.chdir(os.path.join(nap_framework_full_path, testing_apps_dir))

    # Configure, build and package all demos
    phase += 1
    print("============ Phase #%s - Building and packaging demos ============" % phase)
    demo_results = build_and_package(root_output_dir, timestamp, testing_apps_dir, excluded_apps)

    # Package a demo with Napkin
    phase += 1
    print("============ Phase #%s - Packaging demo without Napkin ============" % phase)
    misc_results = package_demo_without_napkin(demo_results, root_output_dir, timestamp)

    # Create, configure, build and package a app from template
    phase += 1
    print("============ Phase #%s - Creating, building and packaging app from template ============" % phase)
    os.chdir(nap_framework_full_path)
    template_results = create_build_and_package_template_app(root_output_dir, timestamp)
    os.chdir(os.path.join(nap_framework_full_path, testing_apps_dir))

    # Configure and build a demo as other build type
    other_build_type = 'Debug' if APP_BUILD_TYPE.lower() == 'release' else 'Release'
    phase += 1
    print("============ Phase #%s - Building demo as %s ============" % (phase, other_build_type.lower()))
    os.chdir(os.path.join(nap_framework_full_path, testing_apps_dir))
    build_other_build_type_demo(other_build_type, misc_results)
    if not misc_results['otherBuildType']:
        eprint("Error: Didn't build %s build type demo" % other_build_type)

    # If running as root on Linux (which is necessary to test websocket functionality) launch pulseaudio for root
    if is_linux_root():
        launch_pulseaudio()

    # Run all demos from normal build output
    phase += 1
    print("============ Phase #%s - Running demos from build output directory ============" % phase)
    os.chdir(os.path.join(nap_framework_full_path, testing_apps_dir))
    run_build_directory_demos(demo_results)

    # Run template app from normal build output
    phase += 1
    print("============ Phase #%s - Running template app from build output directory ============" % phase)
    if 'build' in template_results and template_results['build']['success']:
        os.chdir(os.path.join(nap_framework_full_path, 'apps'))
        run_build_directory_template_app(template_results, nap_framework_full_path)
        os.chdir(os.path.join(nap_framework_full_path, testing_apps_dir))
    else:
        print("Skipping due to build failure")

    # Run other build type demo
    phase += 1
    print("============ Phase #%s - Running %s build type demo ============" % (phase, other_build_type.lower()))
    other_build_type_results = misc_results['otherBuildType']
    if 'build' in other_build_type_results and other_build_type_results['build']['success']:
        run_other_build_type_demo(other_build_type_results, other_build_type)
    else:
        print("Skipping due to build failure")

    # Rename Qt (to avoid dependencies being sourced from there)
    if rename_qt:
        qt_top_level_path = rename_qt_dir(warnings)

    # Run Napkin from normal build output
    phase += 1
    print("============ Phase #%s - Opening Napkin from framework release without app ============" % phase)
    napkin_results = {}
    open_napkin_from_framework_release_without_app(napkin_results, nap_framework_full_path)

    phase += 1
    print("============ Phase #%s - Opening demos in Napkin from framework release ============" % phase)
    open_apps_in_napkin_from_framework_release(demo_results, nap_framework_full_path)

    phase += 1
    print("============ Phase #%s - Opening template app in Napkin from framework release ============" % phase)
    open_template_app_in_napkin_from_framework_release(template_results, nap_framework_full_path)

    os.chdir(starting_dir)

    # Rename NAP framework (to avoid dependencies being sourced from there)
    if rename_framework:
        print("* Renaming NAP framework")
        os.rename(nap_framework_full_path, '%s-rename' % nap_framework_full_path)

    phase += 1
    print("============ Phase #%s - Opening Napkin from packaged app without app ============" % phase)
    open_napkin_from_packaged_app(demo_results, napkin_results, root_output_dir, timestamp)

    phase += 1
    print("============ Phase #%s - Opening demos in Napkin from packaged app ============" % phase)
    # Run Napkin from packaged app
    open_apps_in_napkin_from_packaged_apps(demo_results, root_output_dir, timestamp)

    phase += 1
    print("============ Phase #%s - Opening template app in Napkin from packaged app ============" % phase)
    # Run Napkin from packaged app
    open_template_app_in_napkin_from_packaged_app(template_results, root_output_dir, timestamp)

    # Run all demos from packaged apps
    phase += 1
    print("============ Phase #%s - Running packaged demos ============" % phase)
    run_packaged_demos(demo_results, root_output_dir, timestamp)

    # Run template app from packaged apps
    phase += 1
    print("============ Phase #%s - Running packaged template app ============" % phase)
    if 'package' in template_results and template_results['package']['success']:
        run_packaged_app(template_results, root_output_dir, timestamp, TEMPLATE_APP_NAME)
    else:
        print("Skipping due to package failure")

    # Run demo packaged without Napkin
    phase += 1
    print("============ Phase #%s - Running demo packaged without Napkin ============" % phase)
    results = {} if not 'packagedWithoutNapkin' in misc_results else misc_results['packagedWithoutNapkin']
    if 'package' in results and results['package']['success']:
        run_packaged_app(results, root_output_dir, timestamp, results['name'], False)
    else:
        print("Skipping due to package failure")

    os.chdir(starting_dir)

    # Cleanup
    phase += 1
    print("============ Phase #%s - Clean up ============" % phase)
    cleanup_packaged_apps(demo_results, template_results, napkin_results, misc_results, root_output_dir, timestamp, warnings)

    # If running as root on Linux (which is necessary to test websocket functionality) kill pulseaudio for root
    if is_linux_root():
        kill_pulseaudio()

    # Revert NAP framework rename
    if rename_framework:
        print("* Renaming NAP framework back")
        os.rename('%s-rename' % nap_framework_full_path, nap_framework_full_path)

    # Revert Qt rename
    if rename_qt and not qt_top_level_path is None:
        print("* Renaming Qt directory back")
        os.rename('%s-rename' % qt_top_level_path, qt_top_level_path)

    # Determine run duration
    (minutes, seconds) = divmod(time.time() - duration_start_time, 60)
    formatted_duration = '{:0>2}m{:0>2}s'.format(int(minutes), int(seconds))

    # Determine run success
    run_success = determine_run_success(demo_results, template_results, napkin_results, misc_results, fail_on_unexpected_libs)

    # Report
    if create_json_report:
        phase += 1
        print("============ Phase #%s - Creating JSON report ============" % phase)
        dump_json_report(starting_dir,
            timestamp,
            formatted_duration,
            nap_framework_full_path,
            run_success,
            demo_results,
            template_results,
            napkin_results,
            misc_results,
            force_log_reporting,
            warnings,
            excluded_apps)

    # Log summary
    print("============ Summary ============")
    log_summary(demo_results, template_results, napkin_results, misc_results)

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
        eprint("Error: %s has issues" % nap_framework_path)

    return run_success

def fetch_architecture(nap_framework_path):
    """Retrieve release architecture from the build metadata

    Parameters
    ----------
    nap_framework_path : str
        Command line provided path to the framework

    Returns
    -------
    str
        Architecture identifier
    """

    # Default to
    arch = 'x86_64'
    with open(os.path.join(nap_framework_path, 'cmake', 'build_info.json'), 'r') as build_data:
        if 'architecture' in build_data:
            arch = build_data['architecture']
    return arch

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('NAP_FRAMEWORK_PATH', type=str,
                        help="The framework path to test against")

    parser.add_argument('--testing-apps-dir', type=str,
                        default=DEFAULT_TESTING_APPS_DIR,
                        action='store', nargs='?',
                        help="Directory to test on, relative to framework root (default %s)" % DEFAULT_TESTING_APPS_DIR)
    parser.add_argument('-nj', '--no-json-report', action='store_true',
                        help="Don't create a JSON report to %s" % REPORT_FILENAME)
    parser.add_argument('-fl', '--force-log-reporting', action='store_true',
                        help="If reporting to JSON, include STDOUT and STDERR even if there has been no issue")
    parser.add_argument('-nf', '--no-fake-apps', action='store_true',
                        help="Don't create fake apps for modules that aren't represented in any demos")
    parser.add_argument('--exclude-apps', type=str, default='', action='store',
                        help="Space separated list of apps that are excluded from testing")
    parser.add_argument('--fail-on-unexpected-libs', action='store_true',
                        help="Fail the test run if unexpected libraries are encountered")
    if not is_windows():
        parser.add_argument('-nrf', '--no-rename-framework', action='store_true',
                            help="Don't rename the NAP framework while testing packaged apps")
        parser.add_argument('-nrq', '--no-rename-qt', action='store_true',
                            help="Don't attempt to rename the Qt library dir pointed to by QT_DIR while testing packaged apps")

    args = parser.parse_args()

    # Ensure working directory is location of this file, ensures relative paths are resolved correctly
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    # Ensure package directory exists
    if not os.path.exists(args.NAP_FRAMEWORK_PATH):
        print("Package directory does not exist: {0}".format(args.NAP_FRAMEWORK_PATH))
        sys.exit(1)

    # Import python helpers
    sys.path.append(os.path.join(args.NAP_FRAMEWORK_PATH, 'tools', 'buildsystem', 'common'))
    from nap_shared import get_app_full_module_requirements

    # Don't do any NAP framework / Qt renaming on Windows
    if is_windows():
        args.no_rename_framework = True
        args.no_rename_qt = True

    success = perform_test_run(args.NAP_FRAMEWORK_PATH,
                               args.testing_apps_dir,
                               not args.no_json_report,
                               args.force_log_reporting,
                               not args.no_rename_framework,
                               not args.no_rename_qt,
                               not args.no_fake_apps,
                               args.exclude_apps,
                               args.fail_on_unexpected_libs)
    sys.exit(not success)
