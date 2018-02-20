/*********************************************************************
 *
 * $Id: ydef.h 29064 2017-11-02 16:13:37Z seb $
 *
 * Standard definitions common to all yoctopuce projects
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

#ifndef  YOCTO_DEF_H
#define  YOCTO_DEF_H
#ifdef  __cplusplus
extern "C" {
#endif


#if defined(_WIN32)
// Windows C compiler
#define WINDOWS_API

#ifdef _WIN64
#define __64BITS__
#define __WIN64__
#else
#define __32BITS__
#endif

#ifdef _MSC_VER
typedef unsigned char           u8;
typedef signed char             s8;
typedef unsigned short int      u16;
typedef signed short int        s16;
typedef unsigned long int       u32;
typedef signed long int         s32;
typedef unsigned long long      u64;
typedef signed long long        s64;
#define VARIABLE_SIZE
#define FMTs64 "lld"
#define FMTu64 "llu"
#define FMTx64 "llx"

#else

#ifdef __BORLANDC__
typedef unsigned __int8         u8;
typedef __int8                  s8;
typedef unsigned __int16        u16;
typedef __int16                 s16;
typedef unsigned __int32        u32;
typedef __int32                 s32;
typedef unsigned __int64        u64;
typedef __int64                 s64;
#else
typedef unsigned char           u8;
typedef signed char             s8;
typedef unsigned short int      u16;
typedef signed short int        s16;
typedef unsigned int            u32;
typedef signed int              s32;
typedef unsigned long long      u64;
typedef signed long long        s64;
#endif

#define VARIABLE_SIZE           0
#define FMTs64 "lld"
#define FMTu64 "llu"
#define FMTx64 "llx"

#endif
// end of WINDOWS_API


#elif defined(__C30__)
// Microchip C30
#define MICROCHIP_API
#define __16BITS__

typedef unsigned char           u8;
typedef signed char             s8;
typedef unsigned short int      u16;
typedef signed short int        s16;
typedef unsigned long int       u32;
typedef signed long int         s32;
typedef unsigned long long      u64;
typedef signed long long        s64;
#define VARIABLE_SIZE           0
#define FMTs64 "lld"
#define FMTu64 "llu"
#define FMTx64 "llx"
// end of MICROCHIP_API

#elif defined(__APPLE__)
#include <TargetConditionals.h>

#if TARGET_IPHONE_SIMULATOR
//#warning IOS simulatore platform
#define IOS_API
#elif TARGET_OS_IPHONE
//#warning IOS platform
#define IOS_API
#elif TARGET_OS_MAC
#define OSX_API

#if defined(__i386__)
#define __32BITS__
#elif defined(__x86_64__)
#define OSX_API
#define __64BITS__
#else
#error Unsupported MAC OS X architecture
#endif

#else
#error Unsupported Apple target
#endif

// Mac OS X C compiler
typedef unsigned char           u8;
typedef signed char             s8;
typedef unsigned short int      u16;
typedef signed short int        s16;
typedef unsigned int            u32;
typedef signed int              s32;
#ifdef __LP64__
typedef unsigned long           u64;
typedef signed long             s64;
#else
typedef unsigned long long      u64;
typedef signed long long        s64;
#endif
#define VARIABLE_SIZE           0
#define FMTs64 "ld"
#define FMTu64 "lu"
#define FMTx64 "lx"
#include <pthread.h>
#include <errno.h>
// end of OSX_API OR IOS_API


#elif defined(__linux__)
// gcc compiler on linux
#define LINUX_API

#if defined(__i386__)
#define __32BITS__
#define FMTs64 "lld"
#define FMTu64 "llu"
#define FMTx64 "llx"
#elif defined(__x86_64__)
#define __64BITS__
#define FMTs64 "ld"
#define FMTu64 "lu"
#define FMTx64 "lx"
#else
#define __32BITS__
#define FMTs64 "lld"
#define FMTu64 "llu"
#define FMTx64 "llx"
#endif

#include <stdint.h>
typedef uint8_t                 u8;
typedef int8_t                  s8;
typedef uint16_t                u16;
typedef int16_t                 s16;
typedef uint32_t                u32;
typedef int32_t                 s32;
typedef uint64_t                u64;
typedef int64_t                 s64;
#define VARIABLE_SIZE           0
#include <pthread.h>
#include <errno.h>

#else
#warning UNSUPPORTED ARCHITECTURE, please edit yocto_def.h !
#endif
// end of LINUX_API


typedef u32   yTime;            /* measured in milliseconds */
typedef u32   u31;              /* shorter unsigned integers */
typedef s16   yHash;
typedef u16   yBlkHdl;          /* (yHash << 1) + [0,1] */
typedef yHash yStrRef;
typedef yHash yUrlRef;
typedef s32   YAPI_DEVICE;      /* yStrRef of serial number */
typedef s32   YAPI_FUNCTION;    /* yStrRef of serial + (ystrRef of funcId << 16) */

