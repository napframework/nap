/*********************************************************************
 *
 * $Id: yhash.c 29756 2018-01-25 23:15:28Z seb $
 *
 * Simple hash tables and device/function information store
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

#define __FILE_ID__  "yhash"
#include "yhash.h"
#include <string.h>

#ifdef MICROCHIP_API
__eds__ __attribute__((far, __section__(".yfar1"))) YHashSlot yHashTable[NB_MAX_HASH_ENTRIES];
#include <Yocto/yapi_ext.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include "yproto.h"
#ifdef WINDOWS_API
#include <Windows.h>
#endif
#define __eds__
static YHashSlot  yHashTable[NB_MAX_HASH_ENTRIES];
yCRITICAL_SECTION yHashMutex;
yCRITICAL_SECTION yFreeMutex;
yCRITICAL_SECTION yWpMutex;
yCRITICAL_SECTION yYpMutex;
#endif

//#define DEBUG_YHASH
#ifdef DEBUG_YHASH
#define HLOGF(x)             dbglog x;
#else
#define HLOGF(x)
#endif

#ifndef MICROCHIP_API
static u16 usedDevYdx[NB_MAX_DEVICES/16];
static u16 nextDevYdx = 0;
#endif
static u8  nextCatYdx = 1;
static u16 nextHashEntry = 256;

static yBlkHdl devYdxPtr[NB_MAX_DEVICES];
static yBlkHdl funYdxPtr[NB_MAX_DEVICES];

#ifndef MICROCHIP_API
char SerialNumberStr[YOCTO_SERIAL_LEN] = "";
#endif
yStrRef SerialRef = INVALID_HASH_IDX;

yBlkHdl yWpListHead = INVALID_BLK_HDL;
yBlkHdl yYpListHead = INVALID_BLK_HDL;

// =======================================================================
//   Small block (16 bytes) allocator, for white pages and yellow pages
// =======================================================================

#define BLK(hdl)    (yHashTable[(hdl)>>1].blk[(hdl)&1])
#define WP(hdl)     (BLK(hdl).wpEntry)
#define YC(hdl)     (BLK(hdl).ypCateg)
#define YP(hdl)     (BLK(hdl).ypEntry)
#define YA(hdl)     (BLK(hdl).ypArray)

yBlkHdl freeBlks = INVALID_BLK_HDL;

static yBlkHdl yBlkAlloc(void)
{
    yBlkHdl  res;

    yEnterCriticalSection(&yFreeMutex);
    if(freeBlks != INVALID_BLK_HDL) {
        res = freeBlks;
        freeBlks = BLK(freeBlks).nextPtr;
    } else {
        yEnterCriticalSection(&yHashMutex);
        YASSERT(nextHashEntry < NB_MAX_HASH_ENTRIES);
        res = ((nextHashEntry++) << 1) + 1;
        yLeaveCriticalSection(&yHashMutex);
        BLK(res).blkId = 0;
        BLK(res).nextPtr = INVALID_BLK_HDL;
        freeBlks = res--;
        HLOGF(("yBlkAlloc() uses bucket 0x%x\n",nextHashEntry));
    }
    HLOGF(("yBlkAlloc() returns blkHdl 0x%x\n",res));
    BLK(res).blkId = 0;
    BLK(res).nextPtr = INVALID_BLK_HDL;
    yLeaveCriticalSection(&yFreeMutex);

    return res;
}

static void yBlkFree(yBlkHdl hdl)
{
    HLOGF(("Free blkHdl 0x%x\n",hdl));
    yEnterCriticalSection(&yFreeMutex);
    BLK(hdl).ydx = 0;
    BLK(hdl).blkId = 0;
    BLK(hdl).nextPtr = freeBlks;
    freeBlks = hdl;
    yLeaveCriticalSection(&yFreeMutex);
}

u16 yBlkListLength(yBlkHdl hdl)
{
    u16     res = 0;

    while(hdl != INVALID_BLK_HDL) {
        res++;
        hdl = BLK(hdl).nextPtr;
    }
    return res;
}

yBlkHdl yBlkListSeek(yBlkHdl hdl, u16 pos)
{
    while(hdl != INVALID_BLK_HDL && pos > 0) {
        hdl = BLK(hdl).nextPtr;
        pos--;
    }
    return hdl;
}

// =======================================================================
//   Tiny Hash table support
// =======================================================================

static u16 fletcher16(const u8 *data, u16 len, u16 virtlen)
{
    u16 sum1 = 0xff, sum2 = 0xff - len, plen = 0;

    // process data
    while (len > 0) {
        u16 tlen = len > 21 ? 21 : len;
        len -= tlen;
        plen += tlen;
        do {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    // process zero-padding
    plen = virtlen - plen;
    while(plen > 0) {
        u16 tlen = plen > 21 ? 21 : plen;
        plen -= tlen;
        sum2 += sum1 * tlen;
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return ((sum1 & 0xff) << 8) | (sum2 & 0xff);
}

void yHashInit(void)
{
    yStrRef empty, Module, module, HubPort,Sensor;
    u16     i;

    HLOGF(("yHashInit\n"));
    for(i = 0; i < 256; i++)
        yHashTable[i].next = 0;
    for(i = 0; i < NB_MAX_DEVICES; i++)
        devYdxPtr[i] = INVALID_BLK_HDL;
    for(i = 0; i < NB_MAX_DEVICES; i++)
        funYdxPtr[i] = INVALID_BLK_HDL;
#ifndef MICROCHIP_API
    memset((u8 *)usedDevYdx, 0, sizeof(usedDevYdx));
    yInitializeCriticalSection(&yHashMutex);
    yInitializeCriticalSection(&yFreeMutex);
    yInitializeCriticalSection(&yWpMutex);
    yInitializeCriticalSection(&yYpMutex);
#endif

    // Always init hast table with empty string and Module string
    // This ensures they always get the same magic hash value
    empty  = yHashPutStr("");
    Module = yHashPutStr("Module");
    module = yHashPutStr("module");
    HubPort = yHashPutStr("HubPort");
    Sensor = yHashPutStr("Sensor");
    if(empty != YSTRREF_EMPTY_STRING ||
       Module != YSTRREF_MODULE_STRING ||
       module != YSTRREF_mODULE_STRING ||
       HubPort != YSTRREF_HUBPORT_STRING ||
       Sensor != YSTRREF_SENSOR_STRING) {
        // This should never ever happen, something is really weird
        // No log is possible here (called within yapiInitAPI), so
        // the best we can do to help debugging is a tight loop.
#ifdef MICROCHIP_API
		while(1);
#else
		YPANIC;
#endif
	}
    SerialRef = yHashPutStr(SerialNumberStr);

    yYpListHead = yBlkAlloc();
    YC(yYpListHead).catYdx  = 0;
    YC(yYpListHead).blkId   = YBLKID_YPCATEG;
    YC(yYpListHead).name    = YSTRREF_MODULE_STRING;
    YC(yYpListHead).entries = INVALID_BLK_HDL;
}

#ifndef MICROCHIP_API
void yHashFree(void)
{
    HLOGF(("yHashFree\n"));
    yDeleteCriticalSection(&yHashMutex);
    yDeleteCriticalSection(&yFreeMutex);
    yDeleteCriticalSection(&yWpMutex);
    yDeleteCriticalSection(&yYpMutex);
}
#endif

static yHash yHashPut(const u8 *buf, u16 len, u8 testonly)
{
    u16     hash,i;
    yHash   yhash, prevhash = INVALID_HASH_IDX;
    __eds__ u8 *p;

    hash = fletcher16(buf, len, HASH_BUF_SIZE);
    yhash = hash & 0xff;

    yEnterCriticalSection(&yHashMutex);

    if(yHashTable[yhash].next != 0) {
        // first entry is allocated, search chain
        do {
            if(yHashTable[yhash].hash == hash) {
                // hash match, perform exact comparison
                p = yHashTable[yhash].buff;
                for(i = 0; i < len; i++) if(p[i] != buf[i]) break;
                if(i == len) {
                    // data match, verify padding zeroes for a full match
                    while(i < HASH_BUF_SIZE) if(p[i++] != 0) break;
                    if(i == HASH_BUF_SIZE) {
                        // full match
                        HLOGF(("yHash found at 0x%x\n", yhash));
                        goto exit_ok;
                    }
                }
            }
            // not a match, try next entry in chain
            prevhash = yhash;
            yhash = yHashTable[yhash].next;
        } while(yhash != -1);
        // not found in chain
        if(testonly) goto exit_error;
        YASSERT(nextHashEntry < NB_MAX_HASH_ENTRIES);
        yhash = nextHashEntry++;
    } else {
        // first entry not allocated
        if(testonly) {
        exit_error:
            HLOGF(("yHash entry not found\n", yhash));
            yLeaveCriticalSection(&yHashMutex);
            return -1;
        }
    }

    // create new entry
    yHashTable[yhash].hash = hash;
    yHashTable[yhash].next = -1;
    p = yHashTable[yhash].buff;
    for(i = 0; i < len; i++) p[i] = buf[i];
    while(i < HASH_BUF_SIZE) p[i++] = 0;
    if(prevhash != INVALID_HASH_IDX) {
        yHashTable[prevhash].next = yhash;
    }
    HLOGF(("yHash added at 0x%x\n", yhash));

exit_ok:
    yLeaveCriticalSection(&yHashMutex);
    return yhash;
}

yHash yHashPutBuf(const u8 *buf, u16 len)
{
    if(len > HASH_BUF_SIZE) len = HASH_BUF_SIZE;
    return yHashPut(buf, len, 0);
}

yHash yHashPutStr(const char *str)
{
    u16 len = (u16) YSTRLEN(str);

    if(len > HASH_BUF_SIZE) len = HASH_BUF_SIZE;
    HLOGF(("yHashPutStr('%s'):\n",str));
    return yHashPut((const u8 *)str, len, 0);
}

yHash yHashTestBuf(const u8 *buf, u16 len)
{
    if(len > HASH_BUF_SIZE) len = HASH_BUF_SIZE;
    return yHashPut(buf, len, 1);
}

yHash yHashTestStr(const char *str)
{
    u16 len = (u16) YSTRLEN(str);

    if(len > HASH_BUF_SIZE) len = HASH_BUF_SIZE;
    HLOGF(("yHashTestStr('%s'):\n",str));
    return yHashPut((const u8 *)str, len, 1);
}

void yHashGetBuf(yHash yhash, u8 *destbuf, u16 bufsize)
{
    __eds__ u8 *p;

    HLOGF(("yHashGetBuf(0x%x)\n",yhash));
    YASSERT(yhash >= 0);
#ifdef MICROCHIP_API
    if(yhash >= nextHashEntry || yHashTable[yhash].next == 0) {
        // should never happen !
        memset(destbuf, 0, bufsize);
        return;
    }
#else
    YASSERT(yhash < nextHashEntry);
    YASSERT(yHashTable[yhash].next != 0); // 0 means unallocated, -1 means end of chain
#endif
    if(bufsize > HASH_BUF_SIZE) bufsize = HASH_BUF_SIZE;
    p = yHashTable[yhash].buff;
    while(bufsize-- > 0) {
        *destbuf++ = *p++;
    }
}

void yHashGetStr(yHash yhash, char *destbuf, u16 bufsize)
{
    HLOGF(("yHashGetStr(0x%x):\n",yhash));
    yHashGetBuf(yhash, (u8 *)destbuf, bufsize);
    destbuf[bufsize-1] = 0;
}

#ifdef MICROCHIP_API
// safe since this is a single-thread environment
static char shared_hashbuf[HASH_BUF_SIZE+1];
#endif

u16 yHashGetStrLen(yHash yhash)
{
#ifdef MICROCHIP_API
    u16  i;
#endif

    HLOGF(("yHashGetStrLen(0x%x)\n",yhash));
    YASSERT(yhash >= 0);
#ifdef MICROCHIP_API
    if(yhash >= nextHashEntry || yHashTable[yhash].next == 0) {
        // should never happen
        return 0;
    }
    for(i = 0; i < HASH_BUF_SIZE; i++) {
        if(!yHashTable[yhash].buff[i]) break;
    }
    return i;
#else
    YASSERT(yhash < nextHashEntry);
    YASSERT(yHashTable[yhash].next != 0); // 0 means unallocated
    return (u16) YSTRLEN((char *)yHashTable[yhash].buff);
#endif
}

char *yHashGetStrPtr(yHash yhash)
{
#ifdef MICROCHIP_API
    u16  i;
#endif

    HLOGF(("yHashGetStrPtr(0x%x)\n",yhash));
    YASSERT(yhash >= 0);
    YASSERT(yhash < nextHashEntry);
    YASSERT(yHashTable[yhash].next != 0); // 0 means unallocated
#ifdef MICROCHIP_API
    for(i = 0; i < HASH_BUF_SIZE; i++) {
        char c = yHashTable[yhash].buff[i];
        if(!c) break;
        shared_hashbuf[i] = c;
    }
    shared_hashbuf[i] = 0;
    return shared_hashbuf;
#else
    return (char *)yHashTable[yhash].buff;
#endif
}

#ifndef MICROCHIP_API

/**
* parse the rootUrl string and update the path field of the yAbsUrl structure passed
in argument
*/
static int yComputeRelPath(yAbsUrl* absurl, const char* rootUrl, u8 testonly)
{
    int i, len;
    while (*rootUrl == '/') rootUrl++;
    for (i = 0; i < YMAX_HUB_URL_DEEP && *rootUrl;) {
        for (len = 0; rootUrl[len] && rootUrl[len] != '/'; len++);
        if ((len != 8 || memcmp(rootUrl, "bySerial", 8) != 0) &&
            (len != 3 || memcmp(rootUrl, "api", 3) != 0)) {
            absurl->path[i] = yHashPut((const u8 *)rootUrl, len, testonly);
            if (absurl->path[i] == INVALID_HASH_IDX) return -1;
            i++;
        }
        rootUrl += len;
        while (*rootUrl == '/') rootUrl++;
    }
    if (*rootUrl && testonly) return -1;
    return 0;
}

