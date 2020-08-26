#!/usr/bin/env python

"""This script automates basic testing of opening projects in Napkin in the Source context"""

import argparse
import copy
import datetime
import json
from multiprocessing import cpu_count
import os
import re
from subprocess import call, Popen, PIPE, check_output
import shlex
import shutil
import signal
import sys
import time

# How long to wait for the process to run. This should be long enough that we're sure
# it will have completed initialisation.
WAIT_SECONDS_FOR_PROCESS_HEALTH = 5

# Directory to iterate for testing
DEFAULT_TESTING_PROJECTS_DIR = 'demos'

# Default build type for Napkin
DEFAULT_BUILD_TYPE = 'release'

# Main project structure filename
PROJECT_FILENAME = 'project.json'

# JSON report filename
REPORT_FILENAME = 'report.json'

# Quicker iteration when debugging this script
SCRIPT_DEBUG_ONE_PROJECT_ONLY = False

# Exit code that Napkin will 
NAPKIN_SUCCESS_EXIT_CODE = 180

# Seconds to wait for a Napkin load project and exit with expected exit code
NAPKIN_SECONDS_WAIT_FOR_PROCESS = 30

def run_process_then_stop(cmd, success_exit_code=0, wait_for_seconds=WAIT_SECONDS_FOR_PROCESS_HEALTH, expect_early_closure=False):
    """Run specified command and after the specified number of seconds check that the process is
       still running before closing it

    Parameters
    ----------
    cmd : str
        Command to run
    success_exit_code : int
        Process exit code representing success
    wait_for_seconds : int
        Seconds to wait before determining run success
    expect_early_closure: bool
        Whether process having closed before we kill it is OK

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
    # Split command on Unix
    if sys.platform != 'win32':
        cmd = shlex.split(cmd)
    p = Popen(cmd, stdout=PIPE, stderr=PIPE, env=my_env)

    # Wait for the app to initialise
    waited_time = 0
    while waited_time < wait_for_seconds and p.returncode is None:
        time.sleep(0.5)
        waited_time += 0.5
        p.poll()

    # Track success
    success = True

    if p.returncode == None:
        # Process isn't done, if we were running for Napkin that's a failure
        if expect_early_closure:
            success = False
    else:
        if expect_early_closure:
            # Process done, if the success code matches we've had a successful Napkin run
            success = p.returncode == success_exit_code
        else:
            # Check and make sure the project binary's still running
            print("  Error: Process already done?")
            (stdout, stderr) = p.communicate()
            if type(stdout) == bytes:
                stdout = stdout.decode('utf8')
                stderr = stderr.decode('utf8')
                
            return (False, stdout, stderr)

    # Send SIGTERM and wait a moment to close
    if p.returncode == None:
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

    return (success, stdout, stderr)

def run_napkin_process(project_json_file):
    """Run Napkin, reporting success

    Parameters
    ----------
    project_json_file : str
        Path to JSON file for project to open

    Returns
    -------
    success : bool
        Success
    stdout : str
        STDOUT from process
    stderr : str
        STDERR from process
    """
    return run_process_then_stop('./napkin -p %s --exit-on-failure --exit-on-success' % project_json_file, NAPKIN_SUCCESS_EXIT_CODE, NAPKIN_SECONDS_WAIT_FOR_PROCESS, True)

def open_projects_in_napkin(testing_projects_dir, binary_dir, nap_framework_full_path):
    """Open demos in Napkin

    Parameters
    ----------
    results : dict
        Results for demos
    testing_projects_dir : str
        Projects directory to test against
    binary_dir : str
        Binary output directory
    nap_framework_full_path : str
        Absolute path to NAP framework
    """

    prev_wd = os.getcwd()
    results = {}
    os.chdir(os.path.join(binary_dir, 'napkin'))

    projects_root_dir = os.path.join(nap_framework_full_path, testing_projects_dir) 
    sorted_dirs = os.listdir(projects_root_dir)
    sorted_dirs.sort()
    for demo_name in sorted_dirs:
        demo_path = os.path.join(projects_root_dir, demo_name)
        # Check if path looks sane
        if not os.path.isdir(demo_path) or demo_name.startswith('.'):
            continue

        print("Demo: %s" % demo_name)
        this_project = {}
        results[demo_name] = this_project

        project_json_file = os.path.join(projects_root_dir, demo_name, PROJECT_FILENAME)
        (success, stdout, stderr) = run_napkin_process(project_json_file)
        this_project['openWithNapkin'] = {}
        this_project['openWithNapkin']['success'] = success
        this_project['openWithNapkin']['stdout'] = stdout
        this_project['openWithNapkin']['stderr'] = stderr

        if success:
            print("  Done.")
        else:
            print("  Error: Failed to open project")
            print("  STDOUT: %s" % stdout)
            print("  STDERR: %s" % stderr)

        print("----------------------------")

        if SCRIPT_DEBUG_ONE_PROJECT_ONLY:
            break

    os.chdir(prev_wd)
    return results

def determine_run_success(results):
    """Was the whole suite successful?

    Parameters
    ----------
    results : dict
        Results

    Returns
    -------
    bool
        Success of entire run
    """

    # Check demos for failure
    for demo_name, this_demo in sorted(results.items()):
        if not 'openWithNapkin' in this_demo or not this_demo['openWithNapkin']['success']:
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

def dump_json_report(starting_dir,
                     timestamp,
                     formatted_duration,
                     nap_framework_full_path,
                     run_success,
                     results,
                     always_include_logs):
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
    results : dict
        Results
    always_include_logs : bool
        Whether to force inclusion logs for all processes into report, not just on failure
    """
    
    report = {}

    # Include summary details for whole test run
    report['run'] = {}
    report['run']['success'] = run_success    
    report['run']['duration'] = formatted_duration
    report['run']['startTime'] = timestamp
    report['run']['frameworkPath'] = nap_framework_full_path

    # Add demo results
    results = copy.deepcopy(results)
    # If we aren't forcing logs remove them from each successfully phase
    if not always_include_logs:
        for demo_name, demo in sorted(results.items()):
            for phase in ('openWithNapkin',):
                if phase in demo and demo[phase]['success']:
                    del(demo[phase]['stdout'])
                    del(demo[phase]['stderr'])
    report['projects'] = results

    # Write report
    with open(os.path.join(starting_dir, REPORT_FILENAME), 'w') as f:
        f.write(json.dumps(report, indent=4, sort_keys=True))

