import os
import sys

from PyQt5.QtCore import QSettings
from PyQt5.QtGui import QKeySequence
from PyQt5.QtWidgets import *

from generic.qbasewindow import QBaseWindow
from pynap_json.napjsonwrap import loadFile
from pynap_json.objectpanel import ObjectPanel
from pynap_json.proppanel import PropPanel

LAST_DIR = 'LAST_DIR'


class OpenFileAction(QAction):
    def __init__(self, parent, model):
        super(OpenFileAction, self).__init__('Open...', parent)
        self.__model = model
        self.setShortcut(QKeySequence.Open)
        self.triggered.connect(self.__perform)

    def __perform(self):
        settings = QSettings()
        filename = QFileDialog.getOpenFileName(self.parent(), 'Open NAP Data', settings.value(LAST_DIR),
                                               'NAP JSON Files (*.nap.json *.json)')[0]
        if not filename:
            return

        settings.setValue(LAST_DIR, filename)

        try:
            data = loadFile(filename)
            self.__model.setObjects(data)
        except Exception as e:
            QMessageBox.warning(QApplication.topLevelWidgets()[0], 'Error loading file', str(e))
            raise


def main():
    app = QApplication(sys.argv)
    win = QBaseWindow()

    # win.addDock('Types', TypeHierarchyPanel())



    objectPanel = ObjectPanel()
    win.addDock('Objects', objectPanel)

    propPanel = PropPanel()
    win.addDock('Properties', propPanel)

    filemenu = QMenu('File', win.menuBar())

    openAction = OpenFileAction(filemenu, objectPanel.model())
    filemenu.addAction(openAction)
    win.menuBar().insertMenu(win.windowMenu().menuAction(), filemenu)

    # load last file
    filename = QSettings().value(LAST_DIR)
    if os.path.exists(filename):
        objectPanel.model().setObjects(loadFile(filename))

    objectPanel.selectionChanged.connect(propPanel.setObjects)

    win.show()
    app.exec()


if __name__ == '__main__':
    main()
