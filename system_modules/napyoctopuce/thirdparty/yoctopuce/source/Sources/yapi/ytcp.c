/*********************************************************************
 *
 * $Id: ytcp.c 29739 2018-01-25 17:03:29Z seb $
 *
 * Implementation of a client TCP stack
 *
 * - - - - - - - - - License information: - - - - - - - - -
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing
 *  with Yoctopuce products.
 *
 *  You may reproduce and distribute copies of this file in
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/

#define __FILE_ID__  "ytcp"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ydef.h"
#if defined(WINDOWS_API) && !defined(_MSC_VER)
#define _WIN32_WINNT 0x501
#endif
#ifdef WINDOWS_API
typedef int socklen_t;
#if defined(__BORLANDC__)
#pragma warn -8004
#pragma warn -8019
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma warn +8004
#pragma warn +8019
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#endif
#include "ytcp.h"
#include "yproto.h"
#include "yhash.h"

#ifdef WIN32
    #ifndef WINCE
        #include <iphlpapi.h>
        #if defined(_MSC_VER) || defined (__BORLANDC__)
            #pragma comment(lib, "Ws2_32.lib")
        #endif
    #else
        #pragma comment(lib, "Ws2.lib")
    #endif
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
#endif



//#define DEBUG_SLOW_TCP
//#define TRACE_TCP_REQ
//#define PERF_TCP_FUNCTIONS
#ifdef PERF_TCP_FUNCTIONS


typedef struct {
    yPerfMon  TCPOpen_socket;
    yPerfMon  TCPOpen_connect;
    yPerfMon  TCPOpen_setsockopt_noblock;
    yPerfMon  TCPOpen_setsockopt_nodelay;
    yPerfMon  TCPOpenReq_wait;
    yPerfMon  TCPOpenReq;
    yPerfMon  tmp1;
    yPerfMon  tmp2;
    yPerfMon  tmp3;
    yPerfMon  tmp4;
} yTcpPerfMonSt;

yTcpPerfMonSt yTcpPerf;


#define YPERF_TCP_ENTER(NAME) {yTcpPerf.NAME.count++;yTcpPerf.NAME.tmp=yapiGetTickCount();}
#define YPERF_TCP_LEAVE(NAME) {yTcpPerf.NAME.leave++;yTcpPerf.NAME.totaltime += yapiGetTickCount()- yTcpPerf.NAME.tmp;}


void dumpYTcpPerf(void)
{
    dumpYPerfEntry(&yTcpPerf.TCPOpen_socket,"TCPOpen:socket");
    dumpYPerfEntry(&yTcpPerf.TCPOpen_connect,"TCPOpen:connect");
    dumpYPerfEntry(&yTcpPerf.TCPOpen_setsockopt_noblock,"TCPOpen:sockopt_noblock");
    dumpYPerfEntry(&yTcpPerf.TCPOpen_setsockopt_nodelay,"TCPOpen:sockopt_nodelay");
    dumpYPerfEntry(&yTcpPerf.TCPOpenReq_wait,"TCPOpenReq:wait");
    dumpYPerfEntry(&yTcpPerf.TCPOpenReq,"TCPOpenReq");
    dumpYPerfEntry(&yTcpPerf.tmp1,"TCP:tmp1");
    dumpYPerfEntry(&yTcpPerf.tmp2,"TCP:tmp2");
    dumpYPerfEntry(&yTcpPerf.tmp3,"TCP:tmp3");
    dumpYPerfEntry(&yTcpPerf.tmp4,"TCP:tmp4");
}
#else
#define YPERF_TCP_ENTER(NAME)
#define YPERF_TCP_LEAVE(NAME)
#endif



void  yDupSet(char **storage, const char *val)
{
    int  len = (val ? (int)strlen(val)+1 : 1);

    if(*storage) yFree(*storage);
    *storage = (char*) yMalloc(len);
    if(val) {
        memcpy(*storage, val, len);
    } else {
        **storage = 0;
    }
}

int yNetSetErrEx(u32 line,unsigned err,char *errmsg)
{
    int len;
    if(errmsg==NULL)
        return YAPI_IO_ERROR;
    YSPRINTF(errmsg,YOCTO_ERRMSG_LEN,"%s:%d:tcp(%d):",__FILE_ID__,line,err);
    dbglog("yNetSetErrEx -> %s:%d:tcp(%d)\n",__FILE_ID__,line,err);

#if defined(WINDOWS_API) && !defined(WINCE)
    len=(int)strlen(errmsg);
    FormatMessageA (
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) (errmsg+len),
                    YOCTO_ERRMSG_LEN-len, NULL );
#else
    len=YSTRLEN(errmsg);
    strcpy(errmsg+len, strerror((int)err));
#endif
    return YAPI_IO_ERROR;
}
#if 1
#define yNetLogErr()  yNetLogErrEx(__LINE__,SOCK_ERR)
static int yNetLogErrEx(u32 line,unsigned err)
{
    int retval;
    char errmsg[YOCTO_ERRMSG_LEN];
    retval = yNetSetErrEx(line,err,errmsg);
    dbglog("%s",errmsg);
    return retval;
}
#endif

#ifdef DEBUG_SOCKET_USAGE

#define yclosesocket(skt) yclosesocket_ex(__FILE_ID__, __LINE__, skt)
void yclosesocket_ex(const char *file, int line, YSOCKET skt)
{
    dbglogf(file, line, "close socket %x\n", skt);
    closesocket(skt);
}


#define ysocket(domain, type, protocol) ysocket_ex(__FILE_ID__, __LINE__, domain, type, protocol)
YSOCKET ysocket_ex(const char *file, int line, int domain, int type, int protocol)
{
    YSOCKET skt = socket(domain, type, protocol);
    dbglogf(file, line, "open socket %x (%x,%x,%x)\n", skt, domain, type, protocol);
    return skt;
}

#define ysend(skt, buf, len, flags) ysend_ex(__FILE_ID__, __LINE__, skt, buf, len, flags)
int ysend_ex(const char * file, int line, YSOCKET skt, const char* buffer, int tosend, int  flags)
{
    int res = (int)send(skt, buffer, tosend, flags);
    //dbglogf(file, line, "send socket %x (%d,%x -> %d)\n", skt, tosend, flags, res);
    return  res;
}

#define yrecv(skt, buf, len, flags) yrecv_ex(__FILE_ID__, __LINE__, skt, buf, len, flags)
int yrecv_ex(const char * file, int line, YSOCKET skt, char *buf, int len, int flags)
{
    int res = recv(skt, buf, len, flags);
    //dbglogf(file, line, "read socket %x (%d,%x -> %d)\n", skt, len, flags, res);
    return  res;
}

#else
#define yclosesocket(skt) closesocket(skt)
#define ysocket(domain, type, protocol) socket(domain, type, protocol)
#define ysend(skt, buf, len, flags) send(skt, buf, len, flags)
#define yrecv(skt, buf, len, flags) recv(skt, buf, len, flags)
#endif

void yInitWakeUpSocket(WakeUpSocket *wuce)
{
    wuce->listensock = INVALID_SOCKET;
    wuce->signalsock = INVALID_SOCKET;
}


int yStartWakeUpSocket(WakeUpSocket *wuce, char *errmsg)
{
    u32     optval;
    socklen_t     localh_size;
    struct  sockaddr_in     localh;

    if(wuce->listensock != INVALID_SOCKET || wuce->signalsock != INVALID_SOCKET) {
        return YERRMSG(YAPI_INVALID_ARGUMENT,"WakeUpSocket allready Started");
    }
    //create socket
    wuce->listensock = ysocket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (wuce->listensock == INVALID_SOCKET) {
        return yNetSetErr();
    }
    optval = 1;
    setsockopt(wuce->listensock,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(optval));

    localh_size=sizeof(localh);
    // set port to 0 since we accept any port
    memset(&localh,0,localh_size);
    localh.sin_family = AF_INET;
    localh.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(wuce->listensock,(struct sockaddr *)&localh,localh_size)<0) {
        return yNetSetErr();
    }
    if (getsockname(wuce->listensock,(struct sockaddr *)&localh,&localh_size)<0) {
        return yNetSetErr();
    }
    wuce->signalsock = ysocket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (wuce->signalsock == INVALID_SOCKET) {
        return yNetSetErr();
    }
    if (connect(wuce->signalsock,(struct sockaddr *)&localh,localh_size)<0) {
        return yNetSetErr();
    }
    return YAPI_SUCCESS;
}

int yDringWakeUpSocket(WakeUpSocket *wuce, u8 signal, char *errmsg)
{
    if(ysend(wuce->signalsock,(char*)&signal,1,SEND_NOSIGPIPE) < 0) {
        return yNetSetErr();
    }
    return YAPI_SUCCESS;
}

int yConsumeWakeUpSocket(WakeUpSocket *wuce, char *errmsg)
{
    u8 signal = 0;

    if(yrecv(wuce->listensock,(char*)&signal,1,0) < 0) {
        return yNetSetErr();
    }
    return signal;
}

void yFreeWakeUpSocket(WakeUpSocket *wuce)
{
    if ( wuce->listensock != INVALID_SOCKET) {
        yclosesocket(wuce->listensock);
        wuce->listensock = INVALID_SOCKET;
    }
    if ( wuce->signalsock != INVALID_SOCKET) {
        yclosesocket(wuce->signalsock);
        wuce->signalsock = INVALID_SOCKET;
    }
}




u32 yResolveDNS(const char *name,char *errmsg)
{
    u32 ipv4=0;

    struct addrinfo *infos,*p;
    if(getaddrinfo(name,NULL,NULL,&infos)!=0){
        REPORT_ERR("Unable to resolve hostname");
        return 0;
    }

    // Retrieve each address and print out the hex bytes
    for(p=infos; p != NULL ; p=p->ai_next) {
        if (p->ai_family == AF_INET){
            ipv4 = ((struct sockaddr_in *) p->ai_addr)->sin_addr.s_addr;
            break;
        }
    }
    freeaddrinfo(infos);
    return ipv4;
}


#define YDNS_CACHE_SIZE 16
#define YDNS_CACHE_VALIDITY 600000u //10 minutes
typedef struct {
    yUrlRef url;
    u32     ip;
    u64     time;
} DnsCache;

DnsCache dnsCache[YDNS_CACHE_SIZE];


static u32 resolveDNSCache(yUrlRef url, char *errmsg)
{
    int i, firstFree = -1;
    char buffer[YOCTO_HOSTNAME_NAME];
    u32  ip;

    for (i = 0; i<YDNS_CACHE_SIZE; i++) {
        if (dnsCache[i].url == url) {
            break;
        }
        if (firstFree <0 && dnsCache[i].url == INVALID_HASH_IDX) {
            firstFree = i;
        }
    }
    if (i< YDNS_CACHE_SIZE) {
        if ((u64)(yapiGetTickCount() - dnsCache[i].time) <= YDNS_CACHE_VALIDITY) {
            return dnsCache[i].ip;
        }
        firstFree = i;
    }
    yHashGetUrlPort(url, buffer, NULL, NULL, NULL, NULL, NULL);
    ip = yResolveDNS(buffer, errmsg);
    if (ip != 0 && firstFree < YDNS_CACHE_SIZE) {
        dnsCache[firstFree].url = url;
        dnsCache[firstFree].ip = ip;
        dnsCache[firstFree].time = yapiGetTickCount();
    }
    return ip;
}


/********************************************************************************
* Pure TCP funtions
*******************************************************************************/

int yTcpInit(char *errmsg)
{
    int i;
#ifdef WINDOWS_API
    // Initialize Winsock 2.2
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0){
        return YERRMSG(YAPI_IO_ERROR,"Unable to start Windows Socket");
    }
#endif
    TCPLOG("yTcpInit\n");
    for (i=0; i<YDNS_CACHE_SIZE;i++){
        dnsCache[i].url = INVALID_HASH_IDX;
    }
    return YAPI_SUCCESS;
}

void yTcpShutdown(void)
{

    TCPLOG("yTcpShutdown\n");
#ifdef PERF_TCP_FUNCTIONS
    dumpYTcpPerf();
#endif
#ifdef WINDOWS_API
    WSACleanup();
#endif
}


#define DEFAULT_TCP_ROUND_TRIP_TIME  30
#define DEFAULT_TCP_MAX_WINDOW_SIZE  (4*65536)

