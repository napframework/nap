from functools import partial

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

import nap

from napkin.generic.searchablepopuplist import SearchablePopupList, SearchablePopup
from napkin.pynap import naputils


class ResourceItem(QStandardItem):
    def __init__(self, res):
        super(ResourceItem, self).__init__()
        self.res = res
        self.setText(res.mID)


class ResourceModel(QStandardItemModel):
    def __init__(self):
        super(ResourceModel, self).__init__()
        self.__resman = nap.core.getOrCreateService('nap::ResourceManagerService')
        self.refresh()

    def refresh(self):
        self.removeRows(0, self.rowCount())
        for ob in self.__resman.getObjects():
            self.appendRow(ResourceItem(ob))

    def createObject(self, typename):
        self.__resman.createObject(typename)
        self.refresh()


class ResourcePanel(QWidget):
    def __init__(self):
        super(ResourcePanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self.__customContextMenuRequested)

        self.__treeView = QTreeView()
        self.layout().addWidget(self.__treeView)
        self.__model = ResourceModel()
        self.__treeView.setModel(self.__model)

    def __createEntity(self, typename):
        self.__model.createObject(typename)

    def __customContextMenuRequested(self, pos):

        items = [naputils.fullCPPTypename(c) for c in naputils.napClasses(nap.RTTIObject) if
                 nap.isInstantiable(naputils.fullCPPTypename(c))]

        SearchablePopup.showMenu(self, pos, items, self.__createEntity)
