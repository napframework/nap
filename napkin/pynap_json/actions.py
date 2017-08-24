import os
from PyQt5.QtCore import QSettings
from PyQt5.QtGui import QKeySequence
from PyQt5.QtWidgets import QAction, QFileDialog, QMessageBox, QApplication

from pynap_json.napjsonwrap import loadFile, saveFile
from pynap_json.outlinepanel import ObjectModel

LAST_FILE = 'LAST_DIR'
CURRENT_FILE = None
NAP_FILE_FILTER = 'NAP JSON Files (*.nap.json *.json)'


class OpenFileAction(QAction):
    def __init__(self, parent, model, filename=None):
        super(OpenFileAction, self).__init__('Open...', parent)
        self.__model = model
        self.__filename = filename
        self.setShortcut(QKeySequence.Open)
        self.triggered.connect(self.__perform)

    def __perform(self):
        settings = QSettings()

        filename = self.__filename
        if not filename:
            filename = \
                QFileDialog.getOpenFileName(self.parentWidget(), 'Open NAP Data',
                                            settings.value(LAST_FILE),
                                            NAP_FILE_FILTER)[0]

        if not filename:
            return

        settings.setValue(LAST_FILE, filename)
        global CURRENT_FILE
        CURRENT_FILE = filename

        try:
            data = loadFile(filename)
            self.__model.setObjects(data)
        except Exception as e:
            QMessageBox.warning(QApplication.topLevelWidgets()[0],
                                'Error loading file', str(e))
            raise


class SaveFileAction(QAction):
    def __init__(self, parent, model):
        super(SaveFileAction, self).__init__('Save', parent)
        self.__model = model
        self.setShortcut(QKeySequence.Save)
        self.triggered.connect(self._perform)

    def _perform(self):
        if not CURRENT_FILE:
            SaveFileAsAction(self.parentWidget(), self.__model).trigger()
            return

        self._saveFile(CURRENT_FILE)

    def _saveFile(self, filename):
        saveFile(filename, self.__model.objects())
        print('File saved: %s' % filename)

class SaveFileAsAction(SaveFileAction):
    def __init__(self, parent, model: ObjectModel):
        super(SaveFileAsAction, self).__init__(parent, model)
        self.setText('Save as...')
        self.setShortcut(QKeySequence.SaveAs)

    def _perform(self):
        settings = QSettings()
        lastfile = settings.value(LAST_FILE)
        if lastfile:
            folder = os.path.dirname(lastfile)
        else:
            folder = ''

        filename = QFileDialog.getSaveFileName(self.parentWidget(),
                                               'Select NAP file to save to',
                                               folder, NAP_FILE_FILTER)[0]
        if not filename: return

        if not filename.endswith('.nap.json'):
            filename = '%s.nap.json' % filename

        self._saveFile(filename)