static int yTcpOpen(YSOCKET *newskt, u32 ip, u16 port, u64 mstimeout, char *errmsg)
{
    struct sockaddr_in clientService;
    int iResult;
    u_long flags;
    YSOCKET skt;
    fd_set      readfds, writefds, exceptfds;
    struct timeval timeout;
    int tcp_sendbuffer;
#ifdef WINDOWS_API
    char noDelay=1;
    int optlen;
#else
    int  noDelay=1;
    socklen_t optlen;
#ifdef SO_NOSIGPIPE
    int  noSigpipe=1;
#endif
#endif

    TCPLOG("yTcpOpen %p [dst=%x:%d %dms]\n", newskt, ip, port, mstimeout);

    YPERF_TCP_ENTER(TCPOpen_socket);
    *newskt = INVALID_SOCKET;
    skt = ysocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    YPERF_TCP_LEAVE(TCPOpen_socket);
    if (skt == INVALID_SOCKET) {
        REPORT_ERR("Error at socket()");
        return YAPI_IO_ERROR;
    }
    //dbglog("ytcpOpen %X:%x: skt= %x\n",ip,port,skt);
    YPERF_TCP_ENTER(TCPOpen_connect);
    memset(&clientService, 0, sizeof(clientService));
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = ip;
    clientService.sin_port = htons( port );

    //----------------------
    // Connect to server.
    YPERF_TCP_ENTER(TCPOpen_setsockopt_noblock);
    //set socket as non blocking
#ifdef WINDOWS_API
    flags = 1;
    ioctlsocket(skt, FIONBIO, &flags);
#else
    flags = fcntl(skt, F_GETFL, 0);
    fcntl(skt, F_SETFL, flags | O_NONBLOCK);
#ifdef SO_NOSIGPIPE
    setsockopt(skt, SOL_SOCKET, SO_NOSIGPIPE, (void *)&noSigpipe, sizeof(int));
#endif
#endif
    YPERF_TCP_LEAVE(TCPOpen_setsockopt_noblock);
    connect(skt, ( struct sockaddr *) &clientService, sizeof(clientService) );

    // wait for the connection with a select
    memset(&timeout, 0, sizeof(timeout));
    if (mstimeout != 0) {
        u64 nbsec = mstimeout / 1000;
        timeout.tv_sec = (long)nbsec;
        timeout.tv_usec = ((int) (mstimeout - (nbsec * 1000))) * 1000;
    } else {
        timeout.tv_sec = 20;
    }
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(skt, &readfds);
    FD_SET(skt, &writefds);
    FD_SET(skt, &exceptfds);
    iResult = select((int)skt + 1, &readfds, &writefds, &exceptfds, &timeout);
    if (iResult < 0) {
        REPORT_ERR("Unable to connect to server");
        yclosesocket(skt);
        return YAPI_IO_ERROR;
    }
    if (FD_ISSET(skt, &exceptfds)) {
        yclosesocket(skt);
        return YERRMSG(YAPI_IO_ERROR, "Unable to connect to server");
    }
    if (!FD_ISSET(skt, &writefds)) {
        yclosesocket(skt);
        return YERRMSG(YAPI_IO_ERROR, "Unable to connect to server");
    }
    YPERF_TCP_LEAVE(TCPOpen_connect);
    if ( iResult == SOCKET_ERROR) {
        REPORT_ERR("Unable to connect to server");
        yclosesocket(skt);
        return YAPI_IO_ERROR;
    }
    YPERF_TCP_ENTER(TCPOpen_setsockopt_nodelay);
    if(setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay)) < 0) {
#if 0
        switch(errno) {
            case EBADF:
                dbglog("The argument sockfd is not a valid descriptor.\n");
                break;
            case EFAULT:
                dbglog("The address pointed to by optval is not in a valid part of the process address space. For getsockopt(), "
                        "this error may also be returned if optlen is not in a valid part of the process address space.\n");
                break;
            case EINVAL:
                dbglog("optlen invalid in setsockopt(). In some cases this error can also occur for an invalid value in optval "
                       "(e.g., for the IP_ADD_MEMBERSHIP option described in ip(7)).\n");
                break;
            case ENOPROTOOPT:
                dbglog("The option is unknown at the level indicated.\n");
                break;
            case ENOTSOCK:
                dbglog("The argument sockfd is a file, not a socket.\n");
                break;
        }
#endif
        dbglog("SetSockOpt TCP_NODELAY failed %d\n",errno);
    }
    YPERF_TCP_LEAVE(TCPOpen_setsockopt_nodelay);

    // Get buffer size
    optlen = sizeof(tcp_sendbuffer);
    if (getsockopt(skt, SOL_SOCKET, SO_SNDBUF, (void*)&tcp_sendbuffer, &optlen) >= 0) {
#if 0
        dbglog("Default windows size is %d\n", tcp_sendbuffer);
#endif
        if (tcp_sendbuffer < DEFAULT_TCP_MAX_WINDOW_SIZE) {
            // Set buffer size to 64k
            tcp_sendbuffer = DEFAULT_TCP_MAX_WINDOW_SIZE;
            if (setsockopt(skt, SOL_SOCKET, SO_SNDBUF, (void*)&tcp_sendbuffer, sizeof(tcp_sendbuffer)) < 0) {
#if 0
                switch (errno) {
                case EBADF:
                    dbglog("The argument sockfd is not a valid descriptor.\n");
                    break;
                case EFAULT:
                    dbglog("The address pointed to by optval is not in a valid part of the process address space. For getsockopt(), "
                        "this error may also be returned if optlen is not in a valid part of the process address space.\n");
                    break;
                case EINVAL:
                    dbglog("optlen invalid in setsockopt(). In some cases this error can also occur for an invalid value in optval "
                        "(e.g., for the IP_ADD_MEMBERSHIP option described in ip(7)).\n");
                    break;
                case ENOPROTOOPT:
                    dbglog("The option is unknown at the level indicated.\n");
                    break;
                case ENOTSOCK:
                    dbglog("The argument sockfd is a file, not a socket.\n");
                    break;
                }
#endif
                dbglog("SetSockOpt SO_SNDBUF %d failed %d\n", tcp_sendbuffer, errno);
            }
        }
    } else {
        dbglog("getsockopt: unable to get tcp buffer size\n");
    }

    *newskt = skt;

    return YAPI_SUCCESS;
}

static void yTcpClose(YSOCKET skt)
{
    // cleanup
    yclosesocket(skt);
}


// check it a socket is still valid and empty (ie: nothing to read and writable)
// return 1 if the socket is valid or a error code
static int yTcpCheckSocketStillValid(YSOCKET skt, char * errmsg)
{
    int iResult, res;
    fd_set      readfds,writefds,exceptfds;
    struct timeval timeout;

    // Send an initial buffer
#ifndef WINDOWS_API
retry:
#endif
    memset(&timeout,0,sizeof(timeout));
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(skt,&readfds);
    FD_SET(skt,&writefds);
    FD_SET(skt,&exceptfds);
    res = select((int)skt+1,&readfds,&writefds,&exceptfds,&timeout);
    if (res<0) {
#ifndef WINDOWS_API
        if(SOCK_ERR ==  EAGAIN){
            goto retry;
        } else
#endif
        {
            res = yNetSetErr();
            yTcpClose(skt);
            return res;
        }
    }
    if (FD_ISSET(skt,&exceptfds)) {
        yTcpClose(skt);
        return YERRMSG(YAPI_IO_ERROR, "Exception on socket");
    }
    if (!FD_ISSET(skt,&writefds)) {
        yTcpClose(skt);
        return YERRMSG(YAPI_IO_ERROR, "Socket not ready for write");
    }

    if (FD_ISSET(skt,&readfds)) {
        char buffer[128];
        iResult = (int)yrecv(skt, buffer, sizeof(buffer), 0);
        if (iResult == 0) {
            yTcpClose(skt);
            return YERR(YAPI_NO_MORE_DATA);
        } if ( iResult < 0 ){
            yTcpClose(skt);
            return YERR(YAPI_IO_ERROR);
        } else {
            yTcpClose(skt);
            return YERR(YAPI_DOUBLE_ACCES);
        }
    }
    return 1;
}


static int yTcpWrite(YSOCKET skt, const char *buffer, int len,char *errmsg)
{
    int res;
    int tosend = len;
    const char * p = buffer;

    while (tosend>0) {
        res = (int) ysend(skt, p, tosend, SEND_NOSIGPIPE);
        if (res == SOCKET_ERROR) {
#ifdef WINDOWS_API
            if(SOCK_ERR != WSAEWOULDBLOCK)
#else
            if(SOCK_ERR != EAGAIN)

#endif
            {
                return yNetSetErr();
            }

        } else {
            tosend -= res;
            p += res;
            // unable to send all data
            // wait a bit with a select
            if (tosend != res) {
                struct timeval timeout;
                fd_set      fds;
                memset(&timeout,0,sizeof(timeout));
                // Upload of large files (external firmware updates) may need
                // a long time to process (on OSX: seen more than 40 seconds !)
                timeout.tv_sec = 60;
                FD_ZERO(&fds);
                FD_SET(skt,&fds);
                res = select((int)skt+1,NULL,&fds,NULL,&timeout);
                if (res<0) {
#ifndef WINDOWS_API
                    if(SOCK_ERR ==  EAGAIN){
                        continue;
                    } else
#endif
                    {
                        return yNetSetErr();
                    }
                } else if (res == 0) {
                    return YERRMSG(YAPI_TIMEOUT, "Timeout during TCP write");
                }
            }
        }
    }
    return len;
}


static int yTcpRead(YSOCKET skt, u8 *buffer, int len,char *errmsg)
{
    int iResult = (int)yrecv(skt, (char*)buffer, len, 0);

    if (iResult == 0){
        return YERR(YAPI_NO_MORE_DATA);
    }else if ( iResult < 0 ){
#ifdef WINDOWS_API
        if(SOCK_ERR == WSAEWOULDBLOCK){
            return 0;
        }
#else
        if(SOCK_ERR == EAGAIN){
            return 0;
        }
#endif
        REPORT_ERR("read failed");
        return YAPI_IO_ERROR;
    }
    return iResult;
}


int yTcpDownload(const char *host, const char *url, u8 **out_buffer, u32 mstimeout, char *errmsg)
{
    YSOCKET skt;
    u32 ip;
    int res,len,readed;
    char request[512];
    u8      *replybuf = yMalloc(512);
    int     replybufsize = 512;
    int     replysize = 0;
    fd_set      fds;
    u64 expiration;

    ip = yResolveDNS(host, errmsg);
    if (ip==0) {
        yFree(replybuf);
        return YAPI_IO_ERROR;
    }
    expiration = yapiGetTickCount() + mstimeout;
    if (yTcpOpen(&skt, ip, 80, mstimeout, errmsg)<0) {
        yTcpClose(skt);
        yFree(replybuf);
        return YAPI_IO_ERROR;
    }
    len = YSPRINTF(request,512,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n"
        "Accept-Encoding:\r\nUser-Agent: Yoctopuce\r\n\r\n",url,host);
    //write header
    res = yTcpWrite(skt, request, len, errmsg);
    if(YISERR(res)) {
        goto exit;
    }
    while(expiration - yapiGetTickCount() > 0) {
        struct timeval timeout;
        u64 ms = expiration - yapiGetTickCount();
        memset(&timeout,0,sizeof(timeout));
        timeout.tv_sec = (long) ms / 1000;
        timeout.tv_usec = (int)(ms % 1000) *1000;
        /* wait for data */
        FD_ZERO(&fds);
        FD_SET(skt,&fds);
        res = select((int)skt+1,&fds,NULL,NULL,&timeout);
        if (res<0) {
    #ifndef WINDOWS_API
            if(SOCK_ERR ==  EAGAIN){
                continue;
            } else
    #endif
            {
                res = yNetSetErr();
                goto exit;
            }
        }
        if(replysize + 256 >= replybufsize) {
            // need to grow receive buffer
            int  newsize = replybufsize << 1;
            u8 *newbuf = (u8*) yMalloc(newsize);
            if (replybuf) {
                memcpy(newbuf, replybuf, replysize);
                yFree(replybuf);
            }
            replybuf = newbuf;
            replybufsize = newsize;
        }
        readed = yTcpRead(skt, replybuf + replysize, replybufsize - replysize, errmsg);
        if(readed < 0) {
            // any connection closed by peer ends up with YAPI_NO_MORE_DATA
            if (readed == YAPI_NO_MORE_DATA) {
                res = replysize;
            } else {
                res = readed;
            }
            goto exit;
        } else {
            replysize += readed;
        }
    }
    res = YERR(YAPI_TIMEOUT);

exit:
    yTcpClose(skt);
    if (res < 0) {
        yFree(replybuf);
    } else {
        *out_buffer = replybuf;
    }
    return res;
}




static int yTcpCheckReqTimeout(struct _RequestSt *req, char *errmsg)
{
    if (req->timeout_tm != 0) {
        u64 now = yapiGetTickCount();
        u64 duration = now - req->open_tm;
        u64 last_io = (req->write_tm > req->read_tm ? req->write_tm : req->read_tm);
        u64 idle_durration = now -last_io;

        if (idle_durration < YIO_IDLE_TCP_TIMEOUT) {
            return  YAPI_SUCCESS;
        }
#ifdef DEBUG_SLOW_TCP
        else {
            u64 last_wr = now - req->write_tm;
            u64 last_rd = now - req->read_tm;

            dbglog("Long Idle TCP request %p = %"FMTu64"ms total = %"FMTu64"ms (read=%"FMTu64"ms write=%"FMTu64")\n",
                req, idle_durration, duration, last_rd, last_wr);
        }
#endif
        if (duration > req->timeout_tm) {
            req->errcode = YAPI_TIMEOUT;
            YSPRINTF(req->errmsg, YOCTO_ERRMSG_LEN , "TCP request took too long (%dms)",duration);
            return YERRMSG(YAPI_TIMEOUT, req->errmsg);
        }
#ifdef DEBUG_SLOW_TCP
        else {
            if (duration > (req->timeout_tm - (req->timeout_tm / 4))) {
                dbglog("Slow TCP request %p = %dms\n",req,duration);
                dbglog("req = %s\n",req->headerbuf);
            }
        }
#endif

    }
    return YAPI_SUCCESS;
}


/********************************************************************************
* HTTP request funtions (http request that DO NOT use Websocket)
*******************************************************************************/



// access mutex taken by caller
static int yHTTPOpenReqEx(struct _RequestSt *req, u64 mstimout, char *errmsg)
{
    char buffer[YOCTO_HOSTNAME_NAME], *p,*last,*end;
    u32  ip;
    u16  port;
    int  res;

    YASSERT(req->proto == PROTO_AUTO || req->proto == PROTO_HTTP);

    switch (yHashGetUrlPort(req->hub->url, buffer, &port, NULL, NULL, NULL, NULL)) {
    case NAME_URL:
        ip = resolveDNSCache(req->hub->url, errmsg);
        if (ip == 0) {
            YPERF_TCP_LEAVE(tmp1);
            return YAPI_IO_ERROR;
        }
        break;
    case IP_URL:
        ip = inet_addr(buffer);
        break;
    default:
        res = YERRMSG(YAPI_IO_ERROR, "not an IP hub");
        req->http.skt = INVALID_SOCKET;
        TCPLOG("yTcpOpenReqEx error%p[%x]\n", req, req->http.skt);
        return res;
    }
    TCPLOG("yTcpOpenReqEx %p [%x:%x %d]\n", req, req->http.skt, req->http.reuseskt, mstimout);

    req->replypos = -1; // not ready to consume until header found
    req->replysize = 0;
    memset(req->replybuf, 0, req->replybufsize);
    req->errcode = YAPI_SUCCESS;


    if (req->http.reuseskt != INVALID_SOCKET && (res = yTcpCheckSocketStillValid(req->http.reuseskt, NULL)) == 1) {
        req->http.skt = req->http.reuseskt;
        req->http.reuseskt = INVALID_SOCKET;
    } else {
        req->http.reuseskt = INVALID_SOCKET;
        res = yTcpOpen(&req->http.skt, ip, port, mstimout, errmsg);
        if (YISERR(res)) {
            // yTcpOpen has reset the socket to INVALID
            yTcpClose(req->http.skt);
            req->http.skt = INVALID_SOCKET;
            TCPLOG("yTcpOpenReqEx error %p [%x]\n", req, req->http.skt);
            return res;
        }
    }

    p = req->headerbuf;
    //skip first line
    while(*p && *p != '\r') p++;
    end=p;
    last=p;

    while(*p == '\r' && *(p+1)=='\n' && *(p+2)!='\r') {
        p+=2;
        while(*p && *p != '\r') p++;
        if (YSTRNCMP(last,"\r\nContent-Type",strlen("\r\nContent-Type"))==0){
            unsigned len = (unsigned)(p -  last);
            if(last != end){
                memcpy(end,last,len);
            }
            end += len;
        }
        last = p;
    }
    *end++ = '\r'; *end++ = '\n';
    // insert authorization header in needed
    yEnterCriticalSection(&req->hub->access);
    if(req->hub->http.s_user && req->hub->http.s_realm) {
        char *method = req->headerbuf, *uri;
        char *auth = end;
        // null-terminate method and uri for digest computation
        // ReSharper disable once CppPossiblyErroneousEmptyStatements
        for (uri = method; *uri != ' '; uri++);
        *uri++ = 0;
        // ReSharper disable once CppPossiblyErroneousEmptyStatements
        for(p = uri; *p != ' '; p++);
        *p = 0;
        yDigestAuthorization(auth, (int)(req->headerbuf + req->headerbufsize - auth), req->hub->http.s_user, req->hub->http.s_realm, req->hub->http.s_ha1,
                             req->hub->http.s_nonce, req->hub->http.s_opaque, &req->hub->http.nc, method, uri);
        // restore space separator after method and uri
        *--uri = ' ';
        *p = ' ';
        // prepare to complete request
        end = auth+strlen(auth);
    }
    yLeaveCriticalSection(&req->hub->access);
    if(req->flags & TCPREQ_KEEPALIVE) {
        YSTRCPY(end, (int)(req->headerbuf + req->headerbufsize - end), "\r\n");
    } else {
        YSTRCPY(end, (int)(req->headerbuf + req->headerbufsize - end), "Connection: close\r\n\r\n");
    }
    //write header
    res = yTcpWrite(req->http.skt, req->headerbuf, (int)strlen(req->headerbuf), errmsg);
    if(YISERR(res)) {
        yTcpClose(req->http.skt);
        req->http.skt = INVALID_SOCKET;
        return res;
    }
    if(req->bodysize > 0){
        //write body
        res = yTcpWrite(req->http.skt, req->bodybuf, req->bodysize, errmsg);
        if(YISERR(res)) {
            yTcpClose(req->http.skt);
            req->http.skt = INVALID_SOCKET;
            TCPLOG("yTcpOpenReqEx write failed for Req %p[%x]\n", req, req->http.skt);
            return res;
        }
    }
    req->write_tm = yapiGetTickCount();

    if (req->hub->wuce.listensock != INVALID_SOCKET) {
        return yDringWakeUpSocket(&req->hub->wuce, 1, errmsg);
    } else {
        return YAPI_SUCCESS;
    }
}


