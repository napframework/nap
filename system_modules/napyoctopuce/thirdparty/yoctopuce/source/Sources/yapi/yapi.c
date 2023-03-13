/*********************************************************************
 *
 * $Id: yapi.c 29739 2018-01-25 17:03:29Z seb $
 *
 * Implementation of public entry points to the low-level API
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
#define __FILE_ID__  "yapi"
#include "yapi.h"
#include "yproto.h"
#include "yhash.h"
#include "yjson.h"
#include "yprog.h"

#ifdef WINDOWS_API
#include <time.h>
#else
#include <sys/time.h>
#endif

static YRETCODE  yapiUpdateDeviceList_internal(u32 forceupdate, char *errmsg);
static void  yapiUnregisterHub_internal(const char *url);
static YRETCODE  yapiPreregisterHub_internal(const char *url, char *errmsg);
static YRETCODE  yapiHandleEvents_internal(char *errmsg);
static YRETCODE  yapiHTTPRequestAsyncEx_internal(int tcpchan, const char *device, const char *request, int len, yapiRequestAsyncCallback callback, void *context, char *errmsg);

//#define DEBUG_YAPI_REQ


#ifdef DEBUG_YAPI_REQ

#include <direct.h>
#include <stdio.h>
static int global_req_count = 0;

static int write_onfile(int fileno, char *mode, const char *firstline, int firstline_len, const char * buffer, int bufferlen)
{
    char filename[128];
    FILE *f;
    int retry_count = 1;
    YSPRINTF(filename, 128,"req_trace\\req_%03d.bin", fileno);
    // write on file
retry:
    if (YFOPEN(&f, filename, mode) != 0) {
        if (retry_count--){
            _mkdir("req_trace");
            goto retry;
        }
        return -1;
    }
    fwrite(firstline, 1, firstline_len, f);
    if (buffer) {
        fwrite(buffer, 1, bufferlen, f);
    }
    fclose(f);
    return 0;
}


#define FILE_NO_BIT_MASK 0XFFF

static int YREQ_LOG_START(const char *msg, const char *device, const char * request, int requestsize)
{
    char buffer[2048], buffer2[64];
    struct tm timeinfo;
    time_t rawtime;
    int threadIdx, count, first_len;

    //we may need to add mutex here
    count = global_req_count++;
    int fileno = count & FILE_NO_BIT_MASK;
    // compute time string
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer2, sizeof(buffer2), "%Y-%m-%d %H:%M:%S", &timeinfo);
    //format first info line
    threadIdx = yThreadIndex();
    first_len = YSPRINTF(buffer, 2048, "[%s/%"FMTu64"](%d):no=%d = %s on %s\n",buffer2, yapiGetTickCount(), threadIdx, count, msg, device);
    write_onfile(fileno, "wb", buffer, first_len, request, requestsize);
    return count;
}

static int YREQ_LOG_APPEND(int count, const char *msg, const char * response, int responsesize, u64 start_tm)
{
    char buffer[2048], buffer2[64];
    struct tm timeinfo;
    time_t rawtime;
    int threadIdx, first_len;

    //we may need to add mutex here
    int fileno = count & FILE_NO_BIT_MASK;
    // compute time string
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer2, sizeof(buffer2), "%Y-%m-%d %H:%M:%S", &timeinfo);
    //format first info line
    threadIdx = yThreadIndex();
    first_len = YSPRINTF(buffer, 2048, "[%s/%"FMTu64"](%d):no=%d : %s responsesize=%d\n",buffer2, yapiGetTickCount() - start_tm, threadIdx, count, msg, responsesize);
    // write on file
    write_onfile(fileno, "ab", buffer, first_len, response, responsesize);
    return count;
}

static int YREQ_LOG_APPEND_ERR(int count, const char *msg, const char *errmsg, int error, u64 start_tm)
{
    char buffer[2048], buffer2[64];
    struct tm timeinfo;
    time_t rawtime;
    int threadIdx, first_len;

    //we may need to add mutex here
    int fileno = count & FILE_NO_BIT_MASK;
    // compute time string
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer2, sizeof(buffer2), "%Y-%m-%d %H:%M:%S", &timeinfo);
    //format first info line
    threadIdx = yThreadIndex();
    first_len = YSPRINTF(buffer, 2048, "[%s/%"FMTu64"](%d):no=%d : %s errmsg(%d)=%s\n", buffer2, yapiGetTickCount()- start_tm, threadIdx, count, msg, error, errmsg);
    // write on file
    write_onfile(fileno, "ab", buffer, first_len, NULL, 0);
    return count;
}
#endif


//#define DEBUG_CALLBACK

#ifdef DEBUG_CALLBACK


#include <direct.h>
static int global_callback_count = 0;

static write_cb_onfile(YAPI_FUNCTION fundescr, const char *value)
{
    char filename[128];
    FILE *f;
    int retry_count = 1;
    int fileno = global_callback_count >> 16;
    char *mode ="ab";
    char buffer[2048], buffer2[64];
    struct tm timeinfo;
    time_t rawtime;
    int threadIdx, bufferlen;
    char serial[YOCTO_SERIAL_LEN], funcId[YOCTO_FUNCTION_LEN];

    if ((global_callback_count & 0xffff) == 0) {
        mode = "wb";
    }

    YSPRINTF(filename, 128,"callback_trace\\callback_%d.txt", fileno);
    // compute time string
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer2, sizeof(buffer2), "%Y-%m-%d %H:%M:%S", &timeinfo);
    //format first info line
    threadIdx = yThreadIndex();
    if (value == NULL){
        value = "(NULL)";
    }
    if(ypGetFunctionInfo(fundescr, serial, funcId, NULL, NULL) < 0){
        bufferlen = YSPRINTF(buffer, 2048, "[%s/%"FMTu64"](%d):no=%d : invalid function ID (%X)\n",buffer2, yapiGetTickCount(), threadIdx, global_callback_count, fundescr);
    }else {
        bufferlen = YSPRINTF(buffer, 2048, "[%s/%"FMTu64"](%d):no=%d : %s.%s (%X)=%s\n",buffer2, yapiGetTickCount(), threadIdx, global_callback_count, serial,funcId, fundescr, value);
    }
    global_callback_count++;


    // write on file
retry:
    if (YFOPEN(&f, filename, mode) != 0) {
        if (retry_count--){
            _mkdir("callback_trace");
            goto retry;
        }
        return -1;
    }
    fwrite(buffer, 1, bufferlen, f);
    fclose(f);
    return 0;
}

static write_timedcb_onfile(YAPI_FUNCTION fundescr, double deviceTime, const u8 *report, u32 len)
{
    char buffer[256];
    YSPRINTF(buffer, 2048, "Timed %f %p:%u",deviceTime, report, len);
    return write_cb_onfile(fundescr, buffer);
}

#endif


void yFunctionUpdate(YAPI_FUNCTION fundescr, const char *value)
{
    if(yContext->functionCallback) {
        yEnterCriticalSection(&yContext->functionCallbackCS);
#ifdef DEBUG_CALLBACK
        write_cb_onfile(fundescr, value);
#endif
        yContext->functionCallback(fundescr, value);
        yLeaveCriticalSection(&yContext->functionCallbackCS);
    }
}

void yFunctionTimedUpdate(YAPI_FUNCTION fundescr, double deviceTime, const u8 *report, u32 len)
{
    if(yContext->timedReportCallback) {
        yEnterCriticalSection(&yContext->functionCallbackCS);
#ifdef DEBUG_CALLBACK
        write_timedcb_onfile(fundescr, deviceTime, report, len);
#endif
        yContext->timedReportCallback(fundescr, deviceTime, report, len);
        yLeaveCriticalSection(&yContext->functionCallbackCS);
    }
}


/*****************************************************************************
 Internal functions for hub enumeration
 ****************************************************************************/

typedef enum
{
    ENU_HTTP_START,
    ENU_API,
    ENU_NETWORK_START,
    ENU_NETWORK,
    ENU_NET_ADMINPWD,
    ENU_SERVICE,
    ENU_WP_START,
    ENU_WP_ARRAY,
    ENU_WP_ENTRY,
    ENU_WP_SERIAL,
    ENU_WP_LOGICALNAME,
    ENU_WP_PRODUCTNAME,
    ENU_WP_PRODUCTID,
    ENU_WP_DEVURL,
    ENU_WP_BEACON,
    ENU_WP_INDEX,
    ENU_YP_CONTENT,
    ENU_YP_TYPE,
    ENU_YP_TYPE_LIST,
    ENU_YP_ARRAY,
    ENU_YP_ENTRY,
    ENU_YP_BASETYPE,
    ENU_YP_HARDWAREID,
    ENU_YP_LOGICALNAME,
    ENU_YP_PRODUCTNAME,
    ENU_YP_ADVERTISEDVALUE,
    ENU_YP_INDEX
} ENU_PARSE_STATE;


#ifdef DEBUG_NET_ENUM
const char * ENU_PARSE_STATE_STR[]=
{
    "ENU_HTTP_START",
    "ENU_SERVICE",
    "ENU_WP_START",
    "ENU_WP_ARRAY",
    "ENU_WP_ENTRY",
    "ENU_WP_SERIAL",
    "ENU_WP_LOGICALNAME",
    "ENU_WP_PRODUCTNAME",
    "ENU_WP_PRODUCTID",
    "ENU_WP_DEVURL",
    "ENU_WP_BEACON",
    "ENU_WP_INDEX",
    "ENU_YP_CONTENT",
    "ENU_YP_TYPE",
    "ENU_YP_TYPE_LIST",
    "ENU_YP_ARRAY",
    "ENU_YP_ENTRY",
    "ENU_YP_HARDWAREID",
    "ENU_YP_LOGICALNAME",
    "ENU_YP_PRODUCTNAME",
    "ENU_YP_ADVERTISEDVALUE",
    "ENU_YP_INDEX"
};
#endif

typedef struct {
    HubSt *hub;
    ENU_PARSE_STATE state;
    yStrRef serial;
    yStrRef logicalName;
    union {
        struct {
            yStrRef productName;
            u16     productId;
            yUrlRef hubref;
            s8      beacon;
            u8      devYdx;
        };
        struct{
            yStrRef ypCateg;
            yStrRef funcId;
            char    advertisedValue[YOCTO_PUBVAL_LEN];
            u8      funClass;
            u8      funYdx;
        };
    };
    int     nbKnownDevices;
    yStrRef *knownDevices;
}ENU_CONTEXT;


// return 1 -> if this we should use devUrl instead of registered URL
static int wpSafeCheckOverwrite(yUrlRef registeredUrl, HubSt *hub, yUrlRef devUrl)
{

    yAsbUrlType urlType = yHashGetUrlPort(devUrl, NULL, NULL, NULL, NULL, NULL, NULL);
    yAsbUrlType registeredType;

    if (urlType ==USB_URL){
        // no USB device can unregister previous devices
#ifdef  DEBUG_WP
        dbglog("no USB device can unregister previous devices ( 0x%X) \n",devUrl);
#endif
        return 0;
    }
    registeredType = yHashGetUrlPort(registeredUrl, NULL, NULL, NULL, NULL, NULL, NULL);
    if(registeredType ==USB_URL){
#ifdef DEBUG_WP
        dbglog("unregister same device connected by USB ( 0x%X vs 0x%X) \n",devUrl,hub->url);
#endif
        return 1;
    }else{
        if (registeredUrl !=  devUrl){
            if( devUrl == hub->url){
#ifdef DEBUG_WP
                dbglog("unregister same device connected by a VirtualHub (0x%X vs 0x%X) \n",devUrl,hub->url);
#endif
                return 1;
            }
        }
    }
    return 0;
}



/*****************************************************************************
 * Generic device information stuff
 ***************************************************************************/

void initDevYdxInfos(int devYdx, yStrRef serial)
{
    yGenericDeviceSt *gen = yContext->generic_infos + devYdx;
    yEnterCriticalSection(&yContext->generic_cs);
    memset(gen,0, sizeof(yGenericDeviceSt));
    gen->serial = serial;
    yLeaveCriticalSection(&yContext->generic_cs);
}

void freeDevYdxInfos(int devYdx)
{
    yGenericDeviceSt *gen = yContext->generic_infos + devYdx;
    yEnterCriticalSection(&yContext->generic_cs);
    gen->serial = YSTRREF_EMPTY_STRING;
    yLeaveCriticalSection(&yContext->generic_cs);
}

static void logResult(void *context, const u8 *result, u32 resultlen, int retcode, const char *errmsg)
{
    char buffer[512];
    yGenericDeviceSt *gen = (yGenericDeviceSt*) context;
    int poslen;
    const char * p = (char*) result;
    const char * start = (char*) result;

    if (yContext == NULL || yContext->logDeviceCallback == NULL)
        return;

    if (resultlen < 4) {
        return; //invalid packet
    }

    if (result==NULL || start[0] != 'O' || start[1] != 'K'){
        return; // invalid response
    }
    // drop http header
    while (resultlen >= 4) {
        if (p[0] == '\r' && p[1] == '\n' && p[2] == '\r' && p[3] == '\n'){
            resultlen -= 4;
            p += 4;
            break;
        }
        p++;
        resultlen--;
    }
    start = p;
    //look of '@pos'
    p = start + resultlen - 1;
    poslen = 0;
    while (p > start && *p != '@'){
        if (*p <'0' || *p > '9'){
            poslen = 0;
        } else{
            poslen++;
        }
        p--;
        resultlen--;
    }

    if (*p != '@' ) {
        return;
    }

    memcpy(buffer, p+1, poslen);
    buffer[poslen] = '\0';
    //remove empty line before @pos
    if (resultlen == 0)
        return;
    resultlen-=2;
    p = start;
    yEnterCriticalSection(&yContext->generic_cs);
    gen->deviceLogPos = atoi(buffer);
    yLeaveCriticalSection(&yContext->generic_cs);
    while (resultlen) {
        if (*p == '\n'){
            int linelen = (int)(p - start);
            memcpy(buffer, start, linelen);
            buffer[linelen] = '\0';
            //dbglog("log:[%s]\n", buffer);
            yContext->logDeviceCallback(gen->serial, buffer);
            start = p + 1;
        }
        p++;
        resultlen--;
    }
    yEnterCriticalSection(&yContext->generic_cs);
    gen->flags &= ~(DEVGEN_LOG_PENDING | DEVGEN_LOG_PULLING);
    yLeaveCriticalSection(&yContext->generic_cs);
}

static int yapiRequestOpenWS(YIOHDL_internal *iohdl, HubSt *hub, YAPI_DEVICE dev, int tcpchan, const char *request, int reqlen, u64 mstimeout, yapiRequestAsyncCallback callback, void *context, RequestProgress progress_cb, void *progress_ctx, char *errmsg);
static int yapiRequestOpenHTTP(YIOHDL_internal *iohdl, HubSt *hub, YAPI_DEVICE dev, const char *request, int reqlen, int wait_for_start, u64 mstimeout, yapiRequestAsyncCallback callback, void *context, char *errmsg);
static int yapiRequestOpenUSB(YIOHDL_internal *iohdl, HubSt *hub, YAPI_DEVICE dev, const char *request, int reqlen, u64 unused_timeout, yapiRequestAsyncCallback callback, void *context, char *errmsg);


YRETCODE yapiPullDeviceLogEx(int devydx)
{
    YRETCODE res;
    int     used;
    char    rootdevice[YOCTO_SERIAL_LEN];
    char    request[512];
    int reqlen;
    char    errmsg[YOCTO_ERRMSG_LEN];
    char    *p;
    YAPI_DEVICE dev;
    u32     logPos;
    int     doPull=0;
    yGenericDeviceSt *gen;
    yStrRef serialref;
    YIOHDL_internal iohdl;
    yUrlRef     url;
    yAsbUrlProto proto;
    int i;
    HubSt *hub = NULL;

    yEnterCriticalSection(&yContext->generic_cs);
    gen = yContext->generic_infos + devydx;
    if ( (gen->flags & DEVGEN_LOG_ACTIVATED) &&
         (gen->flags & DEVGEN_LOG_PENDING) &&
         (gen->flags & DEVGEN_LOG_PULLING)==0) {
        doPull=1;
        gen->flags |= DEVGEN_LOG_PULLING;
    }
    logPos = gen->deviceLogPos;
    serialref = gen->serial;
    yLeaveCriticalSection(&yContext->generic_cs);
    if (serialref == YSTRREF_EMPTY_STRING || !doPull) {
        return YAPI_SUCCESS;
    }
    dev = wpSearchEx(serialref);
    YSTRCPY(request, 512, "GET ");
    p= request + 4;
    res = yapiGetDevicePath(dev, rootdevice, p, 512-5, NULL, errmsg);
    if(YISERR(res)) {
        dbglog(errmsg);
        if (res != YAPI_DEVICE_NOT_FOUND) {
            yEnterCriticalSection(&yContext->generic_cs);
            gen->flags &= ~DEVGEN_LOG_PULLING;
            yLeaveCriticalSection(&yContext->generic_cs);
        }
        return res;
    }
    used = YSTRLEN(request);
    p = request + used;
    reqlen = YSPRINTF(p, 512 - used, "logs.txt?pos=%d\r\n\r\n", logPos);
        memset(&iohdl, 0, sizeof(YIOHDL_internal));
    // compute request timeout
    // dispatch request on correct hub (or pseudo usb HUB)
    url = wpGetDeviceUrlRef(dev);

    switch (yHashGetUrlPort(url, NULL, NULL, &proto, NULL, NULL, NULL)) {
    case USB_URL:
        res = yapiRequestOpenUSB(&iohdl, NULL, dev, request, reqlen, YIO_10_MINUTES_TCP_TIMEOUT, logResult, (void*)gen, errmsg);
        break;
    default:
        for (i = 0; i < NBMAX_NET_HUB; i++) {
            if (yContext->nethub[i] && yHashSameHub(yContext->nethub[i]->url, url)) {
                hub = yContext->nethub[i];
                break;
            }
        }
        if (hub == NULL) {
            res = YERR(YAPI_DEVICE_NOT_FOUND);
        } else {
            if (proto == PROTO_WEBSOCKET) {
                res = yapiRequestOpenWS(&iohdl, hub, dev, 0, request, reqlen, YIO_10_MINUTES_TCP_TIMEOUT, logResult, (void*)gen, NULL, NULL, errmsg);
            } else {
               res = yapiRequestOpenHTTP(&iohdl, hub, dev, request, reqlen, 0, YIO_10_MINUTES_TCP_TIMEOUT, logResult, (void*)gen, errmsg);
            }
        }
    }

    if (YISERR(res)) {
        if (res != YAPI_DEVICE_NOT_FOUND && res!= YAPI_DEVICE_BUSY) {
            yEnterCriticalSection(&yContext->generic_cs);
            gen->flags &= ~DEVGEN_LOG_PULLING;
            yLeaveCriticalSection(&yContext->generic_cs);
        }
        return res;
    }
    return res;
}


YRETCODE yapiPullDeviceLog(const char *serial)
{
    YAPI_DEVICE dev;
    int devydx;
    dev = wpSearch(serial);

    devydx = wpGetDevYdx(dev % 0xffff);
    if (devydx<0){
        return YAPI_DEVICE_NOT_FOUND;
    }
    return yapiPullDeviceLogEx(devydx);
}



/*****************************************************************************
  Function:
    void wpSafeRegister( yUrlRef hubUrl, u8 devYdx, yStrRef serialref,yStrRef lnameref, yStrRef productref, u16 deviceid, yUrlRef devUrl,s8 beacon)

  Description:
    Register a new device into whites page (and yellow page of the module function ). This function
    will call the arrivalCallback. This function will work for usb and TCP
    it will check if this arrival should be dropped (if an hub is connected by usb and ip)

  Parameters:
    NetHubSt *hub       : HUB used to access the device (for USB this MUSB be NULL)
    u8 devYdx           : the devYdy relative to the hub (for usb this MUST be MAX_YDX_PER_HUB)
    yStrRef serialref   : the serial of the device
    yStrRef lnameref    : the logical name of the device
    yStrRef productref  : the product name of the device
    u16 deviceid        : the deviceid of the device
    yUrlRef devUrl      : the url of the device (not the one of the hub).
    s8 beacon           : the beacon state
{

 ***************************************************************************/
void wpSafeRegister(HubSt *hub, u8 devYdx, yStrRef serialref,yStrRef lnameref, yStrRef productref, u16 deviceid, yUrlRef devUrl,s8 beacon)
{

    yUrlRef     registeredUrl = wpGetDeviceUrlRef(serialref);
#ifdef DEBUG_WP
    {
        if (hub == NULL){
            dbglog("SAFE WP: register %s(0x%X) form USB\n",yHashGetStrPtr(serialref),serialref);
        } else {
            char host[YOCTO_HOSTNAME_NAME];
            u16  port;
            yAsbUrlType type = yHashGetUrlPort(hub->url,host,&port);
            dbglog("SAFE WP: register %s(0x%X) from %s:%u\n",yHashGetStrPtr(serialref),serialref,host,port);
            dbglog("url    : hub = %d  dev =%d\n",hub->url,devUrl);
        }

    }
    dbglog("name    : %s(0x%X)\n",yHashGetStrPtr(lnameref),lnameref);
    dbglog("product : %s(0x%X)\n",yHashGetStrPtr(productref),productref);
    dbglog("device : %x (%d)\n",deviceid,beacon);
#endif

    if( registeredUrl != INVALID_HASH_IDX && wpSafeCheckOverwrite(registeredUrl,hub,devUrl)){
        wpSafeUnregister(serialref);
    }
    wpRegister(-1, serialref, lnameref, productref, deviceid,devUrl,beacon);
    ypRegister(YSTRREF_MODULE_STRING, serialref, YSTRREF_mODULE_STRING, lnameref, YOCTO_AKA_YFUNCTION, -1, NULL);
    if(hub && devYdx < MAX_YDX_PER_HUB) {
        // Update hub-specific devYdx mapping between enus->devYdx and our wp devYdx
        hub->devYdxMap[devYdx] = wpGetDevYdx(serialref);
    }
    // Forward high-level notification to API user
    if (yContext->arrivalCallback) {
        yEnterCriticalSection(&yContext->deviceCallbackCS);
        yContext->arrivalCallback(serialref);
        yLeaveCriticalSection(&yContext->deviceCallbackCS);
    }
}



/*****************************************************************************
  Function:
    void wpSafeUpdate( yUrlRef hubUrl,u8 devYdx, yStrRef serialref,yStrRef lnameref, yUrlRef devUrl, s8 beacon)

  Description:
    Update whites page (and yellow page of the module function ). This function
    will call the changeCallback if needed. This function will work for usb and TCP
    it will check if this update should be dropped (if an hub is connected by usb and ip)

  Parameters:
    For TCP:
        NetHubSt *hub       : HUB used to access the device (for USB this MUSB be NULL)
        u8 devYdx           : the devYdy relative to the hub (for usb this MUST be MAX_YDX_PER_HUB)
        yStrRef serialref   : the serial of the device
        yStrRef lnameref    : the logical name of the device
        yUrlRef devUrl      : the url of the device (not the one of the hub).
        s8 beacon           : the beacon state

 ***************************************************************************/
