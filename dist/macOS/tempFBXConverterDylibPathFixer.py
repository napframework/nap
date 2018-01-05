#!/usr/bin/python
from subprocess import Popen

FBXCONVERTER_PATH = 'tools/fbxconverter'
BUILD_TYPE_FOR_FBXCONVERTER = 'Release'

def call(cmd):
    proc = Popen(cmd, shell=True)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise Exception(proc.returncode)
    return out

def update_single_link(search, replace, target):
    print ("Upating link to %s in %s" % (search, target))
    cmd = "install_name_tool -change %s %s %s" % (search, replace, target)
    call([cmd])

if __name__ == '__main__':
    update_single_link('@rpath/libmod_naprender.dylib', '@loader_path/../modules/mod_naprender/lib/%s/libmod_naprender.dylib' % BUILD_TYPE_FOR_FBXCONVERTER, FBXCONVERTER_PATH)
    update_single_link('@rpath/libmod_napmath.dylib', '@loader_path/../modules/mod_napmath/lib/%s/libmod_napmath.dylib' % BUILD_TYPE_FOR_FBXCONVERTER, FBXCONVERTER_PATH)
    update_single_link('@rpath/libmod_napscene.dylib', '@loader_path/../modules/mod_napscene/lib/%s/libmod_napscene.dylib' % BUILD_TYPE_FOR_FBXCONVERTER, FBXCONVERTER_PATH)
    update_single_link('@rpath/libnapcore.dylib', '@loader_path/../lib/%s/libnapcore.dylib' % BUILD_TYPE_FOR_FBXCONVERTER, FBXCONVERTER_PATH)
    update_single_link('@rpath/libnaprtti.dylib', '@loader_path/../lib/%s/libnaprtti.dylib' % BUILD_TYPE_FOR_FBXCONVERTER, FBXCONVERTER_PATH)