/**
* create and register a new url with the host form urlref and the path from
rootUrl.
return: the yUrlRef for the new created url
*/
yUrlRef yHashUrlFromRef(yUrlRef urlref, const char* rootUrl)
{
    yAbsUrl huburl;

    // set all path as invalid
    HLOGF(("yHashUrlFromRef('%s')\n", rootUrl));
    yHashGetBuf(urlref, (u8 *)&huburl, sizeof(huburl));
    memset(huburl.path, 0xff, sizeof(huburl.path));

    if (yComputeRelPath(&huburl, rootUrl, 0) < 0) {
        return INVALID_HASH_IDX;
    }
    return yHashPut((const u8 *)&huburl, sizeof(huburl), 0);
}


/**
 * if testonly is non zero it do not add the hash if the hash has not been already recorded
 * in this case we return INVALID_HASH_IDX
 */
yUrlRef yHashUrl(const char *url, const char *rootUrl, u8 testonly, char *errmsg)
{
    yAbsUrl huburl;
    int len,hostlen, domlen,iptest=0;
    const char *end, *p;
    const char *pos,*posplus;
    const char *host=NULL;
    char buffer[8];

    // set all hash as invalid
    HLOGF(("yHashUrl('%s','%s')\n",url,rootUrl));
    memset(&huburl, 0xff, sizeof(huburl));
    huburl.proto = PROTO_AUTO;
    if(*url) {
        if (YSTRNCMP(url, "http://", 7) == 0) {
            url += 7;
            huburl.proto = PROTO_HTTP;
        } else if (YSTRNCMP(url, "ws://", 5) == 0) {
            url += 5;
            huburl.proto = PROTO_WEBSOCKET;
        }
        // search for any authentication info
        for (p = url; *p && *p != '@' && *p != '/'; p++);
        if (*p == '@') {
            for (p = url; *p != ':' && *p != '@'; p++);
            if (*p != ':') {
                if (errmsg) YSTRCPY(errmsg, YOCTO_ERRMSG_LEN, "missing authentication parameter");
                return INVALID_HASH_IDX;
            }
            len = (int)(p - url);
            if (len > HASH_BUF_SIZE) {
                if (errmsg) YSTRCPY(errmsg, YOCTO_ERRMSG_LEN, "username too long");
                return INVALID_HASH_IDX;
            }
            huburl.user = yHashPutBuf((const u8*)url, len);
            HLOGF(("user=%s\n", yHashGetStrPtr(huburl.user)));
            url = ++p;
            while (*p != '@') p++;
            len = (int)(p - url);
            if (len > HASH_BUF_SIZE) {
                if (errmsg) YSTRCPY(errmsg, YOCTO_ERRMSG_LEN, "password too long");
                return INVALID_HASH_IDX;
            }
            huburl.password = yHashPutBuf((const u8*)url, len);
            HLOGF(("passwd=%s\n", yHashGetStrPtr(huburl.password)));
            url = ++p;
        }
        end = strchr(url,'/');
        if (end) {
            p = posplus= end + 1;
            while (*p  && *p != '/') p++;
            len = (int)(p - posplus);
            if (len > 0) {
                if (len > HASH_BUF_SIZE) {
                    if (errmsg) YSTRCPY(errmsg, YOCTO_ERRMSG_LEN, "subdomain too long");
                    return INVALID_HASH_IDX;
                }
                huburl.subdomain = yHashPutBuf((const u8*)posplus, len);
                HLOGF(("subdomain=%s\n", yHashGetStrPtr(huburl.subdomain)));
            }
        }else {
            end = url + strlen(url);
        }
        pos = strchr(url,':');
        posplus = pos + 1;
        if(pos && pos < end ){
            len = (int)(end - posplus);
            if(len > 7){
                if(errmsg) YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,"invalid port");
                return INVALID_HASH_IDX;
            }
            memcpy(buffer,posplus,len);
            buffer[len] = '\0';
            huburl.byip.port = atoi(buffer);
            end =pos;
        }else{
            huburl.byip.port = YOCTO_DEFAULT_PORT;
        }
        HLOGF(("port=%d\n", huburl.byip.port));

        pos = strchr(url,'.');
        posplus = pos + 1;
        if(pos && pos < end ){
            hostlen = (int)(pos - url);
            if(hostlen>HASH_BUF_SIZE){
                if(errmsg) YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,"hostname too long");
                return INVALID_HASH_IDX;
            }
            host = url;
            url = posplus;
        }else{
            hostlen = 0;
        }
        if(hostlen && hostlen <= 3){
            memcpy(buffer, host, hostlen);
            buffer[hostlen] = 0;
            iptest = atoi(buffer);
        }
        if (iptest && iptest< 256 && (end - host) < 16) {
            // this is probably an ip
            huburl.byip.ip = yHashPutBuf((const u8*)host,(u16)(end-host));
            HLOGF(("ip=%s\n", yHashGetStrPtr(huburl.byip.ip)));
        } else {
            domlen = (int)(end - url);
            if (domlen > HASH_BUF_SIZE) {
                if(errmsg) YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,"domain name too long");
                return INVALID_HASH_IDX;
            }
            if (hostlen) {
                huburl.byname.host = yHashPutBuf((const u8*)host, hostlen);
                HLOGF(("host=%s\n", yHashGetStrPtr(huburl.byip.ip)));
            } else {
                huburl.byname.host = INVALID_HASH_IDX;
            }
            huburl.byname.domaine = yHashPutBuf((const u8*)url,domlen);
            HLOGF(("domain(host)=%s\n", yHashGetStrPtr(huburl.byip.ip)));
        }
    }
    if(yComputeRelPath(&huburl, rootUrl, testonly)<0){
        return INVALID_HASH_IDX;
    }
    return yHashPut((const u8 *)&huburl, sizeof(huburl), testonly);
}