#define INVALID_HASH_IDX    -1  /* To use for yHash, yStrRef, yApiRef types */
#define INVALID_BLK_HDL     0   /* To use for yBlkHdl type */

#ifdef MICROCHIP_API
typedef u8              YSOCKET;
typedef s8              YYSBIO;
typedef s8              YTRNKIO;
#else
// we have hardcoded the type of SOCKET to
// prevent to mess up with user own code
#if defined(WINDOWS_API)
#if defined(__64BITS__)
typedef u64             YSOCKET;
#else
typedef u32             YSOCKET;
#endif
#else
typedef int             YSOCKET;
#endif
typedef s32             YUSBIO;
typedef s32             YUSBDEV;
#endif

#define YIO_INVALID      0
#define YIO_USB          1
#define YIO_TCP          2
#define YIO_YSB          3
#define YIO_TRUNK        4
#define YIO_WS           5

#define YIO_DEFAULT_USB_TIMEOUT      2000u
#define YIO_DEFAULT_TCP_TIMEOUT     20000u
#define YIO_1_MINUTE_TCP_TIMEOUT    60000u
#define YIO_10_MINUTES_TCP_TIMEOUT 600000u
#define YIO_IDLE_TCP_TIMEOUT         5000u

#ifdef MICROCHIP_API
// same as yhub devhdl
typedef s16 YIOHDL;
#else
// make sure this union is no more than 8 bytes, YIOHDL is allocated used in all foreign APIs
typedef void *YIOHDL;
#endif

#define YIOHDL_SIZE (sizeof(YIOHDL))

#define INVALID_YHANDLE (-1)

#define S8(x)   ((s8)(x))
#define S16(x)  ((s16)(x))
#define S32(x)  ((s32)(x))
#define S64(x)  ((s64)(x))
#define U8(x)   ((u8)(x))
#define U16(x)  ((u16)(x))
#define U32(x)  ((u32)(x))
#define U64(x)  ((u64)(x))

#define U8ADDR(x)  ((u8 *)&(x))
#define U16ADDR(x) ((u16 *)&(x))

#define ADDRESSOF(x)    (&(x))
#define PTRVAL(x)       (*(x))

#if defined(__PIC24FJ256DA206__)
#define _FAR __eds__
#else
#define _FAR
#endif

#if defined(MICROCHIP_API) || defined(VIRTUAL_HUB)
#define YAPI_IN_YDEVICE
#define YSTATIC
#else
#define YSTATIC static
#endif


//#define DEBUG_CRITICAL_SECTION

#ifdef DEBUG_CRITICAL_SECTION

#include <winsock2.h>
#include <ws2tcpip.h>
#include <WinBase.h>

typedef enum  {
    YCS_UNALLOCATED=0,
    YCS_ALLOCATED=1,
    YCS_DELETED=2
} YCS_STATE;

typedef enum  {
    YCS_NONE=0,
    YCS_INIT=1,
    YCS_LOCK=2,
    YCS_LOCKTRY=3,
    YCS_RELEASE=4,
    YCS_DELETE=5
} YCS_ACTION;

typedef struct {
    int         thread;
    const char* fileid;
    int         lineno;
    YCS_ACTION  action;
} YCS_LOC;

#define YCS_NB_TRACE 5

typedef struct {
    volatile u32                 no;
    volatile YCS_STATE           state;
    volatile int                 lock;
#if defined(WINDOWS_API)
    CRITICAL_SECTION             cs;
#else
    pthread_mutex_t              cs;
#endif
    YCS_LOC             last_actions[YCS_NB_TRACE];
} yCRITICAL_SECTION_ST;

typedef yCRITICAL_SECTION_ST* yCRITICAL_SECTION;

void yInitDebugCS();
void yFreeDebugCS();
void yDbgInitializeCriticalSection(const char* fileid, int lineno, yCRITICAL_SECTION *cs);
void yDbgEnterCriticalSection(const char* fileid, int lineno, yCRITICAL_SECTION *cs);
int yDbgTryEnterCriticalSection(const char* fileid, int lineno, yCRITICAL_SECTION *cs);
void yDbgLeaveCriticalSection(const char* fileid, int lineno, yCRITICAL_SECTION *cs);
void yDbgDeleteCriticalSection(const char* fileid, int lineno, yCRITICAL_SECTION *cs);

#define yInitializeCriticalSection(cs)  yDbgInitializeCriticalSection(__FILE_ID__,__LINE__,cs)
#define yEnterCriticalSection(cs)       yDbgEnterCriticalSection(__FILE_ID__,__LINE__,cs)
#define yTryEnterCriticalSection(cs)    yDbgTryEnterCriticalSection(__FILE_ID__,__LINE__,cs)
#define yLeaveCriticalSection(cs)       yDbgLeaveCriticalSection(__FILE_ID__,__LINE__,cs)
#define yDeleteCriticalSection(cs)      yDbgDeleteCriticalSection(__FILE_ID__,__LINE__,cs)

