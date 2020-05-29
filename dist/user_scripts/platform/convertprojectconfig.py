"""Project.info and module.info conversion script from old to new format.

TODO: Update this docstring to account for versioning

This script will either
    - take a project and convert its json descriptors to the latest format
    - or take the NAP root directory and convert all projects/modules to the latest format

See the main() function below.

"""

import json
import os
import sys

def _convertModuleRef(modulename):
    return {
        'Type': 'nap::ModuleInfo',
        'mID': modulename,
        'ModuleName': modulename,
    }


def _convertServiceConfig(directory):
    filename = os.path.join(directory, 'config.json')
    if not os.path.exists(filename):
        return

    with open(filename, 'r') as fp:
        serviceconfig = json.load(fp)

    for obj in serviceconfig.get('Objects', []):
        yield obj

    os.remove(filename)


def _loadJSON(directory, filename):
    if not os.path.exists(directory):
        raise IOError('Directory not found: %s' % directory)

    filepath = os.path.join(directory, filename)
    if not os.path.exists(filepath):
        print('File not found: %s' % filepath)
        return None, None

    with open(filepath, 'r') as fp:
        data = json.load(fp)

    if _hasBeenConverted(data):
        print('Already converted: %s' % filepath)
        return None, None

    return filepath, data


def convertModuleInfo(directory):
    """Find a moduleinfo file in the specified directory, convert to new format and write to same file"""
    filepath, moduleinfo = _loadJSON(directory, 'module.json')
    if not filepath:
        return

    print('Converting: %s' % filepath)

    newModuleInfo = {
        'Type': 'nap::ModuleInfo',
        'mID': 'ModuleInfo',
        'Dependencies': moduleinfo.get('dependencies', [])
    }

    with open(filepath, 'w') as fp:
        json.dump(newModuleInfo, fp, indent=4)


def convertProjectInfo(directory):
    """Find a projectinfo file in the specified directory, convert to new format and write to same file.
    This will also attempt to merge and convert any existing service configurations into the same file
    and delete the original config.json
    """
    filepath, projectinfo = _loadJSON(directory, 'project.json')
    if not projectinfo:
        return

    print('Converting: %s' % filepath)

    newProjectInfo = {
        'Type': 'nap::ProjectInfo',
        'mID': 'ProjectInfo',
        'Title': projectinfo.get('title'),
        'Version': projectinfo.get('version'),
        'Modules': projectinfo.get('modules', []),
        'ServiceConfigurations': list(_convertServiceConfig(directory)),
    }

    with open(filepath, 'w') as fp:
        json.dump(newProjectInfo, fp, indent=4)


def _hasBeenConverted(data):
    # new version must have these two keys
    return 'Type' in data and 'mID' in data

def convertProject(projectdir):
    convertProjectInfo(projectdir)

    moduledir = os.path.join(projectdir, 'module')
    if os.path.exists(moduledir):
        convertModuleInfo(moduledir)

def convertRepository(rootdirectory):
    print('Convert project and moduleinfo files in nap repository: %s' % rootdirectory)
    projectdirs = [
        'apps',
        'demos',
        'test',
    ]
    for p in projectdirs:
        parentdir = os.path.join(rootdirectory, p)
        for d in os.listdir(parentdir):
            directory = os.path.join(parentdir, d)
            convertProject(directory)

    moduledirs = [
        'modules'
    ]
    for p in moduledirs:
        parentdir = os.path.join(rootdirectory, p)
        for d in os.listdir(parentdir):
            directory = os.path.join(parentdir, d)
            convertModuleInfo(directory)

def main():
    if len(sys.argv) < 2:
        raise Exception('No project directory given')

    # TODO: Find some other means of determining the root dir here
    napRootDir = os.path.realpath('%s/../../..' % os.path.dirname(__file__))
    assert os.path.exists(napRootDir), 'NAP root dir not found at: %s' % napRootDir

    convertRepository(napRootDir)
    # convertProject(sys.argv[1])


if __name__ == '__main__':
    main()