static void yHTTPCloseReqEx(struct _RequestSt *req, int canReuseSocket)
{
    TCPLOG("yHTTPCloseReqEx %p[%d]\n",req, canReuseSocket);

    // mutex already taken by caller
    req->flags &= ~TCPREQ_KEEPALIVE;
    if (req->callback) {
        u32 len = req->replysize - req->replypos;
        u8  *ptr = req->replybuf + req->replypos;
        if (req->errcode == YAPI_NO_MORE_DATA) {
            req->callback(req->context, ptr, len, YAPI_SUCCESS, "");
        } else {
            req->callback(req->context, ptr, len, req->errcode, req->errmsg);
        }
        req->callback = NULL;
        // ASYNC Request are automaticaly released
        req->flags &= ~TCPREQ_IN_USE;
    }

    if(req->http.skt != INVALID_SOCKET) {
        if (canReuseSocket) {
            req->http.reuseskt = req->http.skt;
        } else {
            yTcpClose(req->http.skt);
        }
        req->http.skt = INVALID_SOCKET;
    }
    ySetEvent(&req->finished);
}


static int yHTTPMultiSelectReq(struct _RequestSt **reqs, int size, u64 ms, WakeUpSocket *wuce, char *errmsg)
{
    fd_set      fds;
    struct timeval timeout;
    int         res,i;
    YSOCKET     sktmax=0;

    memset(&timeout, 0, sizeof(timeout));
    timeout.tv_sec = (long)ms/1000;
    timeout.tv_usec = (int)(ms % 1000) *1000;
    /* wait for data */
    //dbglog("select %p\n", reqs);


    FD_ZERO(&fds);
    if (wuce) {
        //dbglog("listensock %p %d\n", reqs, wuce->listensock);
        FD_SET(wuce->listensock, &fds);
        sktmax = wuce->listensock;
    }
    for (i = 0; i < size; i++) {
        struct _RequestSt *req;
        req = reqs[i];
        YASSERT(req->proto == PROTO_AUTO || req->proto == PROTO_HTTP);
        if(req->http.skt == INVALID_SOCKET) {
            return YERR(YAPI_INVALID_ARGUMENT);
        } else {
            //dbglog("sock %p %p:%d\n", reqs, req, req->http.skt);
            FD_SET(req->http.skt, &fds);
            if(req->http.skt > sktmax)
                sktmax = req->http.skt;
        }
    }
    if (sktmax == 0) {
        return YAPI_SUCCESS;
    }
    res = select((int)sktmax + 1, &fds, NULL, NULL, &timeout);
    if (res < 0) {
#ifndef WINDOWS_API
        if(SOCK_ERR ==  EAGAIN){
            return 0;
        } else
#endif
        {
            res = yNetSetErr();
            for (i = 0; i < size; i++) {
                TCPLOG("yHTTPSelectReq %p[%X] (%s)\n", reqs[i], reqs[i]->http.skt, errmsg);
            }
            return res;
        }
    }
    if (res != 0) {
        if (wuce && FD_ISSET(wuce->listensock,&fds)) {
            YPROPERR(yConsumeWakeUpSocket(wuce, errmsg));
        }
        for (i = 0; i < size; i++) {
            struct _RequestSt *req;
            req = reqs[i];
            if (FD_ISSET(req->http.skt, &fds)) {
                yEnterCriticalSection(&req->access);
                if (req->replysize >= req->replybufsize - 256) {
                    // need to grow receive buffer
                    int  newsize = req->replybufsize << 1;
                    u8 *newbuf = (u8*) yMalloc(newsize);
                    memcpy(newbuf, req->replybuf, req->replysize);
                    yFree(req->replybuf);
                    req->replybuf = newbuf;
                    req->replybufsize = newsize;
                }
                res = yTcpRead(req->http.skt, req->replybuf + req->replysize, req->replybufsize - req->replysize, errmsg);
                //dbglog("check %x:%x:%X\n", check, check2, size);

                req->read_tm = yapiGetTickCount();
                if (res < 0) {
                    // any connection closed by peer ends up with YAPI_NO_MORE_DATA
                    req->replypos = 0;
                    req->errcode = YERRTO((YRETCODE) res,req->errmsg);
                    TCPLOG("yHTTPSelectReq %p[%x] connection closed by peer\n",req,req->http.skt);
                    yHTTPCloseReqEx(req, 0);
                } else if (res > 0) {
                    req->replysize += res;
                    if(req->replypos < 0) {
                        // Need to analyze http headers
                        if(req->replysize == 8 && !memcmp(req->replybuf, "0K\r\n\r\n\r\n", 8)) {
                            TCPLOG("yHTTPSelectReq %p[%x] untrashort reply\n",req,req->http.skt);
                            // successful abbreviated reply (keepalive)
                            req->replypos = 0;
                            req->replybuf[0] = 'O';
                            req->errcode = YERRTO(YAPI_NO_MORE_DATA, req->errmsg);
                            yHTTPCloseReqEx(req, 1);
                        } else if(req->replysize >= 4 && !memcmp(req->replybuf, "OK\r\n", 4)) {
                            // successful short reply, let it go through
                            req->replypos = 0;
                        } else if(req->replysize >= 12) {
                            if(memcmp(req->replybuf, "HTTP/1.1 401", 12) != 0) {
                                // no authentication required, let it go through
                                req->replypos = 0;
                            } else {
                                // authentication required, process authentication headers
                                char *method = NULL, *realm = NULL, *qop = NULL, *nonce = NULL, *opaque = NULL;

                                if(!req->hub->http.s_user || req->retryCount++ > 3) {
                                    // No credential provided, give up immediately
                                    req->replypos = 0;
                                    req->replysize = 0;
                                    req->errcode = YERRTO(YAPI_UNAUTHORIZED, req->errmsg);
                                    yHTTPCloseReqEx(req, 0);
                                } else if(yParseWWWAuthenticate((char*)req->replybuf, req->replysize, &method, &realm, &qop, &nonce, &opaque) >= 0) {
                                    // Authentication header fully received, we can close the connection
                                    if (!strcmp(method, "Digest") && !strcmp(qop, "auth")) {
                                        // partial close to reopen with authentication settings
                                        yTcpClose(req->http.skt);
                                        req->http.skt = INVALID_SOCKET;
                                        // device requests Digest qop-authentication, good
                                        yEnterCriticalSection(&req->hub->access);
                                        yDupSet(&req->hub->http.s_realm, realm);
                                        yDupSet(&req->hub->http.s_nonce, nonce);
                                        yDupSet(&req->hub->http.s_opaque, opaque);
                                        if (req->hub->http.s_user && req->hub->http.s_pwd) {
                                            ComputeAuthHA1(req->hub->http.s_ha1, req->hub->http.s_user, req->hub->http.s_pwd, req->hub->http.s_realm);
                                        }
                                        req->hub->http.nc = 0;
                                        yLeaveCriticalSection(&req->hub->access);
                                        // reopen connection with proper auth parameters
                                        // callback and context parameters are preserved
                                        req->errcode = yHTTPOpenReqEx(req, req->timeout_tm, req->errmsg);
                                        if (YISERR(req->errcode)) {
                                            yHTTPCloseReqEx(req, 0);
                                        }
                                    } else {
                                        // unsupported authentication method for devices, give up
                                        req->replypos = 0;
                                        req->errcode = YERRTO(YAPI_UNAUTHORIZED, req->errmsg);
                                        yHTTPCloseReqEx(req, 0);
                                    }
                                }
                            }
                        }
                    }
                    if (req->errcode == YAPI_SUCCESS) {
                        req->errcode = yTcpCheckReqTimeout(req, req->errmsg);
                    }
                }
                yLeaveCriticalSection(&req->access);
            }
        }
    }

    return YAPI_SUCCESS;
}


/********************************************************************************
* WebSocket implementation for generic requests
*******************************************************************************/

static int yWSOpenReqEx(struct _RequestSt *req, int tcpchan, u64 mstimeout, char *errmsg)
{
    HubSt *hub = req->hub;
    RequestSt *r;
    int headlen;
    u8 *p;
    YASSERT(req->proto == PROTO_WEBSOCKET);


    if (req->hub->ws.base_state != WS_BASE_CONNECTED) {
        return YERRMSG(YAPI_IO_ERROR, "Hub is not ready (WebSocket)");
    }

    // merge first line and header
    headlen = YSTRLEN(req->headerbuf);
    req->ws.requestsize = headlen + 4 + req->bodysize;
    req->ws.requestbuf = yMalloc(req->ws.requestsize);
    p = req->ws.requestbuf;
    memcpy(p, req->headerbuf, headlen);
    p += headlen;
    //todo: create request buffer more efficiently
    if (req->bodysize) {
        memcpy(p, req->bodybuf, req->bodysize);
    } else {
        memcpy(p, "\r\n\r\n", 4);
    }
    if (req->callback) {
        yEnterCriticalSection(&hub->access);
        req->ws.asyncId = hub->ws.s_next_async_id++;
        if (hub->ws.s_next_async_id >= 127) {
            hub->ws.s_next_async_id = 48;
        }
        yLeaveCriticalSection(&hub->access);
    }
    req->ws.channel = tcpchan;
    req->timeout_tm = mstimeout;
    //WSLOG("req(%s:%p): open req chan=%d timeout=%dms asyncId=%d\n", req->hub->name, req, tcpchan, (int)mstimeout, req->ws.asyncId);
    YASSERT(tcpchan < MAX_ASYNC_TCPCHAN);
    yEnterCriticalSection(&hub->ws.chan[tcpchan].access);
    req->ws.next = NULL; // just in case
    if (hub->ws.chan[tcpchan].requests) {
        r = hub->ws.chan[tcpchan].requests;
        while (r->ws.next) {
            r =r->ws.next;
        }
        r->ws.next = req;
    } else {
        hub->ws.chan[tcpchan].requests = req;
    }
    yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
    req->write_tm = yapiGetTickCount();
    return yDringWakeUpSocket(&hub->wuce, 1, errmsg);
}



static int yWSSelectReq(struct _RequestSt* req, u64 mstimeout, char* errmsg)
{
    int done = yWaitForEvent(&req->finished, (int)mstimeout);

    REQLOG("ws_req:%p: select for %d ms %d\n", req, (int)mstimeout, done);

    if (done) {
        req->errcode = YAPI_NO_MORE_DATA;
    }
    return YAPI_SUCCESS;
}


static void yWSCloseReqEx(struct _RequestSt *req, int takeCS)
{
    HubSt *hub = req->hub;
    RequestSt *r, *p;
    int tcpchan;
    u32 len;
    u8 *ptr;
#ifdef DEBUG_WEBSOCKET
    u64 duration;
    duration = yapiGetTickCount() - req->open_tm;
    WSLOG("req(%s:%p) close req after %"FMTu64"ms (%"FMTu64"ms) with %d bytes errcode = %d\n", req->hub->name, req, duration, (req->write_tm - req->open_tm), req->replysize, req->errcode);
#endif

    YASSERT(req->proto == PROTO_WEBSOCKET);
    if (req->callback) {
        // async close
        len = req->replysize - req->replypos;
        ptr = req->replybuf + req->replypos;
        if (req->errcode == YAPI_NO_MORE_DATA) {
            req->callback(req->context, ptr, len, YAPI_SUCCESS, "");
        } else {
            req->callback(req->context, ptr, len, req->errcode, req->errmsg);
        }
        req->callback = NULL;
    }


    tcpchan = req->ws.channel;
    YASSERT(tcpchan < MAX_ASYNC_TCPCHAN);
    if (takeCS) {
        yEnterCriticalSection(&hub->ws.chan[tcpchan].access);
    }
    r = hub->ws.chan[tcpchan].requests;
    p = NULL;
    while(r && r !=req) {
        p = r;
        r = r->ws.next;

    }
    YASSERT(r);
    if (r) {
        if (p == NULL) {
            hub->ws.chan[tcpchan].requests = r->ws.next;
        } else {
            p->ws.next = r->ws.next;
        }
    }
    if (takeCS) {
        yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
    }
}



/********************************************************************************
* Generic Request funtions (HTTP or WS)
*******************************************************************************/

#ifdef TRACE_TCP_REQ

static void dumpTCPReq(const char *fileid, int lineno, struct _RequestSt *req)
{
    int w;
    int has_cs  =yTryEnterCriticalSection(&req->access);
    const char *proto;
    const char *state;

    dbglog("dump TCPReq %p from %s:%d\n", req, fileid, lineno);
    if (req->hub){
        dbglog("Hub: %s\n", req->hub->name);
    } else{
        dbglog("Hub: null\n");
    }


    switch (req->state) {
        case REQ_CLOSED:
            state ="state=REQ_CLOSED";
            break;
        case REQ_OPEN:
            state ="state=REQ_OPEN";
            break;
        case REQ_CLOSED_BY_HUB:
            state ="state=REQ_CLOSED_BY_HUB";
            break;
        case REQ_CLOSED_BY_API:
            state ="state=REQ_CLOSED_BY_API";
            break;
        case REQ_ERROR:
            state ="state=REQ_ERROR";
            break;
        default:
            state ="state=??";
            break;
    }

    dbglog("%s retcode=%d (retrycount=%d) errmsg=%s\n", state, req->errcode, req->retryCount, req->errmsg);
    switch(req->proto){
        case PROTO_AUTO: proto ="PROTO_AUTO"; break;
        case PROTO_HTTP: proto ="PROTO_HTTP"; break;
        case PROTO_WEBSOCKET: proto ="PROTO_WEBSOCKET"; break;
        default: proto ="unk"; break;
    }
    dbglog("proto=%s socket=%x (reuse=%x) flags=%x\n", proto, req->http.skt, req->http.reuseskt, req->flags);
    dbglog("time open=%"FMTx64" last read=%"FMTx64" last write=%"FMTx64"  timeout=%"FMTx64"\n", req->open_tm, req->read_tm, req->write_tm, req->timeout_tm);
    dbglog("readed=%d (readpos=%d)\n", req->replysize, req->replysize);
    dbglog("callback=%p context=%p\n", req->callback, req->context);
    if (req->headerbuf){
        dbglog("req[%s]\n", req->headerbuf);
    } else {
        dbglog("null\n");
    }
    w = yWaitForEvent(&req->finished, 0);
    dbglog("finished=%d\n", w);
    if (has_cs) {
        yLeaveCriticalSection(&req->access);
    }

}
#endif


