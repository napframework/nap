import os
import subprocess

from git.repo.base import Repo
from multiprocessing import cpu_count

WORKING_DIR = os.path.dirname(__file__)

THIRDPARTY = 'thirdparty'
THIRDPARTY_DIR = '%s/../thirdparty' % WORKING_DIR
THIRDPARTY_URL = 'https://ae53bb936bc44bbffbac2dbd1f37101838603903@github.com/naivisoftware/thirdparty.git'



def isThirdpartyCloned():
    if not os.path.exists(THIRDPARTY_DIR): return False
    try: Repo(THIRDPARTY_DIR)
    except: return False
    return True


def call(cwd, cmd):
    print('dir: %s' % cwd)
    print('cmd: %s' % cmd)
    proc = subprocess.Popen(cmd, cwd=cwd)
    proc.communicate()
    if proc.returncode != 0:
        exit(proc.returncode)


if __name__ == '__main__':

    print('Refreshing: %s' % THIRDPARTY)
    d = THIRDPARTY_DIR
    if not isThirdpartyCloned():
        call(d, ['git', 'clone', THIRDPARTY_URL])
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