#else
#if defined(MICROCHIP_API)
#define yCRITICAL_SECTION               u8
#define yInitializeCriticalSection(cs)
#define yEnterCriticalSection(cs)
#define yTryEnterCriticalSection(cs)    1
#define yLeaveCriticalSection(cs)
#define yDeleteCriticalSection(cs)
#else

typedef void* yCRITICAL_SECTION;
void yInitializeCriticalSection(yCRITICAL_SECTION *cs);
void yEnterCriticalSection(yCRITICAL_SECTION *cs);
int yTryEnterCriticalSection(yCRITICAL_SECTION *cs);
void yLeaveCriticalSection(yCRITICAL_SECTION *cs);
void yDeleteCriticalSection(yCRITICAL_SECTION *cs);
#endif
#endif


typedef enum {
    YAPI_SUCCESS          = 0,      // everything worked all right
    YAPI_NOT_INITIALIZED  = -1,     // call yInitAPI() first !
    YAPI_INVALID_ARGUMENT = -2,     // one of the arguments passed to the function is invalid
    YAPI_NOT_SUPPORTED    = -3,     // the operation attempted is (currently) not supported
    YAPI_DEVICE_NOT_FOUND = -4,     // the requested device is not reachable
    YAPI_VERSION_MISMATCH = -5,     // the device firmware is incompatible with this API version
    YAPI_DEVICE_BUSY      = -6,     // the device is busy with another task and cannot answer
    YAPI_TIMEOUT          = -7,     // the device took too long to provide an answer
    YAPI_IO_ERROR         = -8,     // there was an I/O problem while talking to the device
    YAPI_NO_MORE_DATA     = -9,     // there is no more data to read from
    YAPI_EXHAUSTED        = -10,    // you have run out of a limited resource, check the documentation
    YAPI_DOUBLE_ACCES     = -11,    // you have two process that try to access to the same device
    YAPI_UNAUTHORIZED     = -12,    // unauthorized access to password-protected device
    YAPI_RTC_NOT_READY    = -13,    // real-time clock has not been initialized (or time was lost)
    YAPI_FILE_NOT_FOUND   = -14     // the file is not found
} YRETCODE;

#define YISERR(retcode)   ((retcode) < 0)

// Yoctopuce arbitrary constants
#define YOCTO_API_VERSION_STR       "1.10"
#define YOCTO_API_VERSION_BCD       0x0110
#include "yversion.h"
#define YOCTO_DEFAULT_PORT          4444
#define YOCTO_VXI_PORT              4445
#define YOCTO_VENDORID              0x24e0
#define YOCTO_DEVID_FACTORYBOOT     1
#define YOCTO_DEVID_BOOTLOADER      2
#define YOCTO_DEVID_HIGHEST         0xfefe
#define YAPI_HASH_BUF_SIZE          28
#define YOCTO_CALIB_TYPE_OFS        30

// Other special ports
#define PORTMAPPER_PORT             111

// Known baseclases
#define YOCTO_AKA_YFUNCTION         0
#define YOCTO_AKA_YSENSOR           1
#define YOCTO_N_BASECLASSES         2

// Standard buffer sizes
#define YOCTO_ERRMSG_LEN            256
#define YOCTO_MANUFACTURER_LEN      20
#define YOCTO_SERIAL_LEN            20
#define YOCTO_BASE_SERIAL_LEN        8
#define YOCTO_PRODUCTNAME_LEN       28
#define YOCTO_FIRMWARE_LEN          22
#define YOCTO_LOGICAL_LEN           20
#define YOCTO_FUNCTION_LEN          20
#define YOCTO_UNIT_LEN              10
#define YOCTO_PUBVAL_SIZE            6 // Size of the data (can be non null terminated)
#define YOCTO_PUBVAL_LEN            16 // Temporary storage, >= YOCTO_PUBVAL_SIZE+2
#define YOCTO_REPORT_LEN             9 // Max size of a timed report, including isAvg flag
#define YOCTO_SERIAL_SEED_SIZE       (YOCTO_SERIAL_LEN - YOCTO_BASE_SERIAL_LEN - 1)

// websocket key from specification v13
#define YOCTO_WEBSOCKET_MAGIC             "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define YOCTO_WEBSOCKET_MAGIC_LEN         36


// firmware description
typedef union {
    u8      asBytes[YOCTO_FIRMWARE_LEN];
    struct {
        char    buildVersion[YOCTO_FIRMWARE_LEN-2];
        u16     yfsSignature;
    } data;
} yFirmwareSt;

