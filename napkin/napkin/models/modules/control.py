import time

from napkin.models import nap


class StartOperator(nap.Operator):
    def __init__(self):
        super(StartOperator, self).__init__()
        self.start = self.addTriggerOutlet('start')
        self.added.connect(self.__onAdded)

    def __onAdded(self, parent: nap.Patch):
        assert isinstance(parent, nap.Patch)
        parent.started.connect(self.__onPatchStarted)

    def __onPatchStarted(self):
        self.start()


class DelayOperator(nap.Operator):
    def __init__(self):
        super(DelayOperator, self).__init__()
        self.inTrigger = self.addTriggerInlet('trigger', self.__exec)
        self.seconds = self.addDataInlet('seconds', float, 1)
        self.outTrigger = self.addTriggerOutlet('done')

    def __exec(self):
        time.sleep(self.seconds())
        self.outTrigger()
