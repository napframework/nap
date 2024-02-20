#!/usr/bin/env python3

import argparse
import datetime
import json
from multiprocessing import cpu_count
import os
from subprocess import call
from sys import platform
import sys
import shutil
import xml.etree.cElementTree as et
import json

from nap_shared import find_app, call_except_on_failure, get_cmake_path

WORKING_DIR = '.'
PACKAGING_DIR = 'packaging'
ARCHIVING_DIR = 'archiving'
BUILDINFO_FILE = 'build_info.json'
APP_INFO_FILE = 'app.json'
PACKAGED_BUILD_TYPE = 'Release'

# Exit codes
ERROR_BAD_INPUT = 1
ERROR_MISSING_APP = 2
ERROR_INVALID_APP_JSON = 3

def package_app(search_app_name, show_created_package, include_napkin, zip_package, bundle, codesign):
    # Find the app
    (app_path, _) = find_app(search_app_name)
    if app_path is None:
        return ERROR_MISSING_APP

    # Process the app info to get app full name and version
    (app_version, app_full_name) = process_app_info(app_path)
    if app_version is None:
        return ERROR_INVALID_APP_JSON

    print("Packaging %s v%s" % (app_full_name, app_version))

    app_name_lower = search_app_name.lower()

    # Build directory names
    script_path = os.path.realpath(__file__)
    nap_root = os.path.abspath(os.path.join(os.path.dirname(script_path), os.pardir))
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    local_bin_dir_name = 'bin_package'
    bin_dir = os.path.join(app_path, local_bin_dir_name)
    build_dir_name = os.path.join(app_path, 'build_package')
    cmake = get_cmake_path()

    # Remove old packaging path if it exists
    if os.path.exists(bin_dir):
        shutil.rmtree(bin_dir, True)
    if os.path.exists(build_dir_name):
        shutil.rmtree(build_dir_name, True)

    if platform.startswith('linux'):
        # Generate makefiles
        call_except_on_failure(WORKING_DIR, [cmake,
                                             '-H%s' % app_path,
                                             '-B%s' % build_dir_name,
                                             '-DNAP_PACKAGED_APP_BUILD=1',
                                             '-DCMAKE_BUILD_TYPE=%s' % PACKAGED_BUILD_TYPE,
                                             '-DPACKAGE_NAPKIN=%s' % int(include_napkin)])

        # Build & install to packaging dir
        call_except_on_failure(build_dir_name, ['make', 'all', 'install', '-j%s' % cpu_count()])

        # Create archive
        if zip_package:
            packaged_to = archive_to_linux_tar_bz2(timestamp, bin_dir, app_full_name, app_version)
        else:
            packaged_to = archive_to_timestamped_dir(timestamp, bin_dir, app_full_name, app_version, 'Linux')

        # Running from X/Wayland session
        gui_session = ('DISPLAY' or 'WAYLAND_DISPLAY') in os.environ

        # Show in Nautilus
        if show_created_package and gui_session:
            # Configurable command to show resulting file
            show_command = os.getenv('NAP_SHOW_FILE_COMMAND', 'nautilus -s %PACKAGE_PATH%')
            show_command = '%s > /dev/null 2>&1 &' % show_command.replace('%PACKAGE_PATH%', packaged_to)

            call([show_command], shell=True)
            # call(["nautilus -s %s > /dev/null 2>&1 &" % packaged_to], shell=True)

    elif platform == 'darwin':
        # Generate project
        call_except_on_failure(WORKING_DIR, [cmake,
                                             '-H%s' % app_path,
                                             '-B%s' % build_dir_name,
                                             '-G', 'Xcode',
                                             '-DNAP_PACKAGED_APP_BUILD=1',
                                             '-DPACKAGE_NAPKIN=%s' % int(include_napkin),
                                             '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'])

        # Build & install to packaging dir
        call_except_on_failure(build_dir_name, ['xcodebuild', '-configuration', PACKAGED_BUILD_TYPE, '-target', 'install'])

        if bundle:
            # Create app bundle
            packaged_to = create_macos_bundle(bin_dir, app_full_name, app_name_lower, app_version, codesign)
        else:
            # Create archive
            if zip_package:
                packaged_to = archive_to_macos_zip(timestamp, bin_dir, app_full_name, app_version)
            else:
                packaged_to = archive_to_timestamped_dir(timestamp, bin_dir, app_full_name, app_version, 'macOS')

        # Show in Finder
        if show_created_package:
            call(["open", "-R", packaged_to])
    else:
        # Generate project
        call_except_on_failure(WORKING_DIR, [cmake,
                                             '-H%s' % app_path,
                                             '-B%s' % build_dir_name,
                                             '-G', 'Visual Studio 16 2019',
                                             '-DNAP_PACKAGED_APP_BUILD=1',
                                             '-DAPP_PACKAGE_BIN_DIR=%s' % local_bin_dir_name,
                                             '-DPACKAGE_NAPKIN=%s' % int(include_napkin)])

        # Build & install to packaging dir
        call_except_on_failure(build_dir_name, [cmake, '--build', '.', '--target', 'install', '--config', PACKAGED_BUILD_TYPE])

        # Remove pdbs for distribution
        for root, dirs, files in os.walk(local_bin_dir_name):
            for file in files:
                if file.lower().endswith('.pdb'):
                    os.remove(os.path.join(root, file))

        # Create archive
        if zip_package:
            packaged_to = archive_to_win64_zip(timestamp, bin_dir, app_full_name, app_version)
        else:
            packaged_to = archive_to_timestamped_dir(timestamp, bin_dir, app_full_name, app_version, 'Win64')

        # Show in Explorer
        if show_created_package:
            call(r'explorer /select,"%s"' % packaged_to)

    # Cleanup
    shutil.rmtree(build_dir_name, True)

    # Clean exit code
    return 0