struct _RequestSt* yReqAlloc(struct _HubSt *hub)
{
    struct _RequestSt* req = yMalloc(sizeof(struct _RequestSt));
    memset(req, 0, sizeof(struct _RequestSt));
    yHashGetUrlPort(hub->url, NULL, NULL, &req->proto, NULL, NULL, NULL);
    TCPLOG("yTcpInitReq %p[%x:%x]\n", req, hub->url, req->proto);
    req->replybufsize = 1500;
    req->replybuf = (u8*)yMalloc(req->replybufsize);
    yInitializeCriticalSection(&req->access);
    yCreateManualEvent(&req->finished, 1);
    req->hub = hub;
    switch (req->proto) {
    case PROTO_AUTO:
    case PROTO_HTTP:
        req->http.reuseskt = INVALID_SOCKET;
        req->http.skt = INVALID_SOCKET;
        break;
    case PROTO_WEBSOCKET:
        break;
    }
    return req;
}



int  yReqOpen(struct _RequestSt *req, int wait_for_start, int tcpchan, const char *request, int reqlen, u64 mstimeout,yapiRequestAsyncCallback callback, void *context, RequestProgress progress_cb, void *progress_ctx, char *errmsg)
{
    int  minlen, i, res;
    u64  startwait;

    YPERF_TCP_ENTER(TCPOpenReq);
    if (wait_for_start <= 0) {
        yEnterCriticalSection(&req->access);
        if (req->flags & TCPREQ_IN_USE) {
            yLeaveCriticalSection(&req->access);
            return YERR(YAPI_DEVICE_BUSY);
        }
    } else {
        YPERF_TCP_ENTER(TCPOpenReq_wait);
        yEnterCriticalSection(&req->access);
        startwait = yapiGetTickCount();
        while (req->flags & TCPREQ_IN_USE) {
            u64 duration;
            // There is an ongoing request to be finished
            yLeaveCriticalSection(&req->access);
            duration = yapiGetTickCount() - startwait;
            if (duration > wait_for_start) {
                dbglog("Last request in not finished after %"FMTu64" ms\n", duration);
#ifdef TRACE_TCP_REQ
                dumpTCPReq(__FILE_ID__, __LINE__, req);
#endif
                return YERRMSG(YAPI_TIMEOUT, "last TCP request is not finished");
            }
            yWaitForEvent(&req->finished, 100);
            yEnterCriticalSection(&req->access);
        }
        YPERF_TCP_LEAVE(TCPOpenReq_wait);
    }


    req->flags = 0;
    if (request[0] == 'G' && request[1] == 'E' && request[2] == 'T') {
        //for GET request discard all exept the first line
        for (i = 0; i < reqlen; i++) {
            if (request[i] == '\r') {
                reqlen = i;
                break;
            }
        }
        if (i > 3) {
            if (request[i - 3] == '&' && request[i - 2] == '.' && request[i - 1] == ' ') {
                req->flags |= TCPREQ_KEEPALIVE;
            }
        }
        req->bodysize = 0;
    } else {
        const char *p = request;
        int bodylen = reqlen - 4;

        while (bodylen > 0 && (p[0] != '\r' || p[1] != '\n' ||
            p[2] != '\r' || p[3] != '\n')) {
            p++, bodylen--;
        }
        p += 4;
        reqlen = (int)(p - request);
        // Build a request body buffer
        if (req->bodybufsize < bodylen) {
            if (req->bodybuf) yFree(req->bodybuf);
            req->bodybufsize = bodylen + (bodylen >> 1);
            req->bodybuf = (char*)yMalloc(req->bodybufsize);
        }
        memcpy(req->bodybuf, p, bodylen);
        req->bodysize = bodylen;
    }
    // Build a request buffer with at least a terminal NUL but
    // include space for Connection: close and Authorization: headers
    minlen = reqlen + 400;
    if (req->headerbufsize < minlen) {
        if (req->headerbuf) yFree(req->headerbuf);
        req->headerbufsize = minlen + (reqlen >> 1);
        req->headerbuf = (char*)yMalloc(req->headerbufsize);
    }
    memcpy(req->headerbuf, request, reqlen);
    req->headerbuf[reqlen] = 0;
    req->retryCount = 0;
    req->callback = callback;
    req->context = context;
    req->progressCb = progress_cb;
    req->progressCtx = progress_ctx;
    req->read_tm = req->write_tm = req->open_tm = yapiGetTickCount();
    req->timeout_tm = mstimeout;



    // Really build and send the request
    if (req->proto == PROTO_AUTO || req->proto == PROTO_HTTP) {
        res = yHTTPOpenReqEx(req, mstimeout, errmsg);
    } else {
        res = yWSOpenReqEx(req, tcpchan, mstimeout, errmsg);
    }
    if(res == YAPI_SUCCESS) {
        req->errmsg[0] = '\0';
        req->flags |= TCPREQ_IN_USE;
        yResetEvent(&req->finished);
        req->state = REQ_OPEN;
    }

    yLeaveCriticalSection(&req->access);

    YPERF_TCP_LEAVE(TCPOpenReq);
    return res;
}

int  yReqSelect(struct _RequestSt *tcpreq, u64 ms, char *errmsg)
{
    if (tcpreq->proto == PROTO_AUTO || tcpreq->proto == PROTO_HTTP) {
        return yHTTPMultiSelectReq(&tcpreq, 1, ms, NULL, errmsg);
    } else {
        return yWSSelectReq(tcpreq, ms, errmsg);
    }
}

int  yReqMultiSelect(struct _RequestSt **tcpreq, int size, u64 ms, WakeUpSocket *wuce, char *errmsg)
{
    // multi select make no sense in Websocket since all data comme from the same socket
    return yHTTPMultiSelectReq(tcpreq, size, ms, wuce, errmsg);
}


int yReqIsEof(struct _RequestSt *req, char *errmsg)
{
    int res;
    yEnterCriticalSection(&req->access);
    if(req->errcode == YAPI_NO_MORE_DATA) {
        res = 1;
    } else if(req->errcode == 0) {
        res = req->errcode = yTcpCheckReqTimeout(req, errmsg);
    } else if(req->errcode == YAPI_UNAUTHORIZED) {
        res = YERRMSG((YRETCODE) req->errcode, "Access denied, authorization required");
    } else {
        res = YERRMSG((YRETCODE) req->errcode, req->errmsg);
    }
    yLeaveCriticalSection(&req->access);
    return res;
}


int yReqGet(struct _RequestSt *req, u8 **buffer)
{
    int avail;

    yEnterCriticalSection(&req->access);
    yTcpCheckReqTimeout(req, req->errmsg);
    if(req->replypos < 0) {
        // data is not yet ready to consume (still processing header)
        avail = 0;
    } else {
        avail = req->replysize - req->replypos;
        if(buffer) {
            *buffer = req->replybuf + req->replypos;
        }
    }
    yLeaveCriticalSection(&req->access);

    return avail;
}


int yReqRead(struct _RequestSt *req, u8 *buffer, int len)
{
    int avail;

    yEnterCriticalSection(&req->access);
    yTcpCheckReqTimeout(req, req->errmsg);
    if(req->replypos < 0) {
        // data is not yet ready to consume (still processing header)
        len = 0;
    } else {
        avail = req->replysize - req->replypos;
        if(len > avail) {
            len = avail;
        }
        if(len && buffer) {
            memcpy(buffer, req->replybuf+req->replypos, len);
        }
        if(req->replypos + len == req->replysize) {
            req->replypos = 0;
            req->replysize = 0;
            if (req->proto == PROTO_WEBSOCKET) {
                if (req->state == REQ_CLOSED || req->state == REQ_CLOSED_BY_HUB) {
                    req->errcode = YAPI_NO_MORE_DATA;
                }
            }

        } else {
            req->replypos += len;
        }
    }
    yLeaveCriticalSection(&req->access);

    return len;
}


void yReqClose(struct _RequestSt *req)
{
    TCPLOG("yTcpCloseReq %p\n", req);
#if 0
    {
        u64 now = yapiGetTickCount();
        u64 duration = now - req->open_tm;
        u64 last_wr = req->write_tm - req->open_tm;
        u64 last_rd = req->read_tm - req->open_tm;

        dbglog("request %p  total=%"FMTu64"ms (read=%"FMTu64"ms write=%"FMTu64")\n",
            req, duration, last_rd, last_wr);
    }
#endif
    yEnterCriticalSection(&req->access);
    if (req->flags &TCPREQ_IN_USE) {

        if (req->proto == PROTO_AUTO || req->proto == PROTO_HTTP) {
            yHTTPCloseReqEx(req, 0);
        } else {
#if 0
            u64 last = req->ws.last_write_tm - req->open_tm;
            u64 first = req->ws.first_write_tm - req->open_tm;

            dbglog("request.ws %p first_write=%"FMTu64"ms last_write=%"FMTu64")\n",
                req, first, last);
#endif
            yWSCloseReqEx(req, 1);
        }
        req->flags &= ~TCPREQ_IN_USE;
    }
    yLeaveCriticalSection(&req->access);
}


int yReqIsAsync(struct _RequestSt *req)
{
    int res;
    yEnterCriticalSection(&req->access);
    res = (req->flags & TCPREQ_IN_USE) && (req->callback != NULL);
    yLeaveCriticalSection(&req->access);
    return res;
}


void yReqFree(struct _RequestSt *req)
{
    TCPLOG("yTcpFreeReq %p\n",req);
    if (req->proto == PROTO_AUTO || req->proto == PROTO_HTTP) {
        if (req->http.skt != INVALID_SOCKET) {
            yTcpClose(req->http.skt);
        }
        if (req->http.reuseskt != INVALID_SOCKET) {
            yTcpClose(req->http.reuseskt);
        }
    } else {
        if (req->ws.requestbuf) yFree(req->ws.requestbuf);
    }
    if(req->headerbuf)  yFree(req->headerbuf);
    if(req->bodybuf)    yFree(req->bodybuf);
    if(req->replybuf)   yFree(req->replybuf);
    yCloseEvent(&req->finished);
    yDeleteCriticalSection(&req->access);
    yFree(req);
    //memset(req, 0, sizeof(struct _RequestSt));
}


int yReqHasPending(struct _HubSt *hub)
{
    int       i;
    RequestSt   *req = NULL;

    if (hub->proto == PROTO_AUTO || hub->proto == PROTO_HTTP) {
        for (i = 0; i < ALLOC_YDX_PER_HUB; i++) {
            req = yContext->tcpreq[i];
            if (req && yReqIsAsync(req)) {
                return 1;
            }
        }
    } else {
        int  tcpchan;
        for (tcpchan = 0; tcpchan < MAX_ASYNC_TCPCHAN; tcpchan++) {
            yEnterCriticalSection(&hub->ws.chan[tcpchan].access);
            if (hub->ws.chan[tcpchan].requests) {
                req = hub->ws.chan[tcpchan].requests;
                while (req && req->ws.requestsize == req->ws.requestpos && req->state == REQ_CLOSED) {
                    req = req->ws.next;
                }
                if (req != NULL) {
                    //dbglog("stil request pending on hub %s (%p)\n", hub->name, req);
                    yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
                    return 1;
                }
            }
            yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
        }
    }
    return 0;
}




/********************************************************************************
* Websocket funtions
*******************************************************************************/

static const char* ws_header_start = " HTTP/1.1\r\nSec-WebSocket-Version: 13\r\nUser-Agent: Yoctopuce\r\nSec-WebSocket-Key: ";
static const char* ws_header_end = "\r\nConnection: keep-alive, Upgrade\r\nUpgrade: websocket\r\n\r\n";

#define YRand32() rand()

/*****************************************************************************
Function:
WORD Base64Encode(BYTE* cSourceData, WORD wSourceLen,
BYTE* cDestData, WORD wDestLen)

Description:
Encodes a binary array to Base-64.

Precondition:
None

Parameters:
cSourceData - Pointer to a string of binary data
wSourceLen	- Length of the binary source data
cDestData	- Pointer to write the Base-64 encoded data
wDestLen	- Maximum length that can be written to cDestData

Returns:
Number of encoded bytes written to cDestData.  This will always be
a multiple of 4.

Remarks:
Encoding cannot be performed in-place.  If cSourceData overlaps with
cDestData, the behavior is undefined.

Encoded data is always at least 1/3 larger than the source data.  It may
be 1 or 2 bytes larger than that.
***************************************************************************/
static u16 Base64Encode(const u8* cSourceData, u16 wSourceLen, u8* cDestData, u16 wDestLen)
{
    u8 i, j;
    u8 vOutput[4];
    u16 wOutputLen;

    wOutputLen = 0;
    while (wDestLen >= 4u)
    {
        // Start out treating the output as all padding
        vOutput[0] = 0xFF;
        vOutput[1] = 0xFF;
        vOutput[2] = 0xFF;
        vOutput[3] = 0xFF;

        // Get 3 input octets and split them into 4 output hextets (6-bits each)
        if (wSourceLen == 0u)
            break;
        i = *cSourceData++;
        wSourceLen--;
        vOutput[0] = (i & 0xFC) >> 2;
        vOutput[1] = (i & 0x03) << 4;
        if (wSourceLen)
        {
            i = *cSourceData++;
            wSourceLen--;
            vOutput[1] |= (i & 0xF0) >> 4;
            vOutput[2] = (i & 0x0F) << 2;
            if (wSourceLen)
            {
                i = *cSourceData++;
                wSourceLen--;
                vOutput[2] |= (i & 0xC0) >> 6;
                vOutput[3] = i & 0x3F;
            }
        }

        // Convert hextets into Base 64 alphabet and store result
        for (i = 0; i < 4u; i++)
        {
            j = vOutput[i];

            if (j <= 25u)
                j += 'A' - 0;
            else if (j <= 51u)
                j += 'a' - 26;
            else if (j <= 61u)
                j += '0' - 52;
            else if (j == 62u)
                j = '+';
            else if (j == 63u)
                j = '/';
            else				// Padding
                j = '=';

            *cDestData++ = j;
        }

        // Update counters
        wDestLen -= 4;
        wOutputLen += 4;
    }

    return wOutputLen;
}







/********************************************************************************
* WebSocket internal function
*******************************************************************************/


//todo : factorise  GenereateWebSockeyKey + VerifyWebsocketKey

// compute a new nonce for http request
// the buffer passed as argument must be at least 28 bytes long
static int GenereateWebSockeyKey(const u8 *url, u32 urllen, char *buffer)
{
    u32 salt[2];
    HASH_SUM ctx;
    u8  rawbuff[16];

    // Our nonce is base64_encoded [ MD5( Rand32,(ytime^Rand), ) ]
    salt[0] = YRand32();
    salt[1] = yapiGetTickCount() & 0xff;
    MD5Initialize(&ctx);
    MD5AddData(&ctx, (u8*)salt, 2);
    MD5AddData(&ctx,url, urllen);
    MD5Calculate(&ctx, rawbuff);
    return Base64Encode(rawbuff, 16, (u8*)buffer, 28);
}


