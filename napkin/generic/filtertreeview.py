from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import *

from generic import qtutils
from generic.qtutils import LeafFilterProxyModel


class FilterTreeView(QWidget):
    def __init__(self):
        super(FilterTreeView, self).__init__()
        self.setLayout(QVBoxLayout())
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.layout().setSpacing(0)

        self.__leFilter = QLineEdit()
        self.__leFilter.setPlaceholderText('filter')
        self.__leFilter.setClearButtonEnabled(True)
        self.__leFilter.textChanged.connect(self.__filterTextChanged)
        self.layout().addWidget(self.__leFilter)

        self.__sortFilter = LeafFilterProxyModel()

        self.__treeView = QTreeView()
        self.__treeView.setModel(self.__sortFilter)
        self.layout().addWidget(self.__treeView)

        self.setContextMenuPolicy(Qt.ActionsContextMenu)

        expandAction = QAction('Expand All')
        expandAction.triggered.connect(self.__onExpandSelected)
        self.addAction(expandAction)

        collapseAction = QAction('Collapse All')
        collapseAction.triggered.connect(self.__onCollapseSelected)
        self.addAction(collapseAction)

    def __onExpandSelected(self):
        for idx in self.selectedIndexes():
            qtutils.expandChildren(self.__treeView, idx, True)

    def __onCollapseSelected(self):
        for idx in self.selectedIndexes():
            qtutils.expandChildren(self.__treeView, idx, False)

    def selectedItem(self):
        return next(self.selectedItems(), None)

    def selectedItems(self):
        for idx in self.selectedIndexes():
            yield self.model().itemFromIndex(idx)

    def selectedIndexes(self):
        for idx in self.__treeView.selectionModel().selectedIndexes():
            yield self.__sortFilter.mapToSource(idx)

    def __filterTextChanged(self, text):
        self.__sortFilter.setFilterRegExp(text)

    def selectionModel(self):
        return self.__treeView.selectionModel()

    def setHeaderHidden(self, b):
        self.__treeView.setHeaderHidden(b)

    def expandAll(self):
        self.__treeView.expandAll()

    def setModel(self, model):
        self.__sortFilter.setSourceModel(model)

    def model(self):
        return self.__sortFilter.sourceModel()