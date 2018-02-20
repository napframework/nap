/*********************************************************************
 *
 * $Id: yprog.h 25587 2016-10-18 14:38:13Z seb $
 *
 * Declaration of firmware upgrade functions
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

#ifndef YPROG_H
#define YPROG_H
#include "ydef.h"

#ifdef MICROCHIP_API
typedef int ProgIface;
#else
#include "yproto.h"
typedef yInterfaceSt ProgIface;
#endif

#if !defined(MICROCHIP_API) || (defined(YHUB) && defined(FLASH_FIRMW_FIRSTPAGE))
#define PROG_SUBDEV
#endif

#define MAX_ROM_ZONES_PER_FILES     16
#define MAX_FLASH_ZONES_PER_FILES   4

#define  BYN_SIGN  ((u32) ('B'| ((u16)'Y'<<8) | ((u32)'N'<<16) ))
#define  BYN_REV_V4 4
#define  BYN_REV_V5 5
#define  BYN_REV_V6 6

#ifndef C30
#pragma pack(push,1)
#endif


#define USE_V6_BYN_FILE


typedef struct{
    union {
        u32 sign;
        struct {
            char a;
            char b;
            char c;
            char d;
        } signchars;
    };
    u16 rev;
    char serial[YOCTO_SERIAL_LEN];
    char pictype[20];
    char product[YOCTO_PRODUCTNAME_LEN];
    char firmware[YOCTO_FIRMWARE_LEN];
}byn_head_sign;

typedef struct{
    u32 nbzones;
    u32 datasize;
}byn_head_v4;

typedef struct{
    char prog_version[YOCTO_FIRMWARE_LEN];
    u16 pad;
    u32 nbzones;
    u32 datasize;
}byn_head_v5;

typedef struct{
    u8  md5chk[16];
    char prog_version[YOCTO_FIRMWARE_LEN]; // 22 bytes
    u8  ROM_nb_zone;
    u8  FLA_nb_zone;
    u32 ROM_total_size;
    u32 FLA_total_size;
}byn_head_v6;

typedef struct{
    byn_head_sign h;
    union {
        byn_head_v6 v6;
        byn_head_v5 v5;
        byn_head_v4 v4;
     };
}byn_head_multi;



#define BYN_HEAD_SIZE_V4    (sizeof(byn_head_sign)+sizeof(byn_head_v4))
#define BYN_HEAD_SIZE_V5    (sizeof(byn_head_sign)+sizeof(byn_head_v5))
#define BYN_HEAD_SIZE_V6    (sizeof(byn_head_sign)+sizeof(byn_head_v6))
#define BYN_MD5_OFS_V6      (sizeof(byn_head_sign)+16)

typedef struct{
    u32 addr_page;
    u32 len;
}byn_zone;

#ifdef CPU_BIG_ENDIAN
void decode_byn_head_multi(byn_head_multi *byn_head);
void decode_byn_zone(byn_zone *zone);
#define DECODE_U16(NUM) ((((NUM) & 0xff00) >> 8) | (((NUM)&0xff) << 8))
#define DECODE_U32(NUM) ((((NUM) >> 24) & 0xff) | (((NUM) << 8) & 0xff0000) | (((NUM) >> 8) & 0xff00) | (((NUM) << 24) & 0xff000000 ))
#else
#define decode_byn_head_multi(dummy) {}
#define decode_byn_zone(dummy) {}
#define DECODE_U16(NUM)  (NUM)
#define DECODE_U32(NUM) (NUM)
#endif

typedef struct{
    u32 addr;
    u32 nbinstr;
    u32 nbblock;
    u8  *ptr;
    u32 len;
}romzone;

typedef struct{
    u32 page;
    u8  *ptr;
    u32 len;
}flashzone;

typedef struct {
    u32         nbrom;
    u32         nbflash;
    romzone     rom[MAX_ROM_ZONES_PER_FILES];
    flashzone   flash[MAX_FLASH_ZONES_PER_FILES];
}newmemzones;

typedef struct{
    ProgIface   iface;
    u32         pr_blk_size;
    u32         er_blk_size;
    u32         last_addr;
    u32         settings_addr;
    u8          devid_family;
    u8          devid_model;
    u16         devid_rev;
    u32         startconfig;
    u32         endofconfig;
#ifndef MICROCHIP_API
    u16         ext_jedec_id;
    u16         ext_page_size;
    u16         ext_total_pages;
    u16         first_code_page;
    u16         first_yfs3_page;
#endif
}BootloaderSt;

// from yfirmupd.c
extern BootloaderSt firm_dev;
extern USB_Packet   firm_pkt;

#ifndef C30
#pragma pack(pop)
#endif
YRETCODE yapiGetBootloadersDevs(char *serials, unsigned int maxNbSerial, unsigned int *totalBootladers, char *errmsg);

// Return 1 if the communication channel to the device is busy
// Return 0 if there is no ongoing transaction with the device
int ypIsSendBootloaderBusy(BootloaderSt *dev);

// Return 0 if there command was successfully queued for sending
// Return -1 if the output channel is busy and the command could not be sent
int ypSendBootloaderCmd(BootloaderSt *dev, const USB_Packet *pkt,char *errmsg);
// Return 0 if a reply packet was available and returned
// Return -1 if there was no reply available
int ypGetBootloaderReply(BootloaderSt *dev, USB_Packet *pkt,char *errmsg);
// Power cycle the device
int ypBootloaderShutdown(BootloaderSt *dev);
int IsValidBynHead(const byn_head_multi *head, u32 size, u16 flags, char *errmsg);

#ifndef MICROCHIP_API
const char* prog_GetCPUName(BootloaderSt *dev);
int ValidateBynCompat(const byn_head_multi *head, u32 size, const char *serial, u16 flags, BootloaderSt *dev, char *errmsg);
int IsValidBynFile(const byn_head_multi *head, u32 size, const char *serial, u16 flags, char *errmsg);
int BlockingRead(BootloaderSt *dev, USB_Packet *pkt, int maxwait, char *errmsg);
int SendDataPacket(BootloaderSt *dev, int program, u32 address, u8 *data, int nbinstr, char *errmsg);
#endif

//#define DEBUG_FIRMWARE
typedef enum
{
    YPROG_DONE = 0u,    // Finished with procedure
    YPROG_WAITING       // Waiting for asynchronous process to complete, call again later
} YPROG_RESULT;


#define MAX_FIRMWARE_LEN  0x100000ul
#define INVALID_FIRMWARE  0xfffffffful
#define FLASH_NB_REBOOT_RETRY  1

typedef enum{
    FLASH_FIND_DEV = 0,
#ifndef MICROCHIP_API
    FLASH_CONNECT,
#endif
    FLASH_GET_INFO,
    FLASH_VALIDATE_BYN,
    FLASH_ERASE,
    FLASH_WAIT_ERASE,
    FLASH_DOFLASH,
    FLASH_GET_INFO_BFOR_REBOOT,
    FLASH_REBOOT,
    FLASH_REBOOT_VALIDATE,
#ifndef MICROCHIP_API
    FLASH_AUTOFLASH,
#endif
    FLASH_SUCCEEDED,
    FLASH_DISCONNECT,
    FLASH_DONE
}FLASH_DEVICE_STATE;

typedef enum {
    FLASH_ZONE_START,
    FLASH_ZONE_PROG,
    FLASH_ZONE_READ,
    FLASH_ZONE_RECV_OK
} FLASH_ZONE_STATE;


#define BLOCK_FLASH_TIMEOUT       4000u
#define PROG_GET_INFO_TIMEOUT    10000u
#define ZONE_VERIF_TIMEOUT        4000u
#define FLASH_SUBDEV_TIMEOUT     59000u
#define YPROG_BOOTLOADER_TIMEOUT 20000u
#define YPROG_FORCE_FW_UPDATE    1u

#ifdef MICROCHIP_API
#define FLASH_ERRMSG_LEN        56
#else
#define FLASH_ERRMSG_LEN        YOCTO_ERRMSG_LEN
#endif


#define PROG_IN_ERROR 0x8000
typedef struct {
#ifndef MICROCHIP_API
    u8                  *firmware;
    yCRITICAL_SECTION   cs;
#endif
    u32                 len;
    union {
        byn_head_multi  bynHead;
        u8              bynBuff[sizeof(byn_head_multi)];
    };
    u16                 flags;
    u16                 currzone;
    s16                 progress;
    FLASH_DEVICE_STATE  stepA;
    FLASH_ZONE_STATE    zst;
    union {
        byn_zone        bz;
        u8              bzBuff[sizeof(byn_zone)];
    };
    yTime               timeout;
    u32                 zOfs;
    u32                 zNbInstr;
    u32                 stepB;
    u16                 flashErase;
    u16                 flashPage;
    u16                 flashAddr;
    char                errmsg[FLASH_ERRMSG_LEN];
} FIRMWARE_CONTEXT;

extern FIRMWARE_CONTEXT fctx;


// memo: u=universal y=yapi h=hub

#ifdef YAPI_IN_YDEVICE
#define uGetFirmware(ofs, dst, size) hProgGetFirmware(ofs, dst, size)
void hProgInit(void);
void hProgFree(void);
#else
#define uGetFirmware(ofs, dst, size) yGetFirmware(ofs, dst, size)
void yProgInit(void);
void yProgFree(void);
YRETCODE yapiCheckFirmware_internal(const char *serial, const char *rev, u32 flags, const char *path, char *buffer, int buffersize, int *fullsize, char *errmsg);
YRETCODE yapiUpdateFirmware_internal(const char *serial, const char *firmwarePath, const char *settings, int force, int startUpdate, char *msg);
#endif



#define uGetFirmwareBynHead(head_ptr) {uGetFirmware(0, (u8*)(head_ptr), sizeof(byn_head_multi));decode_byn_head_multi(head_ptr);}
#define uGetFirmwareBynZone(offset,zone_ptr) {uGetFirmware(offset,(u8*)(zone_ptr),sizeof(byn_zone)); decode_byn_zone(zone_ptr);}

YPROG_RESULT uFlashDevice(void);
int yNetHubGetBootloaders(const char *hubserial, char *buffer, char *errmsg);
#endif
