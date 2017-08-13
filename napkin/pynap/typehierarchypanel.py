from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from napkin.pynap.models import *


class TypeHierarchyPanel(QWidget):
    def __init__(self):
        super(TypeHierarchyPanel, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.__downstreamModel = TypeByRootModel()
        self.__upstreamModel = TypeByLeafModel()

        self.__modelRadioButtons = QWidget()
        self.__modelRadioButtons.setLayout(QHBoxLayout())
        self.__modelRadioButtons.layout().setContentsMargins(0, 0, 0, 0)
        self.__modelRadioButtons.layout().addStretch(1)
        self.layout().addWidget(self.__modelRadioButtons)

        self.__modelRadioGrp = QButtonGroup()
        self.__modelRadioGrp.buttonToggled.connect(self.__refresh)
        self.__rbDownstream = QRadioButton('Downstream')
        self.__rbDownstream.setChecked(True)
        self.__modelRadioGrp.addButton(self.__rbDownstream)
        self.__modelRadioButtons.layout().addWidget(self.__rbDownstream)

        self.__rbUpstream = QRadioButton('Upstream')
        self.__modelRadioGrp.addButton(self.__rbUpstream)
        self.__modelRadioButtons.layout().addWidget(self.__rbUpstream)

        self.__treeView = QTreeView()
        self.__treeView.setHeaderHidden(True)
        self.layout().addWidget(self.__treeView)

        self.__refresh()

    def __refresh(self):
        if self.__rbDownstream.isChecked():
            self.__treeView.setModel(self.__downstreamModel)
            self.__treeView.expandAll()
        elif self.__rbUpstream.isChecked():
            self.__treeView.setModel(self.__upstreamModel)
