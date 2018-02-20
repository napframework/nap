/*********************************************************************
 *
 * $Id: yhash.h 29739 2018-01-25 17:03:29Z seb $
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

#ifndef YHASH_H
#define YHASH_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ydef.h"

//#define DEBUG_WP_LOCK
//#define DEBUG_WP

#define HASH_BUF_SIZE 28
#define HASH_BUF_POW   5 /* HASH_BUF_POW = log_2(HASH_BUF_SIZE+2+2) */
#ifdef MICROCHIP_API
#define NB_MAX_HASH_ENTRIES 1023     /* keep hash table size <32KB on Yocto-Hub */
#define NB_MAX_DEVICES        80     /* base hub + up to 15 shields (up to 4 slave ports) */
#else
#define NB_MAX_HASH_ENTRIES 8192
#define NB_MAX_DEVICES       256
#endif

#define YSTRREF_EMPTY_STRING   0x00ff /* yStrRef value for the empty string    */
#define YSTRREF_MODULE_STRING  0x0020 /* yStrRef value for the string 'Module' */
#define YSTRREF_mODULE_STRING  0x00a3 /* yStrRef value for the string 'module' */
#define YSTRREF_HUBPORT_STRING 0x00d6 /* yStrRef value for the string 'HubPort' */
#define YSTRREF_SENSOR_STRING  0x0001 /* yStrRef value for the string 'Sensor' */

#ifndef CPU_BIG_ENDIAN
#define WORD_TEXT_PR           0x5250
#define WORD_TEXT_OG           0x474f
#else
#define WORD_TEXT_PR           0x5052
#define WORD_TEXT_OG           0x4f47
#endif

typedef enum {
    Y_WP_SERIALNUMBER,
    Y_WP_LOGICALNAME,
    Y_WP_PRODUCTNAME,
    Y_WP_PRODUCTID,
    Y_WP_NETWORKURL,
    Y_WP_BEACON,
    Y_WP_INDEX
} yWPAttribute;

#define YBLKID_WPENTRY    0xf0
#define YBLKID_YPCATEG    0xf1
#define YBLKID_YPARRAY    0xf2
#define YBLKID_YPENTRY    0xf3
#define YBLKID_YPENTRYEND (YBLKID_YPENTRY+YOCTO_N_BASECLASSES-1)

typedef struct {
    u8          devYdx;
    u8          blkId;
    yBlkHdl     nextPtr;
    yStrRef     serial;
    yStrRef     name;
    yStrRef     product;
    u16         devid;
    yUrlRef     url;
    u16         flags;
} yWhitePageEntry;

// WP entry flags
#define YWP_MARK_FOR_UNREGISTER 0x02
#define YWP_BEACON_ON           0x01

typedef struct {
    u8          posYdx;
    u8          blkId;
    yBlkHdl     nextPtr;
    yBlkHdl     entries[6];
} yYellowPageArray;

typedef struct {
    Notification_funydx funInfo;
    u8          blkId;
    yBlkHdl     nextPtr;
    union {
      struct {
#ifndef CPU_BIG_ENDIAN
        yStrRef serialNum;
        yStrRef funcId;
#else
        yStrRef funcId;
        yStrRef serialNum;
#endif
      };
      YAPI_FUNCTION hwId;
    };
    yStrRef     funcName;
    union {
        char    funcVal[YOCTO_PUBVAL_SIZE];
        u16     funcValWords[YOCTO_PUBVAL_SIZE/2];
    };
} yYellowPageEntry;

typedef struct {
    u8          catYdx;
    u8          blkId;
    yBlkHdl     nextPtr;
    yStrRef     name;
    yBlkHdl     entries;
} yYellowPageCateg;

typedef union{
    struct {
        u16     hash;
        yHash   next;
        u8      buff[HASH_BUF_SIZE];
    };
    union {
        struct {
            u8      ydx;
            u8      blkId;
            yBlkHdl nextPtr;
        };
        yWhitePageEntry     wpEntry;
        yYellowPageEntry    ypEntry;
        yYellowPageCateg    ypCateg;
        yYellowPageArray    ypArray;
    } blk[2];
} YHashSlot;

extern char SerialNumberStr[YOCTO_SERIAL_LEN];
extern yStrRef SerialRef;
extern yBlkHdl yWpListHead;
extern yBlkHdl yYpListHead;

#define YMAX_HUB_URL_DEEP           7
#define YOCTO_HOSTNAME_NAME         (HASH_BUF_SIZE*2+2)

typedef enum {
    USB_URL,
    IP_URL,
    NAME_URL
} yAsbUrlType;

typedef enum {
    PROTO_AUTO = 0,
    PROTO_HTTP,
    PROTO_WEBSOCKET
} yAsbUrlProto;



typedef struct{
    union{
        struct{
            yStrRef   host;
            yStrRef   domaine;
            u16       port;
        } byname;
        struct{
            yStrRef   ip;
            yHash     invalid;
            u16       port;
        } byip;
        struct{
            yHash     invalid1;
            yHash     invalid2;
            yStrRef   serial;
        } byusb;
    };
    u16 proto;
    yStrRef user;
    yStrRef password;
    yStrRef subdomain;
    yStrRef path[YMAX_HUB_URL_DEEP];
} yAbsUrl;

