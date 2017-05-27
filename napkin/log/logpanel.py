from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *


class LogModel(QStandardItemModel):

    MAX_MESSAGES = 1000

    def __init__(self, ctx):
        """
        @type ctx: appcontext.AppContext
        """
        self.__ctx = ctx
        super(LogModel, self).__init__()
        self.setHorizontalHeaderLabels(['Level', 'Message'])
        self.__ctx.logMessageReceived.connect(self.__onLogMessageReceived)

    def __onLogMessageReceived(self, lvl, lvlName, text):
        while self.rowCount() > self.MAX_MESSAGES:
            self.removeRow(0)
        self.appendRow([
            QStandardItem(lvlName),
            QStandardItem(text)
        ])


class LogPanel(QWidget):
    def __init__(self, ctx):
        """
        @type ctx: appcontext.AppContext
        """
        self.__ctx = ctx
        super(LogPanel, self).__init__()
        self.__logModel = LogModel(ctx)
        self.__treeView = QTreeView()
        self.__treeView.setModel(self.__logModel)
        self.setLayout(QVBoxLayout())
        self.layout().addWidget(self.__treeView)

        self.__ctx.logMessageReceived.connect(self.__onLogMessageReceived)

    def __onLogMessageReceived(self, lvl, lvlName, text):
        idx = self.__logModel.index(self.__logModel.rowCount() -1, 0)
        self.__treeView.scrollTo(idx)