#!/usr/bin/python
# -*- coding: utf-8 -*-
import os, sys, time
import threading
import weakref
from collections import deque

# Version dependent defs
V2 = sys.version_info[0] == 2
if V2:
    D = __builtins__
    if not isinstance(D, dict):
        D = D.__dict__
    bytes = D['str']
    str = D['unicode']
    xrange = D['xrange']
    basestring = basestring
    long = long
else:
    basestring = str  # to check if instance is string
    bytes, str = bytes, str
    long = int # for the port
    xrange = range


def Property(function):
    """ Property(function)
    
    A property decorator which allows to define fget, fset and fdel
    inside the function.
    
    Note that the class to which this is applied must inherit from object!
    Code based on an example posted by Walker Hale:
    http://code.activestate.com/recipes/410698/#c6
    
    """
    
    # Define known keys
    known_keys = 'fget', 'fset', 'fdel', 'doc'
    
    # Get elements for defining the property. This should return a dict
    func_locals = function()
    if not isinstance(func_locals, dict):
        raise RuntimeError('Property function should "return locals()".')
    
    # Create dict with kwargs for property(). Init doc with docstring.
    D = {'doc': function.__doc__}
    
    # Copy known keys. Check if there are invalid keys
    for key in func_locals.keys():
        if key in known_keys:
            D[key] = func_locals[key]
        else:
            raise RuntimeError('Invalid Property element: %s' % key)
    
    # Done
    return property(**D)


def getErrorMsg():
    """ getErrorMsg()
    Return a string containing the error message. This is usefull, because
    there is no uniform way to catch exception objects in Python 2.x and
    Python 3.x.
    """
    
    # Get traceback info
    type, value, tb = sys.exc_info()
    
    # Store for debugging?
    if True:        
        sys.last_type = type
        sys.last_value = value
        sys.last_traceback = tb
    
    # Print
    err = ''
    try:
        if not isinstance(value, (OverflowError, SyntaxError, ValueError)):
            while tb:
                err = "line %i of %s." % (
                        tb.tb_frame.f_lineno, tb.tb_frame.f_code.co_filename)
                tb = tb.tb_next
    finally:
        del tb
    return str(value) + "\n" + err


