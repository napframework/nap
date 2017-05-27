import json

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import iconstore
import nap
from utils import qtutils

def _nameForDict(dic):
    """
    @type dic: dict
    """
    if 'name' in dic:
        return dic['name']
    for val in dic.values():
        if isinstance(val, str):
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

def _addKeyValRow(parent, key, val):
    keyItem = _item(key, 'ui-text-field-format')
    valItem = _item(val)
    parent.appendRow([keyItem, valItem])


class ModuleModel(QStandardItemModel):
    def __init__(self, ctx, dic):
        super(ModuleModel, self).__init__()
        self.ctx = ctx

        globalItem = _item('Global', 'world')

        modulesItem = _item('Modules', 'wooden-box')
        self.appendRow(modulesItem)
        for mod in dic['modules']:
            modItem = _item(mod['name'], 'box')
            modItem.appendRow(_item(mod['filename'], 'document'))
            modulesItem.appendRow(modItem)

        self.__addTypesItem(globalItem, ctx.core().componentTypes)
        self.__addTypesItem(globalItem, ctx.core().operatorTypes)
        self.__addTypesItem(globalItem, ctx.core().dataTypes)

        typeIcon = iconstore.icon('type')

        typeHierarchyItem = QStandardItem(typeIcon, 'baseTypes')
        types = dic['types']
        for tp in types:
            typeItem = QStandardItem(typeIcon, tp['name'])
            typeHierarchyItem.appendRow(typeItem)
            parentItem = typeItem
            for baseType in tp['basetypes']:
                baseTypeItem = QStandardItem(typeIcon, baseType)
                parentItem.appendRow(baseTypeItem)
                parentItem = baseTypeItem
        self.appendRow(typeHierarchyItem)


        self.appendRow(globalItem)


    def __addTypesItem(self, parent, getter):
        item = _item(getter.__name__, 'folder_brick')
        for t in getter():
            item.appendRow(_item(t, 'brick'))
        parent.appendRow(item)

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
        self.__treeView.setModel(ModuleModel(self.ctx, info))
        # self.__treeView.expandAll()


    def onConnect(self):
        self.ctx.connect(self.tfHost.text())

    def connectionChanged(self, connected, hostname, message):
        self.tfHost.setText(hostname)
        self.lbMessage.setText(message)