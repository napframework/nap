from patch.operator import *


class PrintOperator(Operator):
    displayName = 'Print'

    def __init__(self):
        super(PrintOperator, self).__init__()
        self.print = self._triggerInlet('print', lambda: print(self.str))
        self.str = self._dataInlet('str', str, "Hello")


def concat(a: str = '', b: str = '') -> str: return a + b
Concat = operatorFromFunction(concat)

def lower(s: str = '') -> str: return s.lower()
Lower = operatorFromFunction(lower)

def upper(s: str = '') -> str: return s.upper()
Upper = operatorFromFunction(upper)

def endswith(s: str = '', end: str = '') -> bool: return s.endswith(end)
Endswith = operatorFromFunction(endswith)

def equals(a: str = '', b: str = '') -> bool: return a == b
Equals = operatorFromFunction(equals)

def split(s:str='', delim:str=',')->Iterable[str]: return s.split(delim)
Split = operatorFromFunction(split)