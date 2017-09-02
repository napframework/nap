import json
import os
import sys

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from generic.qbasewindow import QBaseWindow
from napkin.test.patchpanel import PatchPanel
from pynap_json.outlinepanel import OutlinePanel

EXT = '.json'
FILE_FILTER = "json (*%s)" % EXT
DOC_DIR = '~/Documents'

SETTINGS = None

def getSaveFilename(parent):
    filename, _ = QFileDialog.getSaveFileName(parent, 'Save File', '', FILE_FILTER)
    if not filename: return None
    if not filename.lower().endswith(EXT):
        filename = '%s%s' % (filename, EXT)
    return filename


if __name__ == '__main__':
    def __hook(type, value, traceback):
        raise value

    sys.excepthook = __hook
    QCoreApplication.setOrganizationDomain('Naivi')
    QCoreApplication.setApplicationName(__file__)

    app = QApplication(sys.argv)

    patchPanel = PatchPanel()
    outline = OutlinePanel()
    patchPanel.selectionChanged.connect(outline.setObjects)

    win = QBaseWindow()
    win.addDock('Patch', patchPanel)
    win.addDock('Outline', outline)

    def save():
        filename = getSaveFilename(win)
        if not filename: return
        with open(filename, 'w') as fh:
            dic = patchPanel.patch().dict()
            json.dump(dic, fh, indent=2)


    saveAction = QAction('Save')
    saveAction.setShortcut(QKeySequence.Save)
    saveAction.triggered.connect(save)
    win.addAction(saveAction)

    def toggle():
        for n in patchPanel.scene.selectedNodes():
            pass


    toggleDisplayAction = QAction('ToggleNodeDisplay')
    toggleDisplayAction.setShortcut("1")
    toggleDisplayAction.triggered.connect(lambda: print('Toggle'))
    win.addAction(toggleDisplayAction)


    win.show()


    app.exec_()
