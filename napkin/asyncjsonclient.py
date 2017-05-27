import json
import uuid

from PyQt5.QtCore import *
from asynctcpclient import AsyncTCPClient


class SignalEmitter(QObject):
    messageReceived = pyqtSignal(dict)

    def __init__(self):
        super(SignalEmitter, self).__init__()


class _Callable(object):

    def __init__(self, client, name):
        self.__client = client
        self.__name = name

    def __call__(self, *args):
        params = []
        for arg in args:
            params.append(arg)

        payload = {
            'method': self.__name,
            'params': params,
            'jsonrpc': '2.0',
            'id': self.__name
        }
        self.__client.request(json.dumps(payload))

class RPCError(Exception):
    pass

class AsyncJSONClient(AsyncTCPClient):
    messageReceived = None

    def __init__(self, host):
        super(AsyncJSONClient, self).__init__(host)
        self.__signalEmitter = SignalEmitter()
        self.messageReceived = self.__signalEmitter.messageReceived

    def onMessageReceived(self, msg):
        reply = json.loads(msg)
        if 'error' in reply:
            raise RPCError(reply['error']['message'])

        id = reply['id']


        self.messageReceived.emit(json.loads(msg))

    def __getattr__(self, item):
        return _Callable(self, item)
