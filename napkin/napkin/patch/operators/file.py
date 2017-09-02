from napkin.patch.operator import *

class ReadFile(Operator):
    displayName = 'readfile'

    def __init__(self, parent):
        super(ReadFile, self).__init__(parent)
        self.filename = DataInlet(self, str, '')
        self.data = DataOutlet(self, str, self.__readFile)

    def __readFile(self):
        with open(self.filename, 'r') as fh:
            return fh.read()

class WriteFile(Operator):
    displayName = 'writefile'

    def __init__(self, parent):
        super(WriteFile, self).__init__(parent)
        self.filename = DataInlet(self, str, '')
        self.data = DataInlet(self, str, '')