// device description
typedef struct {
    u16     vendorid;
    u16     deviceid;
    u16     devrelease;
    u16     nbinbterfaces;
    char    manufacturer[YOCTO_MANUFACTURER_LEN];
    char    productname[YOCTO_PRODUCTNAME_LEN];
    char    serial[YOCTO_SERIAL_LEN];
    char    logicalname[YOCTO_LOGICAL_LEN];
    char    firmware[YOCTO_FIRMWARE_LEN];
    u8      beacon;
} yDeviceSt;

// definitions for USB protocl

#ifndef C30
#define Nop()
#pragma pack(push,1)
#endif

#define USB_PKT_SIZE            64
#define YPKT_USB_VERSION_NO_RETRY_BCD    0x0207
#define YPKT_USB_VERSION_BCD             0x0208
#define TO_SAFE_U16(safe,unsafe)        {(safe).low = (unsafe)&0xff; (safe).high=(unsafe)>>8;}
#define FROM_SAFE_U16(safe,unsafe)      {(unsafe) = (safe).low |((u16)((safe).high)<<8);}

typedef struct {
    u8 low;
    u8 high;
} SAFE_U16;

#ifndef CPU_BIG_ENDIAN

#define YPKTNOMSK   (0x7)
typedef struct {
    u8 pktno    : 3;
    u8 stream   : 5;
    u8 pkt      : 2;
    u8 size     : 6;
} YSTREAM_Head;
#else
#define YPKTNOMSK   (0x7)
typedef struct {
    u8 stream   : 5;
    u8 pktno    : 3;
    u8 size     : 6;
    u8 pkt      : 2;
} YSTREAM_Head;
#endif
#define YPKT_STREAM         0
#define YPKT_CONF           1

//
// YPKT_CONF packets format
//

#define USB_CONF_RESET      0
#define USB_CONF_START      1

typedef union{
    struct{
        SAFE_U16  api;
        u8  ok;
        u8  ifaceno;
        u8  nbifaces;
    } reset;
    struct{
        u8  nbifaces;
        u8  ack_delay;
    } start;
} USB_Conf_Pkt;

//
// YPKT_STREAM packets can encompass multiple streams
//

#define YSTREAM_EMPTY          0
#define YSTREAM_TCP            1
#define YSTREAM_TCP_CLOSE      2
#define YSTREAM_NOTICE         3
#define YSTREAM_REPORT         4
#define YSTREAM_META           5
#define YSTREAM_REPORT_V2      6
#define YSTREAM_NOTICE_V2      7
#define YSTREAM_TCP_NOTIF      8
#define YSTREAM_TCP_ASYNCCLOSE 9

// Data in YSTREAM_NOTICE stream

#define NOTIFY_1STBYTE_MAXTINY  63
#define NOTIFY_1STBYTE_MINSMALL 128

#define NOTIFY_PKT_NAME        0
#define NOTIFY_PKT_PRODNAME    1
#define NOTIFY_PKT_CHILD       2
#define NOTIFY_PKT_FIRMWARE    3
#define NOTIFY_PKT_FUNCNAME    4
#define NOTIFY_PKT_FUNCVAL     5
#define NOTIFY_PKT_STREAMREADY 6
#define NOTIFY_PKT_LOG         7
#define NOTIFY_PKT_FUNCNAMEYDX 8
#define NOTIFY_PKT_PRODINFO    9

#define NOTIFY_V2_LEGACY       0       // unused (reserved for compatibility with legacy notifications)
#define NOTIFY_V2_6RAWBYTES    1       // largest type: data is always 6 bytes
#define NOTIFY_V2_TYPEDDATA    2       // other types: first data byte holds the decoding format
#define NOTIFY_V2_FLUSHGROUP   3       // no data associated
// Higher values not allowed (Notification_funydx.raw must stay <= 63)

// New types of generic notifications
#define PUBVAL_LEGACY                       0   // 0-6 ASCII characters (normally sent as YSTREAM_NOTICE)
#define PUBVAL_1RAWBYTE                     1   // 1 raw byte  (=2 characters)
#define PUBVAL_2RAWBYTES                    2   // 2 raw bytes (=4 characters)
#define PUBVAL_3RAWBYTES                    3   // 3 raw bytes (=6 characters)
#define PUBVAL_4RAWBYTES                    4   // 4 raw bytes (=8 characters)
#define PUBVAL_5RAWBYTES                    5   // 5 raw bytes (=10 characters)
#define PUBVAL_6RAWBYTES                    6   // 6 hex bytes (=12 characters) (sent as V2_6RAWBYTES)
#define PUBVAL_C_LONG                       7   // 32-bit C signed integer
#define PUBVAL_C_FLOAT                      8   // 32-bit C float
#define PUBVAL_YOCTO_FLOAT_E3               9   // 32-bit Yocto fixed-point format (e-3)

typedef struct{
    char        serial[YOCTO_SERIAL_LEN];
    u8          type;
}Notification_header;

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif

typedef union {
    // This definition must match packet generated in yoctoPacket.c
    u8          raw;                    // originally shifted by NOTIFY_1STBYTE_MINSMALL for smallnot
    struct {
#ifndef CPU_BIG_ENDIAN
        u8      funydx:4;
        u8      typeV2:3;               // note: high bit MUST stay zero to ensure raw value <= 63 for tiny notifications
        u8      isSmall:1;
#else
        u8      isSmall:1;
        u8      typeV2:3;
        u8      funydx:4;
#endif
    } v2;
}Notification_funydx;

typedef struct{
    // This definition must match packet generated in yoctoPacket.c
    Notification_funydx funInfo;
    char        pubval[VARIABLE_SIZE]; // deduce actual size from YSTREAM_head
}Notification_tiny;

typedef struct{
    // This definition must match packet generated in yoctoPacket.c
    Notification_funydx funInfo;
    u8          devydx;
    char        pubval[VARIABLE_SIZE]; // deduce actual size from YSTREAM_head
}Notification_small;

#ifdef _MSC_VER
#pragma warning( pop )
#endif

typedef struct{
    char        name[YOCTO_LOGICAL_LEN];
    u8          beacon;
}Notification_name;

typedef char    Notification_product[YOCTO_PRODUCTNAME_LEN];

typedef struct {
    char        name[YOCTO_PRODUCTNAME_LEN];
    SAFE_U16    deviceid;
}Notification_prodinfo;

typedef struct {
    char        childserial[YOCTO_SERIAL_LEN];
    u8          onoff;
    u8          devydx;
}Notification_child;

typedef struct {
    char        firmware[YOCTO_FIRMWARE_LEN];
    SAFE_U16    vendorid;
    SAFE_U16    deviceid;
}Notification_firmware;

typedef struct {
    char        funcid[YOCTO_FUNCTION_LEN];
    char        funcname[YOCTO_LOGICAL_LEN];
}Notification_funcname;

typedef struct {
    char        funcid[YOCTO_FUNCTION_LEN];
    char        pubval[YOCTO_PUBVAL_SIZE];
}Notification_funcval;

typedef struct {
    char        funcidshort[YOCTO_FUNCTION_LEN-1];
    u8          funclass;       // 0..YOCTO_N_BASECLASSES-1
    char        funcname[YOCTO_LOGICAL_LEN];
    u8          funydx;
}Notification_funcnameydx;

typedef union {
    u8                  firstByte;
    Notification_tiny   tinypubvalnot;
    Notification_small  smallpubvalnot;
    struct {
        Notification_header head;
        union {
            Notification_name           namenot;
            Notification_product        productname;
            Notification_prodinfo       productinfo;
            Notification_child          childserial;
            Notification_firmware       firmwarenot;
            Notification_funcname       funcnamenot;
            Notification_funcval        pubvalnot;
            Notification_funcnameydx    funcnameydxnot;
            u8                          raw;
        };
    };
} USB_Notify_Pkt;

#define NOTIFY_NETPKT_NAME        '0'
#define NOTIFY_NETPKT_PRODNAME    '1'
#define NOTIFY_NETPKT_CHILD       '2'
#define NOTIFY_NETPKT_FIRMWARE    '3'
#define NOTIFY_NETPKT_FUNCNAME    '4'
#define NOTIFY_NETPKT_FUNCVAL     '5'
#define NOTIFY_NETPKT_STREAMREADY '6'
#define NOTIFY_NETPKT_LOG         '7'
#define NOTIFY_NETPKT_FUNCNAMEYDX '8'
#define NOTIFY_NETPKT_PRODINFO    '9'
#define NOTIFY_NETPKT_FLUSHV2YDX  't'
#define NOTIFY_NETPKT_FUNCV2YDX   'u'
#define NOTIFY_NETPKT_TIMEV2YDX   'v'
#define NOTIFY_NETPKT_DEVLOGYDX   'w'
#define NOTIFY_NETPKT_TIMEVALYDX  'x'
#define NOTIFY_NETPKT_FUNCVALYDX  'y'
#define NOTIFY_NETPKT_TIMEAVGYDX  'z'
#define NOTIFY_NETPKT_NOT_SYNC    '@'

#define NOTIFY_NETPKT_VERSION   "01"
#define NOTIFY_NETPKT_START     "YN01"
#define NOTIFY_NETPKT_START_LEN 4
#define NOTIFY_NETPKT_STOP      '\n'
#define NOTIFY_NETPKT_SEP       ','
#define NOTIFY_NETPKT_MAX_LEN   (NOTIFY_NETPKT_START_LEN+1+YOCTO_SERIAL_LEN+1+YOCTO_FUNCTION_LEN+1+YOCTO_LOGICAL_LEN+1+1)