void wpSafeUpdate(HubSt *hub,u8 devYdx, yStrRef serialref,yStrRef lnameref, yUrlRef devUrl, s8 beacon)
{

    yUrlRef     registeredUrl =wpGetDeviceUrlRef(serialref);
#ifdef DEBUG_WP
    {
        if (hub == NULL){
            dbglog("SAFE WP: update %s(0x%X) form USB\n",yHashGetStrPtr(serialref),serialref);
        } else {
            char host[YOCTO_HOSTNAME_NAME];
            u16  port;
            yAsbUrlType type = yHashGetUrlPort(hub->url,host,&port);
            dbglog("SAFE WP: update %s(0x%X) from %s:%u\n",yHashGetStrPtr(serialref),serialref,host,port);
            dbglog("url    : hub = %d  dev =%d\n",hub->url,devUrl);
        }
    }
    dbglog("name    : %s(0x%X)\n",yHashGetStrPtr(lnameref),lnameref);
    dbglog("product : INVALID_HASH_IDX\n");
    dbglog("device : %x (%d)\n",0,beacon);
#endif

    if( registeredUrl != INVALID_HASH_IDX && wpSafeCheckOverwrite(registeredUrl,hub,devUrl)){
#ifdef DEBUG_WP
        dbglog("SAFE WP: drop update %s(0x%X)\n",yHashGetStrPtr(serialref),serialref);
#endif
        return;
    }
    if (wpRegister(-1, serialref, lnameref, INVALID_HASH_IDX, 0, devUrl, beacon)) {
        ypRegister(YSTRREF_MODULE_STRING, serialref, YSTRREF_mODULE_STRING, lnameref, YOCTO_AKA_YFUNCTION, -1, NULL);
        if(hub && devYdx < MAX_YDX_PER_HUB) {
            // Update hub-specific devYdx mapping between enus->devYdx and our wp devYdx
            hub->devYdxMap[devYdx] = wpGetDevYdx(serialref);
        }
        // Forward high-level notification to API user
        if(yContext->changeCallback){
            yEnterCriticalSection(&yContext->deviceCallbackCS);
            yContext->changeCallback(serialref);
            yLeaveCriticalSection(&yContext->deviceCallbackCS);
        }
    }
}



void wpSafeUnregister(yStrRef serialref)
{
    wpPreventUnregister();
    if(wpMarkForUnregister(serialref)){
        // Forward high-level notification to API user before deleting data
        if (yContext->removalCallback) {
            yEnterCriticalSection(&yContext->deviceCallbackCS);
            yContext->removalCallback(serialref);
            yLeaveCriticalSection(&yContext->deviceCallbackCS);
        }
    }
    wpAllowUnregister();
}

static void parseNetWpEntry(ENU_CONTEXT *enus)
{
    int i;

    for(i=0; i<enus->nbKnownDevices ; i++){
        if(enus->knownDevices[i] != INVALID_HASH_IDX &&
            enus->knownDevices[i] == enus->serial){
                // mark the device as present (we sweep it later)
#ifdef DEBUG_WP
                dbglog("parseNetWpEntry %X was present\n",enus->serial);
#endif
                enus->knownDevices[i] = INVALID_HASH_IDX;
                break;
        }
    }

    if(i==enus->nbKnownDevices){
        wpSafeRegister(enus->hub,enus->devYdx,enus->serial,enus->logicalName,enus->productName,enus->productId,enus->hubref,enus->beacon);
    } else{
        wpSafeUpdate(enus->hub,enus->devYdx,enus->serial, enus->logicalName, enus->hubref, enus->beacon);
    }
}

static void unregisterNetDevice(yStrRef serialref)
{
    int devydx;

    if(serialref == INVALID_HASH_IDX) return;
    // Free device tcp structure, if needed
    devydx = wpGetDevYdx(serialref);
    if(devydx >= 0 && yContext->tcpreq[devydx]) {
        yReqFree(yContext->tcpreq[devydx]);
        yContext->tcpreq[devydx] = NULL;
    }
    wpSafeUnregister(serialref);
}

static void ypUpdateNet(ENU_CONTEXT *enus)
{
    if(ypRegister(enus->ypCateg, enus->serial, enus->funcId, enus->logicalName, enus->funClass, enus->funYdx, enus->advertisedValue)){
        // Forward high-level notification to API user
        yFunctionUpdate(((s32)enus->funcId << 16) | enus->serial,enus->advertisedValue);
    }
}


static int yEnuJson(ENU_CONTEXT *enus, yJsonStateMachine *j)
{
    char *point;
#ifdef DEBUG_NET_ENUM
    //dbglog("%s: %s %s\n",ENU_PARSE_STATE_STR[enus->state],yJsonStateStr[j->st],j->token);
#endif
    switch(enus->state){
        case ENU_HTTP_START:
            if(j->st != YJSON_HTTP_READ_CODE || YSTRCMP(j->token,"200")){
                return YAPI_IO_ERROR;
            }
            enus->state = ENU_API;
            break;
        case ENU_API:
            if(j->st !=YJSON_PARSE_MEMBNAME)
                break;
            if(YSTRCMP(j->token,"network")==0){
                enus->state = ENU_NETWORK_START;
            } else if(YSTRCMP(j->token,"services")==0){
                enus->state = ENU_SERVICE;
            } else{
                yJsonSkip(j,1);
            }
            break;
        case ENU_NETWORK_START:
            if(j->st == YJSON_PARSE_STRUCT){
                enus->state = ENU_NETWORK;
            }
            break;
        case ENU_NETWORK:
            if(j->st == YJSON_PARSE_STRUCT){
                enus->state = ENU_API;
            } else if(j->st == YJSON_PARSE_MEMBNAME){
                if(YSTRCMP(j->token,"adminPassword")==0){
                    enus->state = ENU_NET_ADMINPWD;
                } else{
                    yJsonSkip(j,1);
                }
            }
            break;
        case ENU_NET_ADMINPWD:
            NETENUMLOG("adminpwd is set to [%s]\n",j->token);
            enus->hub->writeProtected = (j->token[0] != 0);
            enus->state = ENU_NETWORK;
            break;
        case ENU_SERVICE:
            if(j->st !=YJSON_PARSE_MEMBNAME)
                break;
            if(YSTRCMP(j->token,"whitePages")==0){
                enus->state = ENU_WP_START;
            } else if(YSTRCMP(j->token,"yellowPages")==0){
                enus->state = ENU_YP_CONTENT;
            } else{
                yJsonSkip(j,1);
            }
            break;
        case ENU_WP_START:
            if(j->st ==YJSON_PARSE_ARRAY){
                enus->state = ENU_WP_ARRAY;
            }
            break;
        case ENU_WP_ARRAY:
            if(j->st == YJSON_PARSE_STRUCT){
                enus->state     = ENU_WP_ENTRY;
                enus->serial    = INVALID_HASH_IDX;
                enus->logicalName = INVALID_HASH_IDX;
                enus->productName = INVALID_HASH_IDX;
                enus->productId   = 0;
                enus->hubref      = INVALID_HASH_IDX;
                enus->beacon      = 0;
                enus->devYdx      = -1;
            }else  if(j->st ==YJSON_PARSE_ARRAY){
                enus->state=ENU_SERVICE;
            }else{
                NETENUMLOG("Unknown token %s\n",j->token);
            }
            break;
        case ENU_WP_ENTRY:
            if(j->st ==YJSON_PARSE_STRUCT){
                parseNetWpEntry(enus);
                enus->state     = ENU_WP_ARRAY;
            }else if(j->st ==YJSON_PARSE_MEMBNAME){
                if(YSTRCMP(j->token,"serialNumber")==0){
                    enus->state = ENU_WP_SERIAL;
                } else if(YSTRCMP(j->token,"logicalName")==0){
                    enus->state = ENU_WP_LOGICALNAME;
                } else if(YSTRCMP(j->token,"productName")==0){
                    enus->state = ENU_WP_PRODUCTNAME;
                } else if(YSTRCMP(j->token,"productId")==0){
                    enus->state = ENU_WP_PRODUCTID;
                } else  if(YSTRCMP(j->token,"networkUrl")==0){
                    enus->state = ENU_WP_DEVURL;
                } else if(YSTRCMP(j->token,"beacon")==0){
                    enus->state = ENU_WP_BEACON;
                } else if(YSTRCMP(j->token,"index")==0){
                    enus->state = ENU_WP_INDEX;
                } else{
                    yJsonSkip(j,1);
                }
            }
            break;
        case ENU_WP_SERIAL:
            NETENUMLOG("set serial to %s\n",j->token);
            enus->serial = yHashPutStr(j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_WP_LOGICALNAME:
            NETENUMLOG("set Logical name to %s\n",j->token);
            enus->logicalName = yHashPutStr(j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_WP_PRODUCTNAME:
            NETENUMLOG("set Product name to %s\n",j->token);
            enus->productName = yHashPutStr(j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_WP_PRODUCTID:
            NETENUMLOG("set productid to %s\n",j->token);
            enus->productId = atoi(j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_WP_DEVURL:
            NETENUMLOG("set url to %s\n",j->token);
            if (YSTRCMP(j->token, "/api") == 0){
                enus->hub->serial = enus->serial;
            }
            enus->hubref = yHashUrlFromRef(enus->hub->url, j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_WP_BEACON:
            NETENUMLOG("set beacon to %s\n",j->token);
            enus->beacon = atoi(j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_WP_INDEX:
            NETENUMLOG("set devYdx to %s\n",j->token);
            enus->devYdx = atoi(j->token);
            enus->state  = ENU_WP_ENTRY;
            break;
        case ENU_YP_CONTENT:
            if(j->st == YJSON_PARSE_STRUCT){
                enus->state     = ENU_YP_TYPE;
                enus->ypCateg   = INVALID_HASH_IDX;
            }else{
                NETENUMLOG("Unknown token %s\n",j->token);
            }
            break;
        case ENU_YP_TYPE:
            if(j->st ==YJSON_PARSE_STRUCT){
                enus->state     = ENU_SERVICE;
            }else if(j->st ==YJSON_PARSE_MEMBNAME){
                enus->ypCateg = yHashPutStr(j->token);
                NETENUMLOG("Categ = %s\n",j->token);
                enus->state = ENU_YP_TYPE_LIST;
            }else{
                NETENUMLOG("Unknown token %s\n",j->token);
            }
            break;
        case ENU_YP_TYPE_LIST:
            if(j->st ==YJSON_PARSE_ARRAY){
                enus->state = ENU_YP_ARRAY;
            } else if(j->st ==YJSON_PARSE_STRUCT){
                enus->state = ENU_SERVICE;
            }
            break;
        case ENU_YP_ARRAY:
            if(j->st == YJSON_PARSE_STRUCT){
                enus->state     = ENU_YP_ENTRY;
                enus->serial    = INVALID_HASH_IDX;
                enus->logicalName = INVALID_HASH_IDX;
                enus->funcId    = INVALID_HASH_IDX;
                enus->funClass  = YOCTO_AKA_YFUNCTION;
                enus->funYdx    = -1;
                memset(enus->advertisedValue,0, sizeof(enus->advertisedValue));
            }else  if(j->st ==YJSON_PARSE_ARRAY){
                enus->state = ENU_YP_TYPE;
            }else{
                NETENUMLOG("what is it %s \n",j->token);
            }
            break;
        case ENU_YP_ENTRY:
            if(j->st ==YJSON_PARSE_STRUCT){
                ypUpdateNet(enus);
                enus->state     = ENU_YP_ARRAY;
            }else if(j->st ==YJSON_PARSE_MEMBNAME){
                if(YSTRCMP(j->token,"baseType")==0){
                    enus->state = ENU_YP_BASETYPE;
                } else if(YSTRCMP(j->token,"hardwareId")==0){
                    enus->state = ENU_YP_HARDWAREID;
                } else if(YSTRCMP(j->token,"logicalName")==0){
                    enus->state = ENU_YP_LOGICALNAME;
                } else if(YSTRCMP(j->token,"advertisedValue")==0){
                    enus->state = ENU_YP_ADVERTISEDVALUE;
                } else if(YSTRCMP(j->token,"index")==0){
                    enus->state = ENU_YP_INDEX;
                } else {
                    yJsonSkip(j,1);
                }
            }
            break;

        case ENU_YP_BASETYPE:
            NETENUMLOG("set baseType value to %s\n",j->token);
            enus->funClass = atoi(j->token);
            enus->state  = ENU_YP_ENTRY;
            break;
        case ENU_YP_HARDWAREID:
            point = strchr(j->token,'.');
            if(!point) break; //to be safe discart this field if we do not found '.'
            *point++ = '\0';
            NETENUMLOG("set serial  to %s\n",j->token);
            enus->serial = yHashPutStr(j->token);
            NETENUMLOG("set functionid  to %s\n",point);
            enus->funcId = yHashPutStr(point);
            enus->state  = ENU_YP_ENTRY;
            break;
        case ENU_YP_LOGICALNAME:
            NETENUMLOG("set function  to %s\n",j->token);
            enus->logicalName = yHashPutStr(j->token);
            enus->state  = ENU_YP_ENTRY;
            break;
        case ENU_YP_ADVERTISEDVALUE:
            NETENUMLOG("set advertised value to %s\n",j->token);
            YSTRNCPY(enus->advertisedValue,YOCTO_PUBVAL_LEN,j->token,YOCTO_PUBVAL_SIZE);
            enus->state  = ENU_YP_ENTRY;
            break;
        case ENU_YP_INDEX:
            NETENUMLOG("set funYdx value to %s\n",j->token);
            enus->funYdx = atoi(j->token);
            enus->state  = ENU_YP_ENTRY;
            break;
        default:
            return YAPI_IO_ERROR;
    }
    return YAPI_SUCCESS;
}

// connect to a network hub and do an enumeration
// this function will do TCP IO and will do a timeout if
// the hub is off line.
// USE NO NOT USE THIS FUNCTION BUT yNetHubEnum INSTEAD
static int yNetHubEnumEx(HubSt *hub, ENU_CONTEXT *enus, char *errmsg)
{
    yJsonStateMachine j;
    u8              buffer[1500];
    int             res;
    const char      *request = "GET /api.json \r\n\r\n";// no HTTP/1.1 suffix -> light headers
    yJsonRetCode    jstate = YJSON_NEED_INPUT;
    u64             enumTimeout;
    RequestSt        *req;
#ifdef DEBUG_YAPI_REQ
    int req_count = YREQ_LOG_START("yNetHubEnumEx", hub->name, request, YSTRLEN(request));
    u64 start_tm = yapiGetTickCount();
#endif
    req = yReqAlloc(hub);
    if (YISERR((res = yReqOpen(req, 2 * YIO_DEFAULT_TCP_TIMEOUT, 0, request, YSTRLEN(request), YIO_DEFAULT_TCP_TIMEOUT, NULL, NULL, NULL, NULL, errmsg)))) {
        yReqFree(req);
        return res;
    }
    // init yjson parser
    memset(&j,0,sizeof(j));
    j.st = YJSON_HTTP_START;
    enus->state = ENU_HTTP_START;
    enumTimeout = yapiGetTickCount() + 10000;
    while(jstate == YJSON_NEED_INPUT){
        res = yReqSelect(req, 1000, errmsg);
        if(YISERR(res)){
            break;
        }
        res = yReqRead(req, buffer, sizeof(buffer));
        while(res > 0) {
#ifdef DEBUG_YAPI_REQ
            YREQ_LOG_APPEND(req_count, "yNetHubEnumEx", buffer, res, start_tm);
#endif
            j.src = (char*)buffer;
            j.end = (char*)buffer + res;
            // parse all we can on this buffer
            jstate=yJsonParse(&j);
            while(jstate == YJSON_PARSE_AVAIL){
                if(YISERR(yEnuJson(enus,&j))){
                    jstate = YJSON_FAILED;
                    break;
                }
                jstate=yJsonParse(&j);
            }
            res = yReqRead(req, buffer, sizeof(buffer));
        }
        if(res <= 0) {
            res = yReqIsEof(req, errmsg);
            if(YISERR(res)){
                // any specific error during select
                yReqClose(req);
                yReqFree(req);
                return res;
            }
            if(res == 1) {
                // connection close before end of result
                res = YERR(YAPI_IO_ERROR);
            }
            if (yapiGetTickCount()>= enumTimeout){
                res = YERR(YAPI_TIMEOUT);
            }
        }
    }
    yReqClose(req);
    yReqFree(req);

    if( res == YAPI_SUCCESS ){
        switch(jstate){
            case YJSON_NEED_INPUT:
                return YERRMSG(YAPI_IO_ERROR,"Remote host has close the connection");
            case YJSON_PARSE_AVAIL:
            case YJSON_FAILED:
                return YERRMSG(YAPI_IO_ERROR,"Invalid json data");
            case YJSON_SUCCESS:
                break;
        }
    }

    return YAPI_SUCCESS;
}


// helper for yNetHubEnumEx that will trigger TCP connection (and potentially
// timeout) only when it is really needed.
static int yNetHubEnum(HubSt *hub,int forceupdate,char *errmsg)
{
    ENU_CONTEXT     enus;
    int             i, res;
    yStrRef         knownDevices[128];

    //check if the expiration has expired;
    if(!forceupdate && hub->devListExpires > yapiGetTickCount()) {
        return YAPI_SUCCESS;
    }

    // et base url (then entry point)
    memset(&enus,0,sizeof(enus));
    enus.hub = hub;
    enus.knownDevices = knownDevices;
    enus.nbKnownDevices = wpGetAllDevUsingHubUrl(hub->url, enus.knownDevices,  128);
    if(enus.nbKnownDevices >128){
        return YERRMSG(YAPI_IO_ERROR,"too many device on this Net hub");
    }


    if (hub->mandatory) {
        // if the hub is mandatory we will raise an error
        // and not unregister the connected devices
        if (hub->send_ping && hub->state != NET_HUB_ESTABLISHED) {
            // the hub send ping notification -> we can rely on helperthread status
            if (errmsg) {
                YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "hub %s is not reachable", hub->name);
            }
            return YAPI_IO_ERROR;
        } else {
            // the hub does not send ping notification -> we will to a request and potentialy
            // get a tcp timeout if the hub is not reachable
            res = yNetHubEnumEx(hub, &enus, errmsg);
            if (YISERR(res)) {
                return res;
            }
        }
    } else {
        // if the hub is optional we will not triger an error but
        // instead unregister all know device connecte on this hub
        if (hub->state == NET_HUB_ESTABLISHED) {
            // the hub send ping notification -> we can rely on helperthread status
            res = yNetHubEnumEx(hub, &enus, errmsg);
            if (YISERR(res)) {
                dbglog("error with hub %s : %s",hub->name,errmsg);
            }
        }
    }

    for(i=0; i < enus.nbKnownDevices ;  i++){
        if (enus.knownDevices[i]!=INVALID_HASH_IDX){
            unregisterNetDevice(knownDevices[i]);
        }
    }
    if(hub->state == NET_HUB_ESTABLISHED){
        hub->devListExpires = yapiGetTickCount()+10000; // 10s validity when notification are working properly
    } else {
        hub->devListExpires = yapiGetTickCount()+500;
    }
    return YAPI_SUCCESS;
}


// initialize NetHubSt sctructure. no IO in this function
static HubSt* yapiAllocHub(const char  *url,char *errmsg)
{
    char *name;
    int len;
    yHash huburl;
    yStrRef user, password;
    HubSt* hub;

    huburl = yHashUrl(url, "", 0, errmsg);
    if (huburl == INVALID_HASH_IDX) {
        return NULL;
    }

    hub = yMalloc(sizeof(HubSt));
    memset(hub,0,sizeof(HubSt));
    memset(hub->devYdxMap, 255, sizeof(hub->devYdxMap));
    yInitWakeUpSocket(&hub->wuce);
    // compute an hashed url
    hub->url = huburl;
    len = YSTRLEN(url);
    name = (char*) yMalloc(len+1);
    memcpy(name,url,len+1);
    hub->name = name;
    yHashGetUrlPort(huburl, NULL, NULL, &hub->proto, &user, &password, NULL);
    yFifoInit(&(hub->not_fifo), hub->not_buffer, sizeof(hub->not_buffer));
    yInitializeCriticalSection(&hub->access);

    if (hub->proto != PROTO_WEBSOCKET) {
        if (user != INVALID_HASH_IDX) {
            hub->http.s_user = yHashGetStrPtr(user);
        }
        if (password != INVALID_HASH_IDX) {
            hub->http.s_pwd = yHashGetStrPtr(password);
        }
        hub->http.lastTraffic = yapiGetTickCount();
    } else {
        hub->ws.s_next_async_id = 48;
    }
#ifdef TRACE_NET_HUB
    dbglog("HUB%p: %x->%s allocated \n",hub, hub->url, hub->name);
#endif

    return hub;
}

static void yapiFreeHub(HubSt *hub)
{
#ifdef TRACE_NET_HUB
    dbglog("HUB: %x->%s Deleted \n",hub->url,hub->name);
#endif
    yFreeWakeUpSocket(&hub->wuce);
    if (hub->proto == !PROTO_WEBSOCKET) {
        if (hub->http.s_realm)  yFree(hub->http.s_realm);
        if (hub->http.s_nonce)  yFree(hub->http.s_nonce);
        if (hub->http.s_opaque) yFree(hub->http.s_opaque);
        if (hub->http.notReq) {
            yReqClose(hub->http.notReq);
            yReqFree(hub->http.notReq);
        }
    }
    yDeleteCriticalSection(&hub->access);
    yFifoCleanup(&hub->not_fifo);
    if (hub->name)   yFree(hub->name);
    memset(hub, 0, sizeof(HubSt));
    memset(hub->devYdxMap, 255, sizeof(hub->devYdxMap));
    hub->url = INVALID_HASH_IDX;
    yFree(hub);
}



static void unregisterNetHub(yUrlRef  huburl)
{
    int i;
    u64     timeref;
    int      nbKnownDevices;
    yStrRef  knownDevices[128];
    char     errmsg[YOCTO_ERRMSG_LEN];


    for(i = 0; i < NBMAX_NET_HUB; i++){
        HubSt *hub = yContext->nethub[i];
        if(hub && yHashSameHub(hub->url, huburl)) {
#ifdef TRACE_NET_HUB
            dbglog("HUB: unregister %x->%s  \n",huburl,hub->name);
#endif
            hub->state = NET_HUB_TOCLOSE;
            yThreadRequestEnd(&hub->net_thread);
            yDringWakeUpSocket(&hub->wuce, 0, errmsg);
            // wait for the helper thread to stop monitoring these devices
            timeref = yapiGetTickCount();
            while(yThreadIsRunning(&hub->net_thread) && (yapiGetTickCount() - timeref < YIO_DEFAULT_TCP_TIMEOUT) ) {
                yApproximateSleep(10);
            }
            yThreadKill(&hub->net_thread);
            yapiFreeHub(hub);
            yContext->nethub[i] = NULL;
            break;
        }
    }

    nbKnownDevices = wpGetAllDevUsingHubUrl(huburl,knownDevices,128);
    for(i = 0 ; i < nbKnownDevices; i++) {
        if (knownDevices[i]!=INVALID_HASH_IDX){
            unregisterNetDevice(knownDevices[i]);
        }
    }

}



static void ssdpEntryUpdate(const char *serial, const char *urlToRegister, const char *urlToUnregister)
{
    if (!yContext)
        // API not yet initialized -> drop everthing
        return;
    if (urlToRegister){
        // still valid entry
        if (yContext->hubDiscoveryCallback) {
            yEnterCriticalSection(&yContext->deviceCallbackCS);
            yContext->hubDiscoveryCallback(serial, urlToRegister);
            yLeaveCriticalSection(&yContext->deviceCallbackCS);
        }
    }

    if (yContext->detecttype & Y_DETECT_NET) {
        if (urlToRegister) {
            if (urlToUnregister) {
                yapiUnregisterHub_internal(urlToUnregister);
            }
            yapiPreregisterHub_internal(urlToRegister, NULL);
        }
    }
}


static void initializeAllCS(yContextSt *ctx)
{
        //initialize enumeration CS
    yInitializeCriticalSection(&ctx->updateDev_cs);
    yInitializeCriticalSection(&ctx->handleEv_cs);
    yInitializeCriticalSection(&ctx->enum_cs);
    yInitializeCriticalSection(&ctx->io_cs);
    yInitializeCriticalSection(&ctx->deviceCallbackCS);
    yInitializeCriticalSection(&ctx->functionCallbackCS);
    yInitializeCriticalSection(&ctx->generic_cs);
}

static void deleteAllCS(yContextSt *ctx)
{
        //initialize enumeration CS
    yDeleteCriticalSection(&ctx->updateDev_cs);
    yDeleteCriticalSection(&ctx->handleEv_cs);
    yDeleteCriticalSection(&ctx->enum_cs);
    yDeleteCriticalSection(&ctx->io_cs);
    yDeleteCriticalSection(&ctx->deviceCallbackCS);
    yDeleteCriticalSection(&ctx->functionCallbackCS);
    yDeleteCriticalSection(&ctx->generic_cs);
}


/*****************************************************************************
  API FUNCTIONS
 ****************************************************************************/
#pragma pack(push,1)
typedef union{
    u32 raw;
    struct{
        u8  a;
        u8  b;
        u8  c;
        u8  d;
    } bytes;
} test_compile;
#pragma pack(pop)


static YRETCODE yapiInitAPI_internal(int detect_type,char *errmsg)
{
    test_compile test;
    yContextSt *ctx;
#ifdef PERF_API_FUNCTIONS
    memset(&yApiPerf,0,sizeof(yApiPerf));
#endif

    if(yContext!=NULL)
        return YERRMSG(YAPI_DEVICE_BUSY,"Api already started");
#ifdef __BORLANDC__
#pragma warn - 8066
#pragma warn - 8008
#endif
    if (sizeof(u8) != 1) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of u8");
    if (sizeof(s8) != 1) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of s8");
    if (sizeof(u16) != 2) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of u16");
    if (sizeof(u32) != 4) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of u32");
    if (sizeof(u64) != 8) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of u64");
    if (sizeof(s16) != 2) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of s16");
    if (sizeof(s32) != 4) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of s32");
    if (sizeof(s64) != 8) return YERRMSG(YAPI_INVALID_ARGUMENT,"invalid definition of s64");
    test.raw = 0xdeadbeef;

    if (test.bytes.a == 0xef && test.bytes.d == 0xde) {
        // little endian
        if (sizeof(test_compile) != 4) return YERRMSG(YAPI_INVALID_ARGUMENT, "pragma pack is not supported");
#ifdef CPU_BIG_ENDIAN
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid endianness. Lib is compiled for big endian but is used on little endian cpu");
#endif
    } else {
        // big endian
        if (sizeof(test_compile) != 4) return YERRMSG(YAPI_INVALID_ARGUMENT, "pragma pack is not supported");
#ifndef CPU_BIG_ENDIAN
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid endianness. Lib is compiled for little endian but is used on big endian cpu");
#endif
    }

#ifdef __BORLANDC__
#pragma warn . 8008
#pragma warn . 8066
#endif

    if(atof("1") != 1.0){
#if defined(BUILD_ARMHF)
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Invalid arm architecture (try armel binaries)");
#elif defined(BUILD_ARMEL)
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Invalid arm architecture (try armhf binaries)");
#else
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Invalid architecture");
#endif
    }

    //init safe malloc
    ySafeMemoryInit(64*1024);
#ifdef DEBUG_CRITICAL_SECTION
    yInitDebugCS();
#endif

    ctx=(yContextSt*)yMalloc(sizeof(yContextSt));
    yMemset(ctx,0,sizeof(yContextSt));
    ctx->detecttype=detect_type;

    //initialize enumeration CS
    initializeAllCS(ctx);

    //initialize device pool
    ctx->devs = NULL;
    ctx->devhdlcount = 1;
    if(detect_type & Y_DETECT_USB) {
        int res;
        if(YISERR(res=yUsbInit(ctx,errmsg))){
            deleteAllCS(ctx);
            yFree(ctx);
            return (YRETCODE)res;
        }
    }


    //initialize white/yellow pages support
    yHashInit();

    if (YISERR(yTcpInit(errmsg))){
        deleteAllCS(ctx);
        yFree(ctx);
        return YAPI_IO_ERROR;
    }

    yCreateEvent(&ctx->exitSleepEvent);

    if(detect_type & Y_DETECT_NET) {
        if (YISERR(ySSDPStart(&ctx->SSDP, ssdpEntryUpdate, errmsg))){
            yTcpShutdown();
            yCloseEvent(&yContext->exitSleepEvent);
            deleteAllCS(ctx);
            yFree(ctx);
            return YAPI_IO_ERROR;
        }
    }
    yContext=ctx;
#ifndef YAPI_IN_YDEVICE
    yProgInit();
#endif
    return YAPI_SUCCESS;
}

static int yTcpTrafficPending(void);


static void yapiFreeAPI_internal(void)
{
    u64             timeout = yapiGetTickCount() + 1000000;
    char            errmsg[YOCTO_ERRMSG_LEN];
    int             i;

    if(yContext==NULL)
        return;

#ifdef PERF_API_FUNCTIONS
    dumpYApiPerf();
#endif


    while(yapiGetTickCount() < timeout && (yUsbTrafficPending() || yTcpTrafficPending())) {
        yapiHandleEvents_internal(errmsg);
        yApproximateSleep(50);
    }


#ifndef YAPI_IN_YDEVICE
    yProgFree();
#endif
    yEnterCriticalSection(&yContext->updateDev_cs);
    yEnterCriticalSection(&yContext->handleEv_cs);
    yEnterCriticalSection(&yContext->enum_cs);
    if(yContext->detecttype & Y_DETECT_USB) {
        yUsbFree(yContext,NULL);
    }

     ySSDPStop(&yContext->SSDP);
    //unregister all Network hub
    for(i = 0; i < NBMAX_NET_HUB; i++){
        if (yContext->nethub[i]) {
            unregisterNetHub(yContext->nethub[i]->url);
        }
    }

    yHashFree();
    yTcpShutdown();
    yCloseEvent(&yContext->exitSleepEvent);

    yLeaveCriticalSection(&yContext->updateDev_cs);
    yLeaveCriticalSection(&yContext->handleEv_cs);
    yLeaveCriticalSection(&yContext->enum_cs);
    deleteAllCS(yContext);
    ySafeMemoryDump(yContext);
    yFree(yContext);
    ySafeMemoryStop();
#ifdef DEBUG_CRITICAL_SECTION
    yFreeDebugCS();
#endif

    yContext=NULL;
}


static void yapiRegisterLogFunction_internal(yapiLogFunction logfun)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yEnterCriticalSection(&yContext->enum_cs);
        yContext->log = logfun;
        yLeaveCriticalSection(&yContext->enum_cs);
    }
}


static void yapiRegisterDeviceLogCallback_internal(yapiDeviceLogCallback logCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yEnterCriticalSection(&yContext->enum_cs);
        yContext->logDeviceCallback = logCallback;
        yLeaveCriticalSection(&yContext->enum_cs);
    }
}

static void yapiStartStopDeviceLogCallback_internal(const char *serial,int start)
{
    yStrRef serialref;
    int devydx;
    serialref = yHashPutStr(serial);
    devydx = wpGetDevYdx(serialref);
    if (devydx < 0 )
        return;
    dbglog("activate log %s %d\n", serial, start);
    yEnterCriticalSection(&yContext->generic_cs);
    if (start) {
        yContext->generic_infos[devydx].flags |= DEVGEN_LOG_ACTIVATED;
    } else {
        yContext->generic_infos[devydx].flags &= ~DEVGEN_LOG_ACTIVATED;
    }
    yLeaveCriticalSection(&yContext->generic_cs);
    yapiPullDeviceLogEx(devydx);
}

static void  yapiRegisterDeviceArrivalCallback_internal(yapiDeviceUpdateCallback arrivalCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yEnterCriticalSection(&yContext->enum_cs);
        yContext->arrivalCallback = arrivalCallback;
        if(arrivalCallback != NULL){
            // FIXME: WE SHOULD USE THE hash table to list all known devices
#if 0
            // call callback with already detected devices
            yPrivDeviceSt *p=yContext->devs;
            while(p){
                devGetAcces(p,1);
                if(p->dstatus == YDEV_WORKING){
                    yStrRef serialref = yHashPutStr(p->infos.serial);
                    yContext->arrivalCallback(serialref);
                }
                devReleaseAcces(p);
                p=p->next;
            }
#endif
        }
        yLeaveCriticalSection(&yContext->enum_cs);
    }
}


static void  yapiRegisterDeviceRemovalCallback_internal(yapiDeviceUpdateCallback removalCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yEnterCriticalSection(&yContext->enum_cs);
        yContext->removalCallback = removalCallback;
        yLeaveCriticalSection(&yContext->enum_cs);
    }
}


