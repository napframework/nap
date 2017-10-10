from typing import Tuple

import time

from napkin.models import nap


@nap.Operator
def bool_and(a: bool = False, b: bool = False) -> bool:
    return a and b


@nap.Operator
def bool_or(a: bool = False, b: bool = False) -> bool:
    return a or b


@nap.Operator
def bool_not(b: bool = False) -> bool:
    return not b


class If(nap.Operator):
    def __init__(self):
        super(If, self).__init__()
        self.cond = self.addDataInlet('condition', bool, False)
        self.trig = self.addTriggerInlet('trigger', self.__func)
        self.true = self.addTriggerOutlet('true')
        self.false = self.addTriggerOutlet('false')

    def __func(self):
        if self.cond():
            self.true()
        else:
            self.false()


class Select(nap.Operator):
    def __init__(self):
        super(Select, self).__init__()
        self.cond = self.addDataInlet('condition', bool, False)
        self.false = self.addDataInlet('false', object, None)
        self.true = self.addDataInlet('true', object, None)
        self.out = self.addDataOutlet('out', object, self.__select)

    def __select(self):
        if self.cond():
            return self.true()
        return self.false()


class ForEach(nap.Operator):
    def __init__(self):
        super(ForEach, self).__init__()
        self.__idx = 0
        self.__elm = None
        self.do = self.addTriggerInlet('do', self.__iterate)
        self.iterable = self.addDataInlet('iterable', list, [])
        self.index = self.addDataOutlet('index', int, lambda: self.__idx)
        self.element = self.addDataOutlet('element', object, lambda: self.__elm)
        self.iterate = self.addTriggerOutlet('iterate')
        self.done = self.addTriggerOutlet('done')

    def __iterate(self):
        for i, e in enumerate(self.iterable()):
            self.__idx = i
            self.__elm = e
            self.iterate()
        self.done()

class ForLoop(nap.Operator):

    def __init__(self):
        super(ForLoop, self).__init__()
        self.do = self.addTriggerInlet('do', self.__iterate)
        self.count = self.addDataInlet('count', int, 5)
        self.index = self.addDataOutlet('index', int, lambda: self.__idx)
        self.iterate = self.addTriggerOutlet('iterate')
        self.done = self.addTriggerOutlet('done')
        self.__idx = 0

    def __iterate(self):
        self.__idx = 0
        count = self.count()
        for i in range(count):
            self.__idx = i
            self.iterate()
        self.done()


class CurrentTime(nap.Operator):
    def __init__(self):
        super(CurrentTime, self).__init__()
        self.time = self.addDataOutlet('time', float, self.__getTime)

    def __getTime(self):
        return time.time()
