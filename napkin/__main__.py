"""
MAIN
"""

import sys

from appcontext import AppContext
from coresettings.corewidget import ConnectionWidget
from log.logpanel import LogPanel
from outline.model import *
from outline.outlinewidget import OutlineWidget
from patch.patcheditor import PatchEditor
from PyQt5 import QtCore

if __name__ == '__main__'
    """Kick off the applicatoin"""
    QCoreApplication.setApplicationName('NapEditor')
    QCoreApplication.setOrganizationName('Naivi')

    app = QApplication(sys.argv)
    app.setAttribute(Qt.AA_DontShowIconsInMenus, False)

    ctx = AppContext()
    ctx.registerEditor('nap::PatchComponent', PatchEditor)

    win = MainWindow(ctx)
    win.show()

    sys.exit(app.exec_())