static void  yapiRegisterDeviceChangeCallback_internal(yapiDeviceUpdateCallback changeCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yContext->changeCallback = changeCallback;
    }
}


static void  yapiRegisterFunctionUpdateCallback_internal(yapiFunctionUpdateCallback updateCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yContext->functionCallback = updateCallback;
    }
}

static void  yapiRegisterTimedReportCallback_internal(yapiTimedReportCallback timedReportCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if(!yContext) {
        yapiInitAPI_internal(0,errmsg);
    }
    if(yContext) {
        yContext->timedReportCallback = timedReportCallback;
    }
}



#ifdef DEBUG_NET_NOTIFICATION
static void dumpNotif(const char *buffer)
{
    FILE *f;
    printf("%s",buffer);
    YASSERT(YFOPEN(&f,"c:\\tmp\\api_not.txt","ab")==0);
    fwrite(buffer,1,YSTRLEN(buffer),f);
    fclose(f);
}
#endif


static void wpUpdateTCP(HubSt *hub, const char *serial, const char *name, u8 beacon)
{
    #define LOCALURL_LEN 64
    int status;
    char devUrlBuf[LOCALURL_LEN];
    yStrRef serialref;
    yStrRef lnameref;
    yUrlRef devurl;
    int devydx;

    serialref = yHashPutStr(serial);
    devydx = wpGetDevYdx(serialref);

    if (devydx < 0) {
        // drop notification until we have register the device with
        // a real enumeration
        return;
    }

    if (hub->serial != serialref) {
        // Insert device into white pages
        YSTRCPY(devUrlBuf, LOCALURL_LEN, "/bySerial/");
        YSTRCAT(devUrlBuf, LOCALURL_LEN, serial);
        YSTRCAT(devUrlBuf, LOCALURL_LEN, "/api");
        devurl = yHashUrlFromRef(hub->url, devUrlBuf);
    } else {
        devurl = hub->url;
    }
    lnameref    = yHashPutStr(name);
    status = wpRegister(-1, serialref, lnameref, INVALID_HASH_IDX, 0, devurl, beacon);
    if (status == 0) {
        return; // no change
    }
    ypRegister(YSTRREF_MODULE_STRING, serialref, YSTRREF_mODULE_STRING, lnameref, YOCTO_AKA_YFUNCTION, -1, NULL);
    // Forward high-level notification to API user
    if(yContext->changeCallback){
        yEnterCriticalSection(&yContext->deviceCallbackCS);
        yContext->changeCallback(serialref);
        yLeaveCriticalSection(&yContext->deviceCallbackCS);
    }
}

int handleNetNotification(HubSt *hub)
{
    u16             pos;
    u16             end,size;
    char            buffer[128];
    char            *p;
    u8              pkttype = 0,devydx,funydx,funclass;
    char            *serial = NULL,*name,*funcid,*children;
    char            value[YOCTO_PUBVAL_LEN];
    u8              report[18];
    char            netstop=NOTIFY_NETPKT_STOP;
    char            escapechar = 27;
#ifdef DEBUG_NET_NOTIFICATION
    u32             abspos = hub->notifAbsPos;
    char            Dbuffer[1024];
    u8              throwbuf[1024];
    u16             tmp;
#endif

    // search for start of notification
    size = yFifoGetUsed(&(hub->not_fifo));
    while(size >= NOTIFY_NETPKT_START_LEN) {
        yPeekFifo(&(hub->not_fifo), &pkttype, 1, 0);
        if(pkttype != NOTIFY_NETPKT_STOP) break;
        // drop newline and loop
        yPopFifo(&(hub->not_fifo),NULL,1);
        // note: keep-alive packets don't count in the notification channel position
        size--;
    }
    if(size < NOTIFY_NETPKT_START_LEN) {
        return 0;
    }
    // make sure we have a full notification
    end = ySeekFifo(&(hub->not_fifo), (u8*) &netstop, 1, 0, 0, 0);
    if(end == 0xffff){
        if (yFifoGetFree(&(hub->not_fifo)) == 0) {
            dbglog("Too many invalid notifications, clearing buffer\n");
            yFifoEmpty((&(hub->not_fifo)));
            return 1;
        }
        return 0;
    }
    // make sure we have a full notification
    if (0xffff != ySeekFifo(&(hub->not_fifo), (u8*) &escapechar, 1, 0, end, 0)) {
        // drop notification that contain esc char
        yPopFifo(&(hub->not_fifo), NULL, end + 1);
        return 1;
    }
    // handle short funcvalydx notifications
    if(pkttype >= NOTIFY_NETPKT_FLUSHV2YDX && pkttype <= NOTIFY_NETPKT_TIMEAVGYDX) {
        memset(value, 0, YOCTO_PUBVAL_LEN);
        if (end + 1 > (u16) sizeof(buffer)){
            dbglog("Drop invalid short notification (too long :%d)\n", end + 1);
            hub->notifAbsPos += end + 1;
            return 1;
        }
        yPopFifo(&(hub->not_fifo),(u8*) buffer,end+1);
        hub->notifAbsPos += end+1;
        p = buffer+1;
        devydx = (*p++) - 'A';
        funydx = (*p++) - '0';
        if(funydx & 64) { // high bit of devydx is on second character
            funydx -= 64;
            devydx += 128;
        }
        pos = 0;
        switch (pkttype) {
            case NOTIFY_NETPKT_FUNCVALYDX:
                while(*p != 0 && *p != NOTIFY_NETPKT_STOP && pos < YOCTO_PUBVAL_LEN-1) {
                    value[pos++] = *p++;
                }
                value[pos] = 0;
#ifdef DEBUG_NET_NOTIFICATION
                YSPRINTF(Dbuffer,512,"FuncVYDX >devYdx=%d funYdx=%d val=%s (%d)\n",
                         devydx,funydx,value,abspos);
                dumpNotif(Dbuffer);
#endif
                // Map hub-specific devydx to our devydx
                devydx = hub->devYdxMap[devydx];
                if(devydx < MAX_YDX_PER_HUB) {
                    Notification_funydx funInfo;
                    funInfo.raw = funydx;
                    ypUpdateYdx(devydx,funInfo,value);
                }
                break;
            case NOTIFY_NETPKT_DEVLOGYDX:
                // Map hub-specific devydx to our devydx
                devydx = hub->devYdxMap[devydx];
                if(devydx < MAX_YDX_PER_HUB) {
                    yEnterCriticalSection(&yContext->generic_cs);
                    if (yContext->generic_infos[devydx].flags & DEVGEN_LOG_ACTIVATED) {
                        yContext->generic_infos[devydx].flags |= DEVGEN_LOG_PENDING;
#ifdef DEBUG_NET_NOTIFICATION
                        dbglog("notify device log for devydx %d\n", devydx);
#endif
                    }
                    yLeaveCriticalSection(&yContext->generic_cs);
                }
                break;
            case NOTIFY_NETPKT_TIMEVALYDX:
            case NOTIFY_NETPKT_TIMEAVGYDX:
            case NOTIFY_NETPKT_TIMEV2YDX:
                // Map hub-specific devydx to our devydx
                devydx = hub->devYdxMap[devydx];
                if(devydx >= MAX_YDX_PER_HUB) break;

                report[pos++] = (pkttype == NOTIFY_NETPKT_TIMEVALYDX ? 0 :
                                 (pkttype == NOTIFY_NETPKT_TIMEAVGYDX ? 1 : 2));
                while (isxdigit((u8)p[0]) && isxdigit((u8)p[1]) && pos < sizeof(report)) {
                    int hi_nib = (p[0] <= '9' ? p[0]-'0' : 10+(p[0]&0x4f)-'A');
                    int lo_nib = (p[1] <= '9' ? p[1]-'0' : 10+(p[1]&0x4f)-'A');
                    report[pos++] = hi_nib * 16 + lo_nib;
                    p += 2;
                }
    #ifdef DEBUG_NET_NOTIFICATION
                YSPRINTF(Dbuffer,512,"%s >devYdx=%d funYdx=%d (%d)\n",
                         (pkttype == NOTIFY_NETPKT_TIMEVALYDX ? "TimeValR" : "TimeAvgR"),
                         devydx,funydx,abspos);
                dumpNotif(Dbuffer);
    #endif
                if(funydx == 15) {
                    u32 t = report[1] + 0x100u * report[2] + 0x10000u * report[3] + 0x1000000u * report[4];
                    yEnterCriticalSection(&yContext->generic_cs);
                    yContext->generic_infos[devydx].deviceTime = (double)t + report[5] / 250.0;
                    yLeaveCriticalSection(&yContext->generic_cs);
                } else {
                    Notification_funydx funInfo;
                    YAPI_FUNCTION fundesc;
                    double deviceTime;
                    yEnterCriticalSection(&yContext->generic_cs);
                    deviceTime = yContext->generic_infos[devydx].deviceTime;
                    yLeaveCriticalSection(&yContext->generic_cs);
                    funInfo.raw = funydx;
                    ypRegisterByYdx(devydx, funInfo, NULL, &fundesc);
                    yFunctionTimedUpdate(fundesc, deviceTime, report, pos);
                }
                break;
            case NOTIFY_NETPKT_FUNCV2YDX:
                while(*p != 0 && *p != NOTIFY_NETPKT_STOP && pos < YOCTO_PUBVAL_LEN-1) {
                    value[pos++] = *p++;
                }
                value[pos] = 0;
                // Map hub-specific devydx to our devydx
                devydx = hub->devYdxMap[devydx];
                if(devydx < MAX_YDX_PER_HUB) {
                    Notification_funydx funInfo;
                    unsigned char value8bit[YOCTO_PUBVAL_LEN];
                    memset(value8bit, 0, YOCTO_PUBVAL_LEN);
                    funInfo.raw = funydx;
                    if(decodeNetFuncValV2((u8*)value, &funInfo, (char*)value8bit) >= 0) {
#ifdef DEBUG_NET_NOTIFICATION
                        if(!funInfo.v2.typeV2) {
                            YSPRINTF(Dbuffer,512,"FuncV2YDX >devYdx=%d funYdx=%d val=%s (%d)\n",devydx,funydx,value8bit,abspos);
                        } else {
                            YSPRINTF(Dbuffer,512,"FuncV2YDX >devYdx=%d funYdx=%d val=%d:%02x.%02x.%02x.%02x.%02x.%02x (%d)\n",
                                     devydx, funInfo.v2.funydx, funInfo.v2.typeV2,
                                     value8bit[0],value8bit[1],value8bit[2],value8bit[3],value8bit[4],value8bit[5],abspos);
                        }
                        dumpNotif(Dbuffer);
#endif
                        ypUpdateYdx(devydx,funInfo,(char *)value8bit);
                    }
                }
                break;
            case NOTIFY_NETPKT_FLUSHV2YDX:
                // To be implemented later
                break;
            default:
                break;
        }
        return 1;
    }

    // make sure packet is a valid notification
    pos = ySeekFifo(&(hub->not_fifo), (u8*) (NOTIFY_NETPKT_START), NOTIFY_NETPKT_START_LEN, 0, end, 0);
    if(pos != 0) {
        // does not start with signature, drop everything until stop marker
#ifdef DEBUG_NET_NOTIFICATION
        memset(throwbuf, 0, sizeof(throwbuf));
        tmp = (end > 50 ? 50 : end);
        yPopFifo(&(hub->not_fifo),throwbuf,tmp);
        yPopFifo(&(hub->not_fifo),NULL,end+1-tmp);
        Dbuffer[1023]=0;
        YSPRINTF(Dbuffer,512,"throw %d / %d [%s]\n",
                 end,pos,throwbuf);
        dumpNotif(Dbuffer);
#else
        yPopFifo(&(hub->not_fifo),NULL,end+1);
#endif
        hub->notifAbsPos += end+1;
        return 0;
    }

    // full packet at start of fifo
    size = end - NOTIFY_NETPKT_START_LEN;
    YASSERT(NOTIFY_NETPKT_MAX_LEN > size);
    yPopFifo(&(hub->not_fifo),NULL,NOTIFY_NETPKT_START_LEN);
    yPopFifo(&(hub->not_fifo),(u8*) buffer,size+1);
    buffer[size]=0;
    pkttype = *buffer;
    p = buffer+1;
    if(pkttype == NOTIFY_NETPKT_NOT_SYNC) {
        int testPing;
#ifdef DEBUG_NET_NOTIFICATION
        YSPRINTF(Dbuffer,512,"Sync from %d to %s\n",
             hub->notifAbsPos, p);
        dumpNotif(Dbuffer);
#endif
        hub->notifAbsPos = atoi(p);
        //look if we have a \n just after the sync notification
        // if yes this mean that the hub will send some ping notification
        testPing = ySeekFifo(&(hub->not_fifo), (u8*) &netstop, 1, 0, 1, 0);
        if(testPing == 0){
#ifdef DEBUG_NET_NOTIFICATION
            YSPRINTF(Dbuffer,1024,"HUB: %X->%s will send ping notification\n",hub->url,hub->name);
            dumpNotif(Dbuffer);
#endif
            hub->send_ping = 1;
        }
        return 1;
    }
    hub->notifAbsPos += size+1+NOTIFY_NETPKT_START_LEN;
    if(pkttype != NOTIFY_NETPKT_FUNCVALYDX) {
        serial = p;
        p = strchr(serial,NOTIFY_NETPKT_SEP);
        if(p==NULL) {
#ifdef DEBUG_NET_NOTIFICATION
            YSPRINTF(Dbuffer,512,"no serialFOR %s\n",buffer);
            dumpNotif(Dbuffer);
#endif
            return 0;
        }
        *p++ = 0;
    }


    switch(pkttype){
        case NOTIFY_NETPKT_NAME:
            name = p;
            p = strchr(name,NOTIFY_NETPKT_SEP);
            if(p==NULL){
#ifdef DEBUG_NET_NOTIFICATION
                dbglog("drop: invalid new name (%X)\n",pkttype);
#endif
                return 1;
            }
            *p++ = 0;
#ifdef DEBUG_NET_NOTIFICATION
            dbglog("NOTIFY_PKT_NAME %s : new name is \"%s\" beacon %x\n",serial,name ,*p);
            YSPRINTF(Dbuffer,512,"NAME     >%s  name=%s beacon=%c (%d)\n",serial,name,*p,abspos);
            dumpNotif(Dbuffer);
#endif
            wpUpdateTCP(hub,serial,name,(*p== '1' ? 1:0));
            break;
        case NOTIFY_NETPKT_FUNCNAME:
            funcid = p;
            p = strchr(funcid,NOTIFY_NETPKT_SEP);
            if(p==NULL){
#ifdef DEBUG_NET_NOTIFICATION
                dbglog("drop: invalid funcid (%X:%s)\n",pkttype,serial);
#endif
                return 1;
            }
            *p++ = 0;
            name = p;
#ifdef DEBUG_NET_NOTIFICATION
            dbglog("NOTIFY_PKT_FUNCNAME %s : funcid is \"%s\" name \"%s\"\n",serial,funcid,name);
            YSPRINTF(Dbuffer,512,"FuncNAME >%s  funcid=%s funcname=%s (%d)\n",
                     serial,funcid,name,abspos);
            dumpNotif(Dbuffer);
#endif
            ypUpdateUSB(serial,funcid,name,-1,-1,NULL);
            break;
        case NOTIFY_NETPKT_FUNCVAL:
            funcid = p;
            p = strchr(funcid,NOTIFY_NETPKT_SEP);
            if(p==NULL){
#ifdef DEBUG_NET_NOTIFICATION
                dbglog("drop: invalid funcid (%X)\n",pkttype);
#endif
                return 1;
            }
            *p++ = 0;
            memset(value,0,YOCTO_PUBVAL_LEN);
            memcpy(value,p,YSTRLEN(p));
#ifdef DEBUG_NET_NOTIFICATION
            //dbglog("NOTIFY_PKT_FUNCVAL %s : funcid is \"%s\" value \"%s\"\n",serial,funcid,value);
            YSPRINTF(Dbuffer,512,"FuncVAL  >%s  funcid=%s val=%s (%d)\n",
                serial,funcid,value,abspos);
            dumpNotif(Dbuffer);
#endif
            ypUpdateUSB(serial,funcid,NULL,-1,-1,value);
            break;
        case NOTIFY_NETPKT_FUNCNAMEYDX:
            funcid = p;
            p = strchr(funcid,NOTIFY_NETPKT_SEP);
            if(p==NULL){
#ifdef DEBUG_NET_NOTIFICATION
                dbglog("drop: invalid funcid (%X:%s)\n",pkttype,serial);
#endif
                return 1;
            }
            *p++ = 0;
            name = p;
            p = strchr(name,NOTIFY_NETPKT_SEP);
            if(p==NULL){
#ifdef DEBUG_NET_NOTIFICATION
                dbglog("drop: invalid funcname (%X:%s)\n",pkttype,serial);
#endif
                return 1;
            }
            *p++ = 0;
            funydx = atoi(p);
            p = strchr(p,NOTIFY_NETPKT_SEP);
            if(p==NULL || p[1] < '0'){
                funclass = YOCTO_AKA_YFUNCTION;
            } else {
                funclass = p[1]-'0';
            }
#ifdef DEBUG_NET_NOTIFICATION
            YSPRINTF(Dbuffer,512,"FuncNYDX >%s  funcid=%s funcname=%s funYdx=%d (%d)\n",
                     serial,funcid,name,funydx,abspos);
            dumpNotif(Dbuffer);
#endif
            ypUpdateUSB(serial,funcid,name,funclass,funydx,NULL);
            break;
        case NOTIFY_NETPKT_CHILD:
            children = p;
            p = strchr(children,NOTIFY_NETPKT_SEP);
            if(p==NULL){
#ifdef DEBUG_NET_NOTIFICATION
                dbglog("drop: invalid children notification (%X)\n",pkttype);
#endif
                return 1;
            }
            *p++ = 0;
#ifdef DEBUG_NET_NOTIFICATION
            dbglog("NOTIFY_PKT_CHILDREN %s : new children is \"%s\" plug %x\n",serial,children ,*p);
            YSPRINTF(Dbuffer,512,"CHILD    >%s  childserial=%s on-off=%c (%d)\n",
                     serial, children,*p,abspos);
            dumpNotif(Dbuffer);
#endif
            if ( *p == '0') {
                unregisterNetDevice(yHashPutStr(children));
            }
            break;
        case NOTIFY_NETPKT_LOG:
#ifdef DEBUG_NET_NOTIFICATION
            dbglog("NOTIFY_NETPKT_LOG %s\n", serial);
#endif
            {
                yStrRef serialref = yHashPutStr(serial);
                int devydx = wpGetDevYdx(serialref);
                if (devydx >= 0) {
                    yEnterCriticalSection(&yContext->generic_cs);
                    if (yContext->generic_infos[devydx].flags & DEVGEN_LOG_ACTIVATED) {
                        yContext->generic_infos[devydx].flags |= DEVGEN_LOG_PENDING;
#ifdef DEBUG_NET_NOTIFICATION
                        dbglog("notify device log for %s (%d)\n", serial,devydx);
#endif
                    }
                    yLeaveCriticalSection(&yContext->generic_cs);
                }
            }
            break;
        default:
#ifdef DEBUG_NET_NOTIFICATION
            dbglog("drop: invalid pkttype (%X)\n",pkttype);
            dumpNotif("drop: invalid pkttype\n");
#endif
            break;

    }
    return 1;
}

