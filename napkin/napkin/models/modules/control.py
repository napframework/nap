from napkin.models.nap import *


class StartOperator(Operator):

    def __init__(self):
        super(StartOperator, self).__init__()
        self.start = self.addTriggerOutlet('start')