// return port , get hash of the url an a pointer to a buffer of YOCTO_HOSTNAME_NAME len
yAsbUrlType  yHashGetUrlPort(yUrlRef urlref, char *url, u16 *port, yAsbUrlProto *proto, yStrRef *user, yStrRef *password, yStrRef *subdomain)
{
    yAbsUrl absurl;

    // set all path as invalid
    yHashGetBuf(urlref, (u8 *)&absurl, sizeof(absurl));
    if (proto) *proto = absurl.proto;
    if (user) *user = absurl.user;
    if (password) *password = absurl.password;
    if (subdomain) *subdomain = absurl.subdomain;

    if(absurl.byusb.invalid1 ==INVALID_HASH_IDX && absurl.byusb.invalid2 == INVALID_HASH_IDX){
        // we have an USB address
        if (url) {
            *url='\0';
        }
        if(port) *port = 0;
        return USB_URL;
    }else if(absurl.byip.invalid == INVALID_HASH_IDX){
        // we have an ip address
        if (url) {
            yHashGetStr(absurl.byip.ip,url,16);
        }
        if (port) *port = absurl.byip.port;
        return IP_URL;
    }else{
        char *p = url;
        if(url) {
            // we have an hostname
            if(absurl.byname.host!= INVALID_HASH_IDX){
                yHashGetStr(absurl.byname.host,p,YOCTO_HOSTNAME_NAME);
                p = url + YSTRLEN(url);
                *p++ = '.';
            }
            yHashGetStr(absurl.byname.domaine,p,(u16)(YOCTO_HOSTNAME_NAME - (p-url)));
        }
        if(port) *port = absurl.byname.port;
        return NAME_URL;
    }
}

int yHashSameHub(yUrlRef url_a, yUrlRef url_b)
{
    yAbsUrl absurl_a;
    yAbsUrl absurl_b;

    // set all path as invalid
    yHashGetBuf(url_a, (u8 *)&absurl_a, sizeof(absurl_a));
    yHashGetBuf(url_b, (u8 *)&absurl_b, sizeof(absurl_b));
    if (absurl_a.byname.domaine == absurl_b.byname.domaine &&
        absurl_a.byname.host == absurl_b.byname.host &&
        absurl_a.byname.port == absurl_b.byname.port)
        return 1;
    return 0;
}

#endif

// Return a hash-encoded URL for a local USB/YSB device
yUrlRef yHashUrlUSB(yHash serial)
{
    yAbsUrl huburl;
    // set all hash as invalid
    memset(&huburl, 0xff, sizeof(huburl));
    huburl.proto = PROTO_AUTO;
    // for USB we store only the serial number since
    // we access all devices directly
    huburl.byusb.serial = serial;
    return yHashPut((const u8 *)&huburl, sizeof(huburl), 0);
}

// Return a hash-encoded URL for our local /api
yUrlRef yHashUrlAPI(void)
{
    yAbsUrl huburl;
    // set all hash as invalid
    memset(&huburl, 0xff, sizeof(huburl));
    huburl.proto = PROTO_AUTO;
    return yHashPut((const u8 *)&huburl, sizeof(huburl), 0);
}

// =======================================================================
//   White pages support
// =======================================================================

static void ypUnregister(yStrRef serial);//forward declaration

static int wpLockCount = 0;
static int wpSomethingUnregistered = 0;

static void wpExecuteUnregisterUnsec(void)
{
    yBlkHdl  prev = INVALID_BLK_HDL, next;
    yBlkHdl  hdl, funHdl, nextHdl;
    // Note: do not use u16 below, because of GCC optimization bug
    //       which does not properly handle u16->u64 extension on OSX
    unsigned devYdx;

    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        next = WP(hdl).nextPtr;
        if(WP(hdl).flags & YWP_MARK_FOR_UNREGISTER) {
#ifdef  DEBUG_WP
            {
                char host[YOCTO_HOSTNAME_NAME];
                u16  port;
                yAsbUrlType type = yHashGetUrlPort( WP(hdl).url,host,&port);
                switch(type){
                case USB_URL:
                    dbglog("WP: unregister %s(0x%X) form USB\n",yHashGetStrPtr(WP(hdl).serial),WP(hdl).serial);
                    break;
                default:
                    dbglog("WP: unregister %s(0x%X) from %s:%u\n",yHashGetStrPtr(WP(hdl).serial),WP(hdl).serial,host,port);
                }
            }
#endif


            // first remove YP entry
            ypUnregister(WP(hdl).serial);
            // entry mark as to remove
            if(prev == INVALID_BLK_HDL) {
                yWpListHead = next;
            } else {
                WP(prev).nextPtr = next;
            }
            devYdx = WP(hdl).devYdx;
            funHdl = funYdxPtr[devYdx];
            while(funHdl != INVALID_BLK_HDL) {
                YASSERT(YA(funHdl).blkId == YBLKID_YPARRAY);
                nextHdl = YA(funHdl).nextPtr;
                yBlkFree(funHdl);
                funHdl = nextHdl;
            }
            funYdxPtr[devYdx] = INVALID_BLK_HDL;
            devYdxPtr[devYdx] = INVALID_BLK_HDL;
#ifndef MICROCHIP_API
            if((unsigned) nextDevYdx > devYdx) {
                nextDevYdx = devYdx;
            }
            usedDevYdx[devYdx>>4] &= ~ (u16)(1 << (devYdx&15));
            //dbglog("wpUnregister serial=%X devYdx=%d (next=%d)\n", WP(hdl).serial, devYdx, nextDevYdx);
            freeDevYdxInfos(devYdx);
#endif
            yBlkFree(hdl);
        } else {
            prev = hdl;
        }
        hdl = next;
    }
}

#ifndef DEBUG_WP_LOCK

void wpPreventUnregisterEx(void)
{
    yEnterCriticalSection(&yWpMutex);
    YASSERT(wpLockCount < 128);
    wpLockCount++;
    yLeaveCriticalSection(&yWpMutex);
}

void wpAllowUnregisterEx(void)
{
    yEnterCriticalSection(&yWpMutex);
    YASSERT(wpLockCount > 0);
    wpLockCount--;
    if(wpSomethingUnregistered && !wpLockCount) {
        wpExecuteUnregisterUnsec();
        wpSomethingUnregistered = 0;
    }
    yLeaveCriticalSection(&yWpMutex);
}

#else

void wpPreventUnregisterDbg(const char *file, u32 line)
{
    yEnterCriticalSection(&yWpMutex);
    dbglog("wpPreventUnregisterDbg: %s:%d\n",file,line);
    YASSERT(wpLockCount < 128);
    wpLockCount++;
    yLeaveCriticalSection(&yWpMutex);
}

void wpAllowUnregisterDbg(const char *file, u32 line)
{
    yEnterCriticalSection(&yWpMutex);
    dbglog("wpAllowUnregisterDbg: %s:%d\n",file,line);
    YASSERT(wpLockCount > 0);
    wpLockCount--;
    if(wpSomethingUnregistered && !wpLockCount) {
        wpExecuteUnregisterUnsec();
    }
    yLeaveCriticalSection(&yWpMutex);
}

#endif

// return :
//      0 -> no change
//      1 -> update
//      2 -> first register