static int yTcpTrafficPending(void)
{
    int         i;
    HubSt    *hub;

    for (i = 0; i < NBMAX_NET_HUB; i++) {
        hub = yContext->nethub[i];
        if (hub == NULL || hub->url == INVALID_HASH_IDX)
            continue;
        if (yReqHasPending(hub)) {
            return 1;
        }
    }
    return 0;
}

static void* yhelper_thread(void* ctx)
{
    int         i,towatch;
    u8          buffer[512];
    yThread     *thread=(yThread*)ctx;
    char        errmsg[YOCTO_ERRMSG_LEN];
    HubSt    *hub = (HubSt*) thread->ctx;
    RequestSt    *req, *selectlist[1+ALLOC_YDX_PER_HUB];
    u32         toread;
    int         res;
    int         first_notification_connection=1;
#ifdef DEBUG_NET_NOTIFICATION
    char        Dbuffer[1024];
#endif



    yThreadSignalStart(thread);
    while (!yThreadMustEnd(thread)) {
        // Handle async connections as well in this thread
        for (i = 0; i < ALLOC_YDX_PER_HUB; i++) {
            int devydx = hub->devYdxMap[i];
            if (devydx != 0xff){
                yapiPullDeviceLogEx(devydx);
            }
        }
        towatch=0;
        if (hub->state == NET_HUB_ESTABLISHED || hub->state == NET_HUB_TRYING) {
            selectlist[towatch] = hub->http.notReq;
            towatch++;
        } else if(hub->state == NET_HUB_TOCLOSE) {
            yReqClose(hub->http.notReq);
            hub->state = NET_HUB_CLOSED;
        } else if (hub->state == NET_HUB_DISCONNECTED) {
            u64 now;
            if(hub->http.notReq == NULL) {
                hub->http.notReq = (RequestSt*) yMalloc(sizeof(RequestSt));
                hub->http.notReq = yReqAlloc(hub);
            }
            now = yapiGetTickCount();
            if ( (u64)( now - hub->lastAttempt ) > hub->attemptDelay) {
                char request[256];
#ifdef TRACE_NET_HUB
                dbglog("TRACE(%X->%s): try to open notification socket at %d\n",hub->url,hub->name, hub->notifAbsPos);
#endif
                // reset fifo
                yFifoEmpty(&(hub->not_fifo));
                if (first_notification_connection) {
                    YSPRINTF(request, 256, "GET /not.byn HTTP/1.1\r\n\r\n");
                } else {
                    YSPRINTF(request, 256, "GET /not.byn?abs=%u HTTP/1.1\r\n\r\n", hub->notifAbsPos);
                }
                res = yReqOpen(hub->http.notReq, 2 * YIO_DEFAULT_TCP_TIMEOUT, 0, request, YSTRLEN(request), 0, NULL, NULL, NULL, NULL, errmsg);
                if (YISERR(res)) {
                    hub->attemptDelay = 500 << hub->retryCount;
                    if(hub->attemptDelay > 8000)
                        hub->attemptDelay = 8000;
                    hub->lastAttempt = yapiGetTickCount();
                    hub->retryCount++;
                    yEnterCriticalSection(&hub->access);
                    hub->errcode = ySetErr(res, hub->errmsg, errmsg, NULL, 0);
                    yLeaveCriticalSection(&hub->access);

#ifdef TRACE_NET_HUB
                dbglog("TRACE(%X->%s): unable to open notification socket(%s)\n",hub->url,hub->name,errmsg);
                dbglog("TRACE(%X->%s): retry in %dms (%d retries)\n",hub->url,hub->name,hub->attemptDelay,hub->retryCount);
#endif
                } else {
#ifdef TRACE_NET_HUB
                    dbglog("TRACE(%X->%s): notification socket open\n",hub->url,hub->name);
#endif
#ifdef DEBUG_NET_NOTIFICATION
                    YSPRINTF(Dbuffer,1024,"HUB: %X->%s started\n",hub->url,hub->name);
                    dumpNotif(Dbuffer);
#endif
                    hub->state = NET_HUB_TRYING;
                    hub->retryCount=0;
                    hub->attemptDelay = 500;
                    hub->http.lastTraffic = yapiGetTickCount();
                    hub->send_ping = 0;
                    selectlist[towatch++] = hub->http.notReq;
                    first_notification_connection = 0;
                }
            }
        }

        // Handle async connections as well in this thread
        for (i=0; i < ALLOC_YDX_PER_HUB; i++) {
            req = yContext->tcpreq[i];
            if(req == NULL || req->hub != hub){
                continue;
            }
            if(yReqIsAsync(req)) {
                selectlist[towatch++] = req;
            }
        }

        if(YISERR(yReqMultiSelect(selectlist, towatch, 1000, &hub->wuce, errmsg))){
            dbglog("yTcpMultiSelectReq failed (%s)\n",errmsg);
            yApproximateSleep(1000);
        } else {
            for (i = 0; i < towatch; i++) {
                req = selectlist[i];
                if(req == hub->http.notReq) {
                    toread = yFifoGetFree(&hub->not_fifo);
                    while(toread > 0) {
                        if(toread >= sizeof(buffer)) toread = sizeof(buffer)-1;
                        res = yReqRead(req, buffer, toread);
                        if(res > 0) {
                            buffer[res]=0;
#if 0 //def DEBUG_NET_NOTIFICATION
                            YSPRINTF(Dbuffer,1024,"HUB: %X->%s push %d [\n%s\n]\n",hub->url,hub->name,res,buffer);
                            dumpNotif(Dbuffer);
#endif
                            yPushFifo(&(hub->not_fifo), (u8*)buffer, res);
                            if(hub->state == NET_HUB_TRYING) {
                                int eoh = ySeekFifo(&(hub->not_fifo), (u8 *)"\r\n\r\n", 4, 0, 0, 0);
                                if(eoh != 0xffff) {
                                    if(eoh >= 12) {
                                        yPopFifo(&(hub->not_fifo), (u8 *)buffer, 12);
                                        yPopFifo(&(hub->not_fifo), NULL, eoh+4-12);
                                        if(!memcmp((u8 *)buffer, (u8 *)"HTTP/1.1 200", 12)) {
                                            hub->state = NET_HUB_ESTABLISHED;
                                        }
                                    }
                                    if(hub->state != NET_HUB_ESTABLISHED) {
                                        // invalid header received, give up
                                        char hubname[YOCTO_HOSTNAME_NAME]="";
                                        hub->state = NET_HUB_TOCLOSE;
                                        yHashGetUrlPort(hub->url, hubname, NULL, NULL, NULL, NULL, NULL);
                                        dbglog("Network hub %s cannot provide notifications", hubname);
                                    }
                                }
                            }
                            if(hub->state == NET_HUB_ESTABLISHED) {
                                while(handleNetNotification(hub));
                            }
                            hub->http.lastTraffic = yapiGetTickCount();
                        } else {
                            if (hub->send_ping && ( (u64)(yapiGetTickCount() - hub->http.lastTraffic)) > NET_HUB_NOT_CONNECTION_TIMEOUT){
#ifdef TRACE_NET_HUB

                                dbglog("network hub %s(%x) didn't respond for too long (%d)\n", hub->name, hub->url, res);
#endif
                                yReqClose(req);
                                hub->state = NET_HUB_DISCONNECTED;
                            }
                            // nothing more to be read, exit loop
                            break;
                        }
                        toread = yFifoGetFree(&hub->not_fifo);
                    }
                    res = yReqIsEof(req, errmsg);
                    if (res != 0) {
                        // error or remote close
                        yReqClose(req);
                        hub->state = NET_HUB_DISCONNECTED;
                        if (res == 1) {
                            // remote close
                            YERRMSG(YAPI_IO_ERROR, "Connection closed by remote host");
                            dbglog("Disconnected from network hub %s (%s)\n", hub->name, errmsg);
                        } else {
                            //error
                            hub->attemptDelay = 500 << hub->retryCount;
                            if (hub->attemptDelay > 8000)
                                hub->attemptDelay = 8000;
                            hub->lastAttempt = yapiGetTickCount();
                            hub->retryCount++;
                            yEnterCriticalSection(&hub->access);
                            hub->errcode = ySetErr(res, hub->errmsg, errmsg, NULL, 0);
                            yLeaveCriticalSection(&hub->access);
                        }
#ifdef DEBUG_NET_NOTIFICATION
                        YSPRINTF(Dbuffer, 1024, "Network hub %X->%s has closed the connection for notification\n", hub->url, hub->name);
                        dumpNotif(Dbuffer);
#endif
                    }
                } else if (yReqIsAsync(req)) {
                    res = yReqIsEof(req, errmsg);
                    if(res != 0) {
                        yReqClose(req);
                    }
                }
            }
        }
    }

    if (hub->state == NET_HUB_TOCLOSE) {
        yReqClose(hub->http.notReq);
        hub->state = NET_HUB_CLOSED;
    }

    yThreadSignalEnd(thread);
    return NULL;
}


static YRETCODE  yapiLockFunctionCallBack_internal(char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    yEnterCriticalSection(&yContext->functionCallbackCS);
    return YAPI_SUCCESS;
}


static YRETCODE  yapiUnlockFunctionCallBack_internal(char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    yLeaveCriticalSection(&yContext->functionCallbackCS);
    return YAPI_SUCCESS;
}


static YRETCODE  yapiLockDeviceCallBack_internal(char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    yEnterCriticalSection(&yContext->deviceCallbackCS);
    return YAPI_SUCCESS;
}



static YRETCODE  yapiUnlockDeviceCallBack_internal(char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    yLeaveCriticalSection(&yContext->deviceCallbackCS);
    return YAPI_SUCCESS;
}


static YRETCODE yapiRegisterHubEx(const char* url, int checkacces, char* errmsg)
{
    int i;
    int res;

    if (!yContext) {
        YPROPERR(yapiInitAPI_internal(0,errmsg));
    }

    if (YSTRICMP(url,"usb") == 0) {
        if (!(yContext->detecttype & Y_DETECT_USB)) {
            yEnterCriticalSection(&yContext->enum_cs);
            res = yUsbInit(yContext, errmsg);
            if (!YISERR(res)) {
                yContext->detecttype |= Y_DETECT_USB;
            }
            yLeaveCriticalSection(&yContext->enum_cs);
            YPROPERR(res);
        }
        if  (checkacces) {
            yEnterCriticalSection(&yContext->updateDev_cs);
            res = yUSBUpdateDeviceList(errmsg);
            yLeaveCriticalSection(&yContext->updateDev_cs);
            return res;
        }
    } else if (YSTRICMP(url,"net") == 0) {
        if (!(yContext->detecttype & Y_DETECT_NET)) {
            yEnterCriticalSection(&yContext->enum_cs);
            yContext->detecttype |= Y_DETECT_NET;
            res = ySSDPStart(&yContext->SSDP, ssdpEntryUpdate, errmsg);
            yLeaveCriticalSection(&yContext->enum_cs);
            YPROPERR(res);
        }
        if (checkacces) {
            res = yapiUpdateDeviceList_internal(1, errmsg);
            return res;
        }
    } else {
        HubSt *hubst = NULL;
        int firstfree;
        void* (*thead_handler)(void *);

        hubst = yapiAllocHub(url, errmsg);
        if (hubst == NULL) {
            return YAPI_INVALID_ARGUMENT;
        }
        if (checkacces) {
            hubst->mandatory = 1;
        }
        //look if we allready know this
        yEnterCriticalSection(&yContext->enum_cs);
        firstfree = NBMAX_NET_HUB;
        for (i = 0; i < NBMAX_NET_HUB; i++) {
            if (yContext->nethub[i] && yHashSameHub(yContext->nethub[i]->url, hubst->url))
                break;
            if (firstfree == NBMAX_NET_HUB && yContext->nethub[i] == NULL) {
                firstfree = i;
            }
        }


        if (i >= NBMAX_NET_HUB && firstfree < NBMAX_NET_HUB) {
            i = firstfree;
            // save mapping attributed from first access
#ifdef TRACE_NET_HUB
            dbglog("HUB: register %x->%s \n", hubst->url, hubst->name);
#endif
            yContext->nethub[i] = hubst;
            if (YISERR(res = yStartWakeUpSocket(&yContext->nethub[i]->wuce, errmsg))) {
                yLeaveCriticalSection(&yContext->enum_cs);
                return (YRETCODE)res;
            }
            if (hubst->proto == PROTO_WEBSOCKET) {
                thead_handler = ws_thread;
            } else {
                thead_handler = yhelper_thread;
            }
            //yThreadCreate will not create a new thread if there is already one running
            if (yThreadCreate(&yContext->nethub[i]->net_thread, thead_handler, (void*)yContext->nethub[i]) < 0) {
                yLeaveCriticalSection(&yContext->enum_cs);
                return YERRMSG(YAPI_IO_ERROR, "Unable to start helper thread");
            }
            yDringWakeUpSocket(&yContext->nethub[i]->wuce, 1, errmsg);
        }
        yLeaveCriticalSection(&yContext->enum_cs);
        if (i == NBMAX_NET_HUB) {
            yapiFreeHub(hubst);
            return YERRMSG(YAPI_INVALID_ARGUMENT, "Too many network hub registered");
        }

        if (checkacces) {
            // ensure the thread has been able to connect to the hub
            u64 timeout = yapiGetTickCount() + YIO_DEFAULT_TCP_TIMEOUT;
            while (hubst->state != NET_HUB_ESTABLISHED && hubst->state != NET_HUB_CLOSED && hubst->retryCount==0 && timeout > yapiGetTickCount()) {
                yapiSleep(100, errmsg);
            }
            if (hubst->state != NET_HUB_ESTABLISHED) {
                yEnterCriticalSection(&hubst->access);
                res = YERRMSGSILENT(yContext->nethub[i]->errcode, yContext->nethub[i]->errmsg);
                yLeaveCriticalSection(&hubst->access);
                if (!YISERR(res)) {
                    return YERRMSG(YAPI_IO_ERROR, "hub not ready");
                }
                unregisterNetHub(hubst->url);
                return res;
            }
            yEnterCriticalSection(&yContext->updateDev_cs);
            res = yNetHubEnum(hubst, 1, errmsg);
            yLeaveCriticalSection(&yContext->updateDev_cs);
            if (YISERR(res)) {
                yapiUnregisterHub_internal(url);
            } else if (hubst->proto != PROTO_WEBSOCKET) {
                // for HTTP test admin pass if the hub require it
                if (hubst->writeProtected && hubst->http.s_user && strcmp(hubst->http.s_user, "admin") == 0) {
                    YIOHDL  iohdl;
                    const char* request = "GET /api/module/serial?serial=&. ";
                    char    *reply = NULL;
                    int     replysize = 0;
                    int tmpres = yapiHTTPRequestSyncStartEx_internal(&iohdl, 0, yHashGetStrPtr(hubst->serial) , request, YSTRLEN(request), &reply, &replysize, NULL, NULL, errmsg);
                    if (tmpres == YAPI_UNAUTHORIZED) {
                        return tmpres;
                    }
                    if (tmpres == YAPI_SUCCESS) {
                        yapiHTTPRequestSyncDone_internal(&iohdl, errmsg);
                    }
                }
            }

            return res;
        }

    }
    return YAPI_SUCCESS;
}


