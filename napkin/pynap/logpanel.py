from queue import Queue

import sys
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *


class PipeRedirector(QThread):
    mysignal = pyqtSignal(str)

    def __init__(self):
        QObject.__init__(self)
        self.queue = Queue()
        self.running = True

    def write(self, text):
        self.queue.put(text)

    def run(self):
        while self.running:
            text = self.queue.get()
            self.mysignal.emit(text)

class LogPanel(QWidget):
    def __init__(self):
        super(LogPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.__textedit = QTextEdit()
        self.layout().addWidget(self.__textedit)

        self._piperedirector = PipeRedirector()
        sys.stdout = self._piperedirector
        self._piperedirector.mysignal.connect(self.__onText)

    def __onText(self, text):
        print('Text')
        self.__textedit.append(text)