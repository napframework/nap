from PyQt5.QtCore import QCoreApplication, QSettings
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QMainWindow, QDockWidget


class BaseWindow(QMainWindow):
    __WIN_GEO = 'WindowGeometry'
    __WIN_STATE = 'WindowState'

    def __init__(self):
        super(BaseWindow, self).__init__()
        self.setWindowTitle(QCoreApplication.applicationName())
        self.setDockNestingEnabled(True)
        self.__windowMenu = None
        # self.centralWidget().setVisible(False)

    def windowMenu(self):
        if not self.__windowMenu:
            self.__windowMenu = self.menuBar().addMenu('&Window')
        return self.__windowMenu

    def addDock(self, name, widget, area=Qt.TopDockWidgetArea):
        dock = QDockWidget()
        dock.setObjectName(name)
        dock.setWidget(widget)
        dock.setWindowTitle(name)
        action = self.windowMenu().addAction(name)
        action.setCheckable(True)
        action.setChecked(True)
        self.addDockWidget(area, dock)

    def showEvent(self, e):
        super(BaseWindow, self).showEvent(e)
        s = QSettings()
        v = s.value(self.__WIN_GEO)
        if v:
            self.restoreGeometry(v)
        v = s.value(self.__WIN_STATE)
        if v:
            self.restoreState(v)

    def closeEvent(self, e):
        super(BaseWindow, self).closeEvent(e)
        s = QSettings()
        s.setValue(self.__WIN_STATE, self.saveState())
        s.setValue(self.__WIN_GEO, self.saveGeometry())