int wpRegister(int devYdx, yStrRef serial, yStrRef logicalName, yStrRef productName, u16 productId, yUrlRef devUrl, s8 beacon)
{
    yBlkHdl  prev = INVALID_BLK_HDL;
    yBlkHdl  hdl;
    int      changed=0;

    yEnterCriticalSection(&yWpMutex);

    YASSERT(devUrl != INVALID_HASH_IDX);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).serial == serial) break;
        prev = hdl;
        hdl = WP(prev).nextPtr;
    }
    if(hdl == INVALID_BLK_HDL) {
        hdl = yBlkAlloc();
        changed = 2;
#ifndef MICROCHIP_API
        if(devYdx == -1) devYdx = nextDevYdx;
        YASSERT(!(usedDevYdx[devYdx>>4] & (1 << (devYdx&15))));
        usedDevYdx[devYdx>>4] |= 1 << (devYdx&15);
        if(nextDevYdx == devYdx) {
            nextDevYdx++;
            while(usedDevYdx[nextDevYdx>>4] & (1 << (nextDevYdx&15))) {
                if(nextDevYdx >= NB_MAX_DEVICES) break;
                nextDevYdx++;
            }
        }
        //dbglog("wpRegister serial=%X devYdx=%d\n", serial, devYdx);
        initDevYdxInfos(devYdx,serial);
#endif
        YASSERT(devYdx < NB_MAX_DEVICES);
        devYdxPtr[devYdx] = hdl;
        WP(hdl).devYdx  = (u8)devYdx;
        WP(hdl).blkId   = YBLKID_WPENTRY;
        WP(hdl).serial  = serial;
        WP(hdl).name    = YSTRREF_EMPTY_STRING;
        WP(hdl).product = YSTRREF_EMPTY_STRING;
        WP(hdl).url     = devUrl;
        WP(hdl).devid   = 0;
        WP(hdl).flags   = 0;
        if(prev == INVALID_BLK_HDL) {
            yWpListHead = hdl;
        } else {
            WP(prev).nextPtr = hdl;
        }
#ifdef MICROCHIP_API
    } else if(devYdx != -1 && WP(hdl).devYdx != devYdx) {
        // allow change of devYdx based on hub role
        u16 oldDevYdx = WP(hdl).devYdx;
        if(oldDevYdx < NB_MAX_DEVICES) {
            funYdxPtr[devYdx] = funYdxPtr[oldDevYdx];
            funYdxPtr[oldDevYdx] = INVALID_BLK_HDL;
            devYdxPtr[devYdx] = hdl;
        }
        devYdxPtr[oldDevYdx] = INVALID_BLK_HDL;
        WP(hdl).devYdx  = (u8)devYdx;
#endif
    }
    if(logicalName != INVALID_HASH_IDX)  {
        if(WP(hdl).name != logicalName){
            if(changed==0) changed=1;
            WP(hdl).name = logicalName;
        }
    }
    if(productName != INVALID_HASH_IDX) WP(hdl).product = productName;
    if(productId != 0)                  WP(hdl).devid   = productId;
    WP(hdl).url     = devUrl;
    if(beacon >= 0) {
        WP(hdl).flags = (beacon > 0 ? YWP_BEACON_ON : 0);
    } else {
        WP(hdl).flags &= ~YWP_MARK_FOR_UNREGISTER;
    }

#ifdef DEBUG_WP
    {
        char host[YOCTO_HOSTNAME_NAME];
        u16  port;
        yAsbUrlType type = yHashGetUrlPort(devUrl,host,&port);
        switch(type){
        case USB_URL:
            dbglog("WP: regiser %s(0x%X) form USB (res=%d)\n",yHashGetStrPtr(serial),serial,changed);
            break;
        default:
            dbglog("WP: regiser %s(0x%X) from %s:%u (res=%d)\n",yHashGetStrPtr(serial),serial,host,port,changed);
        }
    }
#endif

    yLeaveCriticalSection(&yWpMutex);
    return changed;
}



yStrRef wpGetAttribute(yBlkHdl hdl, yWPAttribute attridx)
{
    yStrRef res = YSTRREF_EMPTY_STRING;

    yEnterCriticalSection(&yWpMutex);
    if(WP(hdl).blkId == YBLKID_WPENTRY) {
        switch(attridx) {
        case Y_WP_SERIALNUMBER: res = WP(hdl).serial; break;
        case Y_WP_LOGICALNAME:  res = WP(hdl).name; break;
        case Y_WP_PRODUCTNAME:  res = WP(hdl).product; break;
        case Y_WP_PRODUCTID:    res = WP(hdl).devid; break;
        case Y_WP_NETWORKURL:   res = WP(hdl).url; break;
        case Y_WP_BEACON:       res = (WP(hdl).flags & YWP_BEACON_ON ? 1 : 0); break;
        case Y_WP_INDEX:        res = WP(hdl).devYdx; break;
        }
    }
    yLeaveCriticalSection(&yWpMutex);

    return res;
}

void wpGetSerial(yBlkHdl hdl, char *serial)
{
    yEnterCriticalSection(&yWpMutex);
    if(WP(hdl).blkId == YBLKID_WPENTRY) {
        yHashGetStr(WP(hdl).serial, serial, YOCTO_SERIAL_LEN);
    }
    yLeaveCriticalSection(&yWpMutex);
}

void wpGetLogicalName(yBlkHdl hdl, char *logicalName)
{
    yEnterCriticalSection(&yWpMutex);
    if(WP(hdl).blkId == YBLKID_WPENTRY) {
        yHashGetStr(WP(hdl).name, logicalName, YOCTO_LOGICAL_LEN);
    }
    yLeaveCriticalSection(&yWpMutex);
}

int wpMarkForUnregister(yStrRef serial)
{
    yBlkHdl  next;
    yBlkHdl  hdl;
    int      retval=0;
    yEnterCriticalSection(&yWpMutex);

    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        next = WP(hdl).nextPtr;
        if(WP(hdl).serial == serial) {
            if( (WP(hdl).flags & YWP_MARK_FOR_UNREGISTER)==0 ) {
                WP(hdl).flags |= YWP_MARK_FOR_UNREGISTER;
                wpSomethingUnregistered = 1;
                retval = 1;
            }
            break;
        }
        hdl = next;
    }

#ifdef  DEBUG_WP
    {
        char host[YOCTO_HOSTNAME_NAME];
        u16  port;
            if (retval) {
                yAsbUrlType type = yHashGetUrlPort( WP(hdl).url,host,&port);
            switch(type){
            case USB_URL:
                dbglog("WP: mark for unregister %s(0x%X) form USB\n",yHashGetStrPtr(serial),serial);
                break;
            default:
                dbglog("WP: mark for unregister %s(0x%X) from %s:%u\n",yHashGetStrPtr(serial),serial,host,port);
            }
        }else{
            dbglog("WP: mark for unregister %s(0x%X) witch is unregistred!\n",yHashGetStrPtr(serial),serial);
        }
    }
#endif

    yLeaveCriticalSection(&yWpMutex);
    return retval;
}

u16 wpEntryCount(void)
{
    return yBlkListLength(yWpListHead);
}

int wpGetDevYdx(yStrRef serial)
{
    yBlkHdl hdl;
    int     res = -1;

    yEnterCriticalSection(&yWpMutex);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).serial == serial) {
            res = WP(hdl).devYdx;
            break;
        }
        hdl = WP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yWpMutex);

    return res;
}

YAPI_DEVICE wpSearchEx(yStrRef strref)
{
    yBlkHdl hdl,byname;
    YAPI_DEVICE res = -1;

    byname = INVALID_BLK_HDL;

    yEnterCriticalSection(&yWpMutex);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).serial == strref) {
            res = strref;
            break;
        }
        if(WP(hdl).name == strref) byname = hdl;
        hdl = WP(hdl).nextPtr;
    }
    if(hdl == INVALID_BLK_HDL && byname != INVALID_BLK_HDL) {
        res = WP(byname).serial;
    }
    yLeaveCriticalSection(&yWpMutex);

    return res;
}

YAPI_DEVICE wpSearch(const char *device_str)
{
    yStrRef strref = yHashTestStr(device_str);
    if (strref == INVALID_HASH_IDX)
        return -1;
    return wpSearchEx(strref);
}


YAPI_DEVICE wpSearchByNameHash(yStrRef strref)
{
    yBlkHdl hdl;
    YAPI_DEVICE res = -1;

    if(strref == INVALID_HASH_IDX)
        return -1;

    yEnterCriticalSection(&yWpMutex);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).name == strref) {
            res = WP(hdl).serial;
            break;
        }
        hdl = WP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yWpMutex);

    return res;
}

#ifndef MICROCHIP_API

YAPI_DEVICE wpSearchByUrl(const char *host, const char *rootUrl)
{
    yStrRef apiref;
    yBlkHdl hdl;
    YAPI_DEVICE res = -1;

    apiref = yHashUrl(host, rootUrl, 1,NULL);
    if(apiref == INVALID_HASH_IDX) return -1;

    yEnterCriticalSection(&yWpMutex);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).url == apiref) {
            res = WP(hdl).serial;
            break;
        }
        hdl = WP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yWpMutex);

    return res;
}

int wpGetAllDevUsingHubUrl( yUrlRef hubUrl, yStrRef *buffer,int sizeInStrRef)
{
    yBlkHdl hdl;
    int     count=0;
    yAbsUrl hubAbsUrl;
    yHashGetBuf(hubUrl, (u8 *)&hubAbsUrl, sizeof(hubAbsUrl));

    yEnterCriticalSection(&yWpMutex);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        yAbsUrl absurl;
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        yHashGetBuf(WP(hdl).url, (u8 *)&absurl, sizeof(absurl));
        if( absurl.byname.domaine == hubAbsUrl.byname.domaine &&
            absurl.byname.host == hubAbsUrl.byname.host &&
            absurl.byname.port == hubAbsUrl.byname.port ) {
            if(sizeInStrRef){
                *buffer++ = WP(hdl).serial;
                sizeInStrRef--;
            }
            count++;
        }
        hdl = WP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yWpMutex);

    return count;
}