# Create build archive to bzipped tarball on Linux
def archive_to_linux_tar_bz2(timestamp, bin_dir, app_full_name, app_version):
    package_filename = build_package_filename(app_full_name, app_version, 'Linux', timestamp)
    package_filename_with_ext = '%s.%s' % (package_filename, 'tar.bz2')

    # Populate build info into the app
    populate_build_info_into_app(bin_dir, timestamp)

    app_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))
    archive_dir = os.path.join(app_dir, package_filename)

    shutil.move(bin_dir, archive_dir)

    # Archive
    print("Archiving to %s.." % package_filename_with_ext)
    call_except_on_failure(app_dir, ['tar', '-cjvf', package_filename_with_ext, package_filename])

    # Cleanup
    shutil.rmtree(archive_dir)

    # Cleanup
    packaged_to = os.path.join(app_dir, package_filename_with_ext)
    print("Packaged to %s" % packaged_to)
    return os.path.relpath(packaged_to)

def create_macos_bundle(bin_dir, app_full_name, app_name_lower, app_version, codesign):
    # Populate build info into the app
    app_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))

    appJsonFileName = "app.json"
    appJsonPath = os.path.join(app_dir, appJsonFileName)
    appJsonFile = open(appJsonPath)
    appJson = json.load(appJsonFile)
    dataFilePath = appJson["Data"]
    dataDirectory = os.path.dirname(dataFilePath)
    appJsonFile.close()

    # Create app bundle
    app_bundle_dir = os.path.join(app_dir, app_full_name + " " + app_version + ".app")
    if os.path.exists(app_bundle_dir):
        shutil.rmtree(app_bundle_dir)
    os.mkdir(app_bundle_dir)
    app_contents_dir = os.path.join(app_bundle_dir, "Contents")
    os.mkdir(app_contents_dir)
    app_macos_dir = os.path.join(app_contents_dir, "MacOS")
    os.mkdir(app_macos_dir)
    app_resources_dir = os.path.join(app_contents_dir, "Resources")
    os.mkdir(app_resources_dir)
    app_resources_lib_dir = os.path.join(app_resources_dir, "lib")
    os.mkdir(app_resources_lib_dir)
    shutil.copy(os.path.join(bin_dir, app_name_lower), os.path.join(app_macos_dir, app_name_lower)) # copy executable
    shutil.copytree(os.path.join(bin_dir, "cache"), os.path.join(app_macos_dir, "cache")) # copy cache
    shutil.copytree(os.path.join(bin_dir, "lib"), os.path.join(app_macos_dir, "lib")) # copy libraries
    shutil.copytree(os.path.join(bin_dir, dataDirectory), os.path.join(app_resources_dir, dataDirectory)) # copy data folder
    shutil.copy(os.path.join(bin_dir, appJsonFileName), os.path.join(app_macos_dir, appJsonFileName)) # copy project.json
    shutil.copytree(os.path.join(bin_dir, "licenses"), os.path.join(app_resources_dir, "licenses")) # copy licenses
    shutil.copy(os.path.join(bin_dir, "NAP.txt"), os.path.join(app_resources_dir, "NAP.txt")) # copy NAP.txt
    shutil.copytree(os.path.join(bin_dir, "system_modules"), os.path.join(app_resources_dir, "system_modules")) # copy module data
    python_src_path = os.path.join(app_macos_dir, "lib/python3.11")
    if os.path.isdir(python_src_path):
        shutil.move(python_src_path, os.path.join(app_resources_lib_dir, "python3.11")) # copy python modules

    # Change the path to the data json file in app.json
    appJson["Data"] = os.path.join("../Resources", dataFilePath)
    appJsonFile = open(os.path.join(app_macos_dir, appJsonFileName), "w")
    json.dump(appJson, appJsonFile, indent = 4)
    appJsonFile.close()

    # Cleanup
    shutil.rmtree(bin_dir)

    # Try to import plist data from macos folder
    input_keys = []
    input_strings = []
    input_plist_path = os.path.join(app_dir, "macos/Info.plist")
    if os.path.isfile(input_plist_path):
        input_plist = et.parse(input_plist_path)
        if input_plist is not None:
            input_root = input_plist.getroot()
            for key in input_root.iter('key'):
                input_keys.append(key.text)
            for str in input_root.iter('string'):
                input_strings.append(str.text)

    # Create plist tree
    plist = et.Element('plist', {'version': '1.0'})
    dict = et.SubElement(plist, 'dict')
    addPlistValue(dict, "CFBundleGetInfoString", app_name_lower)
    addPlistValue(dict, "CFBundleExecutable", app_name_lower)
    bundle_identifier = "com." + app_name_lower + ".napframework.www"
    addPlistValue(dict, "CFBundleIdentifier", bundle_identifier)
    addPlistValue(dict, "CFBundleName", app_name_lower)
    addPlistValue(dict, "CFBundleInfoDictionaryVersion", "6.0")
    addPlistValue(dict, "CFBundlePackageType", "APPL")
    addPlistValue(dict, "CFBundleVersion", app_version)
    addPlistValue(dict, "NSHighResolutionCapable", "True")

    # Merge input plist data
    for i in range(len(input_keys)):
        addPlistValue(dict, input_keys[i], input_strings[i])

    # Write plist to file
    with open(os.path.join(app_contents_dir, "Info.plist"), "wb") as f:
        et.ElementTree(plist).write(f, encoding='utf-8', xml_declaration=True, default_namespace=None, method='xml', short_empty_elements=False)

    print("Packaged to %s" % app_bundle_dir)

    # Codesign the app bundle
    if not codesign is None:
        print("Codesigning app bundle with signature %s" % codesign)
        cmd = ['codesign', '--deep', '-s', codesign, '-f', '--timestamp', '--signature-size=12000', '-i', bundle_identifier, app_bundle_dir]
        call(cmd)

    # make disk image
    # print("Create disk image")
    # diskimage_source_dir = "DiskImageSource"
    # os.mkdir(diskimage_source_dir)
    # shutil.move(app_bundle_dir, diskimage_source_dir)
    # disk_image_name = app_full_name + " " + app_version
    # disk_image_path = os.path.join(app_dir, disk_image_name  + ".dmg")
    # cmd = ['hdiutil', 'create', '-srcFolder', diskimage_source_dir, '-o', disk_image_name]
    # call(cmd)
    # shutil.rmtree(diskimage_source_dir)

    return app_bundle_dir