#define NOTIFY_PKT_NAME_LEN             (sizeof(Notification_header) + sizeof(Notification_name))
#define NOTIFY_PKT_PRODNAME_LEN         (sizeof(Notification_header) + sizeof(Notification_product))
#define NOTIFY_PKT_PRODINFO_LEN         (sizeof(Notification_header) + sizeof(Notification_prodinfo))
#define NOTIFY_PKT_CHILD_LEN            (sizeof(Notification_header) + sizeof(Notification_child))
#define NOTIFY_PKT_FIRMWARE_LEN         (sizeof(Notification_header) + sizeof(Notification_firmware))
#define NOTIFY_PKT_STREAMREADY_LEN      (sizeof(Notification_header) + sizeof(u8))
#define NOTIFY_PKT_LOG_LEN              (sizeof(Notification_header) + sizeof(u8))
#define NOTIFY_PKT_FUNCNAME_LEN         (sizeof(Notification_header) + sizeof(Notification_funcname))
#define NOTIFY_PKT_FUNCVAL_LEN          (sizeof(Notification_header) + sizeof(Notification_funcval))
#define NOTIFY_PKT_FUNCNAMEYDX_LEN      (sizeof(Notification_header) + sizeof(Notification_funcnameydx))
#define NOTIFY_PKT_TINYVAL_MAXLEN       (sizeof(Notification_tiny) + YOCTO_PUBVAL_SIZE)

// Data in YSTREAM_REPORT stream
//
// Reports are always first in a packet, which
// makes it easy to filter packets that contain
// reports only, and interpret them as fixed offsets

typedef struct {
    union {
      struct {
#ifndef CPU_BIG_ENDIAN
        u8  funYdx:4;   // (LOWEST NIBBLE) function index on device, 0xf==timestamp
        u8  extraLen:3; // Number of extra data bytes in addition to first one
        u8  isAvg:1;    // (HIGHEST BIT) 0:one immediate value (1-4 bytes), 1:min/avg/max (2+4+2 bytes)
#else
        u8  isAvg:1;    // (HIGHEST BIT) 0:one immediate value (1-4 bytes), 1:min/avg/max (2+4+2 bytes)
        u8  extraLen:3; // Number of extra data bytes in addition to first one
        u8  funYdx:4;   // (LOWEST NIBBLE) function index on device, 0xf==timestamp
#endif
      };
      u8    head;
    };
    u8  data[1];        // Payload itself (numbers in little-endian format)
} USB_Report_Pkt_V1;

typedef struct {
    union {
      struct {
#ifndef CPU_BIG_ENDIAN
        u8  funYdx:4;   // (LOWEST NIBBLE) function index on device, 0xf==timestamp
        u8  extraLen:4; // Number of extra data bytes in addition to data[0]
#else
        u8  extraLen:4; // Number of extra data bytes in addition to data[0]
        u8  funYdx:4;   // (LOWEST NIBBLE) function index on device, 0xf==timestamp
#endif
      };
      u8    head;
    };
    union {
      // When extraLen >= 4 && funYdx != 0xf : first data byte describes the payload
      struct {
#ifndef CPU_BIG_ENDIAN
        u8  avgExtraLen:2;      // Number of extra bytes in the first value (average value, signed MeasureVal)
        u8  minDiffExtraLen:2;  // Number of extra bytes to encode the 2nd value (avg - min, always > 0)
        u8  maxDiffExtraLen:2;  // Number of extra bytes to encode the 3rd value (max - avg, always > 0)
        u8  reserved:2;         // unused, currently set to 0
#else
        u8  reserved:2;         // unused, currently set to 0
        u8  maxDiffExtraLen:2;  // Number of extra bytes to encode the 3rd value (max - avg)
        u8  minDiffExtraLen:2;  // Number of extra bytes to encode the 2nd value (avg - min)
        u8  avgExtraLen:2;      // Number of extra bytes in the first value (average)
#endif
      };
      // When extraLen <= 3 (up to 4 bytes of data): live report value (1-4 bytes)
      u8  data[1];        // Payload itself (numbers in little-endian format)
    };
} USB_Report_Pkt_V2;

// data format in USB_Report_Pkt_V2:
//


#define MAX_ASYNC_TCPCHAN 4


// Data in YSTREAM_META stream

#define USB_META_UTCTIME                1
#define USB_META_DLFLUSH                2
#define USB_META_ACK_D2H_PACKET         3
#define USB_META_WS_ANNOUNCE            4
#define USB_META_WS_AUTHENTICATION      5
#define USB_META_WS_ERROR               6
#define USB_META_ACK_UPLOAD             7

#define USB_META_UTCTIME_SIZE           5u
#define USB_META_DLFLUSH_SIZE           1u
#define USB_META_ACK_D2H_PACKET_SIZE    2u
#define USB_META_WS_ANNOUNCE_SIZE       (8 + YOCTO_SERIAL_LEN)
#define USB_META_WS_AUTHENTICATION_SIZE 28u
#define USB_META_WS_ERROR_SIZE          4u
#define USB_META_ACK_UPLOAD_SIZE        6u

#define USB_META_WS_PROTO_V1            1 // adding authentication support
#define USB_META_WS_PROTO_V2            2 // adding API packets throttling