static int VerifyWebsocketKey(const char *data, u16 hdrlen, const char *reference, u16 reference_len)
{
    u8    buf[80];
    const char *magic = YOCTO_WEBSOCKET_MAGIC;
    u8      *sha1;

    // compute correct key
    if (hdrlen >= sizeof(buf)) {
#ifndef MICROCHIP_API
        dbglog("Bad WebSocket header (%d)\n", hdrlen);
#else
        ylog("WS!");
#endif
        return 0;
    }
    memcpy(buf, reference, reference_len);
#ifdef USE_FAR_YFSTR
    apiGetStr(magic.hofs, (char*)buf + CbCtx.websocketkey.len);
#else
    memcpy(buf + reference_len, magic, YOCTO_WEBSOCKET_MAGIC_LEN + 1);
#endif
    sha1 = ySHA1((char *)buf);
    Base64Encode(sha1, 20, buf, 80);
    if (memcmp(buf, data, hdrlen) == 0) {
        return 1;
    }
    return 0;
}








#define WS_CONNEXION_TIMEOUT 10000
#define WS_MAX_DATA_LEN  124


/*
*   send Websocket frame for a hub
*/
static int ws_sendFrame(HubSt *hub, int stream, int tcpchan, const u8 *data, int datalen, char *errmsg)
{
    u32 buffer_32[33];
    u32 mask;
    int i;
    WSStreamHead strym;
    u8 *p = (u8*)buffer_32;
#ifdef DEBUG_SLOW_TCP
    u64 start = yapiGetTickCount();
#endif
    int tcp_write_res;

    YASSERT(datalen <= WS_MAX_DATA_LEN);
#ifdef DEBUG_WEBSOCKET
    // disable masking for debugging
    mask = 0;
#else
    mask = YRand32();
#endif
    // do not start at offset zero on purpose
    // we want the buffer to be aligned on u32
    p[0] = 0x82;
    p[1] = (u8)(datalen + 1) | 0x80;;
    p[2] = ((u8*)&mask)[2];
    p[3] = ((u8*)&mask)[3];

    p[4] = ((u8*)&mask)[0];
    p[5] = ((u8*)&mask)[1];
    strym.tcpchan = tcpchan;
    strym.stream = stream;
    p[6] = strym.encaps ^ p[2];
    if (datalen) {
        p[7] = *data ^ p[3];
    }
    if (datalen > 1) {
        memcpy(buffer_32 + 2, data + 1, datalen - 1);
        for (i = 0; i < (datalen - 1 + 3) >> 2; i++) {
            buffer_32[i + 2] ^= mask;
        }
    }
    tcp_write_res = yTcpWrite(hub->ws.skt, (char*)p, datalen + 7, errmsg);
#ifdef DEBUG_SLOW_TCP
    u64 delta = yapiGetTickCount() - start;
    if (delta > 10) {
        dbglog("WS: yTcpWrite took %"FMTu64"ms (stream=%d chan=%d res=%d)\n", delta, strym.stream, strym.tcpchan, tcp_write_res);
    }
#endif
    return tcp_write_res;
}

/*
*   send authentication meta
*/
static int ws_sendAuthenticationMeta(HubSt *hub, char *errmsg)
{
    USB_Meta_Pkt meta_out;
    memset(&meta_out, 0, sizeof(USB_Meta_Pkt));
    meta_out.auth.metaType = USB_META_WS_AUTHENTICATION;

#if 1
    if (hub->ws.remoteVersion <USB_META_WS_PROTO_V2) {
        meta_out.auth.version = USB_META_WS_PROTO_V1;
    } else {
        meta_out.auth.version = USB_META_WS_PROTO_V2;
    }
#else
    meta_out.auth.version = USB_META_WS_PROTO_V1;
#endif
    if (hub->ws.user != INVALID_HASH_IDX && hub->ws.pass != INVALID_HASH_IDX) {
        u8 ha1[16];
        const char * user = yHashGetStrPtr(hub->ws.user);
        const char * pass = yHashGetStrPtr(hub->ws.pass);
        meta_out.auth.flags = USB_META_WS_AUTH_FLAGS_VALID;
        meta_out.auth.nonce = INTEL_U32(hub->ws.nounce);
        ComputeAuthHA1(ha1, user, pass, hub->ws.serial);
        CheckWSAuth(hub->ws.remoteNounce, ha1, NULL, meta_out.auth.sha1);
    }
    return ws_sendFrame(hub,YSTREAM_META ,0, (const u8*) &meta_out, USB_META_WS_AUTHENTICATION_SIZE, errmsg);
}

static void ws_appendTCPData(RequestSt* req, u8* buffer, int pktlen, int isClose)
{
    if (pktlen) {
        if (req->replybufsize < req->replysize + pktlen) {
            u8 *newbuff;
            req->replybufsize <<= 1;
            newbuff = yMalloc(req->replybufsize);
            memcpy(newbuff, req->replybuf, req->replysize);
            yFree(req->replybuf);
            req->replybuf = newbuff;
        }

        memcpy(req->replybuf + req->replysize, buffer, pktlen);
        req->replysize += pktlen;
    }
    req->read_tm = yapiGetTickCount();
    if (isClose) {
        req->state = REQ_CLOSED;
        ySetEvent(&req->finished);
        if (req->callback != NULL) {
            // async request are automaticaly closed
            yWSCloseReqEx(req, 0);
            yReqFree(req);
        }
    }

}

/*
*   ws_parseIncommingFrame parse incomming Websocket frame
*/
static int ws_parseIncommingFrame(HubSt *hub, u8 *buffer, int pktlen, char *errmsg)
{
    WSStreamHead strym;
    RequestSt *req;
    int flags;
    const char * user;
    const char * pass;
    int maxtcpws;
#ifdef DEBUG_WEBSOCKET
    u64 reltime = yapiGetTickCount() - hub->ws.connectionTime;
#endif

    YASSERT(pktlen > 0);
    strym.encaps = buffer[0];
    buffer++;
    pktlen--;
    switch (strym.stream) {
    case YSTREAM_TCP_NOTIF:
        if (pktlen > 0) {
#if 0
        {
                FILE *f;
                //printf("%s", buffer);
                YASSERT(YFOPEN(&f, "req_trace\\api_not.txt", "ab") == 0);
                fwrite(buffer, 1, pktlen, f);
                fclose(f);
            }
#endif
            yPushFifo(&hub->not_fifo, buffer, pktlen);
            while (handleNetNotification(hub));
        }
        break;
    case YSTREAM_EMPTY:
        return YAPI_SUCCESS;
    case YSTREAM_TCP_ASYNCCLOSE:
        yEnterCriticalSection(&hub->ws.chan[strym.tcpchan].access);
        req = hub->ws.chan[strym.tcpchan].requests;
        while (req != NULL && req->state != REQ_OPEN && req->state != REQ_CLOSED_BY_HUB) {
            req = req->ws.next;
        }
        if (req == NULL) {
            dbglog("Drop incomming TCP trafic on channel (%d/%d)\n", strym.stream, strym.tcpchan);
        } else {
            int asyncid = buffer[pktlen - 1];
            pktlen--;
            if (req->ws.asyncId != asyncid) {
                dbglog("WS: Incorrect async-close signature on tcpChan %d (%d)\n", strym.tcpchan, asyncid);
                break;
            }
            WSLOG("req(%s:%p) close async %d\n", req->hub->name, req, req->ws.asyncId);
            ws_appendTCPData(req, buffer, pktlen, 1);
        }
        yLeaveCriticalSection(&hub->ws.chan[strym.tcpchan].access);
        break;
    case YSTREAM_TCP:
    case YSTREAM_TCP_CLOSE:
        yEnterCriticalSection(&hub->ws.chan[strym.tcpchan].access);
        req = hub->ws.chan[strym.tcpchan].requests;
        while (req != NULL && req->state!= REQ_OPEN && req->state != REQ_CLOSED_BY_HUB) {
            req = req->ws.next;
        }
        if (req == NULL) {
            dbglog("Drop incomming TCP trafic on channel (%d/%d)\n", strym.stream, strym.tcpchan);
        } else {
            if (strym.stream == YSTREAM_TCP_CLOSE) {
                int res = ws_sendFrame(hub, YSTREAM_TCP_CLOSE, strym.tcpchan, NULL, 0, errmsg);
                if (res < 0) {
                    WSLOG("req(%s:%p) unable to ack remote close (%d/%s)\n", req->hub->name, req, res, errmsg);
                }
                //WSLOG("req(%s:%p) close\n", req->hub->name, req);
            }
            ws_appendTCPData(req, buffer, pktlen, strym.stream == YSTREAM_TCP_CLOSE);
        }
        yLeaveCriticalSection(&hub->ws.chan[strym.tcpchan].access);
        break;
    case YSTREAM_META: {
        USB_Meta_Pkt *meta = (USB_Meta_Pkt*)(buffer);
        WSLOG("%"FMTu64": META type=%d len=%d\n",reltime, meta->announce.metaType, pktlen);
        switch (meta->announce.metaType) {
        case USB_META_WS_ANNOUNCE:
            if (meta->announce.version < USB_META_WS_PROTO_V1 || pktlen < USB_META_WS_ANNOUNCE_SIZE) {
                return YAPI_SUCCESS;
            }
            hub->ws.remoteVersion = meta->announce.version;
            hub->ws.remoteNounce = INTEL_U32(meta->announce.nonce);
            maxtcpws = INTEL_U16(meta->announce.maxtcpws);
            if (maxtcpws > 0) {
                hub->ws.tcpMaxWindowSize = maxtcpws;
            }
            YSTRCPY(hub->ws.serial, YOCTO_SERIAL_LEN, meta->announce.serial);
            WSLOG("hub(%s) Announce: %s (v%d / %x)\n", hub->name, meta->announce.serial, meta->announce.version, hub->ws.remoteNounce);
            hub->ws.nounce = YRand32();
            hub->ws.base_state = WS_BASE_AUTHENTICATING;
            hub->ws.connectionTime = yapiGetTickCount();
            return ws_sendAuthenticationMeta(hub, errmsg);
        case USB_META_WS_AUTHENTICATION:
            if (hub->ws.base_state != WS_BASE_AUTHENTICATING)
                return YAPI_SUCCESS;
            if (meta->auth.version < USB_META_WS_PROTO_V1 || (u32)pktlen < USB_META_WS_AUTHENTICATION_SIZE) {
                return YAPI_SUCCESS;
            }
            hub->ws.tcpRoundTripTime = (u32)(yapiGetTickCount() - hub->ws.connectionTime + 1);
            if(hub->ws.tcpMaxWindowSize < 2048 && hub->ws.tcpRoundTripTime < 7) {
                // Fix overly optimistic round-trip on YoctoHubs
                hub->ws.tcpRoundTripTime = 7;
            }
#ifdef DEBUG_WEBSOCKET
            {
                int uploadRate = hub->ws.tcpMaxWindowSize * 1000 / hub->ws.tcpRoundTripTime;
                dbglog("RTT=%dms, WS=%d, uploadRate=%f KB/s\n", hub->ws.tcpRoundTripTime, hub->ws.tcpMaxWindowSize, uploadRate/1000.0);
            }
#endif

            flags = meta->auth.flags;
            if ((flags & USB_META_WS_AUTH_FLAGS_RW) != 0) {
                hub->rw_access = 1;
            }
            if (hub->ws.user != INVALID_HASH_IDX) {
                user = yHashGetStrPtr(hub->ws.user);
            }else {
                user = "";
            }

            if (hub->ws.pass != INVALID_HASH_IDX) {
                pass = yHashGetStrPtr(hub->ws.pass);
            } else {
                pass = "";
            }
            if ((flags & USB_META_WS_AUTH_FLAGS_VALID) != 0) {
                u8 ha1[16];
                ComputeAuthHA1(ha1, user, pass, hub->ws.serial);
                if (CheckWSAuth(hub->ws.nounce, ha1, meta->auth.sha1, NULL)) {
                    hub->ws.base_state = WS_BASE_CONNECTED;
                    hub->state = NET_HUB_ESTABLISHED;
                    hub->retryCount = 0;
                    hub->attemptDelay = 500;
                    WSLOG("hub(%s): connected as %s\n", hub->name, user);
                } else {
                    YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "Authentication as %s failed (%s:%d)", user, __FILE_ID__, __LINE__);
                    return YAPI_UNAUTHORIZED;
                }
            } else {
                if (hub->ws.user == INVALID_HASH_IDX) {
                    hub->ws.base_state = WS_BASE_CONNECTED;
                    hub->state = NET_HUB_ESTABLISHED;
                    hub->retryCount = 0;
                    hub->attemptDelay = 500;
                    WSLOG("hub(%s): connected\n",hub->name);
                } else {
                    if (YSTRCMP(user,"admin")==0  && !hub->rw_access) {
                        YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "Authentication as %s failed", user);
                    } else {
                        YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "Authentication error : hub has no password for %s", user);
                    }
                    return YAPI_UNAUTHORIZED;
                }
            }
            break;
        case USB_META_WS_ERROR:
            if (INTEL_U16(meta->error.htmlcode) == 401) {
                return YERR(YAPI_UNAUTHORIZED);
            } else {
                YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "Remote hub closed connection with error %d", INTEL_U16(meta->error.htmlcode));
                return YAPI_IO_ERROR;
            }
        case USB_META_ACK_UPLOAD:
        {
            int tcpchan = meta->uploadAck.tcpchan;
            yEnterCriticalSection(&hub->ws.chan[tcpchan].access);
            req = hub->ws.chan[tcpchan].requests;
            while (req != NULL && req->state != REQ_OPEN && req->state != REQ_CLOSED_BY_HUB) {
                req = req->ws.next;
            }
            if (req) {
                u32 ackBytes = meta->uploadAck.totalBytes[0] + (meta->uploadAck.totalBytes[1] << 8) + (meta->uploadAck.totalBytes[2] << 16) + (meta->uploadAck.totalBytes[3] << 24);
                u64 ackTime = yapiGetTickCount();
                if (hub->ws.chan[tcpchan].lastUploadAckTime && ackBytes > hub->ws.chan[tcpchan].lastUploadAckBytes) {
                    int deltaBytes;
                    u64 deltaTime;
                    u32 newRate;
                    hub->ws.chan[tcpchan].lastUploadAckBytes = ackBytes;
                    hub->ws.chan[tcpchan].lastUploadAckTime = ackTime;

                    deltaBytes = ackBytes - hub->ws.chan[tcpchan].lastUploadRateBytes;
                    deltaTime = ackTime - hub->ws.chan[tcpchan].lastUploadRateTime;
                    WSLOG("delta  bytes=%d  time=%"FMTu64"ms\n",deltaBytes, deltaTime);


                    if (deltaTime < 500) {
                        yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
                        break; // wait more
                    }
                    if (deltaTime < 1000 && deltaBytes < 65536) {
                        yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
                        break; // wait more
                    }
                    hub->ws.chan[tcpchan].lastUploadRateBytes = ackBytes;
                    hub->ws.chan[tcpchan].lastUploadRateTime = ackTime;
                    if (req->progressCb && req->ws.requestsize) {
                        req->progressCb(req->progressCtx, ackBytes, req->ws.requestsize);
                    }
                    newRate = (u32)(deltaBytes * 1000 / deltaTime);
                    hub->ws.uploadRate = (u32)(0.8 * hub->ws.uploadRate + 0.3 * newRate);
                    WSLOG("New rate: %.2f KB/s (based on %.2f KB in %.2fs)\n", hub->ws.uploadRate / 1000.0, deltaBytes / 1000.0, deltaTime / 1000.0);
                } else {
                    WSLOG("First Ack received (rate=%d)\n", hub->ws.uploadRate);
                    hub->ws.chan[tcpchan].lastUploadAckBytes = ackBytes;
                    hub->ws.chan[tcpchan].lastUploadAckTime = ackTime;
                    hub->ws.chan[tcpchan].lastUploadRateBytes = ackBytes;
                    hub->ws.chan[tcpchan].lastUploadRateTime = ackTime;
                    if (req->progressCb && req->ws.requestsize) {
                        req->progressCb(req->progressCtx, ackBytes, req->ws.requestsize);
                    }
                }
            }
            yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
        }
            break;
        default:
            WSLOG("unhandled Meta pkt %d\n", meta->announce.metaType);
            break;
        }
    }
        break;
    case YSTREAM_NOTICE:
    case YSTREAM_REPORT:
    case YSTREAM_REPORT_V2:
    case YSTREAM_NOTICE_V2:
    default:
        dbglog("Invalid WS stream type (%d)\n", strym.stream);
    }
    return YAPI_SUCCESS;
}

