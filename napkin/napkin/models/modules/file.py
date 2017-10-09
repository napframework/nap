import os
from typing import Tuple

from napkin.models import nap


class ReadFile(nap.Operator):
    def __init__(self):
        super(ReadFile, self).__init__()
        self.trigger = self.addTriggerInlet('read', self.__readFile)
        self.filename = self.addDataInlet('filename', str, '')

        self.exit = self.addTriggerOutlet('done')
        self.data = self.addDataOutlet('data', str, lambda: self.__data)
        self.__data = None

    def __readFile(self):
        with open(self.filename(), 'r') as fh:
            self.__data = fh.read()
        self.exit()


class WriteFile(nap.Operator):
    def __init__(self):
        super(WriteFile, self).__init__()
        self.trigger = self.addTriggerInlet('write', self._write)
        self.filename = self.addDataInlet('filename', str, '')
        self.data = self.addDataInlet('data', str, '')

    def _write(self):
        with open(self.filename(), 'w') as fp:
            fp.write(self.data())


@nap.Operator
def SplitExt(s: str = '') -> Tuple[str, str]: return os.path.splitext(s)
