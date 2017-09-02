from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QWidget, QHBoxLayout, QToolButton

import iconstore


class TypeFilterWidget(QWidget):
    filterChanged = pyqtSignal(object)

    def __init__(self):
        super(TypeFilterWidget, self).__init__()
        self.setLayout(QHBoxLayout())
        self.layout().setSpacing(2)
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.__typeButtons = {}

    def setTypes(self, types):
        for btn in self.__typeButtons.keys():
            del btn

        for t in types:
            btn = QToolButton()
            btn.setCheckable(True)
            btn.setToolTip('Show instances of %s' % t.__name__)
            btn.setIcon(iconstore.iconFor(t))
            btn.clicked.connect(self.__onTypeFilterUpdated)
            self.__typeButtons[btn] = t
            self.layout().addWidget(btn)

    def enabledTypes(self):
        ret = []
        for btn in self.__typeButtons.keys():
            if btn.isChecked():
                ret.append(self.__typeButtons[btn])
        return ret

    def __onTypeFilterUpdated(self):
        self.filterChanged.emit(self.enabledTypes())

    def setTypesEnabled(self, types):
        for btn in self.__typeButtons.keys():
            t = self.__typeButtons[btn]
            if t in types:
                btn.setChecked(True)
            else:
                btn.setChecked(False)