// return 1 if there is still a request pending, 0 if all is done, -1 on error
static int ws_requestStillPending(HubSt* hub)
{
    int tcpchan;
    for (tcpchan = 0; tcpchan < MAX_ASYNC_TCPCHAN; tcpchan++) {
        RequestSt *req = NULL;
        yEnterCriticalSection(&hub->ws.chan[tcpchan].access);
        req = hub->ws.chan[tcpchan].requests;
        while (req && req->state == REQ_CLOSED) {
            req = req->ws.next;
        }
        yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
        if (req) {
            return 1;
        }
    }
    return 0;
}



/*
*   look through all pending request if there is some data that we can send
*
*/
static int ws_processRequests(HubSt* hub, char *errmsg)
{
    int  tcpchan;
    int res;

    if (hub->ws.next_transmit_tm && hub->ws.next_transmit_tm > yapiGetTickCount()) {
        return YAPI_SUCCESS;
    }

    for (tcpchan = 0; tcpchan < MAX_ASYNC_TCPCHAN; tcpchan++) {
        yEnterCriticalSection(&hub->ws.chan[tcpchan].access);
        if (hub->ws.chan[tcpchan].requests) {
            RequestSt *req = hub->ws.chan[tcpchan].requests;
            while (req) {
                while (req && req->ws.requestsize == req->ws.requestpos && (req->state == REQ_CLOSED || req->state == REQ_CLOSED_BY_API)) {
                    req = req->ws.next;
                }
                if (req) {
                    int throttle_start = req->ws.requestpos;
                    int throttle_end = req->ws.requestsize;
                    if (throttle_end > 2108 && hub->ws.remoteVersion >= USB_META_WS_PROTO_V2 && tcpchan == 0) {
                        // Perform throttling on large uploads
                        if (req->ws.requestpos == 0) {
                            // First chunk is always first multiple of full window (124 bytes) above 2KB
                            throttle_end = 2108;
                            // Prepare to compute effective transfer rate
                            hub->ws.chan[tcpchan].lastUploadAckBytes = 0;
                            hub->ws.chan[tcpchan].lastUploadAckTime = 0;
                            // Start with initial RTT based estimate
                            hub->ws.uploadRate = hub->ws.tcpMaxWindowSize * 1000 / hub->ws.tcpRoundTripTime;
                        } else if (hub->ws.chan[tcpchan].lastUploadAckTime == 0) {
                            // first block not yet acked, wait more
                            //WSLOG("wait for first ack");
                            throttle_end = 0;
                        } else {
                            // adapt window frame to available bandwidth
                            int bytesOnTheAir = req->ws.requestpos - hub->ws.chan[tcpchan].lastUploadAckBytes;
                            u32 uploadRate = hub->ws.uploadRate;
                            u64 timeOnTheAir = (yapiGetTickCount() - hub->ws.chan[tcpchan].lastUploadAckTime);
                            u64 toBeSent = 2 * uploadRate + 1024 - bytesOnTheAir + (uploadRate * timeOnTheAir / 1000);
                            if (toBeSent + bytesOnTheAir > DEFAULT_TCP_MAX_WINDOW_SIZE) {
                                toBeSent = DEFAULT_TCP_MAX_WINDOW_SIZE - bytesOnTheAir;
                            }
                            WSLOG("throttling: %d bytes/s (%"FMTu64" + %d = %"FMTu64")\n", hub->ws.uploadRate, toBeSent, bytesOnTheAir, bytesOnTheAir + toBeSent);
                            if (toBeSent < 64) {
                                u64 waitTime = 1000 * (128 - toBeSent) / hub->ws.uploadRate;
                                if (waitTime < 2) waitTime = 2;
                                hub->ws.next_transmit_tm = yapiGetTickCount() + waitTime;
                                WSLOG("WS: %d sent %"FMTu64"ms ago, waiting %"FMTu64"ms...\n", bytesOnTheAir, timeOnTheAir, waitTime);
                                throttle_end = 0;
                            }
                            if (throttle_end > req->ws.requestpos + toBeSent) {
                                // when sending partial content, round up to full frames
                                if (toBeSent > 124) {
                                    toBeSent = (toBeSent / 124) * 124;
                                }
                                throttle_end = req->ws.requestpos + (u32)toBeSent;
                            }
                        }
                    }
                    while (req->ws.requestpos < throttle_end) {
                        int stream = YSTREAM_TCP;
                        int datalen = throttle_end - req->ws.requestpos;
                        if (datalen > WS_MAX_DATA_LEN) {
                            datalen = WS_MAX_DATA_LEN;
                        }
                        if (req->ws.requestpos == 0) {
                            req->ws.first_write_tm = yapiGetTickCount();
                        }

                        if (req->ws.asyncId && (req->ws.requestpos + datalen == req->ws.requestsize)) {
                            // last frame of an async request
                            u8 tmp_data[128];

                            if (datalen == WS_MAX_DATA_LEN) {
                                // last frame is already full we must send the async close in another one
                                res = ws_sendFrame(hub, stream, tcpchan, req->ws.requestbuf + req->ws.requestpos, datalen, errmsg);
                                if (YISERR(res)) {
                                    req->errcode = res;
                                    YSTRCPY(req->errmsg, YOCTO_ERRMSG_LEN, errmsg);
                                    yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
                                    ySetEvent(&req->finished);
                                    return res;
                                }
                                WSLOG("ws_req:%p: send %d bytes on chan%d (%d/%d)\n", req, datalen, tcpchan, req->ws.requestpos, req->ws.requestsize);
                                req->ws.requestpos += datalen;
                                datalen = 0;
                            }
                            stream = YSTREAM_TCP_ASYNCCLOSE;
                            if (datalen) {
                                memcpy(tmp_data, req->ws.requestbuf + req->ws.requestpos, datalen);
                            }
                            tmp_data[datalen] = req->ws.asyncId;
                            res = ws_sendFrame(hub, stream, tcpchan, tmp_data, datalen + 1, errmsg);
                            WSLOG("req(%s:%p) sent async close %d\n", req->hub->name, req, req->ws.asyncId);
                            req->ws.last_write_tm = yapiGetTickCount();
                        } else {
                            res = ws_sendFrame(hub, stream, tcpchan, req->ws.requestbuf + req->ws.requestpos, datalen, errmsg);
                            req->ws.last_write_tm = yapiGetTickCount();
                            //WSLOG("ws_req:%p: sent %d bytes on chan%d (%d/%d)\n", req, datalen, tcpchan, req->ws.requestpos, req->ws.requestsize);
                        }
                        if (YISERR(res)) {
                            req->errcode = res;
                            YSTRCPY(req->errmsg, YOCTO_ERRMSG_LEN, errmsg);
                            yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);
                            ySetEvent(&req->finished);
                            return res;
                        }
                        req->ws.requestpos += datalen;
                    }
                    if (req->ws.requestpos < req->ws.requestsize) {
                        int sent = req->ws.requestpos - throttle_start;
                        // not completely sent, cannot do more for now
                        if (sent && hub->ws.uploadRate > 0) {
                            u64 waitTime = 1000 * sent / hub->ws.uploadRate;
                            if (waitTime < 2) waitTime = 2;
                            hub->ws.next_transmit_tm = yapiGetTickCount() + waitTime;
                            WSLOG("Sent %dbytes, waiting %"FMTu64"ms...\n", sent, waitTime);
                        } else {
                            hub->ws.next_transmit_tm = yapiGetTickCount() + 100;
                        }
                        req = NULL;
                    } else {
                        // end of request get ne following one
                        req = req->ws.next;
                    }
                }
            }
        }
        yLeaveCriticalSection(&hub->ws.chan[tcpchan].access);

    }
    return YAPI_SUCCESS;
}


/*
*   Open Base tcp socket (done in background by yws_thread)
*/
static int ws_openBaseSocket(HubSt* basehub, int first_notification_connection, int mstimout, char* errmsg)
{
    char buffer[YOCTO_HOSTNAME_NAME];
    u32 ip;
    u16 port;
    yAsbUrlProto proto;
    yStrRef user, pass, subdomain;
    int res, tcpchan, request_len;
    char request[256];
    char subdomain_buf[32];
    struct _WSNetHubSt* wshub = &basehub->ws;

    memset(wshub, 0, sizeof(WSNetHub));
    wshub->skt = INVALID_SOCKET;
    wshub->s_next_async_id = 48;

    switch (yHashGetUrlPort(basehub->url, buffer, &port, &proto, &user, &pass, &subdomain)) {
    case NAME_URL:
        ip = resolveDNSCache(basehub->url, errmsg);
        if (ip == 0) {
            return YAPI_IO_ERROR;
        }
        break;
    case IP_URL:
        ip = inet_addr(buffer);
        break;
    default:
        return YERRMSG(YAPI_IO_ERROR, "not an IP hub");
    }
    if (proto == PROTO_HTTP) {
        return YERRMSG(YAPI_IO_ERROR, "not a websocket url");
    }
    if (subdomain== INVALID_HASH_IDX) {
        subdomain_buf[0] = 0;
    }else {
        YSPRINTF(subdomain_buf, 32, "/%s", yHashGetStrPtr(subdomain));
    }

    WSLOG("hub(%s) try to open WS connection at %d\n", basehub->name, basehub->notifAbsPos);
    if (first_notification_connection) {
        YSPRINTF(request, 256, "GET %s/not.byn", subdomain_buf);
    } else {
        YSPRINTF(request, 256, "GET %s/not.byn?abs=%u", subdomain_buf, basehub->notifAbsPos);
    }

    res = yTcpOpen(&wshub->skt, ip, port, mstimout, errmsg);
    if (YISERR(res)) {
        // yTcpOpen has reset the socket to INVALID
        yTcpClose(wshub->skt);
        wshub->skt = INVALID_SOCKET;
        return res;
    }
    wshub->bws_open_tm = yapiGetTickCount();
    wshub->bws_timeout_tm = mstimout;
    wshub->user = user;
    wshub->pass = pass;
    //write header
    request_len = YSTRLEN(request);

    res = yTcpWrite(wshub->skt, request, request_len, errmsg);
    if (YISERR(res)) {
        yTcpClose(wshub->skt);
        wshub->skt = INVALID_SOCKET;
        return res;
    }
    res = yTcpWrite(wshub->skt, ws_header_start, YSTRLEN(ws_header_start), errmsg);
    if (YISERR(res)) {
        yTcpClose(wshub->skt);
        wshub->skt = INVALID_SOCKET;
        return res;
    }

    wshub->websocket_key_len = GenereateWebSockeyKey((u8*)request, request_len, wshub->websocket_key);
    res = yTcpWrite(wshub->skt, wshub->websocket_key, wshub->websocket_key_len, errmsg);
    if (YISERR(res)) {
        yTcpClose(wshub->skt);
        wshub->skt = INVALID_SOCKET;
        return res;
    }

    res = yTcpWrite(wshub->skt, ws_header_end, YSTRLEN(ws_header_end), errmsg);
    if (YISERR(res)) {
        yTcpClose(wshub->skt);
        wshub->skt = INVALID_SOCKET;
        return res;
    }

    wshub->fifo_buffer = yMalloc(2048);
    yFifoInit(&wshub->mainfifo, wshub->fifo_buffer, 2048);
    for (tcpchan = 0; tcpchan < MAX_ASYNC_TCPCHAN; tcpchan++) {
        yInitializeCriticalSection(&wshub->chan[tcpchan].access);
    }
    return YAPI_SUCCESS;
}




/*
*   Close Base tcp socket (done in background by yws_thread)
*/
static void ws_closeBaseSocket(struct _WSNetHubSt *base_req)
{
    int tcpchan;
    yTcpClose(base_req->skt);
    base_req->skt = INVALID_SOCKET;
    for (tcpchan = 0; tcpchan < MAX_ASYNC_TCPCHAN; tcpchan++) {
        yDeleteCriticalSection(&base_req->chan[tcpchan].access);
    }
    yFifoCleanup(&base_req->mainfifo);
    yFree(base_req->fifo_buffer);
}


/*
*   select used by background thread
*/
static int ws_thread_select(struct _WSNetHubSt *base_req, u64 ms, WakeUpSocket *wuce, char *errmsg)
{
    fd_set      fds;
    struct timeval timeout;
    int         res;
    YSOCKET     sktmax = 0;

    memset(&timeout, 0, sizeof(timeout));
    timeout.tv_sec = (long)ms / 1000;
    timeout.tv_usec = (int)(ms % 1000) * 1000;
    /* wait for data */
    FD_ZERO(&fds);
    if (wuce) {
        FD_SET(wuce->listensock, &fds);
        sktmax = wuce->listensock;
    }

    if (base_req->skt == INVALID_SOCKET) {
        return YERR(YAPI_INVALID_ARGUMENT);
    } else {
        FD_SET(base_req->skt, &fds);
        if (base_req->skt > sktmax)
            sktmax = base_req->skt;
    }
    if (sktmax == 0) {
        return YAPI_SUCCESS;
    }
    res = select((int)sktmax + 1, &fds, NULL, NULL, &timeout);
    if (res < 0) {
#ifndef WINDOWS_API
        if (SOCK_ERR == EAGAIN) {
            return 0;
        } else
#endif
        {
            res = yNetSetErr();
            return res;
        }
    }
    if (res != 0) {
        if (wuce && FD_ISSET(wuce->listensock, &fds)) {
            int signal = yConsumeWakeUpSocket(wuce, errmsg);
            //dbglog("exit from sleep with WUCE (%d)\n", signal);
            YPROPERR(signal);
        }
        if (FD_ISSET(base_req->skt, &fds)) {
            int avail = yFifoGetFree(&base_req->mainfifo);
            int readed = 0;
            if (avail) {
                u8 buffer[2048];
                if (avail > 2048) {
                    avail = 2048;
                }
                readed = yTcpRead(base_req->skt, buffer, avail, errmsg);
                if (readed > 0) {
                    yPushFifo(&base_req->mainfifo, buffer, readed);
                }
            }
            return readed;
        }
    }
    return YAPI_SUCCESS;
}



