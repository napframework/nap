import os
import subprocess
from xml.dom import minidom
from xml.etree import ElementTree

_RESOURCE_DIR = '../resources'


def resourcePath(suffix=''):
    return os.path.realpath('%s/%s/%s' %
                            (os.path.dirname(__file__),
                             _RESOURCE_DIR,
                             suffix))


def walkDir(d):
    for dirName, subdirList, fileList in os.walk(d):
        for fname in fileList:
            yield '%s/%s' % (dirName, fname)


def convertSVGtoPNG(size=128):
    dir = resourcePath('icons')

    for f in os.listdir(dir):
        if not f.endswith('.svg'):
            continue
        infilename = '%s/%s' % (dir, f)
        outfilename = '%s/%s.png' % (dir, os.path.splitext(f)[0])
        print(outfilename)
        cmd = [
            'inkscape',
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
    outfile = resourcePath('mainresources.qrc')
    print('Dumping %s' % outfile)

    et = ElementTree
    xml = et.Element('RCC')
    xresource = et.Element('qresource')
    xml.append(xresource)

    resdir = resourcePath()
    for f in __resourceFiles():
        x = et.Element('file')
        p = os.path.relpath(f, resdir)
        x.text = p
        xresource.append(x)

    xmlstring = et.tostring(xml)
    xmldom = minidom.parseString(xmlstring)
    xmlpretty = xmldom.toprettyxml()
    with open(outfile, 'w') as fp:
        fp.write(xmlpretty)


def generateHeader():
    outfile = resourcePath('../src/napkinresources.h')

    with open(outfile, 'w') as fp:
        fp.write(
            '#pragma once\n\n'
            '#include <QString>\n\n'
            'namespace napkin\n{\n'
        )
        resdir = resourcePath()
        for f in __resourceFiles():
            p = os.path.relpath(f, resdir)
            name = p.replace(os.sep, '_').upper()
            name = name.replace('-', '_')
            name = os.path.splitext(name)[0]
            name = name.ljust(30, ' ')
            fp.write('\tstatic const QString QRC_%s = ":/%s";\n' % (name, p))

        fp.write(
            '} // namespace napkin\n\n'
        )

if __name__ == '__main__':
    convertSVGtoPNG()
    generateQRC()
    generateHeader()
