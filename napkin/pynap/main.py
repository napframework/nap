import sys
from napkin.generic.qbasewindow import QBaseWindow

from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from napkin.pynap.typehierarchypanel import TypeHierarchyPanel

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = QBaseWindow()

    win.addDock('Type Hierarchy', TypeHierarchyPanel())

    win.show()
    app.exec()