yUrlRef wpGetDeviceUrlRef(YAPI_DEVICE devdesc)
{
    yBlkHdl  hdl;
    yUrlRef  urlref = INVALID_HASH_IDX;

    yEnterCriticalSection(&yWpMutex);

    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).serial == (u16)devdesc) {
            urlref=WP(hdl).url;
            break;
        }
        hdl = WP(hdl).nextPtr;
    }

    yLeaveCriticalSection(&yWpMutex);

    return urlref;
}

int wpGetDeviceUrl(YAPI_DEVICE devdesc, char *roothubserial, char *request, int requestsize, int *neededsize)
{
    yBlkHdl  hdl;
    yUrlRef  hubref = INVALID_HASH_IDX;
    yStrRef  strref = INVALID_HASH_IDX;
    yAbsUrl  absurl,huburl;
    char     serial[YOCTO_SERIAL_LEN];
    int      fullsize, len,idx;

    yEnterCriticalSection(&yWpMutex);
    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).serial == (u16)devdesc) {
            hubref = WP(hdl).url;
            // store device serial;
            strref = WP(hdl).serial;
            break;
        }
        hdl = WP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yWpMutex);
    if(hubref == INVALID_HASH_IDX)
        return -1;

    yHashGetBuf(hubref, (u8 *)&absurl, sizeof(absurl));
    if(absurl.byusb.invalid1 == INVALID_HASH_IDX && absurl.byusb.invalid2 == INVALID_HASH_IDX) {
        // local device
        strref = absurl.byusb.serial;
        if(strref == 0) strref = devdesc & 0xffff; // ourself
    }else if( absurl.path[0] != INVALID_HASH_IDX){
        // sub device, need to find serial of its root hub
        memcpy(&huburl,&absurl,sizeof(absurl));

        for(idx = 0 ; idx < YMAX_HUB_URL_DEEP && huburl.path[idx] != INVALID_HASH_IDX ; idx++)
            huburl.path[idx] = INVALID_HASH_IDX;
        // search white pages by url
        hubref = yHashTestBuf((u8 *)&huburl, sizeof(huburl));
        strref = INVALID_HASH_IDX;
        yEnterCriticalSection(&yWpMutex);
        hdl = yWpListHead;
        while(hdl != INVALID_BLK_HDL) {
            YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
            if(WP(hdl).url == hubref) {
                strref = WP(hdl).serial;
                break;
            }
            hdl = WP(hdl).nextPtr;
        }
        yLeaveCriticalSection(&yWpMutex);
        if(strref == INVALID_HASH_IDX) return -1;
    }

    // extract root device serial
    if(roothubserial) {
        yHashGetStr(strref, roothubserial, YOCTO_SERIAL_LEN);
    }
    if(!request) requestsize = 0;

    if(absurl.path[0] != INVALID_HASH_IDX){
        if(requestsize > 10) {
            memcpy(request,"/bySerial/",10);
            request+=10;
            requestsize-=10;
        }
        fullsize = 11; // null-terminated slash
    }else{
        if(requestsize > 1) {
            *request++ = '/';
            requestsize--;
        }
        fullsize = 2; // null-terminated slash
    }
    // build relative url
    idx=0;
    while((strref = absurl.path[idx]) != INVALID_HASH_IDX) {
        yHashGetStr(strref, serial, YOCTO_SERIAL_LEN);
        len = (int)strlen(serial)+1;
        fullsize += len;
        if(requestsize>0 && requestsize > len) {
            memcpy(request, serial, len-1);
            request[len-1] = '/';
            request += len;
            requestsize -= len;
        }
        idx++;
    }
    if(neededsize != NULL) *neededsize = fullsize;
    // null-terminate request
    if(requestsize > 0) *request = 0;

    return 0;
}

#endif

int wpGetDeviceInfo(YAPI_DEVICE devdesc, u16 *deviceid, char *productname, char *serial, char *logicalname, u8 *beacon)
{
    yBlkHdl  hdl;

    yEnterCriticalSection(&yWpMutex);

    hdl = yWpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
        if(WP(hdl).serial == (u16)devdesc) {
            // entry found
            if(deviceid)    *deviceid = WP(hdl).devid;
            if(productname) yHashGetStr(WP(hdl).product, productname, YOCTO_PRODUCTNAME_LEN);
            if(serial)      yHashGetStr(WP(hdl).serial, serial, YOCTO_SERIAL_LEN);
            if(logicalname) yHashGetStr(WP(hdl).name, logicalname, YOCTO_LOGICAL_LEN);
            if(beacon)      *beacon = (WP(hdl).flags & YWP_BEACON_ON ? 1 : 0);
            break;
        }
        hdl = WP(hdl).nextPtr;
    }

    yLeaveCriticalSection(&yWpMutex);

    return (hdl != INVALID_BLK_HDL ? 0 : -1);
}



// =======================================================================
//   Yellow pages support
// =======================================================================

// return 1 on change 0 if value are the same as the cache
int ypRegister(yStrRef categ, yStrRef serial, yStrRef funcId, yStrRef funcName, int funClass, int funYdx, const char *funcVal)
{
    yBlkHdl  prev = INVALID_BLK_HDL;
    yBlkHdl  hdl;
    yBlkHdl  cat_hdl;
    yBlkHdl  yahdl;
    u16      i, cnt;
    int      devYdx, changed=0;
    const u16 *funcValWords = (const u16 *)funcVal;

    yEnterCriticalSection(&yYpMutex);

    // locate category node
    hdl = yYpListHead;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(YC(hdl).blkId == YBLKID_YPCATEG);
        if(YC(hdl).name == categ) break;
        prev = hdl;
        hdl = YC(prev).nextPtr;
    }
    if(hdl == INVALID_BLK_HDL) {
        hdl = yBlkAlloc();
        YC(hdl).catYdx  = nextCatYdx++;
        YC(hdl).blkId   = YBLKID_YPCATEG;
        YC(hdl).name    = categ;
        YC(hdl).entries = INVALID_BLK_HDL;
        if(prev == INVALID_BLK_HDL) {
            yYpListHead = hdl;
        } else {
            YC(prev).nextPtr = hdl;
        }
    }
    cat_hdl = hdl;

    // locate entry node
    prev = INVALID_BLK_HDL;
    hdl = YC(cat_hdl).entries;
    while(hdl != INVALID_BLK_HDL) {
        YASSERT(YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND);
        if(YP(hdl).serialNum == serial && YP(hdl).funcId == funcId) break;
        prev = hdl;
        hdl = YP(prev).nextPtr;
    }
    if(hdl == INVALID_BLK_HDL) {
        changed = 1; // new entry-> changed
        hdl = yBlkAlloc();
        if(funClass < 0 || funClass >= YOCTO_N_BASECLASSES) {
            funClass = 0;
        }
        YP(hdl).blkId       = YBLKID_YPENTRY+funClass;
        YP(hdl).serialNum   = serial;
        YP(hdl).funcId      = funcId;
        YP(hdl).funcName    = YSTRREF_EMPTY_STRING;
        YP(hdl).funInfo.raw = 15;
        for(i = 0; i < YOCTO_PUBVAL_SIZE/2; i++) {
            YP(hdl).funcValWords[i] = 0;
        }
        if(prev == INVALID_BLK_HDL) {
            YC(cat_hdl).entries = hdl;
        } else {
            YP(prev).nextPtr = hdl;
        }
    }
    if(funcName != INVALID_HASH_IDX)  {
        if(YP(hdl).funcName != funcName){
            changed=1;
            YP(hdl).funcName = funcName;
        }
    }
    if(categ != YSTRREF_MODULE_STRING) {
        if(funYdx >= 0 && funYdx < 15) {
            YP(hdl).funInfo.raw = funYdx;
        } else {
            funYdx = YP(hdl).funInfo.v2.funydx;
        }
        devYdx = wpGetDevYdx(serial);
        if(devYdx >= 0) {
            cnt = funYdx;
            if(cnt == 255) { // unknown funYdx, prepare to allocate new one
                funYdx = 0;
            }
            prev = INVALID_BLK_HDL;
            yahdl = funYdxPtr[devYdx];
            while(yahdl != INVALID_BLK_HDL) {
                YASSERT(YA(yahdl).blkId == YBLKID_YPARRAY);
                if(cnt < 6) break;
                if(cnt < 255) { // known funYdx
                    cnt -= 6;
                } else {        // unknown funYdx
                    funYdx += 6;
                }
                prev = yahdl;
                yahdl = YA(prev).nextPtr;
            }
            if(cnt == 255) {
                // unknown funYdx, allocate a free bucket
                cnt = 0;
                if(prev != INVALID_BLK_HDL) {
                    for(i = 0; i < 6; i++) {
                        if(YA(prev).entries[i] == INVALID_BLK_HDL) {
                            yahdl = prev;
                            cnt = i;
                            funYdx = funYdx - 6 + i;
                            break;
                        }
                    }
                }
                if (funYdx < 15){
                    YP(hdl).funInfo.raw = funYdx;
                }
            }
            while(yahdl == INVALID_BLK_HDL) {
                yahdl = yBlkAlloc();
                YA(yahdl).blkId = YBLKID_YPARRAY;
                for(i = 0; i < 6; i++) YA(yahdl).entries[i] = INVALID_BLK_HDL;
                if(prev == INVALID_BLK_HDL) {
                    funYdxPtr[devYdx] = yahdl;
                } else {
                    YA(prev).nextPtr = yahdl;
                }
                if(cnt < 6) break;
                cnt -= 6;
                prev = yahdl;
                yahdl = YA(prev).nextPtr;
            }
            YA(yahdl).entries[cnt] = hdl;
        }
        if(funcVal != NULL) {
            for(i = 0; i < YOCTO_PUBVAL_SIZE/2; i++) {
                if(YP(hdl).funcValWords[i] != funcValWords[i]){
                    changed = 1;
                    YP(hdl).funcValWords[i] = funcValWords[i];
                }
            }
        }
    }
    yLeaveCriticalSection(&yYpMutex);
    return changed;
}

