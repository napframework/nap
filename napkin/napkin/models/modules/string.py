from napkin.models import nap


class PrintOperator(nap.Operator):
    def __init__(self):
        super(PrintOperator, self).__init__()
        self.print = self.addTriggerInlet('print', self._doPrint)
        self.str = self.addDataInlet('value', str, "Hello")
        self.done = self.addTriggerOutlet('done')

    def _doPrint(self):
        print(self.str())
        self.done()


class Concat(nap.Operator):
    def __init__(self):
        super(Concat, self).__init__()
        self.a = self.addDataInlet('a', str, '')
        self.b = self.addDataInlet('b', str, '')
        self.result = self.addDataOutlet('result', str, self.__concat)

    def __concat(self):
        return self.a() + self.b()

class Lower(nap.Operator):
    def __init__(self):
        super(Lower, self).__init__()
        self.s = self.addDataInlet('s', str, '')
        self.result = self.addDataOutlet('result', str, self.__lower)

    def __lower(self):
        return self.s().lower()

@nap.Operator
def Lower(s: str = '') -> str: return s.lower()


@nap.Operator
def Upper(s: str = '') -> str: return s.upper()


@nap.Operator
def Endswith(s: str = '', end: str = '') -> bool: return s.endswith(end)


@nap.Operator
def Equals(a: str = '', b: str = '') -> bool: return a == b


@nap.Operator
def Split(s: str = '', delim: str = ',') -> list: return s.split(delim)

# @nap.Operator
# def MultiArgs(a: str = '', b: int = 3, c: bool = True,
#               d: float = 0.3) -> None: return None