# Create build archive to zip on macOS
def archive_to_macos_zip(timestamp, bin_dir, app_full_name, app_version):
    package_filename = build_package_filename(app_full_name, app_version, 'macOS', timestamp)
    package_filename_with_ext = '%s.%s' % (package_filename, 'zip')

    # Populate build info into the app
    populate_build_info_into_app(bin_dir, timestamp)

    app_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))
    archive_dir = os.path.join(app_dir, package_filename)

    shutil.move(bin_dir, archive_dir)

    # Remove unwanted files (eg. .DS_Store)
    call_except_on_failure(archive_dir, ['find', '.', '-name', '.DS_Store', '-type', 'f', '-delete'])

    # Archive
    print("Archiving to %s.." % package_filename_with_ext)
    call_except_on_failure(app_dir, ['zip', '-yr', package_filename_with_ext, package_filename])

    # Cleanup
    shutil.rmtree(archive_dir)

    packaged_to = os.path.join(app_dir, package_filename_with_ext)
    print("Packaged to %s" % packaged_to)
    return os.path.relpath(packaged_to)

# Create build archive to zip on Win64
def archive_to_win64_zip(timestamp, bin_dir, app_full_name, app_version):
    package_filename = build_package_filename(app_full_name, app_version, 'Win64', timestamp)
    package_filename_with_ext = '%s.%s' % (package_filename, 'zip')

    # Populate build info into the app
    populate_build_info_into_app(bin_dir, timestamp)

    app_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))

    # Create our archive dir, used to create a copy level folder within the archive
    archiving_parent_path = os.path.join(app_dir, ARCHIVING_DIR)
    if os.path.exists(archiving_parent_path):
        shutil.rmtree(archiving_parent_path, True)
    os.makedirs(archiving_parent_path)
    archive_path = os.path.join(archiving_parent_path, package_filename)
    shutil.move(bin_dir, archive_path)

    # Create archive
    print("Archiving to %s.." % package_filename_with_ext)
    shutil.make_archive(os.path.join(app_dir, package_filename), 'zip', archiving_parent_path)

    # Cleanup
    shutil.rmtree(archiving_parent_path)

    packaged_to = os.path.join(app_dir, package_filename_with_ext)
    print("Packaged to %s" % packaged_to)
    return os.path.relpath(packaged_to)

