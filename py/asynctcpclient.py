from multiprocessing import Lock
from threading import Thread

import zmq


class AsyncTCPClient(object):
    """ Asynchronous ZMQ TCP client, connecting to a NAP Application
    """

    def __init__(self, host):
        super(AsyncTCPClient, self).__init__()
        self.logging = False
        self.identity = self.newClientName()
        self.host = host
        self.running = True
        thread = Thread(target=self.__clientLoop)
        thread.daemon = True
        thread.start()
        self.sendQueue = []
        self.lastRequest = None
        self.lock = Lock()


    def __log(self, msg):
        if self.logging:
            print(msg)

    @classmethod
    def newClientName(cls):
        """ Generate a client identity
         TODO: Should be uuid
        """
        import faker
        _faker = faker.Factory.create()
        name = _faker.name().split(' ')[-1]
        return str(str(name).replace(' ', '_'))

    def exit(self):
        self.running = False

    def __clientLoop(self):
        self.__log('Client "%s" connecting to "%s"' % (self.identity, self.host))
        context = zmq.Context()
        sock = context.socket(zmq.DEALER)
        sock.setsockopt_string(zmq.IDENTITY, str(self.identity))
        sock.connect(self.host)
        poll = zmq.Poller()
        poll.register(sock, zmq.POLLIN)

        while self.running:
            sockets = dict(poll.poll(100))
            # NOTIFY
            if sock in sockets:
                self.onMessageReceived(sock.recv_string())

            # REQUEST / REPLY
            if self.sendQueue:
                while self.sendQueue:
                    msg = self.sendQueue.pop(0)
                    self.__log('Sending: %s' % msg)
                    sock.send_string(msg, zmq.NOBLOCK)

            else:
                sock.send_string("", zmq.NOBLOCK) # Send heartbeat

    def onMessageReceived(self, msg):
        raise NotImplementedError()

    def __request(self, msg):
        self.lock.acquire()
        if not self.sendQueue or msg != self.sendQueue[-1]:
            self.__log('-> %s' % msg)
            self.sendQueue.append(msg)
        self.lock.release()

    def __onReceive(self, msg):
        self.onMessageReceived(msg)

    def request(self, msg):
        self.__request(msg)

# def clientTest():
#     client = AsyncTCPClient()
#     client.getObjectTree()
#
#     def onMessageReceived(msg):
#         print('Message Received: %s' % msg)
#
#     client.onMessageReceived = onMessageReceived
#
#     while True:
#         try:
#             time.sleep(1)
#             if random.uniform(0,1) < .2:
#                 client.getObjectTree()
#         except KeyboardInterrupt:
#             break
#     client.exit()
#
# if __name__ == '__main__':
#     clientTest()
#     # testSimpleClient()