def perform_test_run(testing_projects_dir, build_type, create_json_report, force_log_reporting):
    """Main entry point to the testing

    Parameters
    ----------
    testing_projects_dir : str
        Directory to iterate for testing, by default 'demos'
    build_type: str
        Build type to use for Napkin
    create_json_report : bool
        Whether to create a report
    force_log_reporting : bool
        Whether to force inclusion logs for all processes into report, not just on failure

    Returns
    -------
    bool
        Success of entire run
    """

    starting_dir = os.getcwd()
    root_output_dir = os.path.abspath('.')

    nap_framework_full_path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir, os.pardir))
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    duration_start_time = time.time()

    build_conf = None
    bin_dir_root = os.path.join(nap_framework_full_path, 'bin')
    for path in os.listdir(bin_dir_root):
        if build_type.lower() in path.lower():
            build_conf = path
            break
    if build_conf is None:
        print("Could not find build output for %s in %s" % (build_type.lower(), bin_dir_root))
        return False
    binary_dir = os.path.join(bin_dir_root, build_conf)

    if not os.path.exists(os.path.join(binary_dir, 'napkin')):
        print("Error, can't find built Napkin, have you done a build all?")
        return False

    print("Framework root: %s" % nap_framework_full_path)
    print("Binary dir.: %s" % binary_dir)

    results = open_projects_in_napkin(testing_projects_dir, binary_dir, nap_framework_full_path)

    # Determine run duration
    (minutes, seconds) = divmod(time.time() - duration_start_time, 60)
    formatted_duration = '{:0>2}m{:0>2}s'.format(int(minutes), int(seconds))

    # Determine run success
    run_success = determine_run_success(results)
    
    # Report
    if create_json_report:
        print("Creating JSON report")
        dump_json_report(starting_dir,
            timestamp,
            formatted_duration,
            nap_framework_full_path,
            run_success,
            results,
            force_log_reporting)

    # Final success log
    if create_json_report:
        print("Report: %s" % REPORT_FILENAME)
    print("Duration: %s" % formatted_duration)
    if run_success:
        print("Passed all tests")
    else:
        print("Issues found")

    return run_success

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--testing-projects-dir', type=str,
                        default=DEFAULT_TESTING_PROJECTS_DIR,
                        action='store', nargs='?',
                        help="Directory to test on, relative to framework root (default %s)" % DEFAULT_TESTING_PROJECTS_DIR)
    parser.add_argument('--build-type', type=str,
                        default=DEFAULT_BUILD_TYPE,
                        action='store', nargs='?',
                        help="Directory to test on, relative to framework root (default %s)" % DEFAULT_BUILD_TYPE)
    parser.add_argument('--json-report', action='store_true',
                        help="Create a JSON report to %s" % REPORT_FILENAME)
    parser.add_argument('--force-log-reporting', action='store_true',
                        help="If reporting to JSON, include STDOUT and STDERR even if there has been no issue")
    args = parser.parse_args()

    success = perform_test_run(args.testing_projects_dir, args.build_type, args.json_report, args.force_log_reporting)
    sys.exit(not success)