// return 1 on change 0 if value are the same as the cache
// WARNING: funcVal MUST BE WORD-ALIGNED
int ypRegisterByYdx(u8 devYdx, Notification_funydx funInfo, const char *funcVal, YAPI_FUNCTION *fundesc)
{
    yBlkHdl  hdl;
    u16      i;
    int      funYdx = funInfo.v2.funydx;
    int      changed=0;
    const u16 *funcValWords = (const u16 *)funcVal;

    yEnterCriticalSection(&yYpMutex);

    // Ignore unknown devYdx
    if(devYdxPtr[devYdx] != INVALID_BLK_HDL) {
        hdl = funYdxPtr[devYdx];
        while(hdl != INVALID_BLK_HDL && funYdx >= 6) {
//          YASSERT(YA(hdl).blkId == YBLKID_YPARRAY);
            if(YA(hdl).blkId != YBLKID_YPARRAY) {
                yLeaveCriticalSection(&yYpMutex);
                return 0; // discard invalid block silently
            }
            hdl = YA(hdl).nextPtr;
            funYdx -= 6;
        }
        // Ignore unknown funYdx
        if(hdl != INVALID_BLK_HDL) {
            YASSERT(YA(hdl).blkId == YBLKID_YPARRAY);
            hdl = YA(hdl).entries[funYdx];
            if(hdl != INVALID_BLK_HDL) {
                YASSERT(YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND);
                if(funcVal) {
                    // apply value change
                    for(i = 0; i < YOCTO_PUBVAL_SIZE/2; i++) {
                        if(YP(hdl).funcValWords[i] != funcValWords[i]) {
                            YP(hdl).funcValWords[i] = funcValWords[i];
                            changed = 1;
                        }
                    }
                    if(YP(hdl).funInfo.raw != funInfo.raw) {
                        YP(hdl).funInfo.raw = funInfo.raw;
                        changed = 1;
                    }
                }
                if(fundesc) {
                    *fundesc = YP(hdl).hwId;
                }
            }
        }
    }

    yLeaveCriticalSection(&yYpMutex);

    return changed;
}

// return -1 on error
// WARNING: funcVal MUST BE WORD-ALIGNED
int     ypGetAttributesByYdx(u8 devYdx, u8 funYdx, yStrRef *serial, yStrRef *logicalName, yStrRef *funcId, yStrRef *funcName, Notification_funydx *funcInfo, char *funcVal)
{
    yBlkHdl  hdl;
    u16      i;
    int      res = -1;
    u16      *funcValWords = (u16 *)funcVal;

    yEnterCriticalSection(&yYpMutex);

    // Ignore unknown devYdx
    if (devYdxPtr[devYdx] != INVALID_BLK_HDL) {
        if (logicalName) {
            hdl = devYdxPtr[devYdx];
            *logicalName = WP(hdl).name;
        }
        hdl = funYdxPtr[devYdx];
        while (hdl != INVALID_BLK_HDL && funYdx >= 6) {
            //          YASSERT(YA(hdl).blkId == YBLKID_YPARRAY);
            if (YA(hdl).blkId != YBLKID_YPARRAY) {
                yLeaveCriticalSection(&yYpMutex);
                return -1; // discard invalid block silently
            }
            hdl = YA(hdl).nextPtr;
            funYdx -= 6;
        }
        // Ignore unknown funYdx
        if (hdl != INVALID_BLK_HDL) {
            YASSERT(YA(hdl).blkId == YBLKID_YPARRAY);
            hdl = YA(hdl).entries[funYdx];
            if (hdl != INVALID_BLK_HDL) {
                YASSERT(YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND);
                if (serial) {
                    *serial = YP(hdl).serialNum;
                }
                if (funcId) {
                    *funcId = YP(hdl).funcId;
                }
                if (funcName) {
                    *funcName = YP(hdl).funcName;
                }
                if (funcInfo) {
                    funcInfo->raw = YP(hdl).funInfo.raw;
                }
                if (funcVal) {
                    // apply value change
                    for (i = 0; i < YOCTO_PUBVAL_SIZE / 2; i++) {
                        funcValWords[i] = YP(hdl).funcValWords[i];
                    }
                }
                res = 0;
            }
        }
    }
    yLeaveCriticalSection(&yYpMutex);
    return res;
}


void ypGetCategory(yBlkHdl hdl, char *name, yBlkHdl *entries)
{
    // category records are never freed
    if(name)    yHashGetStr(YC(hdl).name, name, YOCTO_FUNCTION_LEN);
    if(entries) *entries = YC(hdl).entries;
}

int ypGetAttributes(yBlkHdl hdl, yStrRef *serial, yStrRef *funcId, yStrRef *funcName, Notification_funydx *funcInfo, char *funcVal)
{
    yStrRef serialref = YSTRREF_EMPTY_STRING;
    yStrRef funcidref = YSTRREF_EMPTY_STRING;
    yStrRef funcnameref = YSTRREF_EMPTY_STRING;
    u16     i;
    int     res = -1;
    u16     *funcValWords = (u16 *)funcVal;

    yEnterCriticalSection(&yYpMutex);
    if(YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND) {
        serialref = YP(hdl).serialNum;
        funcidref = YP(hdl).funcId;
        funcnameref = YP(hdl).funcName;
        if(funcVal != NULL) { // intentionally not null terminated !
            for(i = 0; i < YOCTO_PUBVAL_SIZE/2; i++) {
                funcValWords[i] = YP(hdl).funcValWords[i];
            }
        }
        if (funcInfo)
            *funcInfo = YP(hdl).funInfo;
        res = YP(hdl).funInfo.v2.funydx;
    } else {
        if (funcInfo)
            funcInfo->raw = 0;
        if (funcVal) *funcVal = 0;
    }
    yLeaveCriticalSection(&yYpMutex);

    if(serial != NULL)   *serial = serialref;
    if(funcId != NULL)   *funcId = funcidref;
    if(funcName != NULL) *funcName = funcnameref;

    return res;
}

int ypGetType(yBlkHdl hdl)
{
    int res = -1;

    yEnterCriticalSection(&yYpMutex);
    if(YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND) {
        res =YP(hdl).blkId - YBLKID_YPENTRY;
    }
    yLeaveCriticalSection(&yYpMutex);

    return res;
}

static void ypUnregister(yStrRef serial)
{
    yBlkHdl  prev, next;
    yBlkHdl  cat_hdl, hdl;

    yEnterCriticalSection(&yYpMutex);

    // scan all category nodes
    cat_hdl = yYpListHead;
    while(cat_hdl != INVALID_BLK_HDL) {
        YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
        hdl = YC(cat_hdl).entries;
        prev = INVALID_BLK_HDL;
        // scan all yp entries
        while(hdl != INVALID_BLK_HDL) {
            YASSERT(YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND);
            next = YP(hdl).nextPtr;
            if(YP(hdl).serialNum == serial) {
                // entry found, remove it
                if(prev == INVALID_BLK_HDL) {
                    YC(cat_hdl).entries = next;
                } else {
                    YP(prev).nextPtr = next;
                }
                yBlkFree(hdl);
                // continue search on next entries
            } else {
                prev = hdl;
            }
            hdl = next;
        }
        cat_hdl = YC(cat_hdl).nextPtr;
    }

    yLeaveCriticalSection(&yYpMutex);
}

#ifndef MICROCHIP_API

