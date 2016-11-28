import os

def walkDir(d):
    for root, dirs, files in os.walk(d):
        for f in files:
            yield '%s/%s' % (root, f)

