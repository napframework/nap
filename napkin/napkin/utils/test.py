import os

import ffmpeg

from utils.butils import walkDir


def kak():
    print('KAK')




def renamedFile(f):
    name, ext = os.path.splitext(os.path.basename(f))
    return os.path.abspath('%s/%s_QUIET%s' % (os.path.dirname(f), name, ext))


if __name__ == '__main__':
    d = r'C:\Workspace\ofApps\Tommy\bin\data\assets'

    files = []
    for f in walkDir(d):
        if not f.endswith('.avi'):
            continue
        files.append(f)

    for f in files:
        if ffmpeg.hasAudio(f):
            renamed = renamedFile(f)
            print('Stripping audio:\n\t%s' % f)
            ffmpeg.stripAudio(f, renamed)
            if not os.path.exists(renamed):
                raise Exception("Failed to convert: %s" % f)

            os.remove(f)
            os.rename(renamed, f)


