import os
from typing import Tuple

from napkin.models import nap

class ReadFile(nap.Operator):

    def __init__(self):
        super(ReadFile, self).__init__()
        self.filename = self.addDataInlet('filename', str, '')
        self.data = self.addDataOutlet('data', str, self.__readFile)

    def __readFile(self):
        with open(self.filename, 'r') as fh:
            return fh.read()

class WriteFile(nap.Operator):

    def __init__(self):
        super(WriteFile, self).__init__()
        self.filename = self.addDataInlet('filename', str, '')
        self.data = self.addDataInlet('data', str, '')

@nap.Operator
def SplitExt(s: str = '') -> Tuple[str, str]: return os.path.splitext(s)