#define USB_META_WS_AUTH_FLAGS_VALID    1 // set it if the sha1 and nonce are pertinent
#define USB_META_WS_AUTH_FLAGS_RW       2 // set it if RW acces are granted

typedef union {
    struct {
        u8  metaType;      // =USB_META_UTCTIME
        u8  unixTime[4];   // actually a DWORD in little-endian format
    } utcTime;
    struct {
        u8  metaType;      // =USB_META_DLFLUSH (flush datalogger)
    } dlFlush;
    struct {
        u8  metaType;      // =USB_META_ACK_D2H_PACKET (ack last device to host packet)
        u8  pktno;         // = last pktno that the host has received
    } pktAck;
    struct {
        u8  metaType;      // =USB_META_WS_ANNOUNCE (ack last device to host packet)
        u8  version;       // the version of the authentication
        u16 maxtcpws;      // maximum TCP window size, in multiples of 16 bytes
        u32 nonce;         // nonce append to password (in capital hex)
        char serial[YOCTO_SERIAL_LEN];
    } announce;
    struct {
        u8  metaType;      // =USB_META_WS_AUTHENTICATION (authenticate trafic)
        u8  version;       // the version of the authentication
        u16 flags;         // reserved bits
        u32 nonce;         // nonce append to password (in capital hex)
        u8  sha1[20];      // sha1 signature
    } auth;
    struct {
        u8  metaType;      // =USB_META_WS_ANNOUNCE (ack last device to host packet)
        u8  reserved;      // reserved (must be 0)
        u16 htmlcode;      // HTML error code
    } error;
    struct {
        u8  metaType;      // =USB_META_ACK_UPLOAD (ack upload progress every KB)
        u8  tcpchan;       // TCP channel index (as of proto v2, only available on tcpchan 0)
        u8 totalBytes[4];  // actually a DWORD in little-endian format, number of bytes received so far
    } uploadAck;
} USB_Meta_Pkt;



typedef union {
     struct {
#ifdef CPU_BIG_ENDIAN
        u8      stream : 5;           // Encapsulated protocol
        u8      tcpchan : 3;          // Encapsulated TCP subchannel
#else
        u8      tcpchan : 3;          // Encapsulated TCP subchannel
        u8      stream : 5;           // Encapsulated protocol
#endif
    };
    u8 encaps;
} WSStreamHead;




//
// SSDP global definitions for hubs
//

#define YSSDP_PORT 1900
#define YSSDP_MCAST_ADDR_STR  "239.255.255.250"
#define YSSDP_MCAST_ADDR (0xFAFFFFEF)
#define YSSDP_URN_YOCTOPUCE "urn:yoctopuce-com:device:hub:1"

// prototype of the async request completion callback
typedef void (*yapiRequestAsyncCallback)(void *context, const u8 *result, u32 resultlen, int retcode, const char *errmsg);
// prototype of the request progress callback
typedef void (*yapiRequestProgressCallback)(void *context, u32 acked, u32 totalbytes);



//
// PROG packets are only used in bootloader (USB DeviceID=0001/0002)
//

#define PROG_NOP         0 // nothing to do
#define PROG_REBOOT      1 // reset the device
#define PROG_ERASE       2 // erase completely the device
#define PROG_PROG        3 // program the device
#define PROG_VERIF       4 // program the device
#define PROG_INFO        5 // get device info
#define PROG_INFO_EXT    6 // get extended device info (flash bootloader only)

#define MAX_BYTE_IN_PACKET          (60)
#define MAX_INSTR_IN_PACKET         (MAX_BYTE_IN_PACKET/3)

#define ERASE_BLOCK_SIZE_INSTR      512               // the minimal erase size in nb instr
#define PROGRAM_BLOCK_SIZE_INSTR    64                // the minimal program size in nb instr
//define some addresses in bytes too
#define ERASE_BLOCK_SIZE_BADDR      (ERASE_BLOCK_SIZE_INSTR*2)
#define PROGRAM_BLOCK_SIZE_BADDR    (PROGRAM_BLOCK_SIZE_INSTR*2)


