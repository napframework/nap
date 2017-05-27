import faker
import random
import threading

import time
from dummy_thread import start_new_thread
from random import randrange

import zmq
from multiprocessing import Queue

_faker = faker.Factory.create()


def fakeChange():
    return str(_faker.name()).replace(' ', '_').replace('\n', '_')

def fakeData(count=5):
    data = []
    for i in range(5):
        data.append(fakeChange())
    return data

ADDRESS = 'tcp://*:8888'


class ClientIdent(object):
    def __init__(self, ident):
        self.ident = ident
        self.lastHeartbeat = time.time()

class Server(object):

    CLIENT_TIMEOUT = 5

    def __init__(self, host=ADDRESS):
        self.running = True
        self.host = host
        self.context = zmq.Context()

        self.changeQueue = Queue()
        self.clients = {}
        self.data = None

        self.thread = threading.Thread(target=self.__runServer)
        self.thread.daemon = True
        self.thread.start()

    def exit(self):
        self.running = False

    def pushChange(self, msg):
        if not self.clients:
            return
        self.changeQueue.put('[CHNG] %s' % msg)

    def __runServer(self):
        sock = self.context.socket(zmq.ROUTER)
        sock.bind(self.host)
        print('Server running on %s' % self.host)

        poll = zmq.Poller()
        poll.register(sock, zmq.POLLIN)

        while self.running:

            # CLIENT CLEANUP
            currentTime = time.time()
            for ident in self.clients.keys():
                client = self.clients[ident]
                if currentTime - client.lastHeartbeat > self.CLIENT_TIMEOUT:
                    print("==== Disconnected (%s)" % ident)
                    del self.clients[ident]

            # REQUEST / REPLY
            sockets = dict(poll.poll(10))
            if sock in sockets:
                # REQUEST
                ident, msg = sock.recv_multipart()
                if not ident in self.clients.keys():
                    self.clients[ident] = ClientIdent(ident)
                    print('==== Connected (%s)' % ident)
                client = self.clients[ident]
                client.lastHeartbeat = time.time()
                # print('HEARTBEAT %s' % ident)

                # REPLY
                if msg:
                    print('Request (%s): "%s"' % (ident, msg))
                    reply = '[DATA] %s' % ','.join(self.data)
                    sock.send_multipart([ident, reply])

            # NOTIFY
            if not self.changeQueue.empty():
                msg = self.changeQueue.get()
                print('Notify (%s): %s' % (', '.join(self.clients.keys()), msg))
                for ident in self.clients:
                    sock.send_multipart([ident, msg])


    def __backendLoop(self):
        worker = self.context.socket(zmq.DEALER)
        worker.connect('inproc://backend')
        while True:
            ret = worker.recv_multipart()
            print(ret)
            # print('Client "%s" sent "%s"' % (ident, msg))

        worker.close()

def serverTest():
    server = Server()
    server.data = fakeData()

    # Do fake work
    while True:
        try:
            time.sleep(randrange(1, 6))
            change = server.data[randrange(0, len(server.data))]
            server.pushChange(fakeChange())
        except KeyboardInterrupt:
            break

    server.exit()


def serveClient(ctx, ident):
    print('serving client: %s' % ident)
    sock = ctx.socket(zmq.PUB)
    sock.bind('inproc://back')

    while True:
        msg = fakeChange()
        print('Sending "%s"' % msg)
        sock.send_multipart(['a', msg])
        time.sleep(1)

def testSimpleServe():
    ctx = zmq.Context()
    sock = ctx.socket(zmq.ROUTER)
    sock.bind('tcp://*:8888')
    sock.connect('inproc://back')

    poll = zmq.Poller()
    poll.register(sock, zmq.POLLIN)

    clients = []

    while True:
        if sock in dict(poll.poll(1000)):
            ident, msg = sock.recv_multipart()
            start_new_thread(serveClient, (ctx,ident))




if __name__ == '__main__':
    serverTest()
    # testSimpleServe()
