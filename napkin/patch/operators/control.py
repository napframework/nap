from patch.operator import *


class StartOperator(Operator):
    displayName = 'Start'

    def __init__(self):
        super(StartOperator, self).__init__()
        self.start = self._triggerOutlet('trigger')