static void ws_threadUpdateRetryCount(HubSt *hub)
{
    hub->attemptDelay = 500 << hub->retryCount;
    if (hub->attemptDelay > 8000)
        hub->attemptDelay = 8000;
    hub->retryCount++;
#ifdef DEBUG_WEBSOCKET
    dbglog("hub(%s): IO error on ws_thread:(%d) %s\n", hub->name, hub->errcode, hub->errmsg);
    dbglog("hub(%s): retry in %dms (%d retries)\n", hub->name, hub->attemptDelay, hub->retryCount);
#endif

}

/**
 *   Background  thread for WebSocket Hub
 */
void* ws_thread(void* ctx)
{
    char *p;
    yThread *thread = (yThread*)ctx;
    char errmsg[YOCTO_ERRMSG_LEN];
    HubSt *hub = (HubSt*)thread->ctx;
    int res;
    int first_notification_connection = 1;
    u8 header[8];
    char buffer[2048];
    int buffer_ofs = 0;
    int continue_processing;


    yThreadSignalStart(thread);
    WSLOG("hub(%s) start thread \n", hub->name);

    while (!yThreadMustEnd(thread) && hub->state != NET_HUB_TOCLOSE) {

        if (hub->retryCount > 0) {
            u64 timeout = yapiGetTickCount() + hub->attemptDelay;
            do {
                //minimal timouout is allways 500
                yApproximateSleep(100);
            } while (timeout > yapiGetTickCount());
        }
        if (hub->state == NET_HUB_TOCLOSE) {
            break;
        }
        res = ws_openBaseSocket(hub, first_notification_connection, 1000, errmsg);
        hub->lastAttempt = yapiGetTickCount();
        if (YISERR(res)) {
            yEnterCriticalSection(&hub->access);
            hub->errcode = ySetErr(res, hub->errmsg, errmsg, NULL, 0);
            yLeaveCriticalSection(&hub->access);
            ws_threadUpdateRetryCount(hub);
            continue;
        }
        WSLOG("hub(%s) base socket opened (skt=%x)\n", hub->name, hub->ws.skt);
        hub->state = NET_HUB_TRYING;
        hub->ws.base_state = WS_BASE_HEADER_SENT;
        hub->ws.connectionTime = 0;
        hub->ws.tcpRoundTripTime = DEFAULT_TCP_ROUND_TRIP_TIME;
        hub->ws.tcpMaxWindowSize = DEFAULT_TCP_MAX_WINDOW_SIZE;
        errmsg[0] = 0;
        continue_processing = 1;
        do {
            u64 wait;
            u64 now = yapiGetTickCount();
            if (hub->ws.next_transmit_tm >= now) {
                wait = hub->ws.next_transmit_tm - now;
            } else {
                wait = 1000;
            }
            //dbglog("select %"FMTu64"ms on main socket\n", wait);
            res = ws_thread_select(&hub->ws, wait, &hub->wuce, errmsg);
            if (YISERR(res)) {
                WSLOG("hub(%s) ws_thread_select error %d:%s\n", hub->name, res, errmsg);
            }

            if (res > 0) {
                int need_more_data = 0;
                int avail, rw;
                int hdrlen;
                u32 mask;
                int websocket_ok = 0;
                int pktlen;
                do {
                    u16 pos;
                    //something to handle;
                    switch (hub->ws.base_state) {
                    case WS_BASE_HEADER_SENT:
                        pos = ySeekFifo(&hub->ws.mainfifo, (const u8*)"\r\n\r\n", 4, 0, 0, 0);
                        if (pos == 0xffff) {
                            if ((u64)(yapiGetTickCount() - hub->lastAttempt) > WS_CONNEXION_TIMEOUT) {
                                res = YERR(YAPI_TIMEOUT);
                            } else {
                                need_more_data = 1;
                            }
                            break;
                        } else if (pos >= 2044) {
                            res = YERRMSG(YAPI_IO_ERROR, "Bad reply header");
                            // fatal error do not retry to reconnect
                            hub->state = NET_HUB_TOCLOSE;
                            break;
                        }
                        pos = ySeekFifo(&hub->ws.mainfifo, (const u8*)"\r\n", 2, 0, 0, 0);
                        yPopFifo(&hub->ws.mainfifo, (u8*)buffer, pos + 2);
                        if (YSTRNCMP(buffer, "HTTP/1.1 ", 9) != 0) {
                            res = YERRMSG(YAPI_IO_ERROR, "Bad reply header");
                            // fatal error do not retry to reconnect
                            hub->state = NET_HUB_TOCLOSE;
                            break;
                        }
                        p = buffer + 9;
                        if (YSTRNCMP(p, "101", 3) != 0) {
                            res = YERRMSG(YAPI_IO_ERROR, "hub does not support WebSocket");
                            // fatal error do not retry to reconnect
                            hub->state = NET_HUB_TOCLOSE;
                            break;
                        }
                        websocket_ok = 0;
                        pos = ySeekFifo(&hub->ws.mainfifo, (const u8*)"\r\n", 2, 0, 0, 0);
                        while (pos != 0) {
                            yPopFifo(&hub->ws.mainfifo, (u8*)buffer, pos + 2);
                            if (pos > 22 && YSTRNICMP(buffer, "Sec-WebSocket-Accept: ", 22) == 0) {
                                if (!VerifyWebsocketKey(buffer + 22, pos, hub->ws.websocket_key, hub->ws.websocket_key_len)) {
                                    websocket_ok = 1;
                                } else {
                                    res = YERRMSG(YAPI_IO_ERROR, "hub does not use same WebSocket protocol");
                                    // fatal error do not retry to reconnect
                                    hub->state = NET_HUB_TOCLOSE;
                                    break;
                                }
                            }
                            if ((u64)(yapiGetTickCount() - hub->lastAttempt) > WS_CONNEXION_TIMEOUT) {
                                res = YERR(YAPI_TIMEOUT);
                                break;
                            }
                            pos = ySeekFifo(&hub->ws.mainfifo, (const u8*)"\r\n", 2, 0, 0, 0);
                        }
                        yPopFifo(&hub->ws.mainfifo, NULL, 2);
                        if (websocket_ok) {
                            hub->ws.base_state = WS_BASE_SOCKET_UPGRADED;
                            buffer_ofs = 0;
                        } else {
                            res = YERRMSG(YAPI_IO_ERROR, "Invalid WebSocket header");
                            // fatal error do not retry to reconnect
                            hub->state = NET_HUB_TOCLOSE;
                        }
                        break;
                    case WS_BASE_SOCKET_UPGRADED:
                    case WS_BASE_AUTHENTICATING:
                    case WS_BASE_CONNECTED:

                        avail = yFifoGetUsed(&hub->ws.mainfifo);
                        if (avail < 2) {
                            need_more_data = 1;
                            break;
                        }
                        rw = (avail < 7 ? avail : 7);
                        yPeekFifo(&hub->ws.mainfifo, header, rw, 0);
                        pktlen = header[1] & 0x7f;
                        if (pktlen > 125) {
                            // Unsupported long frame, drop all incoming data (probably 1+ frame(s))
                            res = YERRMSG(YAPI_IO_ERROR, "Unsupported long websocket frame");
                            break;
                        }

                        if (header[1] & 0x80) {
                            // masked frame
                            hdrlen = 6;
                            if (avail < hdrlen + pktlen) {
                                need_more_data = 1;
                                break;
                            }
                            memcpy(&mask, header + 2, sizeof(u32));
                        } else {
                            // plain frame
                            hdrlen = 2;
                            if (avail < hdrlen + pktlen) {
                                need_more_data = 1;
                                break;
                            }
                            mask = 0;
                        }

                        if ((header[0] & 0x7f) != 0x02) {
                            // Non-data frame
                            if (header[0] == 0x88) {
                                //if (USBTCPIsPutReady(sock) < 8) return;
                                // websocket close, reply with a close
                                header[0] = 0x88;
                                header[1] = 0x82;
                                mask = YRand32();
                                memcpy(header + 2, &mask, sizeof(u32));
                                header[6] = 0x03 ^ ((u8 *)&mask)[0];
                                header[7] = 0xe8 ^ ((u8 *)&mask)[1];
                                res = yTcpWrite(hub->ws.skt, (char*)header, 8, errmsg);
                                if (YISERR(res)) {
                                    break;
                                }
                                hub->ws.base_state = WS_BASE_OFFLINE;
#ifdef DEBUG_WEBSOCKET
                                dbglog("WS: io error on base socket of %s(%X): %s\n", hub->name, hub->url, errmsg);
#endif
                            } else {
                                // unhandled packet
                                dbglog("unhandled packet:%x%x\n", header[0], header[1]);
                            }
                            yPopFifo(&hub->ws.mainfifo, NULL, hdrlen + pktlen);
                            break;
                        }
                        // drop frame header
                        yPopFifo(&hub->ws.mainfifo, NULL, hdrlen);
                        // append
                        yPopFifo(&hub->ws.mainfifo, (u8*)buffer + buffer_ofs, pktlen);
                        if (mask) {
                            int i;
                            for (i = 0; i < (pktlen + 1 + 3) >> 2; i++) {
                                buffer[buffer_ofs + i] ^= mask;
                            }
                        }

                        if (header[0] == 0x02) {
                            //  fragmented binary frame
                            WSStreamHead strym;
                            strym.encaps = buffer[buffer_ofs];
                            if (strym.stream == YSTREAM_META) {
                                // unsupported fragmented META stream, should never happen
                                dbglog("Warning:fragmented META\n");
                                break;
                            }
                            buffer_ofs += pktlen;
                            break;
                        }

                        res = ws_parseIncommingFrame(hub, (u8*)buffer, buffer_ofs + pktlen, errmsg);
                        if (YISERR(res)) {
                            WSLOG("hub(%s) ws_parseIncommingFrame error %d:%s\n", hub->name, res, errmsg);
                            break;
                        }
                        buffer_ofs = 0;
                        break;
                    case  WS_BASE_OFFLINE:
                        break;
                    }
                } while (!need_more_data && !YISERR(res));
            }
            if (!YISERR(res)) {
                res = ws_processRequests(hub, errmsg);
                if (YISERR(res)) {
                    WSLOG("hub(%s) ws_processRequests error %d:%s\n", hub->name, res, errmsg);
                }
            }

            if (YISERR(res)) {
                continue_processing = 0;
            } else if ((yThreadMustEnd(thread) || hub->state == NET_HUB_TOCLOSE) && !ws_requestStillPending(hub)) {
                continue_processing = 0;
            }
        } while (continue_processing);
        if (YISERR(res)) {
            WSLOG("hub(%s) io error %d:%s\n", hub->name,res, errmsg);
            yEnterCriticalSection(&hub->access);
            hub->errcode = ySetErr(res, hub->errmsg, errmsg, NULL, 0);
            yLeaveCriticalSection(&hub->access);
            ws_threadUpdateRetryCount(hub);
        }
        WSLOG("hub(%s) close base socket %d:%s\n", hub->name, res, errmsg);
        ws_closeBaseSocket(&hub->ws);
        if (hub->state != NET_HUB_TOCLOSE) {
            hub->state = NET_HUB_DISCONNECTED;
        }
    }
    WSLOG("hub(%s) exit thread \n", hub->name);
    hub->state = NET_HUB_CLOSED;
    yThreadSignalEnd(thread);
    return NULL;
}



/********************************************************************************
 * UDP funtions
 *******************************************************************************/

//#define DEBUG_NET_DETECTION

os_ifaces detectedIfaces[NB_OS_IFACES];
int nbDetectedIfaces = 0;


#ifdef WINDOWS_API
YSTATIC int yDetectNetworkInterfaces(u32 only_ip)
{
    INTERFACE_INFO winIfaces[NB_OS_IFACES];
    DWORD          returnedSize, nbifaces, i;
    SOCKET         sock;

    nbDetectedIfaces = 0;
    memset(detectedIfaces, 0, sizeof(detectedIfaces));
    sock = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sock == INVALID_SOCKET){
        yNetLogErr();
        return -1;
    }
    if (WSAIoctl(sock, SIO_GET_INTERFACE_LIST, NULL, 0, winIfaces, sizeof(winIfaces), &returnedSize, NULL, NULL)<0){
        yNetLogErr();
        return -1;
    }

    nbifaces = returnedSize / sizeof(INTERFACE_INFO);
    for (i = 0; i<nbifaces; i++){
        if (winIfaces[i].iiFlags & IFF_LOOPBACK)
            continue;
        if (winIfaces[i].iiFlags & IFF_UP){
            if (winIfaces[i].iiFlags & IFF_MULTICAST)
                detectedIfaces[nbDetectedIfaces].flags |= OS_IFACE_CAN_MCAST;
            if (only_ip != 0 && only_ip != winIfaces[i].iiAddress.AddressIn.sin_addr.S_un.S_addr){
                continue;
            }
            detectedIfaces[nbDetectedIfaces].ip = winIfaces[i].iiAddress.AddressIn.sin_addr.S_un.S_addr;
            detectedIfaces[nbDetectedIfaces].netmask = winIfaces[i].iiNetmask.AddressIn.sin_addr.S_un.S_addr;
            nbDetectedIfaces++;
        }
    }
    return nbDetectedIfaces;

}
#else

#include <net/if.h>
#include <ifaddrs.h>
YSTATIC int yDetectNetworkInterfaces(u32 only_ip)
{
    struct ifaddrs *if_addrs = NULL;
    struct ifaddrs *p = NULL;
#if 1
    nbDetectedIfaces = 0;
    memset(detectedIfaces, 0, sizeof(detectedIfaces));
    if (getifaddrs(&if_addrs) != 0){
        yNetLogErr();
        return -1;
    }
    p = if_addrs;
    while (p) {
        if (p->ifa_addr && p->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *tmp;
            u32 ip, netmask;
            tmp = (struct sockaddr_in*)p->ifa_addr;
            ip = tmp->sin_addr.s_addr;
            if (only_ip != 0 && only_ip != ip){
                p = p->ifa_next;
                continue;
            }
            tmp = (struct sockaddr_in*)p->ifa_netmask;
            netmask = tmp->sin_addr.s_addr;
            if ((p->ifa_flags & IFF_LOOPBACK) == 0){
                if (p->ifa_flags & IFF_UP && p->ifa_flags & IFF_RUNNING){
#ifdef DEBUG_NET_DETECTION
                    ylogf("%s : ", p->ifa_name);
                    ylogIP(ip);
                    ylogf("/");
                    ylogIP(netmask);
                    ylogf(" (%X)\n", p->ifa_flags);
#endif
                    if (p->ifa_flags & IFF_MULTICAST){
                        detectedIfaces[nbDetectedIfaces].flags |= OS_IFACE_CAN_MCAST;
                    }
                    detectedIfaces[nbDetectedIfaces].ip = ip;
                    detectedIfaces[nbDetectedIfaces].netmask = netmask;
                    nbDetectedIfaces++;
                }
            }
#ifdef DEBUG_NET_DETECTION
            else {
                ylogf("drop %s : ", p->ifa_name);
                ylogIP(ip);
                ylogf("/");
                ylogIP(netmask);
                ylogf(" (%X)\n", p->ifa_flags);
            }
#endif
        }
        p = p->ifa_next;
    }

#else
    nbDetectedIfaces = 1;
    memset(detectedIfaces, 0, sizeof(detectedIfaces));
    detectedIfaces[0].flags |= OS_IFACE_CAN_MCAST;
    detectedIfaces[0].ip = INADDR_ANY;
#endif
    return nbDetectedIfaces;
}

