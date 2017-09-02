import sys
from PySide.QtGui import *
from PySide.QtCore import *


def _getMainWindow():
    return QCoreApplication.instance().topLevelWidgets()[0]


class MenuItem(QWidget):
    def __init__(self, action):
        super(MenuItem, self).__init__()
        self.setAutoFillBackground(True)

        self.action = action
        self.setLayout(QHBoxLayout())
        self.layout().setSpacing(0)
        self.layout().setContentsMargins(0, 2, 0, 4)

        self.icon = QLabel()
        self.icon.setMinimumSize(20,20)
        # self.icon.setMaximumSize(16,16)
        self.layout().addWidget(self.icon)

        self.label = QLabel(action.text())
        self.label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Minimum)
        self.layout().addWidget(self.label)

        self.shortcutLabel = QLabel(action.shortcut().toString())
        self.layout().addWidget(self.shortcutLabel)

        self.action.changed.connect(self.changed)

    def changed(self):
        print('changed')

    def enterEvent(self, evt):
        self.setStyleSheet('QWidget{background-color: #F80;}')
        self.action.menu().setActiveAction(self.action)
        self.update()

    def leaveEvent(self, evt):
        self.setStyleSheet('QWidget{background-color: none;}')
        self.update()



class WidgetAction(QWidgetAction):
    def __init__(self, parent=None):
        if not parent:
            parent = _getMainWindow()
        super(WidgetAction, self).__init__(parent)


class Action(WidgetAction):
    def __init__(self, text, shortcut=None):
        super(Action, self).__init__()
        self.setText(text)
        if shortcut:
            self.setShortcut(QKeySequence(shortcut))
        self.setDefaultWidget(MenuItem(self))


class Header(WidgetAction):
    def __init__(self, text):
        super(Header, self).__init__()
        w = QWidget()
        w.setLayout(QHBoxLayout())
        self.setDefaultWidget(w)
        w.layout().addWidget(QLabel(text))
        frame = QFrame()
        frame.setFrameStyle(QFrame.HLine | QFrame.Plain)
        frame.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Minimum)
        w.layout().addWidget(frame)


class Separator(WidgetAction):
    def __init__(self):
        super(Separator, self).__init__()
        frame = QFrame()
        frame.setFrameStyle(QFrame.HLine | QFrame.Plain)
        frame.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Minimum)
        frame.setMinimumSize(10,10)
        self.setDefaultWidget(frame)


class Menu(QMenu):
    def __init__(self, name):
        super(Menu, self).__init__(name)

    def addAction(self, action):
        action.setMenu(self)
        super(Menu, self).addAction(action)

if __name__ == '__main__':
    app = QApplication(sys.argv)

    win = QMainWindow()

    win.setMenuBar(QMenuBar())

    menu = Menu('File')
    menu.addAction(Action('New'))
    menu.addAction(Separator())
    menu.addAction(Action('Open'))
    menu.addSeparator()
    menu.addAction(Action('Recent Files...'))
    menu.addAction(Action('Save'))
    menu.addAction(Separator())
    menu.addAction(Action('Exit'))
    win.menuBar().addMenu(menu)

    menu = Menu('Edit')
    menu.addAction(Header('History'))
    menu.addAction(Action('Undo'))
    menu.addAction(Action('Redo'))
    menu.addAction(Header('Clipboard'))
    menu.addAction(Action('Cut'))
    menu.addAction(Action('Copy', 'Ctrl+C'))
    menu.addAction(Action('Paste', 'Ctrl+V'))
    menu.addSeparator()
    menu.addAction(Action('Settings...'))
    win.menuBar().addMenu(menu)

    win.resize(600, 400)
    win.show()

    sys.exit(app.exec_())
