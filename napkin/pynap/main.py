import sys

from pynap.logpanel import LogPanel
from pynap.propertypanel import PropertyPanel
from pynap.servicepanel import ServicePanel
from generic.basewindow import BaseWindow

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from pynap.resourcepanel import ResourcePanel
from pynap.typehierarchypanel import TypeHierarchyPanel

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = BaseWindow()

    win.addDock('Types', TypeHierarchyPanel())
    win.addDock('Services', ServicePanel())

    resourcePanel = ResourcePanel()
    win.addDock('Resources', resourcePanel)

    propPanel = PropertyPanel()
    win.addDock('Properties', propPanel)

    win.addDock('Log', LogPanel())

    resourcePanel.selectionChanged.connect(propPanel.setItems)

    win.show()
    app.exec()
