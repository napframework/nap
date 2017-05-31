from PyQt5.QtCore import pyqtSignal, QObject


class Object(QObject):

    nameChanged = pyqtSignal(str)
    childAdded = pyqtSignal(object)
    childRemoved = pyqtSignal(object)

    def __init__(self):
        super(Object, self).__init__()


class Entity(Object):
    def __init__(self):
        super(Entity, self).__init__()


class Component(Object):
    def __init__(self):
        super(Component, self).__init__()


class Operator(Object):
    def __init__(self):
        super(Operator, self).__init__()


class Attribute(Object):
    def __init__(self):
        super(Attribute, self).__init__()

class Plug(Object):
    def __init__(self):
        super(Plug, self).__init__()


class Core(QObject):
    """ Core represents a NAP Application structure.
    Use an instance of this class to 'speak' to a NAP application over RPC
    Connects to a NAP Core RPC Server over the network or locally in order to inspect and modify its data.
    """

    # Emits when the NAP rpc server has sent any message
    messageReceived = pyqtSignal()

    # Emit when the root object has been replaced on server side
    rootChanged = pyqtSignal()

    # Emits when module data has been changed, usually when a new connections has been made
    moduleInfoChanged = pyqtSignal(object)

    # Emits when the NAP type hierarchy has changed on the server side
    typeHierarchyChanged = pyqtSignal()

    # Emits when a NAP object has been removed
    objectRemoved = pyqtSignal(object)

    # Emits when a NAP log message has been committed
    logMessageReceived = pyqtSignal(int, str, str)

    # Emits when the client has sent a message and is waiting for a result
    waitingForMessage = pyqtSignal()

    def __init__(self):
        super(Core, self).__init__()

    def root(self):
        raise NotImplementedError()