# Just package to a directory
def archive_to_timestamped_dir(timestamp, bin_dir, app_full_name, app_version, platform):
    package_filename = build_package_filename(app_full_name, app_version, platform, timestamp)

    # Populate build info into the app
    populate_build_info_into_app(bin_dir, timestamp)

    app_dir = os.path.abspath(os.path.join(bin_dir, os.pardir))
    archive_dir = os.path.join(app_dir, package_filename)

    shutil.move(bin_dir, archive_dir)

    # Cleanup
    packaged_to = os.path.join(app_dir, package_filename)
    print("Packaged to directory %s" % packaged_to)
    return packaged_to

# Build the name of our package and populate our JSON build info file
def build_package_filename(app_name, app_version, platform, timestamp):
    package_filename = "%s-%s-%s-%s" % (app_name, app_version, platform, timestamp)
    return package_filename

# Process our JSON app info: Fetch our version and full app name
def process_app_info(app_path):
    with open(os.path.join(app_path, APP_INFO_FILE)) as json_file:
        try:
            app_info = json.load(json_file)
        except ValueError as e:
            print("Invalid app.json: %s" % e)
            return (None, None)

    # Some simple validation on loaded JSON
    if not 'Version' in app_info:
        print("Missing 'Version' in %s" % APP_INFO_FILE)
        return (None, None)
    if not 'Title' in app_info:
        print("Missing 'Title' in %s" % APP_INFO_FILE)
        return (None, None)

    # Read our version
    version = app_info['Version']
    full_app_name = app_info['Title']

    # Return version for population into package name
    return (version, full_app_name)

# Populate build information into our packaged app JSON
def populate_build_info_into_app(app_package_path, timestamp):
    # Read our app info
    app_info_file = os.path.join(app_package_path, APP_INFO_FILE)
    with open(app_info_file) as json_file:
        app_info = json.load(json_file)

    # Populate NAP release info into app info, for now
    nap_root = os.path.join(app_package_path, os.pardir, os.pardir, os.pardir)
    nap_release_info_file = os.path.join(nap_root, 'cmake', BUILDINFO_FILE)
    with open(nap_release_info_file) as json_file:
        app_info['napReleaseInfo'] = json.load(json_file)

    # Add app timestamp and write back out
    app_info['buildTimestamp'] = timestamp
    with open(app_info_file, 'w') as outfile:
        json.dump(app_info, outfile, sort_keys=True, indent=2)

# Add a key/value pair to an OSX plist xml element
def addPlistValue(element, key, value):
    keyElement = et.SubElement(element, "key")
    keyElement.text = key
    valueElement = et.SubElement(element, "string")
    valueElement.text = value

# Main
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("APP_NAME", type=str,
                        help="The app to package")
    parser.add_argument("-ns", "--no-show", action="store_true",
                        help="Don't show the generated package")
    parser.add_argument("-nn", "--no-napkin", action="store_true",
                        help="Don't include napkin")
    parser.add_argument("-nz", "--no-zip", action="store_true",
                        help="Don't zip package")
    parser.add_argument("-b", "--bundle", action="store_true",
                        help="Create MacOS bundle")
    parser.add_argument("-cs", "--codesign", type=str, required=False,
                        help="Codesign app bundle")
    args = parser.parse_args()

    # Package our build
    exit_code = package_app(args.APP_NAME, not args.no_show, not args.no_napkin, not args.no_zip, args.bundle, args.codesign)
    sys.exit(exit_code)