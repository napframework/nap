from PyQt5.QtWidgets import QAction


class Action(QAction):
    def __init__(self, *args):
        super(Action, self).__init__(*args)

class SaveAction(Action):
    def __init__(self):
        super(SaveAction, self).__init__('Save')

class CopyAction(Action):
    def __init__(self):
        super(CopyAction, self).__init__()


