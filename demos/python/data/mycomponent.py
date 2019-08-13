import nap

class MyComponent:

    def __init__(self, entity):
        self.value = 0
        self.print = True
        self.entity = entity

    # called automatically each frame by the python component
    def update(self, currentTime, deltaTime):
        pass

    # called on destruction of the component
    def destroy(self):
        pass

    # set the value to keep
    def setValue(self, value):
        if self.print:
            print("python here! setting value " + str(value))
        self.value = value

    # if we want to print the value to console when received
    def printToConsole(self, doPrint):
        self.print = doPrint

    # return currentrly stored value
    def getValue(self):
        return self.value
