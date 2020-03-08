import argparse
import json
import os
import sys


def loadServiceConfig(filename):
    pass
    # if not os.path.exists()


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


def convertModuleInfo(directory):
    if not os.path.exists(directory):
        raise IOError('Directory not found: %s' % directory)
    filename = os.path.join(directory, 'module.json')
    with open(filename, 'r') as fp:
        moduleinfo = json.load(fp)

    newModuleInfo = {
        'Objects': [
            {
                'Type': 'nap::ModuleInfo',
                'mID': 'ModuleInfo',
                'Dependencies': moduleinfo.get('dependencies', [])
            }
        ]
    }

    print(json.dumps(newModuleInfo, indent=4))


def convertProjectInfo(directory):
    if not os.path.exists(directory):
        raise IOError('Directory not found: %s' % directory)
    filename = os.path.join(directory, 'project.json')
    with open(filename, 'r') as fp:
        projectinfo = json.load(fp)

    newProjectInfo = {
        'Objects': [
            {
                'Type': 'nap::ProjectInfo',
                'mID': 'ProjectInfo',
                'Title': projectinfo.get('title'),
                'Version': projectinfo.get('version'),
                'Modules': projectinfo.get('modules', []),
                'ServiceConfigurations': list(_convertServiceConfig(directory)),
            }
        ]
    }

    print(json.dumps(newProjectInfo, indent=4))


def main():
    if len(sys.argv) < 2:
        raise Exception('No project directory given')

    projectdir = sys.argv[1]
    convertProjectInfo(sys.argv[1])


    moduledir = os.path.join(projectdir, 'module')
    convertModuleInfo(moduledir)




if __name__ == '__main__':
    main()