static int pingURLOnhub(HubSt *hubst, const char *request, int mstimeout, char *errmsg)
{
    yJsonStateMachine j;
    u8              buffer[1500];
    int             res;
    yJsonRetCode    jstate = YJSON_NEED_INPUT;
    u64             globalTimeout;
    RequestSt        *req;

    globalTimeout = yapiGetTickCount() + mstimeout;

    req = yReqAlloc(hubst);
    if (YISERR((res = yReqOpen(req, 2 * YIO_DEFAULT_TCP_TIMEOUT, 0, request, YSTRLEN(request), mstimeout, NULL, NULL, NULL, NULL, errmsg)))) {
        yReqFree(req);
        return res;
    }
    // init yjson parser
    memset(&j, 0, sizeof(j));
    j.st = YJSON_HTTP_START;
    while (jstate == YJSON_NEED_INPUT) {
        res = yReqSelect(req, 500, errmsg);
        if (YISERR(res)) {
            break;
        }
        res = yReqRead(req, buffer, sizeof(buffer));
        while (res > 0) {
            j.src = (char*) buffer;
            j.end = (char*) buffer + res;
            // parse all we can on this buffer
            jstate = yJsonParse(&j);
            while (jstate == YJSON_PARSE_AVAIL) {
                jstate = yJsonParse(&j);
            }
            res = yReqRead(req, buffer, sizeof(buffer));
        }
        if (res <= 0) {
            res = yReqIsEof(req, errmsg);
            if (YISERR(res)) {
                // any specific error during select
                yReqClose(req);
                yReqFree(req);
                return res;
            }
            if (res == 1 && jstate == YJSON_NEED_INPUT) {
                // connection close before end of result
                res = YERR(YAPI_IO_ERROR);
            }
            if (yapiGetTickCount() >= globalTimeout) {
                res = YERR(YAPI_TIMEOUT);
            }
        }
    }
    yReqClose(req);
    yReqFree(req);

    if (res == YAPI_SUCCESS) {
        switch (jstate) {
            case YJSON_NEED_INPUT:
                return YERRMSG(YAPI_IO_ERROR,"Remote host has close the connection");
            case YJSON_PARSE_AVAIL:
            case YJSON_FAILED:
                return YERRMSG(YAPI_IO_ERROR,"Invalid json data");
            case YJSON_SUCCESS:
                break;
        }
    }
    return res;
}

static YRETCODE  yapiTestHub_internal(const char *url, int mstimeout, char *errmsg)
{
    int freeApi = 0;
    int res;

    if(!yContext) {
        YPROPERR(yapiInitAPI_internal(0,errmsg));
        freeApi = 1;
    }

   if(YSTRICMP(url, "usb") == 0) {
        res = YAPI_SUCCESS;
    } else if(YSTRICMP(url, "net") == 0) {
        res = YAPI_SUCCESS;
    }else{
        HubSt *hubst = yapiAllocHub(url, errmsg);
        if (hubst){
#ifdef TRACE_NET_HUB
            dbglog("HUB: test %x->%s \n", hubst->url, hubst->name);
#endif
            if (hubst->proto == PROTO_WEBSOCKET) {
                u64 timeout;
                if (YISERR(res = yStartWakeUpSocket(&hubst->wuce, errmsg))) {
                    yapiFreeHub(hubst);
                    return (YRETCODE)res;
                }
                //yThreadCreate will not create a new thread if there is already one running
                if (yThreadCreate(&hubst->net_thread, ws_thread, (void*)hubst) < 0) {
                    yapiFreeHub(hubst);
                    return YERRMSG(YAPI_IO_ERROR, "Unable to start helper thread");
                }
                yDringWakeUpSocket(&hubst->wuce, 1, errmsg);

                // ensure the thread has been able to connect to the hub
                timeout = yapiGetTickCount() + mstimeout;
                while (hubst->state != NET_HUB_ESTABLISHED && hubst->state != NET_HUB_CLOSED && hubst->retryCount == 0 && timeout > yapiGetTickCount()) {
                    yapiSleep(10, errmsg);
                }
                if (hubst->state != NET_HUB_ESTABLISHED) {
                    yEnterCriticalSection(&hubst->access);
                    res = YERRMSGSILENT(hubst->errcode, hubst->errmsg);
                    yLeaveCriticalSection(&hubst->access);
                    if (!YISERR(res)) {
                        res = YERRMSG(YAPI_IO_ERROR, "hub not ready");
                    }
                }
                if (!YISERR(res)) {
                    res = pingURLOnhub(hubst, "GET /api/module/firmwareRelease.json \r\n\r\n", (int)(timeout - yapiGetTickCount()), errmsg);
                }
                hubst->state = NET_HUB_TOCLOSE;
                yThreadRequestEnd(&hubst->net_thread);
                yDringWakeUpSocket(&hubst->wuce, 0, errmsg);
                // wait for the helper thread to stop monitoring these devices
                yThreadKill(&hubst->net_thread);
            } else {
                res = pingURLOnhub(hubst, "GET /api/module/firmwareRelease.json \r\n\r\n", mstimeout, errmsg);
            }
            yapiFreeHub(hubst);
        } else{
            return YAPI_IO_ERROR;
        }
    }
    if (freeApi) {
        yapiFreeAPI_internal();
    }
    return res;
}


static YRETCODE  yapiRegisterHub_internal(const char *url, char *errmsg)
{
    YRETCODE res;
    res = yapiRegisterHubEx(url,1, errmsg);
    return res;
}

static YRETCODE  yapiPreregisterHub_internal(const char *url, char *errmsg)
{
    YRETCODE res;
    res = yapiRegisterHubEx(url,0, errmsg);
    return res;
}

static void  yapiUnregisterHub_internal(const char *url)
{
    yUrlRef  huburl;

    if(!yContext) {
        return;
    }
    if (YSTRICMP(url,"usb")==0) {
        if(yContext->detecttype & Y_DETECT_USB) {
            yUSBReleaseAllDevices();
            yUsbFree(yContext,NULL);
            yContext->detecttype ^= Y_DETECT_USB;
        }
    }else if(YSTRICMP(url,"net")==0){
        if(yContext->detecttype & Y_DETECT_NET) {
            yContext->detecttype ^= Y_DETECT_NET;
        }
    }else{
        // compute an hashed url
        huburl = yHashUrl(url,"",1,NULL);
        if(huburl == INVALID_HASH_IDX){
            return;
        }
        //look if we allready know this
        yEnterCriticalSection(&yContext->enum_cs);
        unregisterNetHub(huburl);
        yLeaveCriticalSection(&yContext->enum_cs);
    }
}


static YRETCODE  yapiUpdateDeviceList_internal(u32 forceupdate, char *errmsg)
{
    int i;
    YRETCODE err =YAPI_SUCCESS;
    char suberr[YOCTO_ERRMSG_LEN];

    if(yContext==NULL)
        return YERR(YAPI_NOT_INITIALIZED);

    if(forceupdate) {
        yEnterCriticalSection(&yContext->updateDev_cs);
    } else {
        // if we do not force an update
        if(!yTryEnterCriticalSection(&yContext->updateDev_cs)){
            return YAPI_SUCCESS;
        }
    }
    if(yContext->detecttype & Y_DETECT_USB){
        err = yUSBUpdateDeviceList(errmsg);
    }

    for(i = 0; i < NBMAX_NET_HUB; i++){
       if(yContext->nethub[i]){
            int subres;
            if (YISERR(subres = yNetHubEnum(yContext->nethub[i], forceupdate, suberr)) && err == YAPI_SUCCESS) {
                //keep first generated error
                char buffer[YOCTO_HOSTNAME_NAME]="";
                u16  port;
                err = (YRETCODE) subres;
                yHashGetUrlPort(yContext->nethub[i]->url, buffer, &port, NULL, NULL, NULL, NULL);
                if(errmsg) {
                    YSPRINTF(errmsg,YOCTO_ERRMSG_LEN,"Enumeration failed for %s:%d (%s)",buffer,port,suberr);
                }
            }
        }
    }
    yLeaveCriticalSection(&yContext->updateDev_cs);

    return err;
}


static YRETCODE  yapiHandleEvents_internal(char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
     // we need only one thread to handle the event at a time
    if(yTryEnterCriticalSection(&yContext->handleEv_cs)){
        YRETCODE res = (YRETCODE) yUsbIdle();
        yLeaveCriticalSection(&yContext->handleEv_cs);
        return res;
    }
    return YAPI_SUCCESS;
}

u64 test_pkt=0;
u64 test_tout=0;

static YRETCODE  yapiSleep_internal(int ms_duration, char *errmsg)
{
    u64     now, timeout;
    YRETCODE err = YAPI_SUCCESS;


    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    timeout = yapiGetTickCount() + ms_duration;
    do {
        if(err == YAPI_SUCCESS) {
            err = yapiHandleEvents_internal(errmsg);
        }
        now =yapiGetTickCount();
        // todo: we may want to use a samller timeout
        if (now < timeout) {
           if (yWaitForEvent(&yContext->exitSleepEvent, (int) (timeout - now)))
               test_pkt++; //just for testing
           else
               test_tout++;
        }
    } while(yapiGetTickCount() < timeout);

    return err;
}

#ifdef WINDOWS_API
static int                 tickUseHiRes = -1;
static u64                 tickOffset = 0;
static LARGE_INTEGER       tickFrequency;
static LARGE_INTEGER       tickStart;
#endif
u64 YAPI_FUNCTION_EXPORT yapiGetTickCount(void)
{
    u64 res;

#ifdef WINDOWS_API
    LARGE_INTEGER performanceCounter;

    if(tickUseHiRes < 0){
        if (QueryPerformanceFrequency(&tickFrequency)){
            tickUseHiRes = 1;
            tickOffset = (u64)time(NULL) * 1000u;
            QueryPerformanceCounter(&tickStart);
        } else {
            tickUseHiRes = 0;
            tickOffset = (u64)time(NULL) * 1000u - GetTickCount();
        }
        // make sure the offset is always > 0
        if((s64)tickOffset <= 0) tickOffset = 1;
    }
    if(tickUseHiRes>0) {
        QueryPerformanceCounter(&performanceCounter);
        performanceCounter.QuadPart -= tickStart.QuadPart;
        // we add +1 because it is not nice to start with zero
        res = performanceCounter.QuadPart * 1000 / tickFrequency.QuadPart + tickOffset;
    } else {
        res = GetTickCount() + tickOffset;
    }
#else
    //get the current number of microseconds since January 1st 1970
    struct timeval tim;
    gettimeofday(&tim, NULL);
    res = (u64)tim.tv_sec * 1000 + (tim.tv_usec / 1000);
#endif

    return res;
}

u32  yapiGetCNonce(u32 nc)
{
    HASH_SUM ctx;
    u32      md5[4];

#ifdef WINDOWS_API
    LARGE_INTEGER performanceCounter;

    if(tickUseHiRes>0) {
        QueryPerformanceCounter(&performanceCounter);
    } else {
        performanceCounter.QuadPart = GetTickCount();
    }
    MD5Initialize(&ctx);
    MD5AddData(&ctx, (u8 *)&performanceCounter, sizeof(performanceCounter));
#else
    //get the current number of microseconds since January 1st 1970
    struct timeval tim;

    gettimeofday(&tim, NULL);
    MD5Initialize(&ctx);
    MD5AddData(&ctx, (u8 *)&tim, sizeof(tim));
#endif
    MD5AddData(&ctx, (u8 *)&nc, sizeof(nc));
    MD5Calculate(&ctx, (u8 *)md5);

    return md5[1];
}

static int  yapiCheckLogicalName_internal(const char *name)
{
    char c;

    if(!name) return 0;
    if(!*name) return 1;
    if(strlen(name) > 19) return 0;
    while((c = *name++) != 0) {
        if(c < '-') return 0;
        if(c > '-' && c < '0') return 0;
        if(c > '9' && c < 'A') return 0;
        if(c > 'Z' && c < '_') return 0;
        if(c > '_' && c < 'a') return 0;
        if(c > 'z') return 0;
    }
    return 1;
}


static u16  yapiGetAPIVersion_internal(const char **version, const char **apidate)
{
    if(version)
        *version = YOCTO_API_VERSION_STR"."YOCTO_API_BUILD_NO;
    if(apidate)
        *apidate = YOCTO_API_BUILD_DATE;

    return YOCTO_API_VERSION_BCD;
}

static void  yapiSetTraceFile_internal(const char *file)
{
    if(file!=NULL){
        memset(ytracefile,0,TRACEFILE_NAMELEN);
        YSTRNCPY(ytracefile,TRACEFILE_NAMELEN-1,file,TRACEFILE_NAMELEN-1);
    }else{
        ytracefile[0]=0;
    }
}


static YAPI_DEVICE  yapiGetDevice_internal(const char *device_str, char *errmsg)
{
    char    hostname[HASH_BUF_SIZE], c;
    int     i;
    YAPI_DEVICE res;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(!strncmp(device_str, "http://", 7)) {
        for(i = 0; i < HASH_BUF_SIZE-1; i++) {
            c = device_str[7+i];
            if(!c || c == '/') break;
            hostname[i] = c;
        }
        res = wpSearchByUrl(hostname, device_str+7+i);
    } else {
        res = wpSearch(device_str);
    }
    if(res == -1) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    return res;
}


static int  yapiGetAllDevices_internal(YAPI_DEVICE *buffer, int maxsize, int *neededsize, char *errmsg)
{
    int     maxdev,nbreturned;
    yBlkHdl devhdl;
    YAPI_DEVICE devdescr;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(buffer==NULL && neededsize==NULL){
        return YERR(YAPI_INVALID_ARGUMENT);
    }

    nbreturned = 0;
    if(buffer) {
        // This function walks through the white pages without taking the mutex globally
        // it is only reasonably safe because wpGetAttribute properly handles dangerous devhdl.
        // (same principle as used by HTTPSendRec for the REST API)
        maxdev = 0;
        for(devhdl = yWpListHead; devhdl != INVALID_BLK_HDL; devhdl = yBlkListSeek(devhdl, 1)) {
            devdescr = wpGetAttribute(devhdl, Y_WP_SERIALNUMBER);
            if(devdescr < 0) continue;
            maxdev++;
            if(maxsize >= (int)sizeof(YAPI_DEVICE)) {
                maxsize -= sizeof(YAPI_DEVICE);
                *buffer++ = devdescr;
                nbreturned++;
            }
        }
        if(neededsize) *neededsize = sizeof(YAPI_DEVICE) * maxdev;
    } else {
        if(neededsize) *neededsize = sizeof(YAPI_DEVICE) * wpEntryCount();
    }

    return nbreturned;
}


static YRETCODE  yapiGetDeviceInfo_internal(YAPI_DEVICE devdesc, yDeviceSt *infos, char *errmsg)
{
    YUSBDEV devhdl;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(devdesc < 0 || infos == NULL){
        return YERR(YAPI_INVALID_ARGUMENT);
    }

    yHashGetStr(devdesc & 0xffff, infos->serial, YOCTO_SERIAL_LEN);
    devhdl = findDevHdlFromStr(infos->serial);
    if(devhdl != INVALID_YHANDLE) {
        // local device, get all information straight from the source
        devHdlInfo(devhdl, infos);
    } else {
        // not a local device, get available information from white pages
        infos->vendorid = 0x24e0;
        infos->devrelease = 0;
        infos->nbinbterfaces = 1;
        memcpy((u8 *)infos->manufacturer, (u8 *)"Yoctopuce", 10);
        memset(infos->firmware, 0, YOCTO_FIRMWARE_LEN);
        if(wpGetDeviceInfo(devdesc, &infos->deviceid, infos->productname, infos->serial, infos->logicalname, &infos->beacon) < 0){
            return YERR(YAPI_DEVICE_NOT_FOUND);
        }
    }

    return YAPI_SUCCESS;
}

static YRETCODE  yapiGetDevicePath_internal(YAPI_DEVICE devdesc, char *rootdevice, char *request, int requestsize, int *neededsize, char *errmsg)
{
    YRETCODE res;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    if(rootdevice==NULL && request==NULL && neededsize==NULL) {
        return YERR(YAPI_INVALID_ARGUMENT);
    }
    res = (YRETCODE) wpGetDeviceUrl(devdesc, rootdevice, request, requestsize, neededsize);
    if(neededsize) *neededsize += 4;
    if(res < 0) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    return res;
}


static YRETCODE  yapiGetDevicePathEx_internal(const char *serial, char *rootdevice, char *request, int requestsize, int *neededsize, char *errmsg)
{
    YAPI_DEVICE devdescr;
    yUrlRef url;
    char host[YOCTO_HOSTNAME_NAME];
    char buffer[512];
    u16 port;
    yAsbUrlProto proto;

    if (!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    if (rootdevice==NULL && request==NULL && neededsize==NULL) {
        return YERR(YAPI_INVALID_ARGUMENT);
    }
    devdescr = wpSearch(serial);
    if (YISERR(devdescr)) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    url = wpGetDeviceUrlRef(devdescr);
    switch (yHashGetUrlPort(url, host, &port, &proto, NULL, NULL, NULL)) {
    case USB_URL:
        if (rootdevice) {
            *rootdevice = 0;
        }
        if (request && requestsize > 4) {
            YSTRCPY(request, requestsize, "usb");
        }
        if (*neededsize){
            *neededsize = 4;
        }
        break;
    default:
        wpGetDeviceUrl(devdescr, rootdevice, buffer, 512, neededsize);
        if (request) {

            int len = YSPRINTF(request, requestsize, "%s://%s:%d%s", proto == PROTO_WEBSOCKET ? "ws" : "http", host, port, buffer);
            *neededsize = len + 1;
        }
        if (rootdevice && YSTRCMP(rootdevice, serial) == 0) {
            *rootdevice = 0;
        }
    }
    return YAPI_SUCCESS;
}


static YAPI_FUNCTION  yapiGetFunction_internal(const char *class_str, const char *function_str, char *errmsg)
{
    YAPI_FUNCTION res;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    res = ypSearch(class_str, function_str);
    if(res < 0) {
        if(res == -2) {
            return YERRMSG(YAPI_DEVICE_NOT_FOUND, "No function of that class");
        }
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    return res;
}

static int  yapiGetFunctionsByClass_internal(const char *class_str, YAPI_FUNCTION prevfundesc,
                                          YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg)
{
    int res;
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    if(buffer==NULL && neededsize==NULL){
        return YERR(YAPI_INVALID_ARGUMENT);
    }

    res = ypGetFunctions(class_str, -1, prevfundesc, buffer, maxsize, neededsize);
    if(res < 0) {
        return YERR(YAPI_DEVICE_NOT_FOUND); // prevfundesc is invalid or has been unplugged, restart enum
    }

    return res;
}

static int  yapiGetFunctionsByDevice_internal(YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
                                           YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg)
{
    int res;
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    if(buffer==NULL && neededsize==NULL){
        return YERR(YAPI_INVALID_ARGUMENT);
    }
    res = ypGetFunctions(NULL, devdesc, prevfundesc, buffer, maxsize, neededsize);
    if(res < 0) {
        return YERR(YAPI_DEVICE_NOT_FOUND); // prevfundesc is invalid or has been unplugged, restart enum
    }
    return res;
}


static YRETCODE  yapiGetFunctionInfoEx_internal(YAPI_FUNCTION fundesc, YAPI_DEVICE *devdesc, char *serial, char *funcId, char *baseType, char *funcName, char *funcVal, char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(ypGetFunctionInfo(fundesc, serial, funcId, baseType, funcName, funcVal) < 0) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    if(devdesc)
        *devdesc = fundesc & 0xffff;

    return YAPI_SUCCESS;
}


static int yapiRequestOpenUSB(YIOHDL_internal *iohdl, HubSt *hub, YAPI_DEVICE dev, const char *request, int reqlen, u64 unused_timeout, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    char        buffer[512];
    YRETCODE    res;
    int         firsttime = 1;
    u64         timeout;
    int         count = 0;

    yHashGetStr(dev & 0xffff, buffer, YOCTO_SERIAL_LEN);
    timeout = yapiGetTickCount() + YAPI_BLOCKING_USBOPEN_REQUEST_TIMEOUT;
    do {
        count++;
        res = (YRETCODE)yUsbOpen(iohdl, buffer, errmsg);
        if (res != YAPI_DEVICE_BUSY) break;
        yapiHandleEvents_internal(errmsg);
        if (firsttime) {
            //yApproximateSleep(1);
            firsttime = 0;
        }
    } while (yapiGetTickCount() < timeout);

    if (res != YAPI_SUCCESS) {
        return res;
    }
    if (reqlen >= 10 && reqlen <= (int) sizeof(buffer) && !memcmp(request + reqlen - 7, "&. \r\n\r\n", 7)) {
        memcpy(buffer, request, reqlen - 7);
        memcpy(buffer + reqlen - 7, " \r\n\r\n", 5);
        reqlen -= 2;
        request = buffer;
    }
    res = (YRETCODE)yUsbWrite(iohdl, request, reqlen, errmsg);
    if (YISERR(res)) {
        yUsbClose(iohdl, errmsg);
        return res;
    }
    if (callback) {
        res = (YRETCODE)yUsbSetIOAsync(iohdl, callback, context, errmsg);
        if (YISERR(res)) {
            yUsbClose(iohdl, errmsg);
            return res;
        }
    }
    return res;
}


static int yapiRequestOpenHTTP(YIOHDL_internal *iohdl, HubSt *hub, YAPI_DEVICE dev, const char *request, int reqlen, int wait_for_start, u64 mstimeout, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    YRETCODE    res;
    int         devydx;
    RequestSt   *tcpreq;

    devydx = wpGetDevYdx((yStrRef)dev);
    if (devydx < 0) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    yEnterCriticalSection(&yContext->io_cs);
    tcpreq = yContext->tcpreq[devydx];
    if (tcpreq == NULL) {
        tcpreq = yReqAlloc(hub);
        yContext->tcpreq[devydx] = tcpreq;
    }
    yLeaveCriticalSection(&yContext->io_cs);
    if (callback) {
        if (tcpreq->hub->writeProtected) {
            // no need to take the critical section tcpreq->hub->http.authAccess since we only read user an pass
            if (!tcpreq->hub->http.s_user || strcmp(tcpreq->hub->http.s_user, "admin") != 0) {
                return YERRMSG(YAPI_UNAUTHORIZED, "Access denied: admin credentials required");
            }
        }
    }
    if ((tcpreq->hub->send_ping || !tcpreq->hub->mandatory) && tcpreq->hub->state != NET_HUB_ESTABLISHED) {
        if (errmsg) {
            YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "hub %s is not reachable", tcpreq->hub->name);
        }
        return YAPI_IO_ERROR;
    }

    res = (YRETCODE)yReqOpen(tcpreq, wait_for_start, 0, request, reqlen, mstimeout, callback, context, NULL, NULL, errmsg);
    if (res != YAPI_SUCCESS) {
        return res;
    }

    if (callback) {
        res = (YRETCODE)yDringWakeUpSocket(&tcpreq->hub->wuce, 2, errmsg);
        if (res != YAPI_SUCCESS) {
            return res;
        }
    }
    iohdl->tcpreqidx = devydx;
    iohdl->type = YIO_TCP;
    return YAPI_SUCCESS;
}

static int yapiRequestOpenWS(YIOHDL_internal *iohdl, HubSt *hub, YAPI_DEVICE dev, int tcpchan, const char *request, int reqlen, u64 mstimeout, yapiRequestAsyncCallback callback, void *context, RequestProgress progress_cb, void *progress_ctx, char *errmsg)
{
    YRETCODE    res;
    int         devydx;
    RequestSt   *req;

    devydx = wpGetDevYdx((yStrRef)dev);
    if (devydx < 0) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    //dbglog("yapiRequestOpenWS on %p %s\n", hub, callback ? "ASYNC": "");
    if (callback) {
        if (hub->writeProtected && !hub->rw_access) {
            return YERRMSG(YAPI_UNAUTHORIZED, "Access denied: admin credentials required");
        }
    }
    req = yReqAlloc(hub);
    if ((req->hub->send_ping || !req->hub->mandatory) && req->hub->state != NET_HUB_ESTABLISHED) {
        if (errmsg) {
            YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "hub %s is not reachable", req->hub->name);
        }
        return YAPI_IO_ERROR;
    }

    if (req->hub->state != NET_HUB_ESTABLISHED) {
        if (YISERR(req->hub->errcode)) {
            yEnterCriticalSection(&req->hub->access);
            res = YERRMSG(req->hub->errcode, req->hub->errmsg);
            yLeaveCriticalSection(&req->hub->access);
            return res;
        }
        if (errmsg) {
            YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "hub %s is not ready", req->hub->name);
        }
        return YERRMSG(YAPI_TIMEOUT, "hub is not ready");
    }

    res = (YRETCODE)yReqOpen(req, 2 * YIO_DEFAULT_TCP_TIMEOUT, tcpchan, request, reqlen, mstimeout, callback, context, progress_cb, progress_ctx, errmsg);
    if (res != YAPI_SUCCESS) {
        return res;
    }

    iohdl->ws = req;
    iohdl->type = YIO_WS;
    return YAPI_SUCCESS;
}


