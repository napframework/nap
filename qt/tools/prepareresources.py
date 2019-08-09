import logging
import os
import platform
import subprocess
from xml.dom import minidom
from xml.etree import ElementTree

_RESOURCE_DIR = '../resources'
_INKSCAPE = None
_FFMPEG = None

_BASENAME = 'napqt'
_NAMESPACE = 'nap::qt'


def findAppInWindows(appexe):
    try:
        import winreg
    except ImportError:
        import _winreg as winreg

    handle = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                            r"SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\%s" % appexe)

    num_values = winreg.QueryInfoKey(handle)[1]
    for i in range(num_values):
        print(winreg.EnumValue(handle, i))


def resourcePath(suffix=''):
    return os.path.realpath('%s/%s/%s' %
                            (os.path.dirname(__file__),
                             _RESOURCE_DIR,
                             suffix))


def walkDir(d):
    for dirName, subdirList, fileList in os.walk(d):
        for fname in fileList:
            yield '%s/%s' % (dirName, fname)


def searchInFolders(folders, filename):
    for d in folders:
        for f in walkDir(d):
            if os.path.basename(f).lower() == filename:
                return f


def getInkscapeExe():
    global _INKSCAPE
    if _INKSCAPE is not None:
        return _INKSCAPE
    if platform.system() != 'Windows':
        return 'inkscape'

    _INKSCAPE = searchInFolders([
        'c:/program files/inkscape',
        'c:/program files',
        'c:/',
    ], 'inkscape.exe')
    return _INKSCAPE


def getFFMpegExe():
    global _FFMPEG
    if _FFMPEG is not None:
        return _FFMPEG
    if platform.system() != 'Windows':
        return _FFMPEG

    # print(os.path.realpath())

    _FFMPEG = searchInFolders({
        '%s/../../../thirdparty/ffmpeg/bin' % os.path.dirname(__file__)
    }, 'ffmpeg.exe')
    return _FFMPEG


def convertSVGtoPNG(size=128):
    inkscape = getInkscapeExe()
    if not inkscape:
        logging.root.warning('Inkscape not found :(')
        return

    dir = resourcePath('icons')

    for f in os.listdir(dir):
        if not f.endswith('.svg'):
            continue
        infilename = '%s/%s' % (dir, f)
        outfilename = '%s/%s.png' % (dir, os.path.splitext(f)[0])

        # only update when necessary
        if os.path.exists(outfilename) and os.path.getmtime(infilename) < os.path.getmtime(outfilename):
            print('File was up to date: %s' % outfilename)
            continue
        print('Writing: %s' % outfilename)
        cmd = [
            inkscape,
            '-z',
            '-e', outfilename,
            '-w', size,
            '-h', size,
            infilename
        ]
        cmd = [str(c) for c in cmd]
        subprocess.Popen(cmd).communicate()


def __resourceFiles():
    dirs = ('icons', 'fonts')
    filetypes = ('ttf', 'png')
    for d in dirs:
        for f in sorted(walkDir(resourcePath(d))):
            ext = os.path.splitext(f)[1][1:]
            if ext in filetypes:
                yield f


def generateQRC():
    outfile = resourcePath('%s-resources.qrc' % _BASENAME)
    print('Dumping %s' % outfile)

    et = ElementTree
    xml = et.Element('RCC')
    xresource = et.Element('qresource')
    xml.append(xresource)

    resdir = resourcePath()
    for f in __resourceFiles():
        x = et.Element('file')
        p = os.path.relpath(f, resdir).replace(os.sep, '/')
        x.text = p
        xresource.append(x)

    xmlstring = et.tostring(xml)
    xmldom = minidom.parseString(xmlstring)
    xmlpretty = xmldom.toprettyxml()
    with open(outfile, 'w') as fp:
        fp.write(xmlpretty)


def generateHeader():
    outfile = resourcePath('../src/%s-resources.h' % _BASENAME)

    with open(outfile, 'w') as fp:
        fp.write(
            '#pragma once\n\n'
            '#include <QString>\n\n'
            'namespace %s\n{\n' % _NAMESPACE
        )
        resdir = resourcePath()
        for f in __resourceFiles():
            p = os.path.relpath(f, resdir)
            p = p.replace('\\', '/')
            name = p.replace('/', '_').upper()
            name = name.replace('-', '_')
            name = os.path.splitext(name)[0]
            name = name.ljust(30, ' ')
            fp.write('\tstatic const QString QRC_%s = ":/%s";\n' % (name, p))

        fp.write(
            '} // namespace %s\n\n' % _NAMESPACE
        )


def generateICO():
    ffmpeg = getFFMpegExe()
    if not ffmpeg:
        logging.root.warning('FFMPEG not found :(')
        return

    f = '%s-logo.png' % _BASENAME
    folder = resourcePath('icons')
    infilename = '%s/%s' % (folder, f)
    outfilename = '%s/%s.ico' % (folder, os.path.splitext(f)[0])
    print('Writing: %s' % outfilename)

    cmd = [
        ffmpeg,
        '-loglevel', 'panic',
        '-y',
        '-i', infilename,
        outfilename,
    ]
    cmd = [str(c) for c in cmd]
    subprocess.Popen(cmd).communicate()



if __name__ == '__main__':
    convertSVGtoPNG()
    generateQRC()
    generateHeader()

    if os.name == 'nt': # Windows
        generateICO()