class PackageQueue(object):
    """ PackageQueue(N, discard_mode='old')
    
    A queue implementation that can be used in blocking and non-blocking 
    mode and allows peeking. The queue has a limited size. The user
    can specify whether old or new messages should be discarted.
    
    Uses a deque object for the queue and a threading.Condition for
    the blocking.
    
    """
    
    class Empty(Exception):
        def __init__(self):
            Exception.__init__(self, 'pop from an empty PackageQueue')
        pass
    
    
    def __init__(self, N, discard_mode='old'):
        
        # Instantiate queue and condition
        self._q = deque()
        self._condition = threading.Condition()
        
        # Store max number of elements in queue
        self._maxlen = int(N)
        
        # Store discard mode as integer
        discard_mode = discard_mode.lower()
        if discard_mode == 'old':
            self._discard_mode = 1
        elif discard_mode == 'new':
            self._discard_mode = 2
        else:
            raise ValueError('Invalid discard mode.')
    
    
    def full(self):
        """ full()
        
        Returns True if the number of elements is at its maximum right now.
        Note that in theory, another thread might pop an element right 
        after this function returns.
        
        """ 
        return len(self) >= self._maxlen
    
    
    def empty(self):
        """ empty()
        
        Returns True if the number of elements is zero right now. Note 
        that in theory, another thread might add an element right
        after this function returns.
        
        """
        return len(self) == 0
    
    
    def push(self, x):
        """ push(item)
        
        Add an item to the queue. If the queue is full, the oldest
        item in the queue, or the given item is discarted.
        
        """
        
        condition = self._condition
        condition.acquire()
        try:
            q = self._q
        
            if len(q) < self._maxlen:
                # Add now and notify any waiting threads in get()
                q.append(x)
                condition.notify() # code at wait() procedes
            else:
                # Full, either discard or pop (no need to notify)
                if self._discard_mode == 1:
                    q.popleft() # pop old
                    q.append(x)
                elif self._discard_mode == 2:
                    pass # Simply do not add
        
        finally:
            condition.release()
    
    
    def insert(self, x):
        """ insert(x)
        
        Insert an item at the front of the queue. A call to pop() will
        get this item first. This should be used in rare circumstances
        to give an item priority. This method never causes items to
        be discarted.
        
        """
        
        condition = self._condition
        condition.acquire()
        try:
            self._q.appendleft(x)
            condition.notify() # code at wait() procedes
        finally:
            condition.release()

    def remove(self, x):
        """ remove(x)

        Insert an item at the front of the queue. A call to pop() will
        get this item first. This should be used in rare circumstances
        to give an item priority. This method never causes items to
        be discarted.
        
        """
        
        condition = self._condition
        condition.acquire()
        try:
            self._q.remove(x)
            condition.notify() # code at wait() procedes
        finally:
            condition.release()

    
    def pop(self, block=True):
        """ pop(block=True)
        
        Pop the oldest item from the queue. If there are no items in the
        queue:
          * the calling thread is blocked until an item is available
            (if block=True, default);
          * an PackageQueue.Empty exception is raised (if block=False);
          * the calling thread is blocked for 'block' seconds (if block
            is a float).
        
        """
        
        condition = self._condition
        condition.acquire()
        try:
            q = self._q
            
            if not block:
                # Raise empty if no items in the queue
                if not len(q):
                    raise self.Empty()
            elif block is True:
                # Wait for notify (awakened does not guarantee len(q)>0)
                while not len(q):
                    condition.wait()
            elif isinstance(block, float):
                # Wait if no items, then raise error if still no items
                if not len(q):
                    condition.wait(block) 
                    if not len(q):
                        raise self.Empty()
            else:
                raise ValueError('Invalid value for block in PackageQueue.pop().')
            
            # Return item
            return q.popleft()
        
        finally:
            condition.release()
    
    
    def peek(self, index=0):
        """ peek(index=0)
        
        Get an item from the queue without popping it. index=0 gets the
        oldest item, index=-1 gets the newest item. Note that index access
        slows to O(n) time in the middle of the queue (due to the undelying
        deque object).
        
        Raises an IndexError if the index is out of range.
        
        """
        return self._q[index]
    
    
    def __len__(self):
        return self._q.__len__()
    
    
    def clear(self):
        """ clear()
        
        Remove all items from the queue.
        
        """
        
        self._condition.acquire()
        try:
            self._q.clear()
        finally:
            self._condition.release()



class CallableObject(object):
    """ CallableObject(callable)
    
    A class to hold a callable. If it is a plain function, its reference
    is held (because it might be a closure). If it is a method, we keep
    the function name and a weak reference to the object. In this way,
    having for instance a signal bound to a method, the object is not
    prevented from being cleaned up.
    
    """
    __slots__ = ['_ob', '_func']  # Use __slots__ to reduce memory footprint
    
    def __init__(self, c):
        
        # Check
        if not hasattr(c, '__call__'):
            raise ValueError('Error: given callback is not callable.')
        
        # Store funcion and object
        if hasattr(c, '__self__'):
            # Method, store object and method name
            self._ob = weakref.ref(c.__self__)
            self._func = c.__func__.__name__
        elif hasattr(c, 'im_self'): 
            # Method in older Python
            self._ob = weakref.ref(c.im_self)
            self._func = c.im_func.__name__
        else:
            # Plain function
            self._func = c
            self._ob = None
    
    def isdead(self):
        """ Get whether the weak ref is dead. 
        """
        if self._ob:
            # Method
            return self._ob() is None
        else:
            return False
    
    def compare(self, other):
        """ compare this instance with another.
        """
        if self._ob and other._ob:
            return (self._ob() is other._ob()) and (self._func == other._func)
        elif not (self._ob or other._ob):
            return self._func == other._func
        else:
            return False
    
    def __str__(self):
        return self._func.__str__()
    
    def call(self, *args, **kwargs):
        """ call(*args, **kwargs)
        Call the callable. Exceptions are caught and printed.
        """
        if self.isdead():
            return
        
        # Get function
        try:
            if self._ob:
                func = getattr(self._ob(), self._func)
            else:
                func = self._func
        except Exception:
            return
        
        # Call it
        try:
            return func(*args, **kwargs)
        except Exception:
            print('Exception while handling event:')
            print(getErrorMsg())