YRETCODE yapiRequestOpen(YIOHDL_internal *iohdl, int tcpchan, const char *device, const char *request, int reqlen,  yapiRequestAsyncCallback callback, void *context, yapiRequestProgressCallback progress_cb, void *progress_ctx, char *errmsg)
{
    YAPI_DEVICE dev;
    char        buffer[512];
    yUrlRef     url;
    yAsbUrlProto proto;
    int i, len;
    u64 mstimeout = YIO_DEFAULT_TCP_TIMEOUT;
    HubSt *hub = NULL;

    if(!yContext) {
        return YERR(YAPI_NOT_INITIALIZED);
    }

    dev = wpSearch(device);
    if(dev == -1) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    YASSERT(iohdl != NULL);
    memset(iohdl, 0, sizeof(YIOHDL_internal));
    // compute request timeout
    len = (reqlen < YOCTO_SERIAL_LEN + 32 ? reqlen : YOCTO_SERIAL_LEN + 32);
    if (memcmp(request, "GET ", 4) == 0) {
        if (ymemfind((u8*)request + 4, len, (u8*)"/testcb.txt", 11) >= 0) {
            mstimeout = YIO_1_MINUTE_TCP_TIMEOUT;
        } else if (ymemfind((u8*)request + 4, len, (u8*)"/rxmsg.json", 11) >= 0) {
            mstimeout = YIO_1_MINUTE_TCP_TIMEOUT;
        } else if (ymemfind((u8*)request + 4, len, (u8*)"/files.json", 11) >= 0) {
            mstimeout = YIO_1_MINUTE_TCP_TIMEOUT;
        } else if (ymemfind((u8*)request + 4, len, (u8*)"/flash.json", 11) >= 0) {
            mstimeout = YIO_10_MINUTES_TCP_TIMEOUT;
        }
    } else {
        if (ymemfind((u8*)request + 4, len, (u8*)"/upload.html", 12) >= 0) {
            //fixme: use 1 minute timeout for WS
            mstimeout = YIO_10_MINUTES_TCP_TIMEOUT;
        }
    }

    // dispatch request on correct hub (or pseudo usb HUB)
    url = wpGetDeviceUrlRef(dev);
    switch(yHashGetUrlPort(url, buffer, NULL, &proto, NULL, NULL, NULL)) {
    case USB_URL:
        return yapiRequestOpenUSB(iohdl, NULL, dev, request, reqlen, mstimeout, callback, context, errmsg);
    default:
        for (i = 0; i < NBMAX_NET_HUB; i++) {
            if (yContext->nethub[i] && yHashSameHub(yContext->nethub[i]->url, url)) {
                hub = yContext->nethub[i];
                break;
            }
        }
        if (hub == NULL) {
            return YERR(YAPI_DEVICE_NOT_FOUND);
        }
        if (proto == PROTO_WEBSOCKET) {
            return yapiRequestOpenWS(iohdl, hub, dev, tcpchan, request, reqlen, mstimeout, callback, context, progress_cb, progress_ctx, errmsg);
        }  else {
            return yapiRequestOpenHTTP(iohdl, hub, dev, request, reqlen, 2 * YIO_DEFAULT_TCP_TIMEOUT, mstimeout, callback, context, errmsg);
        }
    }
}

static int yapiRequestWaitEndUSB(YIOHDL_internal *iohdl, char **reply, int *replysize, char *errmsg)
{
    u64      timeout;
    yPrivDeviceSt *p;
    int buffpos = 0;
    int res;


    timeout = yapiGetTickCount() + YAPI_BLOCKING_USBREAD_REQUEST_TIMEOUT;
    p = findDevFromIOHdl(iohdl);
    if (p == NULL) {
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    if (p->replybuf == NULL) {
        p->replybufsize = 2048;
        p->replybuf = (char*)yMalloc(p->replybufsize);
    }
    while ((res = (YRETCODE)yUsbEOF(iohdl, errmsg)) == 0) {
        if (yapiGetTickCount() > timeout) {
            yUsbClose(iohdl, NULL);
            return YERRMSG(YAPI_TIMEOUT, "Timeout during device request");
        }
        if (buffpos + 256 > p->replybufsize) {
            char *newbuff;
            p->replybufsize <<= 1;
            newbuff = (char*)yMalloc(p->replybufsize);
            memcpy(newbuff, p->replybuf, buffpos);
            yFree(p->replybuf);
            p->replybuf = newbuff;
        }
        res = (YRETCODE)yUsbReadBlock(iohdl, p->replybuf + buffpos, p->replybufsize - buffpos, timeout, errmsg);
        if (YISERR(res)) {
            yUsbClose(iohdl, NULL);
            return res;
        } else if (res > 0) {
            timeout = yapiGetTickCount() + YAPI_BLOCKING_USBREAD_REQUEST_TIMEOUT;
        }
        buffpos += res;
    }
    *reply = p->replybuf;
    *replysize = buffpos;
    return res;
}


static int yapiRequestWaitEndHTTP(YIOHDL_internal *iohdl, char **reply, int *replysize, char *errmsg)
{
    int res;
    RequestSt *tcpreq = yContext->tcpreq[iohdl->tcpreqidx];

    res = (YRETCODE)yReqIsEof(tcpreq, errmsg);
    while (res == 0) {
        res = (YRETCODE)yReqSelect(tcpreq, 1000, errmsg);
        if (YISERR(res)) {
            yReqClose(tcpreq);
            return res;
        }
        res = (YRETCODE)yReqIsEof(tcpreq, errmsg);
    }
    if (YISERR(res) && res != YAPI_NO_MORE_DATA) {
        yReqClose(tcpreq);
        return res;
    }
    *replysize = yReqGet(tcpreq, (u8**)reply);
    return YAPI_SUCCESS;
}




static int yapiRequestWaitEndWS(YIOHDL_internal *iohdl, char **reply, int *replysize, char *errmsg)
{
    int res;
    RequestSt *tcpreq = iohdl->ws;

    res = (YRETCODE)yReqIsEof(tcpreq, errmsg);
    while (res == 0) {
        res = (YRETCODE)yReqSelect(tcpreq, 1000, errmsg);
        if (YISERR(res)) {
            yReqClose(tcpreq);
            return res;
        }
        res = (YRETCODE)yReqIsEof(tcpreq, errmsg);
    }
    if (YISERR(res) && res != YAPI_NO_MORE_DATA) {
        yReqClose(tcpreq);
        return res;
    }
    *replysize = yReqGet(tcpreq, (u8**)reply);
    return YAPI_SUCCESS;


}



YRETCODE  yapiHTTPRequestSyncStartEx_internal(YIOHDL *iohdl, int tcpchan, const char *device, const char *request, int requestsize, char **reply, int *replysize, yapiRequestProgressCallback progress_cb, void *progress_ctx, char *errmsg)
{
    YRETCODE res;
    YIOHDL_internal *internalio;
#ifdef DEBUG_YAPI_REQ
    int req_count = YREQ_LOG_START("SyncStartEx", device, request, requestsize);
    u64 start_tm = yapiGetTickCount();
#endif


    if (!yContext)
        return YERR(YAPI_NOT_INITIALIZED);


    *reply = NULL;
    internalio = yMalloc(sizeof(YIOHDL_internal));
    memset((u8 *)iohdl, 0, YIOHDL_SIZE);
    if (YISERR(res = yapiRequestOpen(internalio, tcpchan, device, request, requestsize, NULL, NULL, progress_cb, progress_ctx, errmsg))) {
        yFree(internalio);
    } else {

        if (internalio->type == YIO_USB) {
            res = yapiRequestWaitEndUSB(internalio, reply, replysize, errmsg);
        } else if (internalio->type == YIO_TCP) {
            res = yapiRequestWaitEndHTTP(internalio, reply, replysize, errmsg);
        } else if (internalio->type == YIO_WS) {
            res = yapiRequestWaitEndWS(internalio, reply, replysize, errmsg);
        } else {
            yFree(internalio);
            return YERR(YAPI_INVALID_ARGUMENT);
        }

        yEnterCriticalSection(&yContext->io_cs);
        *iohdl = internalio;
        internalio->next = yContext->yiohdl_first;
        yContext->yiohdl_first = internalio;
        yLeaveCriticalSection(&yContext->io_cs);
    }
#ifdef DEBUG_YAPI_REQ
    if (res < 0) {
        YREQ_LOG_APPEND_ERR(req_count, "SyncStartEx", errmsg, res, start_tm);
    } else {
        YREQ_LOG_APPEND(req_count, "SyncStartEx", *reply, *replysize, start_tm);
    }
#endif

    return res;
}


YRETCODE  yapiHTTPRequestSyncDone_internal(YIOHDL *iohdl, char *errmsg)
{
    YIOHDL_internal *r, *p, *arg = *iohdl;
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(iohdl == NULL)
        return YERR(YAPI_INVALID_ARGUMENT);


    yEnterCriticalSection(&yContext->io_cs);
    r = yContext->yiohdl_first;
    p = NULL;
    while(r && r != arg) {
        p = r;
        r = r->next;
    }
    if (r ==NULL || r != arg) {
        yLeaveCriticalSection(&yContext->io_cs);
        return YERR(YAPI_INVALID_ARGUMENT);
    }
    if (p == NULL) {
        yContext->yiohdl_first = r->next;
    } else {
        p->next = r->next;
    }
    yLeaveCriticalSection(&yContext->io_cs);




    if(arg->type == YIO_USB) {
        yUsbClose(arg, errmsg);
    } else if(arg->type == YIO_TCP) {
        RequestSt *tcpreq = yContext->tcpreq[arg->tcpreqidx];
        yReqClose(tcpreq);
    } else {
        yReqClose(arg->ws);
        yReqFree(arg->ws);
    }
    yFree(arg);
    memset((u8 *)iohdl, 0, YIOHDL_SIZE);
    return YAPI_SUCCESS;
}




static void asyncDrop(void *context, const u8 *result, u32 resultlen, int retcode, const char *errmsg)
{
#ifdef DEBUG_YAPI_REQ
    int req_count = (int)(((u8*)context) - ((u8*)NULL));
    YREQ_LOG_APPEND(req_count, "ASync", result, resultlen, 0);
#endif
}


static YRETCODE  yapiHTTPRequestAsyncEx_internal(int tcpchan, const char *device, const char *request, int len, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    YIOHDL_internal iohdl;
    YRETCODE res;
    int retryCount = 1;
#ifdef DEBUG_YAPI_REQ
    int yreq_count = YREQ_LOG_START("ASync", device, request, len);
#endif

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    do {
        if (callback == NULL) {
            callback = asyncDrop;
#ifdef DEBUG_YAPI_REQ
            context = ((u8*)NULL) + yreq_count;
#endif
        }
        res = yapiRequestOpen(&iohdl, tcpchan, device, request, len, callback, context, NULL,NULL,errmsg);
        if(YISERR(res)) {
            if (res == YAPI_UNAUTHORIZED) {
                return res;
            }

            if (retryCount){
                char suberr[YOCTO_ERRMSG_LEN];
                dbglog("ASync request for %s failed. Retrying after yapiUpdateDeviceList\n",device);
                if(YISERR(yapiUpdateDeviceList_internal(1, suberr))){
                    dbglog("yapiUpdateDeviceList failled too with %s\n",errmsg);
                    return res;
                }
            }
        }
    } while (res !=YAPI_SUCCESS && retryCount--);

    return res;
}



static int  yapiHTTPRequest_internal(const char *device, const char *request, char* buffer, int buffsize, int *fullsize, char *errmsg)
{
    YIOHDL  iohdl;
    char    *reply=NULL;
    int     replysize=0;

    if(!buffer || buffsize < 4){
        return YERR(YAPI_INVALID_ARGUMENT);
    }
    YPROPERR(yapiHTTPRequestSyncStartEx_internal(&iohdl, 0, device, request, YSTRLEN(request), &reply, &replysize, NULL, NULL, errmsg));

    if (fullsize)
        *fullsize = replysize;

    if(replysize > buffsize-1) {
        replysize = buffsize-1;
    }
    memcpy(buffer, reply, replysize);
    buffer[replysize] = 0;
    YPROPERR(yapiHTTPRequestSyncDone_internal(&iohdl, errmsg));
    return replysize;
}

static void  yapiRegisterHubDiscoveryCallback_internal(yapiHubDiscoveryCallback hubDiscoveryCallback)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    if (!yContext) {
        yapiInitAPI_internal(0, errmsg);
    }
    if (yContext) {
        yContext->hubDiscoveryCallback = hubDiscoveryCallback;
    }
}

static YRETCODE  yapiTriggerHubDiscovery_internal(char *errmsg)
{
    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);
    // ensure SSDP thread is started
    YPROPERR(ySSDPStart(&yContext->SSDP,ssdpEntryUpdate,errmsg));
    // triger SSDP discovery
    return (YRETCODE) ySSDPDiscover(&yContext->SSDP,errmsg);
}

// used only by VirtualHub
YRETCODE yapiGetBootloadersDevs(char *serials,unsigned int maxNbSerial, unsigned int *totalBootladers, char *errmsg)
{
    int             nbifaces=0;
    yInterfaceSt    *iface;
    yInterfaceSt    *runifaces=NULL;
    int             i;
    u32             totalBoot,copyedBoot;
    char            *s=serials;
    YRETCODE        res;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if((yContext->detecttype & Y_DETECT_USB) ==0) {
        return YERRMSG(YAPI_INVALID_ARGUMENT,"You must init the yAPI with Y_DETECT_USB flag");
    }

    if (YISERR(res = (YRETCODE) yyyUSBGetInterfaces(&runifaces,&nbifaces,errmsg))){
        return res;
    }

    totalBoot=copyedBoot=0;

    for(i=0, iface=runifaces ; i < nbifaces ; i++, iface++){
        if(iface->deviceid != YOCTO_DEVID_BOOTLOADER)
            continue;
        if(serials && copyedBoot < maxNbSerial){
            YSTRCPY(s,YOCTO_SERIAL_LEN*2,iface->serial);
            s+=YOCTO_SERIAL_LEN;
            copyedBoot++;
        }
        totalBoot++;
    }
    // free all tmp ifaces
    if(runifaces){
        yFree(runifaces);
    }
    if(totalBootladers)
        *totalBootladers=totalBoot;

    return (YRETCODE) copyedBoot;

}

//used by API
static YRETCODE  yapiGetBootloaders_internal(char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    int             i;
    char            *p = buffer;
    YRETCODE        res;
    int             size, total, len;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(buffer==NULL || buffersize<1)
        return YERR(YAPI_INVALID_ARGUMENT);

    buffersize--;// reserve space for \0

    size = total = 0;

    if (yContext->detecttype & Y_DETECT_USB) {
        int             nbifaces = 0;
        yInterfaceSt    *iface;
        yInterfaceSt    *runifaces = NULL;

        if (YISERR(res = (YRETCODE)yyyUSBGetInterfaces(&runifaces, &nbifaces, errmsg))){
            return res;
        }

        for (i = 0, iface = runifaces; i < nbifaces; i++, iface++){
            if (iface->deviceid != YOCTO_DEVID_BOOTLOADER)
                continue;

            if (buffer && size < buffersize && buffer != p) {
                *p++ = ',';
                size++;
            }

            len = YSTRLEN(iface->serial);
            total += len;
            if (buffer && size + len < buffersize){
                YSTRCPY(p, buffersize - size, iface->serial);
                p += len;
                size += len;
            }
        }
        // free all tmp ifaces
        if (runifaces){
            yFree(runifaces);
        }

    }


    for (i = 0; i < NBMAX_NET_HUB; i++){
        if (yContext->nethub[i]){
            char bootloaders[4 * YOCTO_SERIAL_LEN];
            char hubserial[YOCTO_SERIAL_LEN];
            int res, j;
            char *serial;
            yHashGetStr(yContext->nethub[i]->serial, hubserial, YOCTO_SERIAL_LEN);
            res = yNetHubGetBootloaders(hubserial, bootloaders, errmsg);
            if (YISERR(res)) {
                return res;
            }
            for (j = 0, serial = bootloaders; j < res; j++, serial += YOCTO_SERIAL_LEN){
                if (buffer && size < buffersize && buffer != p) {
                    *p++ = ',';
                    size++;
                }

                len = YSTRLEN(serial);
                total += len;
                if (buffer && size + len < buffersize){
                    YSTRCPY(p, buffersize - size, serial);
                    p += len;
                    size += len;
                }
            }
        }
    }

    //ensure buffer is null terminated;
    *p = 0;
    if(fullsize)
        *fullsize = total;

    return (YRETCODE) size;

}

#ifndef YAPI_IN_YDEVICE

static int  yapiGetSubdevices_internal(const char *serial, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    int             i;
    char            *p = buffer;
    int             size, total;

    if(!yContext)
        return YERR(YAPI_NOT_INITIALIZED);

    if(buffer==NULL || buffersize<1)
        return YERR(YAPI_INVALID_ARGUMENT);

    buffersize--;// reserve space for \0
    size = total = 0;
    for (i = 0; i < NBMAX_NET_HUB; i++) {
        char hubserial[YOCTO_SERIAL_LEN];

        if (yContext->nethub[i] == NULL)
            continue;

        yHashGetStr(yContext->nethub[i]->serial, hubserial, YOCTO_SERIAL_LEN);
        if (YSTRCMP(serial, hubserial) == 0) {
            yStrRef  knownDevices[128];
            int j, nbKnownDevices;
            nbKnownDevices = wpGetAllDevUsingHubUrl(yContext->nethub[i]->url, knownDevices, 128);
            total = nbKnownDevices * YOCTO_SERIAL_LEN + nbKnownDevices;
            if (buffersize > total) {
                int isfirst = 1;
                for (j = 0; j < nbKnownDevices; j++) {
                    if (knownDevices[j] == yContext->nethub[i]->serial)
                        continue;
                    if (!isfirst)
                        *p++ = ',';
                    yHashGetStr(knownDevices[j], p, YOCTO_SERIAL_LEN);
                    p += YSTRLEN(p);
                    isfirst = 0;
                }
            }
            break;
        }
    }

    //ensure buffer is null terminated;
    size = (int) (p - buffer);
    *p++ = 0;
    if (fullsize)
        *fullsize = total;

    return size;

}


static const char*  yapiJsonValueParseArray(yJsonStateMachine *j, const char *path, int *result, char *errmsg);


