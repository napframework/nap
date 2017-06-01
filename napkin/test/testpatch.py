import random
import sys
from collections import namedtuple

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from random_words.random_words import RandomWords

from patch.graphscene import GraphScene
from patch.graphview import GraphView
from patch.inoutnodeitem import InputOutputNodeItem
from utils.qtutils import QBaseWindow




if __name__ == '__main__':
    def __hook(type, value, traceback):
        raise value
    sys.excepthook = __hook
    QCoreApplication.setOrganizationDomain('Naivi')
    QCoreApplication.setApplicationName(__file__)

    app = QApplication(sys.argv)

    win = QBaseWindow()
    view = GraphView()
    scene = GraphScene()
    view.setScene(scene)

    rw = RandomWords()

    for i in range(100):
        op = InputOutputNodeItem(rw.random_word())
        for n in range(random.randint(0, 4)):
            op.addInlet(rw.random_word())
        for n in range(random.randint(0, 4)):
            op.addOutlet(rw.random_word())
        op.setPos(random.randint(-800, 800), random.randint(-800, 800))
        scene.addNode(op)

    win.setCentralWidget(QWidget())
    win.centralWidget().setLayout(QVBoxLayout())
    win.centralWidget().layout().addWidget(view)
    win.centralWidget().layout().setContentsMargins(0, 0, 0, 0)

    win.centralWidget().layout().addWidget(QPushButton('Le Butten'))

    win.show()

    app.exec_()

