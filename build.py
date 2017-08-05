import os
import subprocess
from multiprocessing import cpu_count
from sys import platform

WORKING_DIR = os.path.dirname(__file__)

THIRDPARTY = 'thirdparty'
THIRDPARTY_DIR = '%s/thirdparty' % WORKING_DIR
THIRDPARTY_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/thirdparty.git'


def isLocalGitRepo(d):
    if not os.path.exists(d): return False
    try:
        call(d, ['git', 'rev-parse'])
    except:
        return False
    return True


def call(cwd=WORKING_DIR, cmd=None):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd)
    proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)


def installDependenciesLinux():
    d = WORKING_DIR
    call(d, ['sudo', 'apt-get', 'install',
             'cmake',
             'clang-3.8',
             ])


def installDependenciesOSX():
    d = WORKING_DIR
    # https://stackoverflow.com/questions/25535407/bypassing-prompt-to-press-return-in-homebrew-install-script
    # ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" </dev/null
    call(d, ['ruby', '-e', '"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"'])

    call(d, ['brew', 'install',
             'cmake'
             ])


def installDependencies():
    print('Installing dependencies')
    if platform == "linux" or platform == "linux2":
        installDependenciesLinux()
    elif platform == "darwin":
        installDependenciesOSX()
    elif platform == "win32":
        # Windows...
        pass


def main():
    # installDependencies()
    print('Updating repo')
    d = WORKING_DIR
    call(d, ['git', 'fetch', '--all'])

    print('Refreshing: %s' % THIRDPARTY)
    d = THIRDPARTY_DIR
    if not isLocalGitRepo(d):
        call(None, ['git', 'clone', THIRDPARTY_URL])
    else:
        call(d, ['git', 'fetch', '--all'])
        call(d, ['git', 'reset', '--hard', 'master'])

    print('Building RTTR')
    d = '%s/rttr' % THIRDPARTY_DIR
    call(d, ['cmake', '.'])
    call(d, ['make', 'install', '-j%s' % cpu_count()])

    print('Building NAPCore')
    d = WORKING_DIR
    call(d, ['cmake', '.'])
    call(d, ['make', 'napcore', '-j%s' % cpu_count()])


if __name__ == '__main__':
    main()