void  yHashInit(void);
yHash yHashPutBuf(const u8 *buf, u16 len);
yHash yHashPutStr(const char *str);
yHash yHashTestBuf(const u8 *buf, u16 len);
yHash yHashTestStr(const char *str);
void  yHashGetBuf(yHash yhash, u8 *destbuf, u16 bufsize);
void  yHashGetStr(yHash yhash, char *destbuf, u16 bufsize);
u16   yHashGetStrLen(yHash yhash);
char  *yHashGetStrPtr(yHash yhash);
#ifndef MICROCHIP_API
yUrlRef yHashUrlFromRef(yUrlRef urlref, const char *rootUrl);
yUrlRef yHashUrl(const char *host, const char *rootUrl, u8 testonly, char *errmsg);
yAsbUrlType  yHashGetUrlPort(yUrlRef urlref, char *url, u16 *port, yAsbUrlProto *proto, yStrRef *user, yStrRef *password, yStrRef *subdomain);
int yHashSameHub(yUrlRef url_a, yUrlRef url_b);
void  yHashFree(void);
#endif
yUrlRef yHashUrlUSB(yHash serial);
yUrlRef yHashUrlAPI(void);
u16     yBlkListLength(yBlkHdl hdl);
yBlkHdl yBlkListSeek(yBlkHdl hdl, u16 pos);
int     wpRegister(int devYdx, yStrRef serial, yStrRef logicalName, yStrRef productName, u16 productId, yUrlRef devUrl, s8 beacon);
yStrRef wpGetAttribute(yBlkHdl hdl, yWPAttribute attridx);
void    wpGetSerial(yBlkHdl hdl, char *serial);
void    wpGetLogicalName(yBlkHdl hdl, char *logicalName);
#ifdef DEBUG_WP_LOCK
void    wpPreventUnregisterDbg(const char *file, u32 line);
void    wpAllowUnregisterDbg(const char *file, u32 line);
#define wpPreventUnregister()   wpPreventUnregisterDbg(__FILE__,__LINE__)
#define wpAllowUnregister()     wpAllowUnregisterDbg(__FILE__,__LINE__)
#else
void    wpPreventUnregisterEx(void);
void    wpAllowUnregisterEx(void);
#define wpPreventUnregister()   wpPreventUnregisterEx()
#define wpAllowUnregister()     wpAllowUnregisterEx()
#endif
int     wpMarkForUnregister(yStrRef serial);
int     wpGetDevYdx(yStrRef serial);
YAPI_DEVICE wpSearchByNameHash(yStrRef strref);
#ifndef MICROCHIP_API
u16     wpEntryCount(void);
YAPI_DEVICE wpSearchEx(yStrRef strref);
YAPI_DEVICE wpSearch(const char *device_str);
YAPI_DEVICE wpSearchByUrl(const char *host, const char *rootUrl);
int     wpGetAllDevUsingHubUrl( yUrlRef hubUrl, yStrRef *buffer,int sizeInStrRef);

yUrlRef wpGetDeviceUrlRef(YAPI_DEVICE devdesc);
int     wpGetDeviceUrl(YAPI_DEVICE devdesc, char *roothubserial, char *request, int requestsize, int *neededsize);
YAPI_FUNCTION ypSearch(const char *class_str, const char *func_str);
int     ypGetFunctions(const char *class_str, YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
                       YAPI_FUNCTION *buffer,int maxsize,int *neededsize);
int     ypGetFunctionInfo(YAPI_FUNCTION fundesc, char *serial, char *funcId, char *baseType, char *funcName, char *funcVal);
#endif
int     ypGetFunctionsEx(yStrRef categref, YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc, YAPI_FUNCTION *buffer, int maxsize, int *neededsize);
int     wpGetDeviceInfo(YAPI_DEVICE devdesc, u16 *deviceid, char *productname, char *serial, char *logicalname, u8 *beacon);
int     ypRegister(yStrRef categ, yStrRef serial, yStrRef funcId, yStrRef funcName, int funClass, int funYdx, const char *funcVal);
// WARNING: funcVal MUST BE WORD-ALIGNED
int     ypRegisterByYdx(u8 devYdx, Notification_funydx funInfo, const char *funcVal, YAPI_FUNCTION *fundesc);
// WARNING: funcVal MUST BE WORD-ALIGNED
int     ypGetAttributesByYdx(u8 devYdx, u8 funYdx, yStrRef *serial, yStrRef *logicalName, yStrRef *funcId, yStrRef *funcName, Notification_funydx *funcInfo, char *funcVal);
void    ypGetCategory(yBlkHdl hdl, char *name, yBlkHdl *entries);
int     ypGetAttributes(yBlkHdl hdl, yStrRef *serial, yStrRef *funcId, yStrRef *funcName, Notification_funydx *funcInfo, char *funcVal);
int     ypGetType(yBlkHdl hdl);
int     ypGetBootDevHdl(const char *serial);
s16     ypFindBootloaders(yStrRef *serials, u16 maxSerials);
int     decodeNetFuncValV2(const u8 *p, Notification_funydx *funInfo, char *funcVal);

#ifdef  __cplusplus
}
#endif
#endif


