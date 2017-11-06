import json
import sys

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from generic.basewindow import BaseWindow
from napkin.views.patchpanel import PatchPanel
# from pynap_json.outlinepanel import OutlinePanel

EXT = 'json'
FILE_FILTER = "json (*.%s)" % EXT
DOC_DIR = '~/Documents'

SETTINGS = None

def getSaveFilename(parent):
    filename, _ = QFileDialog.getSaveFileName(parent, 'Save File', '', FILE_FILTER)
    if not filename: return None
    if not filename.lower().endswith(EXT):
        filename = '%s%s' % (filename, EXT)
    return filename


class MainWindow(BaseWindow):
    def __init__(self):
        super(MainWindow, self).__init__()

        self.patchPanel = PatchPanel()
        self.addDock('Patch', patchPanel);




if __name__ == '__main__':
    def __hook(type, value, traceback):
        raise value

    sys.excepthook = __hook
    QCoreApplication.setOrganizationDomain('Naivi')
    QCoreApplication.setApplicationName(__file__)

    app = QApplication(sys.argv)



    patchPanel = PatchPanel()
    # outline = OutlinePanel()
    # patchPanel.selectionChanged.connect(outline.setObjects)

    win = BaseWindow()
    win.addDock('Patch', patchPanel)
    # win.addDock('Outline', outline)

    toolbar = win.addToolBar('Actions')
    toolbar.addAction('Run').triggered.connect(patchPanel.patch().run)

    def save():
        filename = getSaveFilename(win)
        if not filename: return
        if not filename.lower().endswith('.%s' % EXT):
            filename = '%s.%s' % (filename, EXT)
        with open(filename, 'w') as fh:
            dic = patchPanel.patch().dict()
            json.dump(dic, fh, indent=2)


    saveAction = QAction('&Save')
    saveAction.setShortcut(QKeySequence.Save)
    saveAction.triggered.connect(save)
    win.addAction(saveAction)

    fileMenu = QMenu('&File')
    win.menuBar().insertMenu(win.windowMenu().menuAction(), fileMenu)
    fileMenu.addAction(saveAction)

    def toggle():
        for n in patchPanel.scene.selectedNodes():
            pass


    toggleDisplayAction = QAction('ToggleNodeDisplay')
    toggleDisplayAction.setShortcut("1")
    toggleDisplayAction.triggered.connect(lambda: print('Toggle'))
    win.addAction(toggleDisplayAction)


    win.show()


    app.exec_()
