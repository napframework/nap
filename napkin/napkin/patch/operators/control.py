from napkin.patch.operator import *


class StartOperator(Operator):
    displayName = 'Start'

    def __init__(self, parent):
        super(StartOperator, self).__init__(parent)
        self.start = TriggerOutlet(self)



