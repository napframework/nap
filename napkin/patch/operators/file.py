from patch.operator import *

class ReadFile(Operator):
    displayName = 'readfile'

    def __init__(self):
        super(ReadFile, self).__init__()
        self.filename = self._dataInlet('filename', str, '')
        self.data = self._dataOutlet('data', str, self.__readFile)
        # self.start = self._triggerInlet('trigger')

    def __readFile(self):
        with open(self.filename, 'r') as fh:
            return fh.read()

class WriteFile(Operator):
    displayName = 'writefile'

    def __init__(self):
        self.filename = self._dataInlet('filename', str, '')
        self.data = self._dataInlet('data', str, '')
