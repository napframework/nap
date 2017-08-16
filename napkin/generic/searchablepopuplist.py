import sys
from functools import partial

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

from napkin.generic import butils


class SearchablePopupList(QMenu):
    itemSelected = pyqtSignal(object)

    def __init__(self, parent, items):
        super(SearchablePopupList, self).__init__(parent)
        self.setMinimumSize(200, 200)
        self.setLayout(QVBoxLayout())
        self.layout().setSpacing(0)
        self.layout().setContentsMargins(0, 0, 0, 0)

        self.tf = QLineEdit()
        self.tf.textChanged.connect(self.__onFilterChanged)
        self.layout().addWidget(self.tf)

        self.tree = QTreeView()
        self.tree.setHeaderHidden(True)
        self.tree.setRootIsDecorated(False)
        self.sortFilter = QSortFilterProxyModel()
        self.model = QStandardItemModel()
        for item in items:
            m = QStandardItem(str(item))
            m.setEditable(False)
            m.item = item
            self.model.appendRow(m)

        self.sortFilter.setSourceModel(self.model)
        self.sortFilter.setFilterCaseSensitivity(False)
        self.tree.setModel(self.sortFilter)
        self.tree.clicked.connect(self.__onClicked)
        self.layout().addWidget(self.tree)

    def __onFilterChanged(self, text):
        self.sortFilter.setFilterFixedString(text)
        self.tree.selectionModel().setCurrentIndex(self.sortFilter.index(0, 0),
                                                   QItemSelectionModel.ClearAndSelect)

    def keyPressEvent(self, evt: QKeyEvent):
        if evt.key() == Qt.Key_Enter or evt.key() == Qt.Key_Return:
            self.applyAction()
        return super(SearchablePopupList, self).keyPressEvent(evt)

    def __onClicked(self):
        self.applyAction()

    def applyAction(self):
        idx = self.tree.selectionModel().selectedIndexes()
        if not idx: return
        idx = idx[0]
        item = self.model.itemFromIndex(self.sortFilter.mapToSource(idx))
        self.itemSelected.emit(item.item)
        self.close()

    def showEvent(self, evt):
        self.tf.setFocus()


class SearchFieldAction(QWidgetAction):
    def __init__(self, parent):
        super(SearchFieldAction, self).__init__(parent)
        self.lineEdit = QLineEdit()
        self.lineEdit.setPlaceholderText('filter')
        self.lineEdit.setClearButtonEnabled(True)
        self.lineEdit.textChanged.connect(self.__textChanged)
        self.setDefaultWidget(self.lineEdit)

        self.__otherActions = None

        QTimer.singleShot(0, self.__stealFocus)

    def __stealFocus(self):
        self.lineEdit.setFocus()

    def __gatherOtherItems(self):
        if not self.__otherActions is None:
            return
        self.__otherActions = []
        for a in self.parent().actions():
            if a == self:
                continue
            self.__otherActions.append(a)

    def __textChanged(self, txt):
        self.__gatherOtherItems()

        hasActive = False
        self.parent().setActiveAction(None)
        for a in self.__otherActions:
            # b = txt.lower() in a.text().lower()
            b = butils.fuzzymatches(txt.lower(), a.text().lower())
            a.setVisible(b)
            if not hasActive and b:
                self.parent().setActiveAction(a)
                hasActive = True


class SearchablePopup(QMenu):
    itemSelected = pyqtSignal(object)

    def __init__(self, parent):
        super(SearchablePopup, self).__init__(parent)
        self.__searchField = SearchFieldAction(self)
        self.addAction(self.__searchField)

    @staticmethod
    def showMenu(parent, pos, items, callback):
        menu = SearchablePopup(parent)
        for item in items:
            action = QAction(str(item), menu)
            action.item = item
            action.triggered.connect(partial(callback, item))
            menu.addAction(action)

        menu.exec_(parent.mapToGlobal(pos))


if __name__ == '__main__':
    app = QApplication(sys.argv)

    win = QMainWindow()
    win.setContextMenuPolicy(Qt.CustomContextMenu)


    def showMenu(pos):
        menu = QMenu(win)
        line = SearchFieldAction(menu)
        menu.addAction(line)

        menu.addMenu('Kak')
        menu.addMenu('Kak')
        menu.addMenu('Broekhoest')
        menu.addMenu('Kak')
        menu.addMenu('Kak')
        menu.addMenu('Berry Braadslee')
        menu.exec_(win.mapToGlobal(pos))


    win.customContextMenuRequested.connect(showMenu)
    win.show()

    app.exec_()
