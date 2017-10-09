from napkin.models import nap


class PrintOperator(nap.Operator):
    def __init__(self):
        super(PrintOperator, self).__init__()
        self.print = self.addTriggerInlet('print', self._doPrint)
        self.str = self.addDataInlet('value', str, "Hello")

    def _doPrint(self):
        print(self.str())


@nap.Operator
def Concat(a: str = '', b: str = '') -> str: return a + b


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
