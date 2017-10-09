from napkin.models.nap import *


class StartOperator(Operator):
    def __init__(self):
        super(StartOperator, self).__init__()
        self.start = self.addTriggerOutlet('start')
        self.added.connect(self.__onAdded)

    def __onAdded(self, parent: Patch):
        assert isinstance(parent, Patch)
        parent.started.connect(self.__onPatchStarted)

    def __onPatchStarted(self):
        self.start()