YAPI_FUNCTION ypSearch(const char *class_str, const char *func_or_name)
{
    yStrRef     categref = INVALID_HASH_IDX;
    yStrRef     devref, funcref;
    yBlkHdl     cat_hdl, hdl, byname;
    int         abstract = 0;
    const char  *dotpos = func_or_name;
    char        categname[HASH_BUF_SIZE];
    YAPI_FUNCTION  res = -1;
    YAPI_FUNCTION  best_name = -1;
    int         i;

    // first search for the category node
    if (!strcmp(class_str, "Function")) {
        cat_hdl = INVALID_BLK_HDL;
    }else if (!strcmp(class_str, "Sensor")) {
        abstract = YOCTO_AKA_YSENSOR;
        cat_hdl = INVALID_BLK_HDL;
    } else {
        categref = yHashTestStr(class_str);
        if(categref == INVALID_HASH_IDX)
            return -2; // no device of this type so far
        yEnterCriticalSection(&yYpMutex);
        cat_hdl = yYpListHead;
        while(cat_hdl != INVALID_BLK_HDL) {
            YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
            if(YC(cat_hdl).name == categref) break;
            cat_hdl = YC(cat_hdl).nextPtr;
        }
        yLeaveCriticalSection(&yYpMutex);
        if(cat_hdl == INVALID_BLK_HDL)
            return -2; // no device of this type so far
    }

    // analyse function string
    while(*dotpos && *dotpos != '.') dotpos++;
    if(!*dotpos) {
        // search for a function by pure logical name
        funcref = yHashTestStr(func_or_name);
        if(funcref == INVALID_HASH_IDX)
            return -1;
        yEnterCriticalSection(&yYpMutex);
        if(categref != INVALID_HASH_IDX) {
            // search within defined function category
            hdl = YC(cat_hdl).entries;
            while(hdl != INVALID_BLK_HDL) {
                if(YP(hdl).funcName == funcref) {
                    res = YP(hdl).serialNum + ((u32)(YP(hdl).funcId) << 16);
                    break;
                }
                hdl = YP(hdl).nextPtr;
            }
        } else {
            // search by pure logical name within abstract basetype
            hdl = INVALID_BLK_HDL;
            for(cat_hdl = yYpListHead; cat_hdl != INVALID_BLK_HDL; cat_hdl = YC(cat_hdl).nextPtr) {
                YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
                hdl = YC(cat_hdl).entries;
                while (hdl != INVALID_BLK_HDL) {
                    // check functions matching abstract baseclass, skip others
                    if ((abstract == YOCTO_AKA_YFUNCTION || YP(hdl).blkId == YBLKID_YPENTRY + abstract) && YP(hdl).funcName == funcref) {
                        res = YP(hdl).serialNum + ((u32)(YP(hdl).funcId) << 16);
                        break;
                    }
                    hdl = YP(hdl).nextPtr;
                }
                if(hdl != INVALID_BLK_HDL) break;
            }
        }
        yLeaveCriticalSection(&yYpMutex);
        if(hdl != INVALID_BLK_HDL) return res;
        // not found, fallback to assuming that str_func is a logical name or serial number
        // of a module with an implicit function name (like serial.module for instance)
        devref = funcref;
        categname[0] = class_str[0] | 0x20; // lowercase first letter
        for(i = 1; (categname[i] = class_str[i]) != 0; i++);
        funcref = yHashTestStr(categname);
        if(funcref == INVALID_HASH_IDX)
            return -1;
    } else {
        if(dotpos==func_or_name){
            // format is ".funcid"
            devref = INVALID_HASH_IDX;
        }else{
            // format is "device.funcid"
            devref = yHashTestBuf((u8 *)func_or_name, (u16)( dotpos-func_or_name));
			if(devref == INVALID_HASH_IDX)
				return -1;
        }
        funcref = yHashTestStr(dotpos+1);
        if(funcref == INVALID_HASH_IDX)
            return -1;
    }

    if(devref!= INVALID_HASH_IDX){
        // locate function identified by devref.funcref by first resolving devref
        byname = INVALID_BLK_HDL;
        yEnterCriticalSection(&yWpMutex);
        hdl = yWpListHead;
        while(hdl != INVALID_BLK_HDL) {
            YASSERT(WP(hdl).blkId == YBLKID_WPENTRY);
            if(WP(hdl).serial == devref) break;
            if(WP(hdl).name == devref) byname = hdl;
            hdl = WP(hdl).nextPtr;
        }
        yLeaveCriticalSection(&yWpMutex);
        if(hdl == INVALID_BLK_HDL) {
            if(byname == INVALID_BLK_HDL)
                return -1;
            // device found by logicalname
            devref = WP(byname).serial;
        }
    }
    // device found, now we can search for function by serial.funcref
    yEnterCriticalSection(&yYpMutex);
    if(categref != INVALID_HASH_IDX) {
        // search within defined function category
        hdl = YC(cat_hdl).entries;
        while(hdl != INVALID_BLK_HDL) {
            if(devref==INVALID_HASH_IDX || YP(hdl).serialNum == devref) {
                if(YP(hdl).funcId == funcref) {
                    res = YP(hdl).serialNum + ((u32)(YP(hdl).funcId) << 16);
                    break;
                }
                if (best_name != -1 && YP(hdl).funcName == funcref) {
                    best_name = YP(hdl).serialNum + ((u32)(YP(hdl).funcId) << 16);
                }
            }
            hdl = YP(hdl).nextPtr;
        }
    } else {
        // search by pure logical name within abstract basetype
        for(cat_hdl = yYpListHead; cat_hdl != INVALID_BLK_HDL; cat_hdl = YC(cat_hdl).nextPtr) {
            YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
            hdl = YC(cat_hdl).entries;
            while(hdl != INVALID_BLK_HDL) {
                // check functions matching abstract baseclass, skip others
                if ((abstract == YOCTO_AKA_YFUNCTION || YP(hdl).blkId == YBLKID_YPENTRY + abstract) && (devref == INVALID_HASH_IDX || YP(hdl).serialNum == devref)) {
                    if (YP(hdl).funcId == funcref) {
                        res = YP(hdl).serialNum + ((u32)(YP(hdl).funcId) << 16);
                        break;
                    }
                    if (best_name != -1 && YP(hdl).funcName == funcref) {
                        best_name = YP(hdl).serialNum + ((u32)(YP(hdl).funcId) << 16);
                    }
                }
                hdl = YP(hdl).nextPtr;
            }
            if(hdl != INVALID_BLK_HDL) break;
        }
    }
    if (res == -1 && best_name != -1) {
        res = best_name;
    }
    yLeaveCriticalSection(&yYpMutex);

    return res;
}


int ypGetFunctions(const char *class_str, YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
                   YAPI_FUNCTION *buffer,int maxsize,int *neededsize)
{
    yStrRef categref = INVALID_HASH_IDX;
    yBlkHdl cat_hdl, hdl;
    int     abstract = 0;
    int     maxfun = 0, nbreturned = 0;
    YAPI_FUNCTION fundescr=0;
    int     use = (prevfundesc==0);// if prefuncdesc == 0  use any functions

    if(class_str) {
        if (!strcmp(class_str, "Function")) {
            abstract = YOCTO_AKA_YFUNCTION;
        } else if (!strcmp(class_str, "Sensor")) {
            abstract = YOCTO_AKA_YSENSOR;
        } else {
            categref = yHashTestStr(class_str);
            if(categref == INVALID_HASH_IDX) {
                if(*neededsize) *neededsize = 0;
                return 0;
            }
        }
    }
    yEnterCriticalSection(&yYpMutex);
    for(cat_hdl = yYpListHead; cat_hdl != INVALID_BLK_HDL; cat_hdl = YC(cat_hdl).nextPtr) {
        YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
        if(categref == INVALID_HASH_IDX) {
            // search any type of function, but skip Module
            if(YC(cat_hdl).name == YSTRREF_MODULE_STRING) continue;
        } else {
            // search for a specific function type
            if(YC(cat_hdl).name != categref) continue;
        }
        hdl = YC(cat_hdl).entries;
        while(hdl != INVALID_BLK_HDL) {
            // if an abstract baseclass is specified, skip others
            if(abstract && YP(hdl).blkId != YBLKID_YPENTRY+abstract) {
                hdl = YP(hdl).nextPtr;
                continue;
            }
            if(devdesc == -1 || YP(hdl).serialNum == (u16)devdesc) {
                if(!use && prevfundesc == fundescr){
                    use = 1;
                }
                fundescr = YP(hdl).hwId;
                if(use) {
                    maxfun++;
                    if(maxsize >= (int)sizeof(YAPI_FUNCTION)) {
                        maxsize -= sizeof(YAPI_FUNCTION);
                        if (buffer){
                            *buffer++ = fundescr;
                            nbreturned++;
                        }
                    }
                }
            }
            hdl = YP(hdl).nextPtr;
        }
        // if we were looking for a specific category, we found it
        if(categref != INVALID_HASH_IDX) break;
    }
    yLeaveCriticalSection(&yYpMutex);

    if(neededsize) *neededsize = sizeof(YAPI_FUNCTION) * maxfun;
    return nbreturned;
}


// This function should only be called after seizing ypMutex
static yBlkHdl functionSearch(YAPI_FUNCTION fundesc)
{
    yBlkHdl cat_hdl, hdl;
    yStrRef funcref, categref;
    char    funcname[YOCTO_FUNCTION_LEN], *p;

    funcref = (u16)(fundesc >> 16);
    yHashGetStr(funcref, funcname, YOCTO_FUNCTION_LEN);
    funcname[0] &= ~0x20; // uppercase first letter
    for(p = funcname+1; *p > '9'; p++);
    *p = 0;
    categref = yHashTestStr(funcname);
    if(categref == INVALID_HASH_IDX)
        return INVALID_BLK_HDL; // no device of this type so far, should never happen

    cat_hdl = yYpListHead;
    while(cat_hdl != INVALID_BLK_HDL) {
        YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
        if(YC(cat_hdl).name == categref) break;
        cat_hdl = YC(cat_hdl).nextPtr;
    }
    if(cat_hdl == INVALID_BLK_HDL)
        return INVALID_BLK_HDL; // no device of this type so far, should never happen

    hdl = YC(cat_hdl).entries;
    while(hdl != INVALID_BLK_HDL) {
        if(YP(hdl).hwId == fundesc) {
            return hdl;
        }
        hdl = YP(hdl).nextPtr;
    }
    return INVALID_BLK_HDL; // device not found, most probably unplugged
}

