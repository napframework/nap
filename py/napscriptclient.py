from jsonrpc2_zeromq.client import RPCClient

class Object(object):

    __rpc = RPCClient("tcp://localhost:8888")

    def __init__(self, ptr):
        self.__ptr = ptr

    @classmethod
    def root(cls):
        return Object(cls.__rpc.getRoot())

    def setName(self, name):
        self.__rpc.setName(self.__ptr, name)

    def name(self):
        return self.__rpc.getName(self.__ptr)

    def typename(self):
        return self.__rpc.getTypeName(self.__ptr)

    def parent(self):
        return self.__rpc.getParent(self.__ptr)

    def addChild(self, typename, name):
        return self.__rpc.addChild(self.__ptr, typename, name);

    def addEntity(self, name):
        return Object(self.__rpc.addEntity(self.__ptr, name))

    def children(self):
        for childPtr in self.__rpc.getAllChildren(self.__ptr):
            yield Object(childPtr)

    def __repr__(self):
        return '<%s> %s' % (self.typename(), self.name())


def run():
    root = Object.root()
    root.setName('root')

    print(root)
    for c in root.children():
        print(c)


if __name__ == '__main__':
    run()