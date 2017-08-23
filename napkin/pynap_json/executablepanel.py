import os
import subprocess

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from generic.fileselectorwidget import FileSelectorWidget
from pynap_json.constants import FILENAME_FILTER_EXE


class ExecutablePanel(QWidget):
    def __init__(self):
        super(ExecutablePanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.__filenameField = FileSelectorWidget(
            persistenceKey='LAST_EXECUTABLE',
            filenameFilter=FILENAME_FILTER_EXE)
        self.__filenameField.filenameChanged.connect(self.__onFilenameChanged)
        self.layout().addWidget(self.__filenameField)

        self.__argumentsField = QLineEdit()
        self.__argumentsField.textChanged.connect(self.__onArgumentsChanged)
        self.layout().addWidget(self.__argumentsField)

        buttonsWidget = QWidget()
        buttonsWidget.setLayout(QHBoxLayout())
        buttonsWidget.layout().setContentsMargins(0, 0, 0, 0)
        self.layout().addWidget(buttonsWidget)
        self.__killButton = QPushButton('Kill')
        buttonsWidget.layout().addWidget(self.__killButton)
        self.__runButton = QPushButton('Start')
        self.__runButton.clicked.connect(self.__onRun)
        buttonsWidget.layout().addWidget(self.__runButton)

        self.layout().addStretch(1)


        self.__process = None

        self.__refreshAvailability()

        self.__timer = QTimer()
        self.__timer.setInterval(1000)
        self.__timer.timeout.connect(self.__onTimer)

    def __onFilenameChanged(self, filename):
        self.__refreshAvailability()

    def __onArgumentsChanged(self, args):
        self.__refreshAvailability()
        settings = QSettings()
        settings.setValue('LAST_EXECUTABLE_ARGUMENTS', args)

    def __refreshAvailability(self):
        self.__runButton.setEnabled(
            os.path.exists(self.__filenameField.filename()))
        self.__killButton.setEnabled(bool(self.__process))

    def __onTimer(self):
        if not self.__process:
            return
        if not os.kill(self.__process.pid, 0):
            self.__process = None

        self.__refreshAvailability()

    def __onRun(self):
        cmd = [self.__filenameField.filename()]
        args = self.__argumentsField.text()
        if args:
            cmd += [args]

        cmd = ' '.join(cmd)
        print('Running: %s' % cmd)
        self.__process = subprocess.Popen(cmd)
        self.__refreshAvailability()

    def __onKill(self):
        if not self.__process:
            return

        self.__process.kill()
        self.__refreshAvailability()