class Event(object):
    """ Event(callable, *args, **kwargs)
    
    An Event instance represents something that is going to be done.
    It consists of a callable and arguments to call it with.
    
    Instances of this class populate the event queue.
    
    """
    __slots__ = ['_callable', '_args', '_kwargs', '_timeout'] 
    def __init__(self, callable, *args, **kwargs):
        if isinstance(callable, CallableObject):
            self._callable = callable
        else:
            self._callable = CallableObject(callable)
        self._args = args
        self._kwargs = kwargs
    
    def dispatch(self):
        """ dispatch()
        Call the callable with the arguments and keyword-arguments specified
        at initialization.
        """
        self._callable.call(*self._args, **self._kwargs)


class Signal(object):
    """ Signal()
    
    The purpose of a signal is to provide an interface to coennetc/disconnect 
    to events and to fire them. 
    
    One can coennetc() or disconnect() a callable to the signal. When emitted, an
    event is created for each bound handler. Therefore, the event loop
    must run for signals to work.
    
    Some signals call the handlers using additional arguments to 
    specify specific information.
    
    """
    
    def __init__(self):
        super(Signal, self).__init__()
        self._handlers = []
        self._thread = threading.current_thread()
    
    @property
    def type(self):
        """ The type (__class__) of this event. 
        """
        return self.__class__
    
    
    def connect(self, func):
        """ connect(func)
        
        Add an eventhandler to this event.             
        
        The callback/handler (func) must be a callable. It is called
        with one argument: the event instance, which can contain 
        additional information about the event.
        
        """
        
        # make callable object (checks whether func is callable)
        cnew = CallableObject(func)
        
        # add the handler
        self._handlers.append(cnew)


    def disconnect(self, func=None):
        """ disconnect(func=None)
        
        Unsubscribe a handler and remove event of func, If func is None, remove all handlers and remove all event from _event_queue

        """
        if func is None:
            for c  in self._handlers[:]:
                self.removeEventFromQueue(c)
            self._handlers[:] = []
        else:
            cref = CallableObject(func)
            for c in [c for c in self._handlers]:
                # remove if callable matches func or object is destroyed
                if c.compare(cref) or c.isdead():
                    self._handlers.remove(c)

            self.removeEventFromQueue(cref)

    def removeEventFromQueue(self, cref):
        '''
            remove event of cref from _event_queue
        '''
        def removeEvents():
            removeEvents = []
            for event in eventLoop._event_queue._q:
                if event._callable.compare(cref) or event._callable.isdead():
                    removeEvents.append(event)
            for event in removeEvents:
                eventLoop._event_queue.remove(event)

        if cref._ob is not None and cref._ob():
            if isinstance(cref._ob(), QPObject):
                if isinstance(cref._ob().thread, EventThread):
                    eventLoop = cref._ob().thread.eventLoop
                else:
                    eventLoop = QPLoop.instance()
                removeEvents()
            else:
                raise RuntimeError("%s must be an QPObject and the it should  belongs to thread which upport EventLoop" % func._ob())
        else:
            try:
                eventLoop = QPLoop.instance()
                removeEvents()
            except:
                raise RuntimeError("%s must be an QPObject and the it should  belongs to thread which upport EventLoop" % func._ob())

    def emit(self, *args, **kwargs):
        """ emit(*args, **kwargs)
        
        Emit the signal, calling all bound callbacks with *args and **kwargs.
        An event is queues for each callback registered to this signal.
        Therefore it is safe to call this method from another thread.
        
        """
        
        # Add an event for each callback
        toremove = []
        for func in self._handlers:
            if func.isdead():
                toremove.append(func)
            else:
                event = Event(func, *args, **kwargs)
                if func._ob is not None and func._ob():
                    if isinstance(func._ob(), QPObject):
                        if isinstance(func._ob().thread, EventThread):
                            eventLoop = func._ob().thread.eventLoop
                        else:
                            eventLoop = QPLoop.instance()
                        eventLoop.post_event(event)
                    else:
                        raise RuntimeError("%s must be an QPObject and the it should  belongs to thread which upport EventLoop" % func._ob())
                else:
                    try:
                        eventLoop = QPLoop.instance()
                        eventLoop.post_event(event)
                    except:
                        raise RuntimeError("%s must be an QPObject and the it should  belongs to thread which upport EventLoop" % func._ob())

        # Remove dead ones
        for func in toremove:
            self._handlers.remove(func)

    def emit_now(self, *args, **kwargs):
        """ emit_now(*args, **kwargs)
        
        Emit the signal *now*. All handlers are called from the calling
        thread. Beware, this should only be done from the same thread
        that runs the event loop.
        
        """
        
        # Add an event for each callback
        toremove = []
        for func in self._handlers:
            if func.isdead():
                toremove.append(func)
            else:
                func.call(*args, **kwargs)
        
        # Remove dead ones
        for func in toremove:
            self._handlers.remove(func)