int ypGetFunctionInfo(YAPI_FUNCTION fundesc, char *serial, char *funcId, char *baseType, char *funcName, char *funcVal)
{
    yBlkHdl hdl;
    u16     i;
    u16     *funcValWords = (u16 *)funcVal;

    yEnterCriticalSection(&yYpMutex);
    hdl = functionSearch(fundesc);
    if(hdl != INVALID_BLK_HDL) {
        if(serial)   yHashGetStr(YP(hdl).serialNum, serial, YOCTO_SERIAL_LEN);
        if(funcId)   yHashGetStr(YP(hdl).funcId, funcId, YOCTO_FUNCTION_LEN);
        if (baseType) {
            int type = YOCTO_AKA_YFUNCTION;
            if (YP(hdl).blkId >= YBLKID_YPENTRY && YP(hdl).blkId <= YBLKID_YPENTRYEND) {
                type = YP(hdl).blkId - YBLKID_YPENTRY;
            }
            if (type == YOCTO_AKA_YSENSOR) {
                YSTRCPY(baseType, YOCTO_FUNCTION_LEN, "Sensor");
            } else {
                YSTRCPY(baseType, YOCTO_FUNCTION_LEN, "Function");
            }
        }
        if(funcName) yHashGetStr(YP(hdl).funcName, funcName, YOCTO_LOGICAL_LEN);
        if(funcVal != NULL) { // null-terminate
            for(i = 0; i < YOCTO_PUBVAL_SIZE/2; i++) {
                funcValWords[i] = YP(hdl).funcValWords[i];
            }
            funcVal[6] = 0;
        }
    } else {
        if(funcVal != NULL) funcVal[0] = 0;
    }
    yLeaveCriticalSection(&yYpMutex);

    return (hdl == INVALID_BLK_HDL ? -1 : 0);
}

#endif


int ypGetFunctionsEx(yStrRef categref, YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
    YAPI_FUNCTION *buffer, int maxsize, int *neededsize)
{
    yBlkHdl cat_hdl, hdl;
    int     abstract = 0;
    int     maxfun = 0, nbreturned = 0;
    YAPI_FUNCTION fundescr = 0;
    int     use = (prevfundesc == 0);// if prefuncdesc == 0  use any functions

    if (categref == YSTRREF_SENSOR_STRING) {
        abstract = YOCTO_AKA_YSENSOR;
    }
    yEnterCriticalSection(&yYpMutex);
    for (cat_hdl = yYpListHead; cat_hdl != INVALID_BLK_HDL; cat_hdl = YC(cat_hdl).nextPtr) {
        YASSERT(YC(cat_hdl).blkId == YBLKID_YPCATEG);
        if (categref == INVALID_HASH_IDX) {
            // search any type of function, but skip Module
            if (YC(cat_hdl).name == YSTRREF_MODULE_STRING) continue;
        } else {
            // search for a specific function type
            if (YC(cat_hdl).name != categref) continue;
        }
        hdl = YC(cat_hdl).entries;
        while (hdl != INVALID_BLK_HDL) {
            // if an abstract baseclass is specified, skip others
            if (abstract && YP(hdl).blkId != YBLKID_YPENTRY + abstract) {
                hdl = YP(hdl).nextPtr;
                continue;
            }
            if (devdesc == -1 || YP(hdl).serialNum == (u16)devdesc) {
                if (!use && prevfundesc == fundescr) {
                    use = 1;
                }
                fundescr = YP(hdl).hwId;
                if (use) {
                    maxfun++;
                    if (maxsize >= (int)sizeof(YAPI_FUNCTION)) {
                        maxsize -= sizeof(YAPI_FUNCTION);
                        if (buffer) {
                            *buffer++ = fundescr;
                            nbreturned++;
                        }
                    }
                }
            }
            hdl = YP(hdl).nextPtr;
        }
        // if we were looking for a specific category, we found it
        if (categref != INVALID_HASH_IDX) break;
    }
    yLeaveCriticalSection(&yYpMutex);

    if (neededsize) *neededsize = sizeof(YAPI_FUNCTION) * maxfun;
    return nbreturned;
}



s16 ypFindBootloaders(yStrRef *serials, u16 maxSerials)
{
    yBlkHdl     cat_hdl, hdl;
    s16         res = 0;

    // first search for the category node
    yEnterCriticalSection(&yYpMutex);
    cat_hdl = yYpListHead;
    while(cat_hdl != INVALID_BLK_HDL) {
        if(YC(cat_hdl).name == YSTRREF_HUBPORT_STRING) break;
        cat_hdl = YC(cat_hdl).nextPtr;
    }
    yLeaveCriticalSection(&yYpMutex);
    if(cat_hdl == INVALID_BLK_HDL)
        return -2; // no hubPort registered so far

    yEnterCriticalSection(&yYpMutex);
    hdl = YC(cat_hdl).entries;
    while(hdl != INVALID_BLK_HDL) {
        if(YP(hdl).funcValWords[0]==WORD_TEXT_PR && YP(hdl).funcValWords[1]==WORD_TEXT_OG) {
            if(res++ < maxSerials) {
                *serials++ = YP(hdl).funcName;
            }
        }
        hdl = YP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yYpMutex);

    return res;
}

#ifdef MICROCHIP_API
int ypGetBootDevHdl(const char *serial)
{
    yBlkHdl cat_hdl, hdl;
    yStrRef serialRef;
    char    funcid[9];
    s16     devYdx;

    serialRef = yHashTestStr(serial);
    if(serialRef == INVALID_HASH_IDX)
        return -1; // unknown serial

    // search for the category node
    yEnterCriticalSection(&yYpMutex);
    cat_hdl = yYpListHead;
    while(cat_hdl != INVALID_BLK_HDL) {
        if(YC(cat_hdl).name == YSTRREF_HUBPORT_STRING) break;
        cat_hdl = YC(cat_hdl).nextPtr;
    }
    yLeaveCriticalSection(&yYpMutex);
    if(cat_hdl == INVALID_BLK_HDL)
        return -2; // no hubPort registered so far

    yEnterCriticalSection(&yYpMutex);
    hdl = YC(cat_hdl).entries;
    while(hdl != INVALID_BLK_HDL) {
        if(YP(hdl).funcName == serialRef &&
           YP(hdl).funcValWords[0] == WORD_TEXT_PR &&
           YP(hdl).funcValWords[1] == WORD_TEXT_OG) {
            break;
        }
        hdl = YP(hdl).nextPtr;
    }
    yLeaveCriticalSection(&yYpMutex);
    if(hdl == INVALID_BLK_HDL)
        return -3; // serial not connected in PROG mode

    yHashGetStr(YP(hdl).funcId, funcid, sizeof(funcid));
    if(funcid[7] <'1' || funcid[7] > '4')
        return -3; // invalid function id
    devYdx = wpGetDevYdx(YP(hdl).serialNum);
    if(devYdx == hubDevYdx) {
        // The 3 root ports use devhdl 0-2
        return funcid[7] - '1';
    }

    // ports on shield use hub devYdx+(1..4)
    return devYdx + funcid[7] - '0';
}
#endif


// Network notification format: 7x7bit (mapped to 7 chars in range 32..159)
//                              used to represent 1 flag (RAW6BYTES) + 6 bytes
// INPUT:  [R765432][1076543][2107654][3210765][4321076][5432107][6543210]
// OUTPUT: [R][-byte 0][-byte 1-][-byte 2-][-byte 3-][-byte 4-][-byte 5-]
//
// return the effective number of bytes produced; if < 6 remaining are padded with 0
// input and output buffer may be the same one
// encoding function is defined in yhub.c
//
int decodeNetFuncValV2(const u8 *p, Notification_funydx *funInfo, char *funcVal)
{
    u16 ch = *p;
    int len = 0;

    if(ch < 32 || ch > 32+127) {
        return -1;
    }
    // get the 7 first bits
    ch -= 32;
    funInfo->v2.typeV2 = (ch & 0x40 ? NOTIFY_V2_6RAWBYTES : NOTIFY_V2_TYPEDDATA);
    // clear flag
    ch &= 0x3f;
    while(len < YOCTO_PUBVAL_SIZE) {
        u8 newCh = *++p;
        if(newCh == 0 || newCh == NOTIFY_NETPKT_STOP) {
            memset(funcVal+len, 0, YOCTO_PUBVAL_SIZE-len);
            break;
        }
        if(newCh < 32 || newCh > 32+127) {
            return -1;
        }
        newCh -= 32;
        ch = (ch << 7) + newCh;
        funcVal[len] = (ch >> (5-len));
        len++;
    }
    return len;
}

