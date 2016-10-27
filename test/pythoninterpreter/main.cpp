#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <iostream>

// Some handy macros to help with error checking
// When prototyping, it's a good idea to check every
// system call for errors, these macros help to keep
// the code uncluttered.

#define CHECK(e) \
 ((e)? \
  (void)0: \
  (fprintf(stderr, "'%s' failed at %s:%d\n - %s\n", \
           #e, __FILE__, __LINE__,strerror(errno)), \
   exit(0)))

#define CHECKSYS(e) (CHECK((e)==0))
#define CHECKFD(e) (CHECK((e)>=0))

// We are told not to use signal, due to portability problems
// so we will define a similar function ourselves with sigaction
void setsignal(int signal, sighandler_t handler)
{
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = handler;
    CHECKSYS(sigaction(signal,&sa,NULL));
}

// Make a suitable server socket, as a small concession to
// security, we will hardwire the loopback address as the
// bind address. People elsewhere can come in through an SSH
// tunnel.
int makeserversock(int port)
{
    int serversock = socket(AF_INET,SOCK_STREAM,0);
    CHECKFD(serversock);
    sockaddr_in saddr;
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int optval = 1;
    CHECKSYS(setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR,
                        &optval, sizeof optval));
    CHECKSYS(bind(serversock,(sockaddr*)&saddr,sizeof(saddr)));
    CHECKSYS(listen(serversock,10));
    return serversock;
}

// Copy data between our socket fd and the master
// side of the pty. A simple epoll loop.
int runforwarder(int mpty, int sockfd)
{
    static const int MAX_EVENTS = 10;
    int epollfd = epoll_create(MAX_EVENTS);
    CHECKFD(epollfd);
    epoll_event event;
    memset (&event, 0, sizeof(event));
    event.events = EPOLLIN;
    event.data.fd = sockfd;
    CHECKSYS(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event));
    event.data.fd = mpty;
    CHECKSYS(epoll_ctl(epollfd, EPOLL_CTL_ADD, mpty, &event));
    char ibuff[256];
    while (true) {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        // Maybe treat EINTR specially here.
        CHECK(nfds >= 0);
        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            if (events[i].events & EPOLLIN) {
                ssize_t nread = read(fd,ibuff,sizeof(ibuff));
                CHECK(nread >= 0);
                if (nread == 0) {
                    goto finish;
                } else {
                    write(mpty+sockfd-fd,ibuff,nread);
                }
            } else if (events[i].events & (EPOLLERR|EPOLLHUP)) {
                goto finish;
            } else {
                fprintf(stderr, "Unexpected event for %d: 0x%x\n",
                        fd, events[i].events);
                goto finish;
            }
        }
    }
    finish:
    CHECKSYS(close(mpty));
    CHECKSYS(close(sockfd));
    CHECKSYS(close(epollfd));
    return 0;
}

// The "application" functions to be accessible from
// the embedded interpreter
int myinit()
{
    srand(time(NULL));
    return 0;
}

int myfunc()
{
    return rand();
}

// Python wrappers around our application functions
static PyObject*
emb_init(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":init")) return NULL;
    return Py_BuildValue("i", myinit());
}

static PyObject*
emb_func(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":func")) return NULL;
    return Py_BuildValue("i", myfunc());
}

static PyMethodDef EmbMethods[] = {
        {"init", emb_init, METH_VARARGS,
                "(Re)initialize the application."},
        {"func", emb_func, METH_VARARGS,
                "Run the application"},
        {NULL, NULL, 0, NULL}
};

int runinterpreter(char *argname, int fd)
{
    CHECKFD(dup2(fd,0));
    CHECKFD(dup2(fd,1));
    CHECKFD(dup2(fd,2));
    CHECKSYS(close(fd));

    Py_SetProgramName(argname);
    Py_Initialize();
    Py_InitModule("emb", EmbMethods);
    PyRun_SimpleString("from time import time,ctime\n");
    PyRun_SimpleString("from emb import init,func\n");
    PyRun_SimpleString("print('Today is',ctime(time()))\n");
    PyRun_SimpleString("import readline\n");
    PyRun_InteractiveLoop(stdin, "-");
    Py_Finalize();

    return 0;
}

int main(int argc, char *argv[])
{
    int port = 8888;
    std::cout << "Running on port 8888";
//    if (argc > 1) {
//        port = atoi(argv[1]);
//    } else {
//        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//        exit(0);
//    }
//
//    port = 8888;

    setsignal(SIGCHLD, SIG_IGN);
    int serversock = makeserversock(port);
    while (true) {
        int sockfd = accept(serversock,NULL,NULL);
        CHECKFD(sockfd);
        if (fork() != 0) {
            // Server side, close new connection and continue
            CHECKSYS(close(sockfd));
        } else {
            // Client side, close server socket
            CHECKSYS(close(serversock)); serversock = -1;
            // Create a pseudo-terminal
            int mpty = posix_openpt(O_RDWR);
            CHECKFD(mpty);
            CHECKSYS(grantpt(mpty)); // pty magic
            CHECKSYS(unlockpt(mpty));
            // Start our own session
            CHECK(setsid()>0);
            int spty = open(ptsname(mpty),O_RDWR);
            // spty is now our controlling terminal
            CHECKFD(spty);
            // Now split into two processes, one copying data
            // between socket and pty; the other running the
            // actual interpreter.
            if (fork() != 0) {
                CHECKSYS(close(spty));
                // Ignore sigint here
                setsignal(SIGINT, SIG_IGN);
                return runforwarder(sockfd,mpty);
            } else {
                CHECKSYS(close(sockfd));
                CHECKSYS(close(mpty));
                // Default sigint here - will be replace by interpreter
                setsignal(SIGINT, SIG_DFL);
                return runinterpreter(argv[0],spty);
            }
        }
    }
}