static void skipJsonStruct(yJsonStateMachine *j)
{
#ifdef DEBUG_JSON_PARSE
    dbglog("skip  %s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
#endif
    yJsonParse(j);
    do {
#ifdef DEBUG_JSON_PARSE
        dbglog("... %s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
#endif
        yJsonSkip(j, 1);
    } while (yJsonParse(j) == YJSON_PARSE_AVAIL && j->st != YJSON_PARSE_STRUCT);
}


static void skipJsonArray(yJsonStateMachine *j)
{
#ifdef DEBUG_JSON_PARSE
    dbglog("skip  %s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
#endif
    yJsonParse(j);
    do {
#ifdef DEBUG_JSON_PARSE
        dbglog("... %s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
#endif
        yJsonSkip(j, 1);
    } while (yJsonParse(j) == YJSON_PARSE_AVAIL && j->st != YJSON_PARSE_ARRAY);
}


static const char*  yapiJsonValueParseStruct(yJsonStateMachine *j, const char *path, int *result, char *errmsg)
{

    int len = 0;
    const char *p = path;

    while (*p && *p != '|'){
        p++;
        len++;
    }

    while (yJsonParse(j) == YJSON_PARSE_AVAIL) {
        if (j->st == YJSON_PARSE_MEMBNAME) {
            if (YSTRNCMP(path, j->token, len) == 0){
                if (*p) {
#ifdef DEBUG_JSON_PARSE
                    dbglog("recurse %s %s(%d):%s\n", j->token, yJsonStateStr[j->st], j->st, j->token);
#endif
                    yJsonParse(j);
                    if (j->st == YJSON_PARSE_STRUCT) {
                        return yapiJsonValueParseStruct(j, ++p, result, errmsg);
                    } else if (j->st == YJSON_PARSE_ARRAY) {
                        return yapiJsonValueParseArray(j, ++p, result, errmsg);
                    } else{
                        *result = YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid JSON struct");
                        return "";
                    }
                } else{
                    const char *start_of_json;
#ifdef DEBUG_JSON_PARSE
                    dbglog("found %s %s(%d):%s\n", j->token, yJsonStateStr[j->st], j->st, j->token);
#endif
                    yJsonParse(j);
                    start_of_json = j->state_start;
                    switch (j->st){
                    case YJSON_PARSE_STRING:
                        while (j->next == YJSON_PARSE_STRINGCONT) {
                            yJsonParse(j);
                        }
                    case YJSON_PARSE_NUM:
                        *result = (u32)(j->state_end - start_of_json);
                        return  start_of_json;
                    case YJSON_PARSE_STRUCT:
                        skipJsonStruct(j);
                        *result = (u32)(j->state_end - start_of_json);
                        return  start_of_json;
                    case YJSON_PARSE_ARRAY:
                        skipJsonArray(j);
                        *result = (u32)(j->state_end - start_of_json);
                        return  start_of_json;
                    default:
                        *result = YERRMSG(YAPI_INVALID_ARGUMENT, "Only String and numerical target are supported");
                        return "";
                    }
                }
            } else {
#ifdef DEBUG_JSON_PARSE
                dbglog("skip %s %s(%d):%s\n", j->token, yJsonStateStr[j->st], j->st, j->token);
#endif
                yJsonSkip(j, 1);
            }
        }
#ifdef DEBUG_JSON_PARSE
        else{
            dbglog("%s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
        }
#endif
    }
    *result = YERRMSG(YAPI_INVALID_ARGUMENT, "Path not found");
    return "";
}


static const char*  yapiJsonValueParseArray(yJsonStateMachine *j, const char *path, int *result, char *errmsg)
{

    int len = 0;
    const char *p = path;
    char buffer[16];
    int index, count = 0;
    yJsonState array_type;

    while (*p && *p != '|'){
        p++;
        len++;
    }
    YASSERT(len < 16);
    memcpy(buffer, path, len);
    buffer[len] = 0;
    index = atoi(buffer);

    if (yJsonParse(j) != YJSON_PARSE_AVAIL) {
        *result = YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid JSON array");
        return "";
    }

    array_type = j->st;
    if (j->st != YJSON_PARSE_STRUCT) {
#ifdef DEBUG_JSON_PARSE
        dbglog("debug %s %s(%d):%s\n", j->token, yJsonStateStr[j->st], j->st, j->token);
#endif
        *result = YERRMSG(YAPI_NOT_SUPPORTED, "Unsupported JSON array");
        return "";
    }
    do {
        if (index == count) {
                return yapiJsonValueParseStruct(j, ++p, result, errmsg);
        } else {
#ifdef DEBUG_JSON_PARSE
            dbglog("skip  %s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
#endif
            yJsonParse(j);
            do {
#ifdef DEBUG_JSON_PARSE
                dbglog("... %s(%d):%s\n", yJsonStateStr[j->st], j->st, j->token);
#endif
                yJsonSkip(j, 1);
            } while (yJsonParse(j) == YJSON_PARSE_AVAIL && j->st != array_type);
        }
        count++;
    } while (yJsonParse(j) == YJSON_PARSE_AVAIL);

    *result = YERRMSG(YAPI_INVALID_ARGUMENT, "Path not found");
    return "";
}


static int  yapiJsonDecodeString_internal(const char *json_string, char *output)
{
    yJsonStateMachine j;
    char *p = output;
    int maxsize = YSTRLEN(json_string);

    j.src = json_string;
    j.end = j.src + maxsize;
    j.st = YJSON_START;
    yJsonParse(&j);
    do {
        int len = YSTRLEN(j.token);
        yMemcpy(p, j.token, len);
        p += len;
    } while (j.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j) == YJSON_PARSE_AVAIL);
    *p = 0;

    return (u32)(p - output);
}




int yapiJsonGetPath_internal(const char *path, const char *json_data, int json_size, int withHTTPheader, const char **output, char *errmsg)
{
    yJsonStateMachine j;
    int result;

    j.src = json_data;
    j.end = j.src + json_size;
    if (withHTTPheader) {
        j.st = YJSON_HTTP_START;
        if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_CODE) {
            return YERRMSG(YAPI_IO_ERROR, "Failed to parse HTTP header");
        }
        if (YSTRCMP(j.token, "200")) {
            return YERRMSG(YAPI_IO_ERROR, "Unexpected HTTP return code");
        }
        if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_MSG) {
            return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
        }
    }else {
        j.st = YJSON_START;
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        *output = "";
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Not a JSON struct");
    }

     *output = yapiJsonValueParseStruct(&j, path, &result, errmsg);
    return result;
}


typedef struct _fullAttrInfo {
    char        func[32];
    char        attr[32];
    char        value[256];
} fullAttrInfo;



static fullAttrInfo* parseSettings(const char *settings, int *count)
{
    yJsonStateMachine j;
    int               nbAttr=0,allocAttr=0;
    fullAttrInfo     *attrBuff=NULL;
    char              func[32];
    char              attr[32];

    // Parse HTTP header
    j.src = settings;
    j.end = j.src + YSTRLEN(settings);
    j.st = YJSON_START;
    if(yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        nbAttr = -1;
        goto exit;
    }
    while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
        YSTRCPY(func,32,j.token);
        if (YSTRCMP(j.token, "services") == 0){
            yJsonSkip(&j, 1);
        } else{
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
                nbAttr = -1;
                goto exit;
            }
            while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
                YSTRCPY(attr, 32, j.token);
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                    nbAttr = -1;
                    goto exit;
                }

                if (j.st != YJSON_PARSE_STRUCT) {
                    if (nbAttr == allocAttr) {
                        //grow the buffer
                        fullAttrInfo     *tmp = attrBuff;
                        if (allocAttr){
                            allocAttr *= 2;
                        } else {
                            allocAttr = 64;
                        }
                        attrBuff = yMalloc(allocAttr * sizeof(fullAttrInfo));
                        if (tmp) {
                            memcpy(attrBuff, tmp, nbAttr * sizeof(fullAttrInfo));
                            yFree(tmp);
                        }
                    }
                    YSTRCPY(attrBuff[nbAttr].func, 32, func);
                    YSTRCPY(attrBuff[nbAttr].attr, 32, attr);
                    YSPRINTF(attrBuff[nbAttr].value, 256, "%s", j.token);
                    while (j.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j) == YJSON_PARSE_AVAIL) {
                        YSTRCAT(attrBuff[nbAttr].value, 256, j.token);
                    }
                    nbAttr++;
                } else {
                    do {
                        yJsonParse(&j);
                    } while (j.st != YJSON_PARSE_STRUCT);
                }
            }
            if (j.st != YJSON_PARSE_STRUCT){
                nbAttr = -1;
                goto exit;
            }
        }
    }
    if(j.st != YJSON_PARSE_STRUCT) {
        nbAttr = -1;
        goto exit;
    }
exit:
    *count = nbAttr;
    if (nbAttr < 0 && attrBuff) {
        yFree(attrBuff);
        attrBuff = NULL;
    }
    return attrBuff;
}



static YRETCODE  yapiGetAllJsonKeys_internal(const char *json_buffer, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    fullAttrInfo    *attrs;
    int             attrs_count;
    int             j, totalsize=0;
    int             len;
    const char      *sep = "";

    attrs = parseSettings(json_buffer, &attrs_count);
    if (!attrs) {
        return YERR(YAPI_IO_ERROR);
    }

    if (buffersize < 16) {
        return YERRMSG(YAPI_INVALID_ARGUMENT, "buffer too small");;
    }

    buffer[0] = '[';
    totalsize++;

    for (j = 0; j < attrs_count; j++) {
        char tmpbuf[1024];
        char *p, *d;
        len = YSPRINTF(tmpbuf, 1024, "%s\"%s/%s=", sep, attrs[j].func, attrs[j].attr);
        if (len < 0) {
            yFree(attrs);
            return YERR(YAPI_IO_ERROR);
        }
        p = attrs[j].value;
        d = tmpbuf + len;
        while (*p && len < 1020) {
            if (*p == '"' || *p == '\\') {
                *d++ = '\\';
                len++;
            }
            *d++ = *p++;
            len++;

        }
        *d = 0;
        YSTRCAT(d, 1024-len, "\"");
        len ++;
        YASSERT(len == YSTRLEN(tmpbuf));
        sep = ",";
        if (buffersize > totalsize+len) {
            memcpy(buffer + totalsize, tmpbuf, len);
        }
        totalsize += len;
    }

    if (buffersize > totalsize ) {
        buffer[totalsize] = ']';
    }
    totalsize++;
    *fullsize = totalsize;
    yFree(attrs);
    return YAPI_SUCCESS;
}

#endif

void yapiRegisterRawNotificationCb(yRawNotificationCb callback)
{
    if(!yContext)
        return;

    yEnterCriticalSection(&yContext->enum_cs);
    yContext->rawNotificationCb = callback;
    yLeaveCriticalSection(&yContext->enum_cs);
}

void yapiRegisterRawReportCb(yRawReportCb callback)
{
    if(!yContext)
        return;

    yEnterCriticalSection(&yContext->enum_cs);
    yContext->rawReportCb = callback;
    yLeaveCriticalSection(&yContext->enum_cs);
}

void yapiRegisterRawReportV2Cb(yRawReportV2Cb callback)
{
    if(!yContext)
        return;

    yEnterCriticalSection(&yContext->enum_cs);
    yContext->rawReportV2Cb = callback;
    yLeaveCriticalSection(&yContext->enum_cs);
}


//#define YDLL_TRACE_FILE "dll_trace.txt"

#ifdef YDLL_TRACE_FILE

typedef enum
{
    trcInitAPI = 0,
    trcFreeAPI,
    trcRegisterLogFunction,
    trcRegisterDeviceLogCallback,
    trcStartStopDeviceLogCallback,
    trcRegisterDeviceArrivalCallback,
    trcRegisterDeviceRemovalCallback,
    trcRegisterDeviceChangeCallback,
    trcRegisterFunctionUpdateCallback,
    trcRegisterTimedReportCallback,
    trcLockFunctionCallBack,
    trcUnlockFunctionCallBack,
    trcLockDeviceCallBack,
    trcUnlockDeviceCallBack,
    trcTestHub,
    trcRegisterHub,
    trcPreregisterHub,
    trcUnregisterHub,
    trcUpdateDeviceList,
    trcHandleEvents,
    trcSleep,
    trcCheckLogicalName,
    trcGetAPIVersion,
    trcSetTraceFile,
    trcGetDevice,
    trcGetAllDevices,
    trcGetDeviceInfo,
    trcGetDevicePath,
    trcGetDevicePathEx,
    trcGetFunction,
    trcGetFunctionsByClass,
    trcGetFunctionsByDevice,
    trcGetFunctionInfo,
    trcGetFunctionInfoEx,
    trcHTTPRequestSyncStartEx,
    trcHTTPRequestSyncStart,
    trcHTTPRequestSyncStartOutOfBand,
    trcHTTPRequestSyncDone,
    trcHTTPRequestAsyncEx,
    trcHTTPRequestAsync,
    trcHTTPRequestAsyncOutOfBand,
    trcHTTPRequest,
    trcRegisterHubDiscoveryCallback,
    trcTriggerHubDiscovery,
    trcGetBootloaders,
    trcJsonDecodeString,
    trcJsonGetPath,
    trcGetAllJsonKeys,
    trcCheckFirmware,
    trcUpdateFirmware,
    trcUpdateFirmwareEx,
    trcGetSubdevices,
    trcGetMem,
    trcFreeMem,
    trcGetSubDevcies
} TRC_FUN;

static const char * trc_funname[] =
{
    "initApi",
    "freeApi",
    "RegLog",
    "RegDeviceLog",
    "StartStopDevLog",
    "RegDeviceArrival",
    "RegDeviceRemoval",
    "RegDeviceChange",
    "RegUpdateCallback",
    "RegTimedCallback",
    "LockFunCback",
    "UnLockFunCback",
    "LockDeviceCback",
    "UnLockDeviceCback",
    "TestHub",
    "RegHub",
    "PreRegHub",
    "UnRegHub",
    "UpDL",
    "HE",
    "Sl",
    "CheckLName",
    "GetAPIVersion",
    "SetTrcFile",
    "GDev",
    "GAllDev",
    "GDevInfo",
    "GDevPath",
    "GDevPathEx",
    "GFun",
    "GFunByClass",
    "GFunByDev",
    "GFunInfo",
    "GFunInfoEx",
    "ReqSyncStartEx",
    "ReqSyncStart",
    "ReqSyncStartOB",
    "ReqSyncDone",
    "ReqAsyncEx",
    "ReqAsync",
    "ReqAsyncOB",
    "Req",
    "RegHubDiscovery",
    "THubDiscov",
    "GBoot",
    "JsonDecStr",
    "JsonGetPath",
    "GAllJsonK",
    "CkFw",
    "UpFw",
    "UpFwEx",
    "GetSubdev",
    "getmem",
    "freemem",
    "getsubdev"
};

static const char *dlltracefile = YDLL_TRACE_FILE;

static void write_line(const char *ptr, int len)
{
    FILE *f;
    if (YFOPEN(&f,dlltracefile,"a+") != 0) {
        return;
    }
    fwrite(ptr,1,len,f);
    fclose(f);

}

static u64 trc_basetime = 0;

static void trace_dll(u64 t, char prefix, TRC_FUN trcfun, const char *action)
{
    char buffer[512];
    if (trc_basetime == 0) {
        trc_basetime = t;
    }
    int len = YSPRINTF(buffer, 512, "%"FMTu64"%c%s%s\n", (t-trc_basetime), prefix, trc_funname[trcfun], action);
    write_line(buffer, len);
}

#define YDLL_CALL_ENTER(funname)    TRC_FUN trcfun = funname;\
                                    u64 dll_start, dll_stop;\
                                    char dbg_msg[128];\
                                    dll_start = yapiGetTickCount();\
                                    trace_dll(dll_start, '>' ,trcfun, "");

#define YDLL_CALL_LEAVEVOID()        dll_stop = yapiGetTickCount();\
                                            YSPRINTF(dbg_msg, 128, ":%"FMTs64, (dll_stop - dll_start)); \
                                            trace_dll(dll_start, '<' ,trcfun, dbg_msg);

#define YDLL_CALL_LEAVE(value)     dll_stop = yapiGetTickCount();\
                                            YSPRINTF(dbg_msg, 128, ":%d=%"FMTx64, (dll_stop - dll_start), (int)(value), (u64)(value)); \
                                            trace_dll(dll_start, '<' ,trcfun, dbg_msg);
#define YDLL_CALL_LEAVEPTR(value)     dll_stop = yapiGetTickCount();\
                                            YSPRINTF(dbg_msg, 128, ":%p", (dll_stop - dll_start), (value)); \
                                            trace_dll(dll_start, '<' ,trcfun, dbg_msg);
#else
#define YDLL_CALL_ENTER(funname)
#define YDLL_CALL_LEAVEVOID()
#define YDLL_CALL_LEAVE(value)
#define YDLL_CALL_LEAVEPTR(value)
#endif


