import sys
import typing

import math
from collections import namedtuple

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

from patch.inputoutputnodeitem import InputOutputNodeItem
from patch.patchscene import PatchScene
from patch.patchview import PatchView
from patch._plugitem import _PlugItem
from utils.qtutils import QBaseWindow
Margins = namedtuple('Margins', ['top', 'right', 'bottom', 'left'])


if __name__ == '__main__':
    def __hook(type, value, traceback):
        raise value
    sys.excepthook = __hook
    QCoreApplication.setOrganizationDomain('Naivi')
    QCoreApplication.setApplicationName(__file__)

    app = QApplication(sys.argv)

    win = QBaseWindow()
    view = PatchView()
    scene = PatchScene()
    view.setScene(scene)

    op = InputOutputNodeItem('SimpleOperator')
    op.addInlet('MyInlet')
    op.addInlet('AnotherInput With Long Name')
    op.addOutlet('out')

    scene.addNode(op)

    win.setCentralWidget(QWidget())
    win.centralWidget().setLayout(QVBoxLayout())
    win.centralWidget().layout().addWidget(view)
    win.centralWidget().layout().setContentsMargins(0, 0, 0, 0)

    win.centralWidget().layout().addWidget(QPushButton('Le Butten'))

    win.show()

    app.exec_()

