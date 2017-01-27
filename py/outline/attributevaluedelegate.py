from PyQt5.QtCore import Qt, QEvent, QPoint, QRect
from PyQt5.QtWidgets import QStyledItemDelegate, QStyleOptionButton, QStyle, QApplication


class AttributeValueDelegate(QStyledItemDelegate):
    def __init__(self, *args):
        super(AttributeValueDelegate, self).__init__(*args)
        self.__triggerValue = '<class \'nap.TRIGGER\'>'
        self.__isTrigger = False

    def createEditor(self, widget, option, index):
        newValue = index.data()
        self.__isTrigger = (newValue == self.__triggerValue)

        if isinstance(index.data(), bool) or self.__isTrigger:
            pass
        else:
            return super(AttributeValueDelegate, self).createEditor(widget,
                                                                    option,
                                                                    index)

    def paint(self, painter, option, index):
        self.__isTrigger = (index.data() == self.__triggerValue)
        if self.__isTrigger:
            self.paintTrigger(painter, option, index)
        elif isinstance(index.data(), bool):
            self.paintBool(painter, option, index)
        else:
            return super(AttributeValueDelegate, self).paint(painter, option,
                                                             index)

    def paintBool(self, painter, option, index):
        styleOption = QStyleOptionButton()
        value = index.data()
        if index.flags() & Qt.ItemIsEditable:
            styleOption.state |= QStyle.State_Enabled
        else:
            styleOption.state |= QStyle.State_ReadOnly

        if index.data():
            styleOption.state |= QStyle.State_On
        else:
            styleOption.state |= QStyle.State_Off

        styleOption.rect = self.__buttonRect(option)
        QApplication.style().drawControl(QStyle.CE_CheckBox, styleOption,
                                         painter)

    def paintTrigger(self, painter, option, index):
        styleOption = QStyleOptionButton()
        styleOption.rect = option.rect
        styleOption.state |= QStyle.State_Enabled
        styleOption.state |= QStyle.State_On
        QApplication.style().drawControl(QStyle.CE_PushButton, styleOption,
                                         painter)

    def editorEvent(self, event, model, option, index):
        value = index.data()
        if isinstance(value, bool):
            return self.__checkBoxEvent(event, model, option, index)

        if value == self.__triggerValue:
            return self.__triggerEvent(event, model, option, index)

        return super(AttributeValueDelegate, self).editorEvent(event, model, option, index)

    def __triggerEvent(self, event, model, option, index):
        if event.type() == QEvent.MouseButtonPress or event.type() == QEvent.KeyPress:
            self.setModelData(None, model, index)
        return True

    def __checkBoxEvent(self, event, model, option, index):
        if not index.flags() & Qt.ItemIsEditable:
            return False

        if event.type() == QEvent.MouseButtonPress:
            return False
        if event.type() == QEvent.MouseButtonRelease or event.type() == QEvent.MouseButtonDblClick:
            if event.button() != Qt.LeftButton or not self.__checkBoxRect(
                    option).contains(event.pos()):
                return False
            if event.type() == QEvent.MouseButtonDblClick:
                return True
        elif event.type() == QEvent.KeyPress:
            if event.key() != Qt.Key_Space and event.key() != Qt.Key_Select:
                return False
            else:
                return False

        # Change the checkbox-state
        self.setModelData(None, model, index)
        return True

    def setModelData(self, editor, model, index):
        newValue = index.data()
        if isinstance(newValue, bool):
            model.setData(index, newValue, Qt.EditRole)
        elif newValue == self.__triggerValue:
            model.setData(index, newValue, Qt.EditRole)
        else:
            super(AttributeValueDelegate, self).setModelData(editor, model, index)

    def __buttonRect(self, option):
        styleOption = QStyleOptionButton()
        buttonRect = QApplication.style().subElementRect(
            QStyle.SE_CheckBoxIndicator, styleOption, None)
        buttonPoint = QPoint(option.rect.x() +
                                 option.rect.width() / 2 -
                                 buttonRect.width() / 2,
                                 option.rect.y() +
                                 option.rect.height() / 2 -
                                 buttonRect.height() / 2)
        return QRect(buttonPoint, buttonRect.size())