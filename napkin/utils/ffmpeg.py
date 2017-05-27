import os
import subprocess

import re

__EXES = {}
__EXE_SEARCH_DIRS = [
    'C:/Program Files/ffmpeg'
]


def hasAudio(filename):
    result = __execute('%s -i "%s" -show_streams -select_streams a -loglevel error' % (__ffprobeExe(), filename))
    result = ''.join(result)
    return bool(result.strip())


def stripAudio(infilename, outfilename):
    result = list(__execute('%s -i "%s" -c copy -an "%s" -y' % (__ffmpegExe(), infilename, outfilename)))


def __findRecursively(searchDirs, filename, ignoreCase=True):
    if ignoreCase:
        filename = filename.lower()
    for d in searchDirs:
        for root, dirs, files in os.walk(d):
            path = root.split('/')
            for f in files:
                fullpath = os.path.abspath('%s/%s' % (root, f))
                if ignoreCase:
                    fullpath = fullpath.lower()
                if fullpath.endswith(filename):
                    return fullpath


def __execute(cmd):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    while True:
        line = proc.stdout.readline()
        err = proc.stderr.readline()
        if line != '':
            yield line
        elif err != '':
            yield err
        else:
            break


def __findFile(filename):
    global __EXES
    if not filename in __EXES or not os.path.exists(__EXES[filename]):
        __EXES[filename] = __findRecursively(__EXE_SEARCH_DIRS, filename)
    if not __EXES[filename]:
        raise Exception('File not found: "%s"' % filename)
    return __EXES[filename]


def __ffmpegExe():
    return __findFile('ffmpeg.exe')


def __ffprobeExe():
    return __findFile('ffprobe.exe')