class TheTimerThread(threading.Thread):
    """ TheTimerThread is a singleton thread that is used by all timers
    and delayed events to wait for a while (in a separate thread) and then
    post an event to the event-queue. By sharing a single thread timers
    stay lightweight and there is no time spend on initializing or tearing
    down threads. The downside is that when there are a lot of timers running
    at the same time, adding a timer may become a bit inefficient because
    the registered objects must be sorted each time an object is added.
    """
    def __init__(self):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self._exit = False
        self._timers = []
        self._somethingChanged = False
        self._condition = threading.Condition(threading.Lock())
    
    def stop(self, timeout=1.0):
        self._exit = True
        self._condition.acquire()
        try:
            self._condition.notify()
        finally:
            self._condition.release()
        self.join(timeout)
    
    def add(self, timer):
        """ add(timer)
        Add item to the list of objects to track. The object should
        have a _timeout attribute, representing the time.time() at which 
        it runs out, and an _on_timeout() method to call when it does. 
        """
        # Check
        if not (hasattr(timer, '_timeout') and hasattr(timer, '_on_timeout')):
            raise ValueError('Cannot add this object to theTimerThread.')
        # Add item
        self._condition.acquire()
        try:
            if timer not in self._timers:
                self._timers.append(timer)
                self._sort()
                self._somethingChanged = True
            self._condition.notify()
        finally:
            self._condition.release()
    
    def _sort(self):
        self._timers = sorted(self._timers, 
                key=lambda x: x._timeout, reverse=True)
    
    def discard(self, timer):
        """Stop the timer if it hasn't finished yet"""
        self._condition.acquire()
        try:
            if timer in self._timers:
                self._timers.remove(timer)
            self._somethingChanged = True
            self._condition.notify()
        finally:
            self._condition.release()
    
    def run(self):
        self._condition.acquire()
        try:
            self._mainloop()
        finally:
            self._condition.release()
    
    def _mainloop(self):
        while not self._exit:
            
            # Set flag
            self._somethingChanged = False
            
            # Wait here, in wait() the undelying lock is released
            if self._timers:
                timer = self._timers[-1]
                timeout = timer._timeout - time.time()
                if timeout > 0:
                    self._condition.wait(timeout)
            else:
                timer = None
                self._condition.wait()
            
            # Here the lock has been re-acquired. Take action?
            if self._exit:
                break
            if (timer is not None) and (not self._somethingChanged):
                if timer._on_timeout():
                    self._sort()  # Keep and resort
                else:
                    self._timers.pop() # Pop

# Instantiate and start the single timer thread
# We can do this as long as we do not wait for the threat, and the threat
# does not do any imports:
# http://docs.python.org/library/threading.html#importing-in-threaded-code
theTimerThread = TheTimerThread()
theTimerThread.start()



