/*********************************************************************
 *
 * $Id: ytcp.h 28224 2017-07-31 14:53:54Z seb $
 *
 *  Declaration of a client TCP stack
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


#ifndef YTCP_H
#define YTCP_H



#ifdef  __cplusplus
extern "C" {
#endif

#include "ydef.h"


#ifdef WINDOWS_API
/**************************************************************
 *
 *  WINDOWS IMPLEMENTATION
 *
 **************************************************************/

//SOCKET RELATED DEFIITIONS AND INCLUDE
#else
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket(s) close(s)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#ifdef MSG_NOSIGNAL
#define SEND_NOSIGPIPE MSG_NOSIGNAL
#else
#define SEND_NOSIGPIPE 0
#endif

#ifdef WINDOWS_API
#define SOCK_ERR    (WSAGetLastError())
#else
#define SOCK_ERR    (errno)
#endif
#define REPORT_ERR(msg) if(errmsg){ YSPRINTF(errmsg,YOCTO_ERRMSG_LEN,"%s (%s:%d / errno=%d)",(msg), __FILE_ID__, __LINE__, SOCK_ERR);errmsg[YOCTO_ERRMSG_LEN-1]='\0';}

#define yNetSetErr()  yNetSetErrEx(__LINE__,SOCK_ERR,errmsg)

int yNetSetErrEx(u32 line,unsigned err,char *errmsg);

#define YTCP_REMOTE_CLOSE 1

struct _HubSt;
struct _RequestSt;

typedef struct {
    YSOCKET listensock;
    YSOCKET signalsock;
} WakeUpSocket;

void yDupSet(char **storage, const char *val);
void yInitWakeUpSocket(WakeUpSocket *wuce);
int  yStartWakeUpSocket(WakeUpSocket *wuce, char *errmsg);
int  yDringWakeUpSocket(WakeUpSocket *wuce, u8 signal, char *errmsg);
int  yConsumeWakeUpSocket(WakeUpSocket *wuce, char *errmsg);
void yFreeWakeUpSocket(WakeUpSocket *wuce);
int yTcpDownload(const char *host, const char *url, u8 **out_buffer, u32 mstimeout, char *errmsg);

int  yTcpInit(char *errmsg);
void yTcpShutdown(void);
u32  yResolveDNS(const char *name,char *errmsg);



struct _RequestSt * yReqAlloc( struct _HubSt *hub);
int  yReqOpen(struct _RequestSt *tcpreq, int wait_for_start, int tcpchan, const char *request, int reqlen, u64 mstimeout, yapiRequestAsyncCallback callback, void *context, yapiRequestProgressCallback progress_cb, void *progress_ctx, char *errmsg);
int  yReqIsAsync(struct _RequestSt *req);
int  yReqSelect(struct _RequestSt *tcpreq, u64 ms, char *errmsg);
int  yReqMultiSelect(struct _RequestSt **tcpreq, int size, u64 ms, WakeUpSocket *wuce, char *errmsg);
int  yReqIsEof(struct _RequestSt *tcpreq, char *errmsg);
int  yReqGet(struct _RequestSt *tcpreq, u8 **buffer);
int  yReqRead(struct _RequestSt *rcoreq, u8 *buffer, int len);
void yReqClose(struct _RequestSt *tcpreq);
void yReqFree(struct _RequestSt *tcpreq);
int  yReqHasPending(struct _HubSt *hub);


void* ws_thread(void* ctx);


#include "ythread.h"

#define OS_IFACE_CAN_MCAST 1

typedef struct {
    u32 flags;
    u32 ip;
    u32 netmask;
} os_ifaces;

#ifdef YAPI_IN_YDEVICE
extern os_ifaces detectedIfaces[];
extern int nbDetectedIfaces;
int yDetectNetworkInterfaces(u32 only_ip);

#endif

#define SSDP_UUID_LEN   48
#define SSDP_URL_LEN    48

typedef struct
{
    char        serial[YOCTO_SERIAL_LEN];
    char        uuid[SSDP_UUID_LEN];
    char        url[SSDP_URL_LEN];
    u64         detectedTime;
    u64         maxAge;
} SSDP_CACHE_ENTRY;


// prototype of the ssdp hub discovery callback
// will be called on discover, refresh, and expiration
typedef void (*ssdpHubDiscoveryCallback)(const char *serial, const char *urlToRegister, const char *urlToUnregister);

#define NB_SSDP_CACHE_ENTRY 32
#define NB_OS_IFACES 8


typedef struct {
	int started;
	ssdpHubDiscoveryCallback callback;
    YSOCKET request_sock[NB_OS_IFACES];
    YSOCKET notify_sock[NB_OS_IFACES];
    yThread thread;
	SSDP_CACHE_ENTRY*   SSDPCache[NB_SSDP_CACHE_ENTRY];
} SSDPInfos;

int 	ySSDPStart(SSDPInfos *SSDP, ssdpHubDiscoveryCallback callback, char *errmsg);
int		ySSDPDiscover(SSDPInfos *SSDP, char *errmsg);
void	ySSDPStop(SSDPInfos *SSDP);

#ifdef  __cplusplus
}
#endif
#endif