typedef union {
    u8  raw[64];
    u16 words[32];
    struct {
#ifndef CPU_BIG_ENDIAN
        u8  size : 5;
        u8  type : 3;
#else
        u8  type : 3;
        u8  size : 5;
#endif
        u8  addres_high;
        u16 adress_low;
        u8  data[MAX_BYTE_IN_PACKET];
    } pkt;
    struct {
#ifndef CPU_BIG_ENDIAN
        u8   size : 5;
        u8   type : 3;
#else
        u8   type : 3;
        u8   size : 5;
#endif
        u8   pad;
        u16  pr_blk_size;
        u16  devidl;
        u16  devidh;
        u32  settings_addr;
        u32  last_addr;
        u32  config_start;
        u32  config_stop;
        u16  er_blk_size;
    } pktinfo;
    struct {
#ifndef CPU_BIG_ENDIAN
        u8   size : 5;
        u8   type : 3;
#else
        u8   type : 3;
        u8   size : 5;
#endif
        u8   dwordpos_lo;
#ifndef CPU_BIG_ENDIAN
        u16  pageno : 14;
        u16  dwordpos_hi : 2;
#else
        u8  pageno_lo;
        u8  misc_hi;
#endif
        union {
            u16  npages;    // for PROG_ERASE
            u16  btsign;    // for PROG_REBOOT
            u8   data[MAX_BYTE_IN_PACKET]; // for PROG_PROG
        } opt;
    } pkt_ext;
    struct {
#ifndef CPU_BIG_ENDIAN
        u8   size : 5;
        u8   type : 3;
#else
        u8   type : 3;
        u8   size : 5;
#endif
        u8   version;
        u16  pr_blk_size;
        u16  devidl;
        u16  devidh;
        u32  settings_addr;
        u32  last_addr;
        u32  config_start;
        u32  config_stop;
        u16  er_blk_size;
        u16  ext_jedec_id;
        u16  ext_page_size;
        u16  ext_total_pages;
        u16  first_code_page;
        u16  first_yfs3_page;
    } pktinfo_ext;
} USB_Prog_Packet;

#ifndef CPU_BIG_ENDIAN
#define SET_PROG_POS_PAGENO(PKT_EXT, PAGENO, POS)  {\
                                (PKT_EXT).dwordpos_lo = (POS) & 0xff;\
                                (PKT_EXT).dwordpos_hi = ((POS) >> 8) & 3;\
                                (PKT_EXT).pageno = (PAGENO) & 0x3fff;}
#define GET_PROG_POS_PAGENO(PKT_EXT, PAGENO, POS)  {\
                                POS = (PKT_EXT).dwordpos_lo + ((u32)(PKT_EXT).dwordpos_hi <<8);\
                                PAGENO = (PKT_EXT).pageno;}

#define INTEL_U16(NUM)  (NUM)
#define INTEL_U32(NUM) (NUM)
#else
#define SET_PROG_POS_PAGENO(PKT_EXT, PAGENO, POS)  {\
                                (PKT_EXT).dwordpos_lo = (POS) & 0xff;\
                                (PKT_EXT).pageno_lo = (PAGENO) & 0xff;\
                                (PKT_EXT).misc_hi = (((PAGENO) >>8) & 0x3f)+ (((POS) & 0x300) >>2) ;}
#define GET_PROG_POS_PAGENO(PKT_EXT, PAGENO, POS)  {\
                                POS = (PKT_EXT).dwordpos_lo + (((u16)(PKT_EXT).misc_hi << 2) & 0x300);\
                                PAGENO = (PKT_EXT).pageno_lo + (((u16)(PKT_EXT).misc_hi & 0x3f) << 8);}
#define INTEL_U16(NUM) ((((NUM) & 0xff00) >> 8) | (((NUM)&0xff) << 8))
#define INTEL_U32(NUM) ((((NUM) >> 24) & 0xff) | (((NUM) << 8) & 0xff0000) | (((NUM) >> 8) & 0xff00) | (((NUM) << 24) & 0xff000000 ))

#endif



#define START_APPLICATION_SIGN   0
#define START_BOOTLOADER_SIGN   ('b'| ('T'<<8))
#define START_AUTOFLASHER_SIGN  ('b'| ('F'<<8))


typedef union {
    u8              data[USB_PKT_SIZE];
    u16             data16[USB_PKT_SIZE/2];
    u32             data32[USB_PKT_SIZE/4];
    YSTREAM_Head    first_stream;
    USB_Prog_Packet prog;
    struct{
        YSTREAM_Head    head;
        USB_Conf_Pkt    conf;
    } confpkt;
} USB_Packet;

#ifndef C30
#pragma pack(pop)
#endif

//device indentifications PIC24FJ256DA210 family
#define FAMILY_PIC24FJ256DA210 0X41
#define PIC24FJ128DA206     0x08
#define PIC24FJ128DA106     0x09
#define PIC24FJ128DA210     0x0A
#define PIC24FJ128DA110     0x0B
#define PIC24FJ256DA206     0x0C
#define PIC24FJ256DA106     0x0D
#define PIC24FJ256DA210     0x0E
#define PIC24FJ256DA110     0x0F

//device indentifications PIC24FJ64GB004 family
#define FAMILY_PIC24FJ64GB004 0x42
#define PIC24FJ32GB002      0x03
#define PIC24FJ64GB002      0x07
#define PIC24FJ32GB004      0x0B
#define PIC24FJ64GB004      0x0F

// Spansion Flash JEDEC id
#define JEDEC_SPANSION_4MB  0x16
#define JEDEC_SPANSION_8MB  0x17

#define YESC                (27u)

#ifdef  __cplusplus
}
#endif

#endif
