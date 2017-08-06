import os
import subprocess
from multiprocessing import cpu_count
from sys import platform

WORKING_DIR = '.'

THIRDPARTY = 'thirdparty'
THIRDPARTY_DIR = '%s/thirdparty' % WORKING_DIR
THIRDPARTY_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/thirdparty.git'
NAP_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/nap.git'
NAP_BRANCH = 'build'
BUILD_DIR = 'build'

def isLocalGitRepo(d):
    if not os.path.exists(d): return False
    try:
        call(d, ['git', 'rev-parse'])
    except:
        return False
    return True


def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out


def installDependenciesLinux():
    d = WORKING_DIR
    call('.', ['sudo', 'apt-get', '--assume-yes', 'install',
               'cmake',
               'build-essential',
               'libsdl2-dev',
               'libglew-dev',
               'libassimp-dev',
               'libglm-dev',
               'libtclap-dev',
               'libfreeimage-dev',
               ])


def isBrewInstalled():
    try:
        return os.path.exists(call(WORKING_DIR, ['which', 'brew']))
    except:
        return False

def installDependenciesOSX():
    d = WORKING_DIR
    for pack in ['cmake', 'sdl2', 'glew', 'glm', 'assimp', 'tclap']:
        try:
            call(d, ['brew', 'install', pack])
        except:
            print('Failed installing %s' % pack)


def installDependencies():
    print('Installing dependencies')
    if platform == "linux" or platform == "linux2":
        installDependenciesLinux()
    elif platform == "darwin":
        installDependenciesOSX()
    elif platform == "win32":
        # Windows...
        pass


def gitPull():
    print('Updating repo')
    d = WORKING_DIR
    call(d, ['git', 'pull', NAP_URL])


def main():
    installDependencies()

    print('Refreshing: %s' % THIRDPARTY)
    d = THIRDPARTY_DIR
    if not isLocalGitRepo(d):
        call(None, ['git', 'clone', THIRDPARTY_URL])
    else:
        call(d, ['git', 'pull'])

    print('Building RTTR')
    d = '%s/rttr' % THIRDPARTY_DIR
    if platform in ["linux", "linux2", "darwin"]:
        call(d, ['cmake', '.'])
        call(d, ['make', 'install', '-j%s' % cpu_count()])
    else:
        call(d, ['cmake', '--build', '.', '--target', 'install'])

    print('Building')

    # targets = ['napcore', 'rendertest', 'steef', 'serializationtest']
    targets = ['napcore', 'serializationtest']
    if platform in ["linux", "linux2", "darwin"]:
        call(WORKING_DIR, ['cmake', '-H.', '-Bbuild'])
    else:
        bd = '%s/build64' % WORKING_DIR
        if not os.path.exists(bd): os.makedirs(bd)
        call(bd, ['cmake', '-G', 'Visual Studio 14 2015 Win64', '..'])

    for t in targets:
        if platform in ["linux", "linux2", "darwin"]:
            d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
            call(d, ['make', t, '-j%s' % cpu_count()])
        else:
            d = WORKING_DIR
            call(d, ['cmake', '--build', 'build64', '--target', t])

if __name__ == '__main__':
    main()
