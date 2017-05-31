import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qtutils import QBaseWindow, FlowLayout

from utils import butils


def iconFiles():
    for f in butils.walkDir(r'D:\Workspace\Icons'):
        if not f.lower().endswith('.png'):
            continue
        yield f


class Window(QBaseWindow):
    def __init__(self):
        super(Window, self).__init__()

        w = QWidget()
        w.setLayout(FlowLayout())
        for f in iconFiles():
            lb = QLabel()
            lb.setPixmap(QPixmap(f))
            w._layout().addWidget(lb)

        scroll = QScrollArea()
        # scroll.setMinimumSize(100,100)
        scroll.setLayout(QVBoxLayout())
        scroll.setWidget(w)

        p = QWidget()
        p.setLayout(QVBoxLayout())
        p._layout().addWidget(scroll)

        self.setCentralWidget(p)

if __name__ == '__main__':
    QCoreApplication.setApplicationName('Iconviewer')
    QCoreApplication.setOrganizationName('Naivi')

    app = QApplication(sys.argv)
    win = Window()
    win.show()

    app.exec_()