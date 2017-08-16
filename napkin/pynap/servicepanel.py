import sys

import os
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

import nap

class ServiceItem(QStandardItem):
    def __init__(self, service):
        super(ServiceItem, self).__init__()
        self.service = service
        self.setText(service.__class__.__name__)
        print(dir(service))

class ServiceModel(QStandardItemModel):
    def __init__(self):
        super(ServiceModel, self).__init__()
        self.refresh()

    def refresh(self):
        self.removeRows(0, self.rowCount())

        for service in nap.core.getServices():
            self.appendRow(ServiceItem(service))


class ServicePanel(QWidget):
    def __init__(self):
        super(ServicePanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.__treeView = QTreeView()
        self.__treeView.setHeaderHidden(True)
        self.layout().addWidget(self.__treeView)
        self.__model = ServiceModel()
        self.__treeView.setModel(self.__model)


    def __createEntity(self):
        self.resrcManager.createObject('nap::Entity')

    def __customContextMenuRequested(self, pos):
        menu = QMenu()
        menu.addAction('Create Entity').triggered.connect(self.__createEntity)
        menu.exec(self.mapToGlobal(pos))


