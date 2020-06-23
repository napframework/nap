"""Project.info and module.info conversion script from old to new format.

TODO: Update this docstring to account for versioning

This script will either
    - take a project and convert its json descriptors to the latest format
    - or take the NAP root directory and convert all projects/modules to the latest format

See the main() function below.

"""

import json
import logging
import os
import sys
from collections import OrderedDict

LOG = logging.getLogger(os.path.basename(os.path.dirname(__file__)))


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

    return filepath, data


def _findDataFile(rootDir):
    projname = os.path.basename(rootDir)
    assert projname, 'Expected root dir to be in a project directory: %s' % rootDir
    dataDir = os.path.join(rootDir, 'data')

    # check for data dir first
    if not os.path.exists(dataDir):
        print('Expected data dir: %s' % dataDir)
        return

    # attempt to find common files
    guesses = (
        os.path.join(dataDir, '%s.json' % projname),
        os.path.join(dataDir, 'data.json'),
    )
    return next((g for g in guesses if os.path.exists(g)), None)


def convertModuleInfo(directory):
    """Find a moduleinfo file in the specified directory, convert to new format and write to same file"""
    filepath, modInfoJson = _loadJSON(directory, 'module.json')
    if not filepath:
        return

    if any(t not in modInfoJson for t in ('Type', 'mID')):
        print('Converting: %s' % filepath)

        newModInfoJson = OrderedDict((
            ('Type', 'nap::ModuleInfo'),
            ('mID', 'ModuleInfo'),
            ('RequiredModules', modInfoJson.get('dependencies', [])),
        ))

    else:
        newModInfoJson = modInfoJson

    with open(filepath, 'w') as fp:
        json.dump(newModInfoJson, fp, indent=4)


def convertProjectInfo(directory):
    """Find a projectinfo file in the specified directory, convert to new format and write to same file.
    This will also attempt to merge and convert any existing service configurations into the same file
    and delete the original config.json
    """
    filepath, projInfoJson = _loadJSON(directory, 'project.json')
    if not projInfoJson:
        return

    # has this file been converted yet?
    if any(t not in projInfoJson for t in ('Type', 'mID')):
        print('Converting: %s' % filepath)

        newProjInfoJson = OrderedDict((
            ('Type', 'nap::ProjectInfo'),
            ('mID', 'ProjectInfo'),
            ('Title', projInfoJson.get('title')),
            ('Version', projInfoJson.get('version')),
            ('RequiredModules', projInfoJson.get('modules', [])),
            ('ServiceConfig', list(_convertServiceConfig(directory))),
        ))

    else:
        newProjInfoJson = projInfoJson

    # -- ensure some values
    newProjInfoJson.setdefault('PathMapping', 'cache/path_mapping.json')
    newProjInfoJson.setdefault('Data', _findDataFile(directory) or '')

    with open(filepath, 'w') as fp:
        json.dump(newProjInfoJson, fp, indent=4)


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