#endif




static const char *discovery =
                "M-SEARCH * HTTP/1.1\r\n"
                "HOST:" YSSDP_MCAST_ADDR_STR ":" TOSTRING(YSSDP_PORT) "\r\n"
                "MAN:\"ssdp:discover\"\r\n"
                "MX:5\r\n"
                "ST:" YSSDP_URN_YOCTOPUCE"\r\n"
                "\r\n";


#define SSDP_NOTIFY "NOTIFY * HTTP/1.1\r\n"
#define SSDP_M_SEARCH "M-SEARCH * HTTP/1.1\r\n"
#define SSDP_HTTP "HTTP/1.1 200 OK\r\n"
#define SSDP_LINE_MAX_LEN 80u

#define UDP_IN_FIFO yFifoBuf


static char  hexatochar(char hi_c, char lo_c)
{
    u8 hi, lo;
    hi = ((u8)(hi_c)& 0x1f) ^ 0x10;
    lo = ((u8)(lo_c) & 0x1f) ^ 0x10;
    if (hi & 0x10) hi -= 7;
    if (lo & 0x10) lo -= 7;
    return (hi << 4) + lo;
}

static int uuidToSerial(const char * uuid, char *serial)
{
    int i;
    int len, padlen;
    char *s = serial;
    const char *u = uuid;

    for (i = 0, u = uuid; i < 4; i++, u += 2){
        *s++ = hexatochar(*u, *(u + 1));
    }
    u++;
    for (; i< 6; i++, u += 2){
        *s++ = hexatochar(*u, *(u + 1));
    }
    u++;
    for (; i< 8; i++, u += 2){
        *s++ = hexatochar(*u, *(u + 1));
    }
    *s++ ='-';
    u = strstr(uuid, "-COFF-EE");
    if (u == NULL) {
        return -1;
    }
    u += 8;
    while (*u == '0') u++;
    len = YSTRLEN(u);
    if (YSTRNCMP(serial, "VIRTHUB0", YOCTO_BASE_SERIAL_LEN) == 0) {
        padlen = YOCTO_SERIAL_SEED_SIZE - 1;
    } else {
        padlen = 5;
    }
    for (i = len; i < padlen; i++) {
        *s++ = '0';
    }
    *s = 0;
    YSTRCAT(serial, YOCTO_SERIAL_LEN, u);
    return 0;
}


static void ySSDPUpdateCache(SSDPInfos *SSDP, const char *uuid, const char * url, int cacheValidity)
{
    int i;

    if(cacheValidity<=0)
        cacheValidity = 1800;
    cacheValidity*=1000;

    for (i = 0; i < NB_SSDP_CACHE_ENTRY; i++){
        SSDP_CACHE_ENTRY *p = SSDP->SSDPCache[i];
        if (p == NULL)
            break;
        if (YSTRCMP(uuid,p->uuid) == 0) {
            p->detectedTime = yapiGetTickCount();
            p->maxAge = cacheValidity;

            if (YSTRCMP(url, p->url)){
                if (SSDP->callback) {
                    SSDP->callback(p->serial, url, p->url);
                }
                YSTRCPY(p->url, SSDP_URL_LEN, url);
            } else {
                if (SSDP->callback){
                    SSDP->callback(p->serial, url, NULL);
                }
            }
            return;
        }
    }
    if (i < NB_SSDP_CACHE_ENTRY){
        SSDP_CACHE_ENTRY *p = (SSDP_CACHE_ENTRY*) yMalloc(sizeof(SSDP_CACHE_ENTRY));
        YSTRCPY(p->uuid, SSDP_URL_LEN, uuid);
        if (uuidToSerial(p->uuid, p->serial) < 0) {
            yFree(p);
            return;
        }
        YSTRCPY(p->url,SSDP_URL_LEN,url);
        p->detectedTime = yapiGetTickCount();
        p->maxAge = cacheValidity;
        SSDP->SSDPCache[i] = p;
        if (SSDP->callback){
            SSDP->callback(p->serial, p->url, NULL);
        }
    }
}

static void ySSDPCheckExpiration(SSDPInfos *SSDP)
{
    int i;
    u64  now =yapiGetTickCount();

    for (i = 0; i<NB_SSDP_CACHE_ENTRY; i++) {
        SSDP_CACHE_ENTRY *p = SSDP->SSDPCache[i];
        if (p == NULL)
            break;
        if ((u64) (now - p->detectedTime) > p->maxAge) {
            p->maxAge = 0;
            if (SSDP->callback) {
                SSDP->callback(p->serial, NULL, p->url);
            }
        }
    }
}




static void ySSDP_parseSSPDMessage(SSDPInfos *SSDP, char *message,int msg_len)
{
    int len =0;
    char *p,*start,*lastsep;
    char *location=NULL;
    char *usn=NULL;
    char *cache=NULL;

    if(len>=msg_len){
        return;
    }

    if (memcmp(message,SSDP_HTTP,YSTRLEN(SSDP_HTTP))==0) {
        len=YSTRLEN(SSDP_HTTP);
    } else if (memcmp(message,SSDP_NOTIFY,YSTRLEN(SSDP_NOTIFY))==0) {
        len=YSTRLEN(SSDP_NOTIFY);
    }
    if (len){
        //dbglog("SSDP Message:\n%s\n",message);
        start = p = lastsep= message +len;
        msg_len-=len;
        while( msg_len && *p ){
            switch(*p) {
            case ':':
                if (lastsep == start){
                    lastsep = p;
                }
                break;
            case '\r':
                if (p==start){
                    // \r\n\r\n ->end
                    if(msg_len>1) msg_len=1;
                    break;
                }

                if (lastsep == start){
                    //no : on the line -> drop this message
                    return;
                }
                *lastsep++=0;
                if (*lastsep==' ') lastsep++;
                *p=0;
                if (strcmp(start,"LOCATION")==0){
                    location=lastsep;
                }else if (strcmp(start,"USN")==0){
                    usn=lastsep;
                }else if (strcmp(start,"CACHE-CONTROL")==0){
                    cache=lastsep;
                }
                break;
            case '\n':
                start =lastsep= p+1;
                break;
            }
            p++;
            msg_len--;
        }
        if(location && usn && cache){
            const char *uuid,*urn;
            int         cacheVal;
            //dbglog("SSDP: location: %s %s %s\n\n",location,usn,cache);
            // parse USN
            p=usn;
            // ReSharper disable once CppPossiblyErroneousEmptyStatements
            while (*p && *p++!=':');
            if (!*p) return;
            uuid=p;
            // ReSharper disable once CppPossiblyErroneousEmptyStatements
            while (*p && *p++!=':');
            if (*p != ':') return;
            *(p++-1)=0;
            if (!*p) return;
            urn=p;
            // parse Location
            if(YSTRNCMP(location,"http://",7)==0){
                location += 7;
            }
            p=location;
            while (*p && *p != '/') p++;
            if(*p=='/') *p=0;
            p=cache;
            // ReSharper disable once CppPossiblyErroneousEmptyStatements
            while (*p && *p++!='=');
            if(!*p) return;
            cacheVal = atoi(p);
            if (YSTRCMP(urn,YSSDP_URN_YOCTOPUCE)==0){
                ySSDPUpdateCache(SSDP, uuid, location, cacheVal);
            }
        }
    }
#if 0
    else {
        dbglog("SSDP drop invalid message:\n%s\n",message);
    }
#endif
}



static void* ySSDP_thread(void* ctx)
{
    yThread     *thread=(yThread*)ctx;
    SSDPInfos *SSDP = (SSDPInfos*)thread->ctx;
    fd_set      fds;
    u8          buffer[1536];
    struct timeval timeout;
    int         res, received, i;
    YSOCKET     sktmax;
    yFifoBuf    inFifo;


    yThreadSignalStart(thread);
    yFifoInit(&inFifo,buffer,sizeof(buffer));

    while (!yThreadMustEnd(thread)) {
        memset(&timeout,0,sizeof(timeout));
        timeout.tv_sec = (long)1;
        timeout.tv_usec = (int)0;
        /* wait for data */
        FD_ZERO(&fds);
        sktmax = 0;
        for (i = 0; i < nbDetectedIfaces; i++) {
            FD_SET(SSDP->request_sock[i], &fds);
            if (SSDP->request_sock[i] > sktmax) {
                sktmax = SSDP->request_sock[i];
            }
            if(SSDP->notify_sock[i] != INVALID_SOCKET) {
                FD_SET(SSDP->notify_sock[i], &fds);
                if (SSDP->notify_sock[i] > sktmax) {
                    sktmax = SSDP->notify_sock[i];
                }
            }
        }
        res = select((int)sktmax + 1, &fds, NULL, NULL, &timeout);
        if (res<0) {
    #ifndef WINDOWS_API
            if(SOCK_ERR ==  EAGAIN){
                continue;
            } else
    #endif
            {
                yNetLogErr();
                break;
            }
        }

        if(!yContext) continue;
        ySSDPCheckExpiration(SSDP);
        if (res != 0) {
            for (i = 0; i < nbDetectedIfaces; i++) {
                if (FD_ISSET(SSDP->request_sock[i], &fds)) {
                    received = (int)yrecv(SSDP->request_sock[i], (char*)buffer, sizeof(buffer)-1, 0);
                    if (received>0) {
                        buffer[received] = 0;
                        ySSDP_parseSSPDMessage(SSDP, (char*)buffer, received);
                    }
                }
                if (FD_ISSET(SSDP->notify_sock[i], &fds)) {
                    received = (int)yrecv(SSDP->notify_sock[i], (char *)buffer, sizeof(buffer)-1, 0);
                    if (received > 0) {
                        buffer[received] = 0;
                        ySSDP_parseSSPDMessage(SSDP, (char*)buffer, received);
                    }
                }
            }
        }
    }
    yFifoCleanup(&inFifo);
    yThreadSignalEnd(thread);
    return NULL;
}


int ySSDPDiscover(SSDPInfos *SSDP, char *errmsg)
{
    int sent, len, i;
    struct sockaddr_in sockaddr_dst;

    for (i = 0; i < nbDetectedIfaces; i++) {
        memset(&sockaddr_dst, 0, sizeof(struct sockaddr_in));
        sockaddr_dst.sin_family = AF_INET;
        sockaddr_dst.sin_port = htons(YSSDP_PORT);
        sockaddr_dst.sin_addr.s_addr = inet_addr(YSSDP_MCAST_ADDR_STR);
        len = (int)strlen(discovery);
        sent = (int)sendto(SSDP->request_sock[i], discovery, len, 0, (struct sockaddr *)&sockaddr_dst, sizeof(struct sockaddr_in));
        if (sent < 0) {
            return yNetSetErr();
        }
    }
    return YAPI_SUCCESS;
}


int ySSDPStart(SSDPInfos *SSDP, ssdpHubDiscoveryCallback callback, char *errmsg)
{
    u32     optval;
    int     i;
    socklen_t    socksize;
    struct sockaddr_in     sockaddr;
    struct ip_mreq     mcast_membership;

    if (SSDP->started)
        return YAPI_SUCCESS;

    memset(SSDP, 0, sizeof(SSDPInfos));
    SSDP->callback = callback;
    yDetectNetworkInterfaces(0);

    for (i = 0; i < nbDetectedIfaces; i++) {
        //create M-search socker
        SSDP->request_sock[i] = ysocket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (SSDP->request_sock[i] == INVALID_SOCKET) {
            return yNetSetErr();
        }

        optval = 1;
        setsockopt(SSDP->request_sock[i], SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
#ifdef SO_REUSEPORT
        setsockopt(SSDP->request_sock[i], SOL_SOCKET, SO_REUSEPORT, (char *)&optval, sizeof(optval));
#endif

        // set port to 0 since we accept any port
        socksize = sizeof(sockaddr);
        memset(&sockaddr, 0, socksize);
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = detectedIfaces[i].ip;
        if (bind(SSDP->request_sock[i], (struct sockaddr*) &sockaddr, socksize) < 0) {
            return yNetSetErr();
        }
        //create NOTIFY socker
        SSDP->notify_sock[i] = ysocket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (SSDP->notify_sock[i] == INVALID_SOCKET) {
            return yNetSetErr();
        }

        optval = 1;
        setsockopt(SSDP->notify_sock[i], SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
#ifdef SO_REUSEPORT
        setsockopt(SSDP->notify_sock[i], SOL_SOCKET, SO_REUSEPORT, (char *)&optval, sizeof(optval));
#endif

        // set port to 0 since we accept any port
        socksize = sizeof(sockaddr);
        memset(&sockaddr, 0, socksize);
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(YSSDP_PORT);
        sockaddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(SSDP->notify_sock[i], (struct sockaddr *)&sockaddr, socksize) < 0) {
            return yNetSetErr();
        }

        mcast_membership.imr_multiaddr.s_addr = inet_addr(YSSDP_MCAST_ADDR_STR);
        mcast_membership.imr_interface.s_addr = INADDR_ANY;
        if (setsockopt(SSDP->notify_sock[i], IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mcast_membership, sizeof(mcast_membership)) < 0){
            dbglog("Unable to add multicat membership for SSDP");
            yNetLogErr();
            yclosesocket(SSDP->notify_sock[i]);
            SSDP->notify_sock[i] = INVALID_SOCKET;
        }
    }
    //yThreadCreate will not create a new thread if there is already one running
    if(yThreadCreate(&SSDP->thread,ySSDP_thread,SSDP)<0){
        return YERRMSG(YAPI_IO_ERROR,"Unable to start helper thread");
    }
    SSDP->started++;
    return ySSDPDiscover(SSDP,errmsg);
    //return YAPI_SUCCESS;
}


void  ySSDPStop(SSDPInfos *SSDP)
{
    int i;

    if(yThreadIsRunning(&SSDP->thread)) {
        u64 timeref;
        yThreadRequestEnd(&SSDP->thread);
        timeref = yapiGetTickCount();
        while(yThreadIsRunning(&SSDP->thread) && (yapiGetTickCount() - timeref < 1000) ) {
            yApproximateSleep(10);
        }
        yThreadKill(&SSDP->thread);
    }

    //unregister all detected hubs
    for (i = 0; i<NB_SSDP_CACHE_ENTRY; i++){
        SSDP_CACHE_ENTRY *p = SSDP->SSDPCache[i];
        if(p== NULL)
            continue;
        if (p->maxAge) {
            yapiUnregisterHub(p->url);
            p->maxAge=0;
            if (SSDP->callback)
                SSDP->callback(p->serial, NULL, p->url);
        }
        yFree(p);
    }

    for (i = 0; i < nbDetectedIfaces; i++) {

        if (SSDP->request_sock[i] != INVALID_SOCKET) {
            yclosesocket(SSDP->request_sock[i]);
            SSDP->request_sock[i] = INVALID_SOCKET;
        }
        if (SSDP->notify_sock[i] != INVALID_SOCKET) {
            yclosesocket(SSDP->notify_sock[i]);
            SSDP->notify_sock[i] = INVALID_SOCKET;
        }
    }
    SSDP->started--;
}
