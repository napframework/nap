import os
import subprocess
from multiprocessing import cpu_count
from sys import platform
import sys
import shutil

WORKING_DIR = '.'

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
	             'python3-dev',
               'libsdl2-dev',
               'libglew-dev',
               'libassimp-dev',
               'libglm-dev',
               'libtclap-dev',
               'libfreeimage-dev',
               'ffmpeg',
               'libavcodec-dev',
               'libavformat-dev',
               'libavutil-dev',
               'doxygen'
               ])


def isBrewInstalled():
    try:
        return os.path.exists(call(WORKING_DIR, ['which', 'brew']))
    except:
        return False


def installDependenciesOSX():
    d = WORKING_DIR
    for pack in ['cmake', 'sdl2', 'glew', 'glm', 'assimp', 'tclap', 'ffmpeg', 'mpg123']:
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

# main run
if __name__ == '__main__':
    installDependencies()