class Timer(Signal):
    """ Timer(interval=1.0, oneshot=False) 
    
    Timer class. You can coennetc callbacks to the timer. The timer is 
    fired when it runs out of time. 
    
    Parameters
    ----------
    interval : number
        The interval of the timer in seconds.
    oneshot : bool
        Whether the timer should do a single shot, or run continuously.
    
    """
    
    def __init__(self, interval=1.0, oneshot=False):
        Signal.__init__(self)
        
        # store Timer specific properties        
        self.interval = interval
        self.oneshot = oneshot
        #
        self._timeout = 0
    
    
    @Property
    def interval():
        """ Set/get the timer's interval in seconds.
        """
        def fget(self):
            return self._interval
        def fset(self, value):
            if not isinstance(value, (int, float)):
                raise ValueError('interval must be a float or integer.')
            if value <= 0:
                raise ValueError('interval must be larger than 0.')
            self._interval = float(value)
        return locals()
    
    
    @Property
    def oneshot():
        """ Set/get whether this is a oneshot timer. If not is runs
        continuously.
        """
        def fget(self):
            return self._oneshot
        def fset(self, value):
            self._oneshot = bool(value)
        return locals()
    
    
    @property
    def running(self):
        """ Get whether the timer is running. 
        """
        return self._timeout > 0
    
    
    def start(self, interval=None, oneshot=None):
        """ start(interval=None, oneshot=None)
        
        Start the timer. If interval or oneshot are not given, 
        their current values are used.
        
        """
        # set properties?
        if interval is not None:
            self.interval = interval
        if oneshot is not None:
            self.oneshot = oneshot
        
        # put on
        self._timeout = time.time() + self.interval
        theTimerThread.add(self)
    
    
    def stop(self):
        """ stop()
        
        Stop the timer from running. 
        
        """
        theTimerThread.discard(self)
        self._timeout = 0
    
    
    def _on_timeout(self):
        """ Method to call when the timer finishes. Called from 
        event-loop-thread.
        """
        
        # Emit signal
        self.emit()
        #print('timer timeout', self.oneshot)
        # Do we need to stop it now, or restart it
        if self.oneshot:
            # This timer instance is removed from the list of Timers
            # when the timeout is reached.
            self._timeout = 0
            return False
        else:
            # keep in the thread
            self._timeout = time.time() + self.interval
            return True



