from typing import Any

from napkin.patch.operator import *


def bool_and(a: bool = False, b: bool = False) -> bool: return a and b


And = operatorFromFunction(bool_and)


def bool_or(a: bool = False, b: bool = False) -> bool: return a or b


Or = operatorFromFunction(bool_or)


def bool_not(b: bool = False) -> bool: return not b


Not = operatorFromFunction(bool_not)


class If(Operator):
    displayName = 'If'

    def __init__(self, parent):
        super(If, self).__init__(parent)
        self.cond = DataInlet(self, bool, False)
        self.trig = TriggerInlet(self, self.__func)
        self.true = TriggerOutlet(self)
        self.false = TriggerOutlet(self)

    def __func(self):
        if self.cond():
            self.true()
        else:
            self.false()


class Select(Operator):
    displayName = 'Select'

    def __init__(self, parent):
        super(Select, self).__init__(parent)
        self.cond = DataInlet(self, bool, False)
        self.false = DataInlet(self, object, None)
        self.true = DataInlet(self, object, None)
        self.out = DataOutlet(self, object, self.__select)

    def __select(self):
        if self.cond():
            return self.true()
        return self.false()


class ForEach(Operator):
    displayName = 'ForEach'

    def __init__(self, parent):
        super(ForEach, self).__init__(parent)
        self.__idx = 0
        self.__elm = None
        self.do = TriggerInlet(self, self.__iterate)
        self.iterable = DataInlet(self, list, [])
        self.index = DataOutlet(self, int, lambda: self.__idx)
        self.element = DataOutlet(self, object, lambda: self.__elm)
        self.iterate = TriggerOutlet(self)
        self.done = TriggerOutlet(self)

    def __iterate(self):
        for i, e in enumerate(self.iterable()):
            self.__idx = i
            self.__elm = e
            self.iterate()
        self.done()


def eval(code: str = '') -> object: return eval(code)


Eval = operatorFromFunction(eval)
