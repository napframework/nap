"""
Napkin's main entry point
"""
import sys
import traceback

from appcontext import AppContext
from mainwindow import MainWindow
from outline.model import *
from patch.patcheditor import PatchEditor

_APPLICATION_NAME = 'Napkin'

if __name__ == '__main__':
    """Kick off the applicatoin"""
    QCoreApplication.setApplicationName(_APPLICATION_NAME)
    QCoreApplication.setOrganizationName('Naivi')

    # Start real work
    app = QApplication(sys.argv)
    app.setAttribute(Qt.AA_DontShowIconsInMenus, False)

    ctx = AppContext()
    ctx.registerEditor('nap::PatchComponent', PatchEditor)

    win = MainWindow(ctx)


    # Hack to enable exception printing on crash
    def excepthookForward(typ, value, tback):
        raise value


    sys.excepthook = excepthookForward

    win.show()
    sys.exit(app.exec_())
