import os
from PyQt5.QtCore import pyqtSignal, QSettings
from PyQt5.QtWidgets import *


class FileSelectorWidget(QWidget):
    filenameChanged = pyqtSignal(str)

    def __init__(self, persistenceKey='LastFileName', filenameFilter=''):
        super(FileSelectorWidget, self).__init__()
        self.persistenceKey = persistenceKey

        self.filenameFilter = filenameFilter

        self.setLayout(QHBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self._filenameField = QLineEdit()
        self.layout().addWidget(self._filenameField)

        self._browseButton = QToolButton()
        self._browseButton.setText('...')
        self._browseButton.clicked.connect(self.__onBrowseFile)
        self.layout().addWidget(self._browseButton)

        # set initial value
        settings = QSettings()
        lastvalue = settings.value(persistenceKey)
        self._filenameField.setText(lastvalue)

    def filename(self):
        return self._filenameField.text()

    def __onBrowseFile(self):
        settings = QSettings()
        lastvalue = settings.value(self.persistenceKey)
        if lastvalue:
            folder = os.path.dirname(lastvalue)
        else:
            folder = '.'

        filename = QFileDialog.getOpenFileName(self, 'Select file', folder,
                                               self.filenameFilter)[0]
        if not filename:
            return

        settings.setValue(self.persistenceKey, filename)

        self._filenameField.setText(filename)