YRETCODE YAPI_FUNCTION_EXPORT yapiInitAPI(int detect_type, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcInitAPI);
    res = yapiInitAPI_internal(detect_type, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

void YAPI_FUNCTION_EXPORT yapiFreeAPI(void)
{
    YDLL_CALL_ENTER(trcFreeAPI);
    yapiFreeAPI_internal();
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterLogFunction(yapiLogFunction logfun)
{
    YDLL_CALL_ENTER(trcRegisterLogFunction);
    yapiRegisterLogFunction_internal(logfun);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterDeviceLogCallback(yapiDeviceLogCallback logCallback)
{
    YDLL_CALL_ENTER(trcRegisterDeviceLogCallback);
    yapiRegisterDeviceLogCallback_internal(logCallback);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiStartStopDeviceLogCallback(const char *serial, int start)
{
    YDLL_CALL_ENTER(trcStartStopDeviceLogCallback);
    yapiStartStopDeviceLogCallback_internal(serial, start);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterDeviceArrivalCallback(yapiDeviceUpdateCallback arrivalCallback)
{
    YDLL_CALL_ENTER(trcRegisterDeviceArrivalCallback);
    yapiRegisterDeviceArrivalCallback_internal(arrivalCallback);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterDeviceRemovalCallback(yapiDeviceUpdateCallback removalCallback)
{
    YDLL_CALL_ENTER(trcRegisterDeviceRemovalCallback);
    yapiRegisterDeviceRemovalCallback_internal(removalCallback);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterDeviceChangeCallback(yapiDeviceUpdateCallback changeCallback)
{
    YDLL_CALL_ENTER(trcRegisterDeviceChangeCallback);
    yapiRegisterDeviceChangeCallback_internal(changeCallback);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterFunctionUpdateCallback(yapiFunctionUpdateCallback updateCallback)
{
    YDLL_CALL_ENTER(trcRegisterFunctionUpdateCallback);
    yapiRegisterFunctionUpdateCallback_internal(updateCallback);
    YDLL_CALL_LEAVEVOID();
}

void YAPI_FUNCTION_EXPORT yapiRegisterTimedReportCallback(yapiTimedReportCallback timedReportCallback)
{
    YDLL_CALL_ENTER(trcRegisterTimedReportCallback);
    yapiRegisterTimedReportCallback_internal(timedReportCallback);
    YDLL_CALL_LEAVEVOID();
}

YRETCODE YAPI_FUNCTION_EXPORT yapiLockFunctionCallBack(char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcLockFunctionCallBack);
    res = yapiLockFunctionCallBack_internal(errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiUnlockFunctionCallBack(char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcUnlockFunctionCallBack);
    res = yapiUnlockFunctionCallBack_internal(errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiLockDeviceCallBack(char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcLockDeviceCallBack);
    res = yapiLockDeviceCallBack_internal(errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiUnlockDeviceCallBack(char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcUnlockDeviceCallBack);
    res = yapiUnlockDeviceCallBack_internal(errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiTestHub(const char *url, int mstimeout, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcTestHub);
    res = yapiTestHub_internal(url, mstimeout, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiRegisterHub(const char *url, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcRegisterHub);
    res = yapiRegisterHub_internal(url, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiPreregisterHub(const char *url, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcPreregisterHub);
    res = yapiPreregisterHub_internal(url, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

void YAPI_FUNCTION_EXPORT yapiUnregisterHub(const char *url)
{
    YDLL_CALL_ENTER(trcUnregisterHub);
    yapiUnregisterHub_internal(url);
    YDLL_CALL_LEAVEVOID();
}

YRETCODE YAPI_FUNCTION_EXPORT yapiUpdateDeviceList(u32 forceupdate, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcUpdateDeviceList);
    res = yapiUpdateDeviceList_internal(forceupdate, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiHandleEvents(char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHandleEvents);
    res = yapiHandleEvents_internal(errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiSleep(int ms_duration, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcSleep);
    res = yapiSleep_internal(ms_duration, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

int YAPI_FUNCTION_EXPORT yapiCheckLogicalName(const char *name)
{
    int res;
    YDLL_CALL_ENTER(trcCheckLogicalName);
    res = yapiCheckLogicalName_internal(name);
    YDLL_CALL_LEAVE(res);
    return res;
}
u16 YAPI_FUNCTION_EXPORT yapiGetAPIVersion(const char **version, const char **apidate)
{
    u16 res;
    YDLL_CALL_ENTER(trcGetAPIVersion);
    res = yapiGetAPIVersion_internal(version, apidate);
    YDLL_CALL_LEAVE(res);
    return res;
}

void YAPI_FUNCTION_EXPORT yapiSetTraceFile(const char *file)
{
    YDLL_CALL_ENTER(trcSetTraceFile);
    yapiSetTraceFile_internal(file);
    YDLL_CALL_LEAVEVOID();
}

YAPI_DEVICE YAPI_FUNCTION_EXPORT yapiGetDevice(const char *device_str, char *errmsg)
{
    YAPI_DEVICE res;
    YDLL_CALL_ENTER(trcGetDevice);
    res = yapiGetDevice_internal(device_str, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

int YAPI_FUNCTION_EXPORT yapiGetAllDevices(YAPI_DEVICE *buffer, int maxsize, int *neededsize, char *errmsg)
{
    int res;
    YDLL_CALL_ENTER(trcGetAllDevices);
    res = yapiGetAllDevices_internal(buffer, maxsize, neededsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiGetDeviceInfo(YAPI_DEVICE devdesc, yDeviceSt *infos, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetDeviceInfo);
    res = yapiGetDeviceInfo_internal(devdesc, infos, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiGetDevicePath(YAPI_DEVICE devdesc, char *rootdevice, char *request, int requestsize, int *neededsize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetDevicePath);
    res = yapiGetDevicePath_internal(devdesc, rootdevice, request, requestsize, neededsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiGetDevicePathEx(const char * serial, char *rootdevice, char *request, int requestsize, int *neededsize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetDevicePathEx);
    res = yapiGetDevicePathEx_internal(serial, rootdevice, request, requestsize, neededsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YAPI_FUNCTION YAPI_FUNCTION_EXPORT yapiGetFunction(const char *class_str, const char *function_str,char *errmsg)
{
    YAPI_FUNCTION res;
    YDLL_CALL_ENTER(trcGetFunction);
    res = yapiGetFunction_internal(class_str, function_str, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

int YAPI_FUNCTION_EXPORT yapiGetFunctionsByClass(const char *class_str, YAPI_FUNCTION prevfundesc,
                                YAPI_FUNCTION *buffer, int maxsize, int *neededsize, char *errmsg)
{
    int res;
    YDLL_CALL_ENTER(trcGetFunctionsByClass);
    res = yapiGetFunctionsByClass_internal(class_str, prevfundesc, buffer, maxsize, neededsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

int YAPI_FUNCTION_EXPORT yapiGetFunctionsByDevice(YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
                                YAPI_FUNCTION *buffer, int maxsize, int *neededsize, char *errmsg)
{
    int res;
    YDLL_CALL_ENTER(trcGetFunctionsByDevice);
    res = yapiGetFunctionsByDevice_internal(devdesc, prevfundesc, buffer, maxsize, neededsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiGetFunctionInfo(YAPI_FUNCTION fundesc, YAPI_DEVICE *devdesc, char *serial, char *funcId, char *funcName, char *funcVal, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetFunctionInfo);
    res = yapiGetFunctionInfoEx_internal(fundesc, devdesc, serial, funcId, NULL, funcName, funcVal, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}


YRETCODE YAPI_FUNCTION_EXPORT yapiGetFunctionInfoEx(YAPI_FUNCTION fundesc, YAPI_DEVICE *devdesc, char *serial, char *funcId, char *baseType, char *funcName, char *funcVal, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetFunctionInfoEx);
    res = yapiGetFunctionInfoEx_internal(fundesc, devdesc, serial, funcId, baseType, funcName, funcVal, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}


YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncStartEx(YIOHDL *iohdl, const char *device, const char *request, int requestsize, char **reply, int *replysize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestSyncStartEx);
    res = yapiHTTPRequestSyncStartEx_internal(iohdl, 0, device, request, requestsize, reply, replysize, NULL, NULL, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncStart(YIOHDL *iohdl, const char *device, const char *request, char **reply, int *replysize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestSyncStart);
    res = yapiHTTPRequestSyncStartEx_internal(iohdl, 0, device, request, YSTRLEN(request), reply, replysize, NULL, NULL, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}


YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncStartOutOfBand(YIOHDL *iohdl, int channel, const char *device, const char *request, int requestsize, char **reply, int *replysize, yapiRequestProgressCallback progress_cb, void *progress_ctx, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestSyncStartOutOfBand);
    res = yapiHTTPRequestSyncStartEx_internal(iohdl, channel, device, request, requestsize, reply, replysize, progress_cb, progress_ctx, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}


YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncDone(YIOHDL *iohdl, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestSyncDone);
    res = yapiHTTPRequestSyncDone_internal(iohdl, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestAsyncEx(const char *device, const char *request, int len, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestAsyncEx);
    res = yapiHTTPRequestAsyncEx_internal(0, device, request, len, callback, context, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestAsync(const char *device, const char *request, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestAsync);
    res = yapiHTTPRequestAsyncEx_internal(0, device, request, YSTRLEN(request), callback, context, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestAsyncOutOfBand(int channel, const char *device, const char *request, int requestsize, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcHTTPRequestAsyncOutOfBand);
    res = yapiHTTPRequestAsyncEx_internal(channel, device, request, YSTRLEN(request), callback, context, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}


int YAPI_FUNCTION_EXPORT yapiHTTPRequest(const char *device, const char *request, char* buffer,int buffsize, int *fullsize, char *errmsg)
{
    int res;
    YDLL_CALL_ENTER(trcHTTPRequest);
    res = yapiHTTPRequest_internal(device, request, buffer, buffsize, fullsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

void YAPI_FUNCTION_EXPORT yapiRegisterHubDiscoveryCallback(yapiHubDiscoveryCallback hubDiscoveryCallback)
{
    YDLL_CALL_ENTER(trcRegisterHubDiscoveryCallback);
    yapiRegisterHubDiscoveryCallback_internal(hubDiscoveryCallback);
    YDLL_CALL_LEAVEVOID();
}

YRETCODE YAPI_FUNCTION_EXPORT yapiTriggerHubDiscovery(char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcTriggerHubDiscovery);
    res = yapiTriggerHubDiscovery_internal(errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiGetBootloaders(char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetBootloaders);
    res = yapiGetBootloaders_internal(buffer, buffersize, fullsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}
#ifndef YAPI_IN_YDEVICE

int YAPI_FUNCTION_EXPORT yapiJsonDecodeString(const char *json_string, char *output)
{
    int res;
    YDLL_CALL_ENTER(trcJsonDecodeString);
    res = yapiJsonDecodeString_internal(json_string, output);
    YDLL_CALL_LEAVE(res);
    return res;
}
int YAPI_FUNCTION_EXPORT yapiJsonGetPath(const char *path, const char *json_data, int json_size, const char  **result, char *errmsg)
{
    int res;
    char *tmp;
    YDLL_CALL_ENTER(trcJsonGetPath);
    res = yapiJsonGetPath_internal(path, json_data, json_size, 0, result, errmsg);
    YDLL_CALL_LEAVE(res);
    if (res > 0) {
        tmp = yMalloc(res);
        memcpy(tmp, *result, res);
        *result = tmp;
    }
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiGetAllJsonKeys(const char *json_buffer, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetAllJsonKeys);
    res = yapiGetAllJsonKeys_internal(json_buffer, buffer, buffersize, fullsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YAPI_FUNCTION_EXPORT void* yapiGetMem(int size)
{
    void *res;
    YDLL_CALL_ENTER(trcGetMem);
    res = yMalloc(size);
    YDLL_CALL_LEAVEPTR(res);
    return res;
}

YAPI_FUNCTION_EXPORT void yapiFreeMem(void *ptr)
{
    YDLL_CALL_ENTER(trcFreeMem);
    yFree(ptr);
    YDLL_CALL_LEAVEVOID()
}
YRETCODE YAPI_FUNCTION_EXPORT yapiCheckFirmware(const char *serial, const char *rev, const char *path, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcCheckFirmware);
    res = yapiCheckFirmware_internal(serial, rev, 0, path, buffer, buffersize, fullsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiUpdateFirmware(const char *serial, const char *firmwarePath, const char *settings, int startUpdate, char *msg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcUpdateFirmware);
    res = yapiUpdateFirmware_internal(serial, firmwarePath, settings, 0, startUpdate, msg);
    YDLL_CALL_LEAVE(res);
    return res;
}

YRETCODE YAPI_FUNCTION_EXPORT yapiUpdateFirmwareEx(const char *serial, const char *firmwarePath, const char *settings, int force, int startUpdate, char *msg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcUpdateFirmwareEx);
    res = yapiUpdateFirmware_internal(serial, firmwarePath, settings, force, startUpdate, msg);
    YDLL_CALL_LEAVE(res);
    return res;
}


YRETCODE YAPI_FUNCTION_EXPORT yapiGetSubdevices(const char *serial, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    YRETCODE res;
    YDLL_CALL_ENTER(trcGetSubdevices);
    res = yapiGetSubdevices_internal(serial, buffer, buffersize, fullsize, errmsg);
    YDLL_CALL_LEAVE(res);
    return res;
}

#endif

/*****************************************************************************
 Same function but defined with stdcall
 ****************************************************************************/
#if defined(WINDOWS_API) && defined(__32BITS__) && defined(_MSC_VER)

//typedef void YAPI_FUNCTION_EXPORT(_stdcall *vb6_yapiLogFunction)(BSTR log, u32 loglen);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiDeviceUpdateCallback)(YAPI_DEVICE devdescr);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiFunctionUpdateCallback)(YAPI_FUNCTION fundescr, BSTR value);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiTimedReportCallback)(YAPI_FUNCTION fundesc, double timestamp, const u8 *bytes, u32 len);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiHubDiscoveryCallback)(BSTR serial, BSTR url);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiDeviceLogCallback)(YAPI_DEVICE devdescr, BSTR line);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiRequestAsyncCallback)(void *context, int retcode, BSTR result, u32 resultlen);
typedef void YAPI_FUNCTION_EXPORT (_stdcall *vb6_yapiLogFunction)(BSTR log, u32 loglen);


typedef struct vb6_callback {
    vb6_yapiRequestAsyncCallback    callback;
    void                            *context;
} vb_callback_cache_entry;

static vb_callback_cache_entry vb_callback_cache[NB_MAX_DEVICES];


void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiStartStopDeviceLogCallback(const char *serial,int start);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiInitAPI(int type,char *errmsg);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiFreeAPI(void);
//void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterLogFunction(vb6_yapiLogFunction logfun);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceLogCallback(vb6_yapiDeviceLogCallback logCallback);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceArrivalCallback(vb6_yapiDeviceUpdateCallback arrivalCallback);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceRemovalCallback(vb6_yapiDeviceUpdateCallback removalCallback);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceChangeCallback(vb6_yapiDeviceUpdateCallback changeCallback);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterFunctionUpdateCallback(vb6_yapiFunctionUpdateCallback updateCallback);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterTimedReportCallback(vb6_yapiTimedReportCallback timedReportCallback);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiLockFunctionCallBack( char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUnlockFunctionCallBack(char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiLockDeviceCallBack( char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUnlockDeviceCallBack(char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterHub(const char *rooturl, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiPreregisterHub(const char *rooturl, char *errmsg);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUnregisterHub(const char *url);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUpdateDeviceList(u32 forceupdate, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHandleEvents(char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiSleep(int duration_ms, char *errmsg);
u64 YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetTickCount(void);
int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiCheckLogicalName(const char *name);
u16 YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetAPIVersion(BSTR *version, BSTR *apidate);
u16 YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetAPIVersionEx(char *version, char *apidate);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiSetTraceFile(const char *file);
YAPI_DEVICE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetDevice(const char *device_str,char *errmsg);
int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetAllDevices(YAPI_DEVICE *buffer,int maxsize,int *neededsize,char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetDeviceInfo(YAPI_DEVICE devdesc,yDeviceSt *infos,char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetDevicePath(YAPI_DEVICE devdesc, char *rootdevice, char *path, int pathsize, int *neededsize, char *errmsg);
YAPI_FUNCTION YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunction(const char *class_str, const char *function_str,char *errmsg);
int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunctionsByClass(const char *class_str, YAPI_FUNCTION prevfundesc, YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg);
int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunctionsByDevice(YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc, YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunctionInfo(YAPI_FUNCTION fundesc,YAPI_DEVICE *devdesc,char *serial,char *funcId,char *funcName,char *funcVal,char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestSyncStartEx(YIOHDL *iohdl, const char *device, const char *request, int requestsize, char **reply, int *replysize, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestSyncStart(YIOHDL *iohdl, const char *device, const char *request, char **reply, int *replysize, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestSyncDone(YIOHDL *iohdl, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestAsync(const char *device, const char *request, vb6_yapiRequestAsyncCallback callback, void *context, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestAsyncEx(const char *device, const char *request, int requestsize, vb6_yapiRequestAsyncCallback callback, void *context, char *errmsg);
int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequest(const char *device, const char *request, char* buffer,int buffsize,int *fullsize, char *errmsg);
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterHubDiscoveryCallback(vb6_yapiHubDiscoveryCallback hubDiscoveryCallback);
YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiTriggerHubDiscovery(char *errmsg);

static BSTR vb6_version = NULL;
static BSTR vb6_apidate = NULL;


void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiStartStopDeviceLogCallback(const char *serial,int start)
{
    yapiStartStopDeviceLogCallback(serial, start);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiInitAPI(int type,char *errmsg)
{
    return yapiInitAPI(type, errmsg);
}

void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiFreeAPI(void)
{
    yapiFreeAPI();
    if (vb6_version) {
        SysFreeString(vb6_version);
        vb6_version = NULL;
    }
    if (vb6_apidate) {
        SysFreeString(vb6_apidate);
        vb6_apidate = NULL;
    }
}

void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterLogFunction(vb6_yapiLogFunction logfun);

static BSTR newBSTR(const char * str)
{
    int len = YSTRLEN(str);
    int newlen = MultiByteToWideChar(CP_ACP, 0, str, len, NULL, 0);
    BSTR newstring = SysAllocStringLen(0, newlen);
    MultiByteToWideChar(CP_ACP, 0, str, len, newstring, newlen);
    return newstring;
}

// stdcall implementation of yapiRegisterLogFunction
static vb6_yapiLogFunction vb6_yapiLogFunctionFWD = NULL;
void yapiLogFunctionCdeclToStd(const char *log, u32 loglen)
{

    if (vb6_yapiLogFunctionFWD) {
        BSTR bstrstr = newBSTR(log);
        vb6_yapiLogFunctionFWD(bstrstr, loglen);
        SysFreeString(bstrstr);
    }
}
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterLogFunction(vb6_yapiLogFunction logfun)
{
    vb6_yapiLogFunctionFWD = logfun;
    if (logfun) {
        yapiRegisterLogFunction(yapiLogFunctionCdeclToStd);
    } else {
        yapiRegisterLogFunction(NULL);
    }
}


static vb6_yapiDeviceLogCallback vb6_yapiRegisterDeviceLogCallbackFWD = NULL;
void yapiRegisterDeviceLogCallbackFWD(YAPI_DEVICE devdescr, const char *line)
{
    if (vb6_yapiRegisterDeviceLogCallbackFWD) {
        BSTR bstrstr = newBSTR(line);
        vb6_yapiRegisterDeviceLogCallbackFWD(devdescr, bstrstr);
        SysFreeString(bstrstr);
    }
}
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceLogCallback(vb6_yapiDeviceLogCallback logCallback)
{
    vb6_yapiRegisterDeviceLogCallbackFWD = logCallback;
    if (logCallback) {
        yapiRegisterDeviceLogCallback(yapiRegisterDeviceLogCallbackFWD);
    } else {
        yapiRegisterDeviceLogCallback(NULL);
    }
}

static vb6_yapiDeviceUpdateCallback vb6_yapiRegisterDeviceArrivalCallbackFWD = NULL;
void yapiRegisterDeviceArrivalCallbackFWD(YAPI_DEVICE devdescr)
{
    if (vb6_yapiRegisterDeviceArrivalCallbackFWD)
        vb6_yapiRegisterDeviceArrivalCallbackFWD(devdescr);
}
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceArrivalCallback(vb6_yapiDeviceUpdateCallback arrivalCallback)
{
    vb6_yapiRegisterDeviceArrivalCallbackFWD = arrivalCallback;
    if (arrivalCallback) {
        yapiRegisterDeviceArrivalCallback(yapiRegisterDeviceArrivalCallbackFWD);
    } else {
        yapiRegisterDeviceArrivalCallback(NULL);
    }
}

static vb6_yapiDeviceUpdateCallback vb6_yapiRegisterDeviceRemovalCallbackFWD = NULL;
void yapiRegisterDeviceRemovalCallbackFWD(YAPI_DEVICE devdescr)
{
    if (vb6_yapiRegisterDeviceRemovalCallbackFWD)
        vb6_yapiRegisterDeviceRemovalCallbackFWD(devdescr);
}

void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceRemovalCallback(vb6_yapiDeviceUpdateCallback removalCallback)
{
    vb6_yapiRegisterDeviceRemovalCallbackFWD = removalCallback;
    if (removalCallback) {
        yapiRegisterDeviceRemovalCallback(yapiRegisterDeviceRemovalCallbackFWD);
    } else {
        yapiRegisterDeviceRemovalCallback(NULL);
    }
}

static vb6_yapiDeviceUpdateCallback vb6_yapiRegisterDeviceChangeCallbackFWD = NULL;
void yapiRegisterDeviceChangeCallbackFWD(YAPI_DEVICE devdescr)
{
    if (vb6_yapiRegisterDeviceChangeCallbackFWD)
        vb6_yapiRegisterDeviceChangeCallbackFWD(devdescr);
}
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterDeviceChangeCallback(vb6_yapiDeviceUpdateCallback changeCallback)
{
    vb6_yapiRegisterDeviceChangeCallbackFWD = changeCallback;
    if (changeCallback) {
        yapiRegisterDeviceChangeCallback(yapiRegisterDeviceChangeCallbackFWD);
    } else {
        yapiRegisterDeviceChangeCallback(NULL);
    }
}

static vb6_yapiFunctionUpdateCallback vb6_yapiRegisterFunctionUpdateCallbackFWD = NULL;
void yapiRegisterFunctionUpdateCallbackFWD(YAPI_FUNCTION fundescr, const char *value)
{
    if (vb6_yapiRegisterFunctionUpdateCallbackFWD && value) {
        BSTR bstrstr = newBSTR(value);
        vb6_yapiRegisterFunctionUpdateCallbackFWD(fundescr, bstrstr);
        SysFreeString(bstrstr);
    }
}
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterFunctionUpdateCallback(vb6_yapiFunctionUpdateCallback updateCallback)
{
    vb6_yapiRegisterFunctionUpdateCallbackFWD = updateCallback;
    if (updateCallback) {
        yapiRegisterFunctionUpdateCallback(yapiRegisterFunctionUpdateCallbackFWD);
    } else {
        yapiRegisterFunctionUpdateCallback(NULL);
    }
}

static vb6_yapiTimedReportCallback vb6_yapiRegisterTimedReportCallbackFWD = NULL;
void yapiRegisterTimedReportCallbackFWD(YAPI_FUNCTION fundesc, double timestamp, const u8 *bytes, u32 len)
{
    if (vb6_yapiRegisterTimedReportCallbackFWD)
        vb6_yapiRegisterTimedReportCallbackFWD(fundesc, timestamp, bytes, len);
}

void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterTimedReportCallback(vb6_yapiTimedReportCallback timedReportCallback)
{
    vb6_yapiRegisterTimedReportCallbackFWD = timedReportCallback;
    if (timedReportCallback) {
        yapiRegisterTimedReportCallback(yapiRegisterTimedReportCallbackFWD);
    } else {
        yapiRegisterTimedReportCallback(NULL);
    }
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiLockFunctionCallBack(char *errmsg)
{
    return yapiLockFunctionCallBack(errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUnlockFunctionCallBack(char *errmsg)
{
    return yapiUnlockFunctionCallBack(errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiLockDeviceCallBack(char *errmsg)
{
    return yapiLockDeviceCallBack(errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUnlockDeviceCallBack(char *errmsg)
{
    return yapiUnlockDeviceCallBack(errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterHub(const char *rooturl, char *errmsg)
{
    return yapiRegisterHub(rooturl, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiPreregisterHub(const char *rooturl, char *errmsg)
{
    return yapiPreregisterHub(rooturl, errmsg);
}

void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUnregisterHub(const char *url)
{
    yapiUnregisterHub(url);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiUpdateDeviceList(u32 forceupdate, char *errmsg)
{
    return yapiUpdateDeviceList(forceupdate, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHandleEvents(char *errmsg)
{
    return yapiHandleEvents(errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiSleep(int duration_ms, char *errmsg)
{
    return yapiSleep(duration_ms, errmsg);
}

u64 YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetTickCount(void)
{
    return yapiGetTickCount();
}

int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiCheckLogicalName(const char *name)
{
    return yapiCheckLogicalName(name);
}

u16 YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetAPIVersion(BSTR *version, BSTR *apidate)
{
    const char * versionA, *apidateA;
    u16 res = yapiGetAPIVersion(&versionA, &apidateA);
    *version = newBSTR(versionA);
    *apidate = newBSTR(apidateA);
    return res;
}

u16 YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetAPIVersionEx(char *version, char *apidate)
{
    const char * versionA, *apidateA;
    u16 res = yapiGetAPIVersion(&versionA, &apidateA);
    YSTRCPY(version, YOCTO_ERRMSG_LEN, versionA);
    YSTRCPY(apidate, YOCTO_ERRMSG_LEN, apidateA);
    return res;
}


void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiSetTraceFile(const char *file)
{
    yapiSetTraceFile(file);
}

YAPI_DEVICE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetDevice(const char *device_str,char *errmsg)
{
    return yapiGetDevice(device_str, errmsg);
}

int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetAllDevices(YAPI_DEVICE *buffer, int maxsize, int *neededsize, char *errmsg)
{
    return yapiGetAllDevices(buffer, maxsize, neededsize, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetDeviceInfo(YAPI_DEVICE devdesc, yDeviceSt *infos, char *errmsg)
{
    return yapiGetDeviceInfo(devdesc, infos, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetDevicePath(YAPI_DEVICE devdesc, char *rootdevice, char *path, int pathsize, int *neededsize, char *errmsg)
{
    return yapiGetDevicePath(devdesc, rootdevice, path, pathsize, neededsize, errmsg);
}

YAPI_FUNCTION YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunction(const char *class_str, const char *function_str,char *errmsg)
{
    return yapiGetFunction(class_str, function_str, errmsg);
}

int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunctionsByClass(const char *class_str, YAPI_FUNCTION prevfundesc, YAPI_FUNCTION *buffer, int maxsize, int *neededsize, char *errmsg)
{
    return yapiGetFunctionsByClass(class_str, prevfundesc, buffer, maxsize, neededsize, errmsg);
}

int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunctionsByDevice(YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc, YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg)
{
    return yapiGetFunctionsByDevice(devdesc, prevfundesc, buffer, maxsize, neededsize, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiGetFunctionInfo(YAPI_FUNCTION fundesc,YAPI_DEVICE *devdesc,char *serial,char *funcId,char *funcName,char *funcVal,char *errmsg)
{
    return yapiGetFunctionInfo(fundesc, devdesc, serial, funcId, funcName, funcVal, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestSyncStartEx(YIOHDL *iohdl, const char *device, const char *request, int requestsize, char **reply, int *replysize, char *errmsg)
{
    return yapiHTTPRequestSyncStartEx(iohdl, device, request, requestsize, reply, replysize, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestSyncStart(YIOHDL *iohdl, const char *device, const char *request, char **reply, int *replysize, char *errmsg)
{
    return yapiHTTPRequestSyncStart(iohdl, device, request, reply, replysize, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestSyncDone(YIOHDL *iohdl, char *errmsg)
{
    return yapiHTTPRequestSyncDone(iohdl, errmsg);
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestAsync(const char *device, const char *request, vb6_yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    return vb6_yapiHTTPRequestAsyncEx(device, request, YSTRLEN(request), callback, context, errmsg);
}

static void vb6_callback_fwd(void *context, const u8 *result, u32 resultlen, int retcode, const char *errmsg)
{
    YAPI_DEVICE devydx = (YAPI_DEVICE) context;
    void *vb6_context = vb_callback_cache[devydx].context;
    BSTR bstrstr = newBSTR((char*) result);
    vb_callback_cache[devydx].callback(vb6_context, retcode,  bstrstr, resultlen);
    SysFreeString(bstrstr);
}


YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequestAsyncEx(const char *device, const char *request, int requestsize, vb6_yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    YAPI_DEVICE devydx = wpSearch(device);
    vb_callback_cache[devydx].callback = callback;
    vb_callback_cache[devydx].context = context;
    return yapiHTTPRequestAsyncEx(device, request, requestsize, vb6_callback_fwd, (void*)devydx, errmsg);
}

int YAPI_FUNCTION_EXPORT __stdcall vb6_yapiHTTPRequest(const char *device, const char *request, char* buffer, int buffsize, int *fullsize, char *errmsg)
{
    return yapiHTTPRequest(device, request, buffer, buffsize, fullsize, errmsg);
}

static vb6_yapiHubDiscoveryCallback vb6_yapiHubDiscoveryCallbackFWD = NULL;
void yapiHubDiscoverCdeclToStdllbackFWD(const char *serial, const char *url)
{
    if (vb6_yapiHubDiscoveryCallbackFWD) {
        BSTR bstrserial = newBSTR(serial);
        BSTR bstrurl = newBSTR(url);
        vb6_yapiHubDiscoveryCallbackFWD(bstrserial, bstrurl);
        SysFreeString(bstrurl);
        SysFreeString(bstrserial);
    }
}
void YAPI_FUNCTION_EXPORT __stdcall vb6_yapiRegisterHubDiscoveryCallback(vb6_yapiHubDiscoveryCallback hubDiscoveryCallback)
{
    vb6_yapiHubDiscoveryCallbackFWD = hubDiscoveryCallback;
    if (hubDiscoveryCallback){
        yapiRegisterHubDiscoveryCallback(yapiHubDiscoverCdeclToStdllbackFWD);
    } else {
        yapiRegisterHubDiscoveryCallback(NULL);
    }
}

YRETCODE YAPI_FUNCTION_EXPORT __stdcall vb6_yapiTriggerHubDiscovery(char *errmsg)
{
    return yapiTriggerHubDiscovery(errmsg);
}


#endif

