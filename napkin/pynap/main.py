import sys
import os

# TODO: Dirty hack to get things going right now
from pynap.logpanel import LogPanel

p = '%s/../../lib/Clang-Debug-x86_64' % os.path.dirname(__file__)
sys.path.append(p)

from pynap.propertypanel import PropertyPanel
from pynap.servicepanel import ServicePanel
from generic.qbasewindow import QBaseWindow

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from pynap.resourcepanel import ResourcePanel
from pynap.typehierarchypanel import TypeHierarchyPanel

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = QBaseWindow()

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
