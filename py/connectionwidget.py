from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import iconstore
from utils import qtutils

def _nameForDict(dic):
    """
    @type dic: dict
    """
    if 'name' in dic:
        return dic['name']
    for val in dic.values():
        if isinstance(val, basestring):
            return val
    return '<dict>'

def _item(text, icon=None):
    if isinstance(text, dict):
        text = ModuleModel.__nameForDict(text)

    item = QStandardItem(str(text))
    item.setEditable(False)
    if icon:
        item.setIcon(iconstore.icon(icon))
    return item

def toHuman(s):
    return s

def _addTypesItem(parent, dic, key):
    if not key in dic:
        return
    item = _item(toHuman(key), 'folder_brick')
    for t in dic[key]:
        item.appendRow(_item(t, 'brick'))
    parent.appendRow(item)

def _addKeyValRow(parent, key, val):
    keyItem = _item(key, 'ui-text-field-format')
    valItem = _item(val)
    parent.appendRow([keyItem, valItem])


class ModuleModel(QStandardItemModel):
    def __init__(self, dic):
        super(ModuleModel, self).__init__()

        globalItem = _item('Global', 'world')

        modulesItem = _item('Modules', 'wooden-box')
        self.appendRow(modulesItem)
        for mod in dic['modules']:
            modItem = _item(mod['name'], 'box')
            modItem.appendRow(_item(mod['filename'], 'document'))
            _addTypesItem(modItem, mod, 'dataTypes')
            _addTypesItem(modItem, mod, 'componentTypes')
            _addTypesItem(modItem, mod, 'operatorTypes')
            modulesItem.appendRow(modItem)

        _addTypesItem(globalItem, dic, 'dataTypes')
        _addTypesItem(globalItem, dic, 'componentTypes')
        _addTypesItem(globalItem, dic, 'operatorTypes')


        self.appendRow(globalItem)





class ConnectionWidget(QWidget):
    def __init__(self, ctx):
        """
        @type ctx: appcontext.AppContext
        """
        self.ctx = ctx
        super(ConnectionWidget, self).__init__()
        self.setMinimumHeight(10)

        self.setLayout(QVBoxLayout())

        connectionLayout = QHBoxLayout()
        connectionLayout.setContentsMargins(0, 0, 0, 0)
        self.tfHost = QLineEdit()
        connectionLayout.addWidget(self.tfHost)
        self.btConnect = QPushButton('Refresh')
        connectionLayout.addWidget(self.btConnect)
        self.btConnect.clicked.connect(self.onConnect)
        self.lbMessage = QLabel()
        connectionLayout.addWidget(self.lbMessage)
        self.layout().addLayout(connectionLayout)

        self.__treeView = QTreeView()
        self.__treeView.setHeaderHidden(True)
        self.layout().addWidget(self.__treeView)

        ctx.connectionChanged.connect(self.connectionChanged)
        ctx.core().moduleInfoChanged.connect(self.__onModuleInfoChanged)

    def __onModuleInfoChanged(self, info):
        self.__treeView.setModel(ModuleModel(info))
        # self.__treeView.expandAll()


    def onConnect(self):
        self.ctx.connect(self.tfHost.text())

    def connectionChanged(self, connected, hostname, message):
        self.tfHost.setText(hostname)
        self.lbMessage.setText(message)