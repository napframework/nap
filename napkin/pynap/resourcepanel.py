from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

from napkin.pynap.models import *
import nap

class ResourcePanel(QWidget):
    def __init__(self):
        super(ResourcePanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self.__customContextMenuRequested)

        self.service = nap.core.getOrCreateService('nap::ResourceManagerService')


    def __createEntity(self):
        self.service.createObject('nap::Entity')

    def __customContextMenuRequested(self, pos):
        menu = QMenu()
        menu.addAction('Create Entity').triggered.connect(self.__createEntity)
        menu.exec(self.mapToGlobal(pos))