class QPLoop(object):
    """ 
        
    """
    _instance = None

    def __init__(self, defaut_event_queue_len=10000, defaut_period_event_t=3.0):
        super(QPLoop, self).__init__()

        self.defaut_event_queue_len = defaut_event_queue_len
        self.defaut_period_event_t = defaut_period_event_t

        # Event queues
        self._event_queue = PackageQueue(defaut_event_queue_len, 'new')
        
        # Flag to stop event loop
        self._stop_event_loop = False
        
        # Flag to signal whether we are in an event loop
        # Can be set externally if the event loop is hijacked.
        self._in_event_loop = False
        
        # To allow other event loops to embed the QPLoop
        self._embedding_callback1 = None # The reference
        self._embedding_callback2 = None # Used in post_event

    @classmethod
    def instance(cls):
        '''
            The global instance shoud be an  singleton lived in MainThread.
        '''
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @classmethod
    def isMainLoopSupported(cls):
        if cls._instance is None:
            return False
        else:
            return True

    @classmethod
    def isMainLoopStarted(cls):
        if cls._instance is None:
            return False
        else:
            return cls._instance._in_event_loop

    def call_later(self, func, timeout=0.0, *args, **kwargs):
        """ call_later(func, timeout=0.0, *args, **kwargs)
        
        Call the given function after the specified timeout.
        
        Parameters
        ----------
        func : callable
            The function to call.
        timeout : number
            The time to wait in seconds. If zero, the event is put on the event
            queue. If negative, the event will be put at the front of the event
            queue, so that it's processed asap.
        args : arguments
            The arguments to call func with.
        kwargs: keyword arguments.
            The keyword arguments to call func with.
        
        """
        
        # Wrap the object in an event
        event = Event(func, *args, **kwargs)
        
        # Put it in the queue
        if timeout > 0:
            self.post_event_later(event, timeout)
        elif timeout < 0:
            self.post_event_asap(event) # priority event
        else:
            self.post_event(event)
    
    
    def post_event(self, event):
        """ post_event(events)
        
        Post an event to the event queue.
        
        """
        self._event_queue.push(event)
        #
        if  self._embedding_callback2 is not None:
            self._embedding_callback2 = None
            self._embedding_callback1()
    
    
    def post_event_asap(self, event):
        """ post_event_asap(event)
        
        Post an event to the event queue. Handle as soon as possible;
        putting it in front of the queue.
        
        """
        self._event_queue.insert(event)
        #
        if  self._embedding_callback2 is not None:
            self._embedding_callback2 = None
            self._embedding_callback1()
    
    
    def post_event_later(self, event, delay):
        """ post_event_later(event, delay)
        
        Post an event to the event queue, but with a certain delay.
        
        """
        event._timeout = time.time() + delay
        theTimerThread.add(event)
        # Calls post_event in due time
    
    
    def process_events(self, block=True):
        """ process_events(block=False)
        
        Process all events currently in the queue. 
        This function should be called periodically
        in order to keep the event system running.
        
        block can be False (no blocking), True (block), or a float 
        blocking for maximally 'block' seconds.
        
        """
        # Reset callback for the embedding event loop
        self._embedding_callback2 = self._embedding_callback1
        
        # Process events
        try:
            while True:
                event = self._event_queue.pop(block)
                event.dispatch()
                # block = False # Proceed until there are now more events
        except PackageQueue.Empty:
            # print("PackageQueue empty")
            pass
    
    
    def start_event_loop(self):
        """ start_event_loop()
        
        Enter an event loop that keeps calling self.process_events().
        The event loop can be stopped using stop_event_loop().
        
        """

        # Set flags
        self._stop_event_loop = False
        self._in_event_loop = True
        
        try:
            # block until event comes
            while not self._stop_event_loop:
                self.process_events(True)
        finally:
            # Unset flag
            self._in_event_loop = False

    def start(self):
        self.start_event_loop()

    def stop_event_loop(self):
        """ stop_event_loop()
        
        Stops the event loop if it is running.
        
        """
        if not self._stop_event_loop:
            # Signal stop
            self._stop_event_loop = True
            # Push an event so that process_events() unblocks
            def dummy(): pass
            self.post_event(Event(dummy))
    
    
    def embed_event_loop(self, callback):
        """ embed_event_loop(callback)
        
        Embed the QPLoop event loop in another event loop. The given callback
        is called whenever a new  QPLoop event is created. The callback
        should create an event in the other event-loop, which should
        lead to a call to the process_events() method. The given callback
        should be thread safe.
        
        Use None as an argument to disable the embedding. 
        
        """
        self._embedding_callback1 = callback
        self._embedding_callback2 = callback

# Global instance of QPLoop
globalLoop = QPLoop.instance()


class EventThread(threading.Thread):

    """Each eventThread has an evntloop and start when initialized"""

    def __init__(self, *arg, **kwargs):
        super(EventThread, self).__init__()
        self._eventLoop = QPLoop()
        self.start()

    def run(self):
        self.exec_()

    def exec_(self):
        self._eventLoop.start_event_loop()

    @property
    def eventLoop(self):
        return self._eventLoop


class QPObject(object):
    '''
        Basic object of eventloop 
    '''

    def __init__(self, *args, **kwargs):
        super(QPObject, self).__init__()
        self._thread = threading.current_thread()

    @property
    def thread(self):
        '''
            return the thread associate with QPObject instance
        '''
        return self._thread

    def moveToThread(self, thread):
        '''
            change the thread associate with QPObject instance
        '''
        if isinstance(thread, threading.Thread):
            if thread.name == 'MainThread':
                pass
            else:
                flag_thread = isinstance(thread, EventThread)
                flag_loop = hasattr(thread, 'eventLoop')
                if flag_thread and flag_loop:
                    self._thread = thread
                    for attr, obj in self.__dict__.items():
                        if isinstance(obj, QPObject):
                            obj.moveToThread(thread)
                    return True
                elif flag_thread and not flag_loop:
                    raise RuntimeError('%s must has an eventLoop.' %repr(thread))
                else:
                    raise RuntimeError('%s must be an EventThread.' % repr(thread))
        else:
            raise RuntimeError('%s must be an thread object.' % repr(thread))
