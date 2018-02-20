/*********************************************************************
 *
 * $Id: yprog.c 29739 2018-01-25 17:03:29Z seb $
 *
 * Implementation of firmware upgrade functions
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

#define __FILE_ID__ "yprog"
#include "ydef.h"
#ifdef YAPI_IN_YDEVICE
#include "Yocto/yocto.h"
#endif
#ifdef MICROCHIP_API
#include <Yocto/yapi_ext.h>
#else
#include "yproto.h"
#ifndef WINDOWS_API
#include <dirent.h>
#include <sys/stat.h>
#endif
#endif
#include "yhash.h"
#include "yjson.h"
#include "yprog.h"
#include <stdio.h>
//#define DEBUG_FIRMWARE

#ifndef YAPI_IN_YDEVICE
// Public implementation of uProgXXX function (used only in public API).
// Init and Free are automatically called from yapiInit and yapiFree

FIRMWARE_CONTEXT fctx;
// these two variable have been extracted from FIRMWARE_CONTEXT
// to prevent some compiler to misalign them (GCC on raspberry PI)
BootloaderSt            firm_dev;
USB_Packet              firm_pkt;

#ifdef __BORLANDC__
#pragma warn - 8066
#pragma warn - 8008
#pragma warn - 8065
#endif

void yProgInit(void)
{
    // BYN header must have an even number of bytes
    YASSERT((sizeof(byn_head_multi)& 1) == 0);

    memset(&fctx, 0, sizeof(fctx));
    fctx.stepA = FLASH_DONE;
    memset(&firm_dev, 0, sizeof(firm_dev));
    yContext->fuCtx.global_progress = 100;
    yInitializeCriticalSection(&fctx.cs);
}

void  yProgFree(void)
{
    int fuPending;
    do {

        yEnterCriticalSection(&fctx.cs);
        if (yContext->fuCtx.global_progress <0 || yContext->fuCtx.global_progress >= 100) {
            fuPending = 0;
        } else{
            fuPending = 1;
        }
        yLeaveCriticalSection(&fctx.cs);
        if (fuPending){
            yApproximateSleep(1);
        }
    } while (fuPending);

    if (yContext->fuCtx.serial)
        yFree(yContext->fuCtx.serial);
    if (yContext->fuCtx.firmwarePath)
        yFree(yContext->fuCtx.firmwarePath);
    if (yContext->fuCtx.settings)
        yFree(yContext->fuCtx.settings);
    yDeleteCriticalSection(&fctx.cs);
    memset(&fctx, 0, sizeof(fctx));
}

#endif

#ifdef MICROCHIP_API
static
#endif
const char* prog_GetCPUName(BootloaderSt *dev)
{
	const char * res="";
	switch(dev->devid_family){
	case FAMILY_PIC24FJ256DA210:
        switch(dev->devid_model){
#ifndef MICROCHIP_API
            case PIC24FJ128DA206 :
                return "PIC24FJ128DA206";
            case PIC24FJ128DA106 :
                return "PIC24FJ128DA106";
            case PIC24FJ128DA210 :
                return "PIC24FJ128DA210";
            case PIC24FJ128DA110 :
                return "PIC24FJ128DA110";
            case PIC24FJ256DA206 :
                return "PIC24FJ256DA206";
            case PIC24FJ256DA106 :
                return "PIC24FJ256DA106";
            case PIC24FJ256DA210 :
                return "PIC24FJ256DA210";
            case PIC24FJ256DA110 :
                return "PIC24FJ256DA110";
			default:
			   res = "Unknown CPU model(family PIC24FJ256DA210)";
			   break;
#else
            case PIC24FJ256DA206 :
                return "PIC24FJ256DA206";
            default: ;
#endif
		}
        break;
    case FAMILY_PIC24FJ64GB004:
        switch(dev->devid_model){
#ifndef MICROCHIP_API
            case PIC24FJ32GB002 :
                return "PIC24FJ32GB002";
            case PIC24FJ64GB002 :
                return "PIC24FJ64GB002";
            case PIC24FJ32GB004 :
                return "PIC24FJ32GB004";
            case PIC24FJ64GB004 :
                return "PIC24FJ64GB004";
            default:
				res= "Unknown CPU model(family PIC24FJ64GB004)";
				break;
#else
            case PIC24FJ64GB002 :
                return "PIC24FJ64GB002";
			default:
				break;
#endif
        }
        break;
    }
	return res;
}


//used by yprogrammer
static int  checkHardwareCompat(BootloaderSt *dev,const char *pictype)
{
    const char *cpuname=prog_GetCPUName(dev);
    if(YSTRICMP(cpuname,pictype)!=0){
        return 0;
    }
    return 1;
}



#ifdef MICROCHIP_API

int IsValidBynHead(const byn_head_multi *head, u32 size, u16 flags, char *errmsg)
{
    if(head->h.sign != BYN_SIGN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Not a firmware file");
    }
    if(YSTRLEN(head->h.serial) >= YOCTO_SERIAL_LEN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Bad serial");
    }
    if(YSTRLEN(head->h.product) >= YOCTO_PRODUCTNAME_LEN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Bad product name");
    }
    if(YSTRLEN(head->h.firmware) >= YOCTO_FIRMWARE_LEN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Bad firmware revision");
    }
    switch(head->h.rev) {
        case BYN_REV_V4:
            if( head->v4.nbzones > MAX_ROM_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many zones");
            }
            if(head->v4.datasize != size -(sizeof(byn_head_sign)+sizeof(byn_head_v4))){
                return YERRMSG(YAPI_INVALID_ARGUMENT, "Incorrect file size");
            }
            return YAPI_SUCCESS;
        case BYN_REV_V5:
            //we do not check prog_version on YoctoHubs on purpose
            if( head->v5.nbzones > MAX_ROM_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many zones");
            }
            if(head->v5.datasize != size -(sizeof(byn_head_sign)+sizeof(byn_head_v5))){
                return YERRMSG(YAPI_INVALID_ARGUMENT, "Incorrect file size");
            }
            return YAPI_SUCCESS;
        case BYN_REV_V6:
            //we do not check prog_version on YoctoHubs on purpose
            if( head->v6.ROM_nb_zone > MAX_ROM_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many ROM zones");
            }
            if( head->v6.FLA_nb_zone > MAX_FLASH_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many FLASH zones");
            }
            return YAPI_SUCCESS;
        default:
            break;
    }
    return YERRMSG(YAPI_INVALID_ARGUMENT, "Please upgrade the hub device first");
}

#else

int IsValidBynHead(const byn_head_multi *head, u32 size, u16 flags, char *errmsg)
{
    if(head->h.sign != BYN_SIGN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Not a valid .byn file");
    }
    if(YSTRLEN(head->h.serial) >= YOCTO_SERIAL_LEN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid serial");
    }
    if(YSTRLEN(head->h.product) >= YOCTO_PRODUCTNAME_LEN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid product name");
    }
    if(YSTRLEN(head->h.firmware) >= YOCTO_FIRMWARE_LEN){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid firmware revision");
    }

    switch(head->h.rev) {
        case BYN_REV_V4:
            if( head->v4.nbzones > MAX_ROM_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many zones in .byn file");
            }
            if(head->v4.datasize != size -(sizeof(byn_head_sign)+sizeof(byn_head_v4))){
               return YERRMSG(YAPI_INVALID_ARGUMENT, "Incorrect file size or corrupt file");
            }
            return YAPI_SUCCESS;
        case BYN_REV_V5:
            if(YSTRLEN(head->v5.prog_version) >= YOCTO_SERIAL_LEN){
                return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid programming tools revision or corrupt file");
            }
#ifndef YBUILD_PATCH_WITH_BUILD
            if((flags & YPROG_FORCE_FW_UPDATE) == 0 && head->v5.prog_version[0]){
                 int byn = atoi(head->v5.prog_version);
                 int tools=atoi(YOCTO_API_BUILD_NO);
                 if(byn>tools){
                     return YERRMSG(YAPI_VERSION_MISMATCH, "This firmware is too recent, please upgrade your VirtualHub or Yoctopuce library");
                 }
            }
#endif
            if( head->v5.nbzones > MAX_ROM_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many zones in .byn file");
            }
            if(head->v5.datasize != size -(sizeof(byn_head_sign)+sizeof(byn_head_v5))){
               return YERRMSG(YAPI_INVALID_ARGUMENT, "Incorrect file size or corrupt file");
            }
            return YAPI_SUCCESS;
        case BYN_REV_V6:
            if(YSTRLEN(head->v6.prog_version) >= YOCTO_SERIAL_LEN){
                return YERRMSG(YAPI_INVALID_ARGUMENT, "Invalid programming tools revision or corrupt file");
            }
#ifndef YBUILD_PATCH_WITH_BUILD
            if ((flags & YPROG_FORCE_FW_UPDATE) == 0 && head->v6.prog_version[0]){
                int byn = atoi(head->v6.prog_version);
                int tools=atoi(YOCTO_API_BUILD_NO);
                if(byn>tools){
                    return YERRMSG(YAPI_VERSION_MISMATCH, "This firmware is too recent, please upgrade your VirtualHub or Yoctopuce library");
                }
            }
#endif
            if( head->v6.ROM_nb_zone > MAX_ROM_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many ROM zones in .byn file");
            }
            if( head->v6.FLA_nb_zone > MAX_FLASH_ZONES_PER_FILES){
                return YERRMSG(YAPI_INVALID_ARGUMENT,"Too many FLASH zones in .byn file");
            }
            return YAPI_SUCCESS;
        default:
            break;
    }
    return YERRMSG(YAPI_INVALID_ARGUMENT, "Unsupported file format, please upgrade your VirtualHub or Yoctopuce library");
}
#endif

int ValidateBynCompat(const byn_head_multi *head, u32 size, const char *serial, u16 flags, BootloaderSt *dev, char *errmsg)
{
    YPROPERR(IsValidBynHead(head, size, flags, errmsg));
    if(serial && YSTRNCMP(head->h.serial,serial,YOCTO_BASE_SERIAL_LEN)!=0){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "This BYN file is not designed for your device");
    }
    if(dev && !checkHardwareCompat(dev,head->h.pictype)){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "This BYN file is not designed for your device");
    }
    return 0;
}

#ifndef MICROCHIP_API
// user by yprogrammer
int  IsValidBynFile(const byn_head_multi *head, u32 size, const char *serial, u16 flags, char *errmsg)
{
    HASH_SUM ctx;
    u8       md5res[16];
    int      res;

    res = ValidateBynCompat(head, size, serial, flags, NULL, errmsg);
    if(res == YAPI_SUCCESS && head->h.rev == BYN_REV_V6) {
        // compute MD5
        MD5Initialize(&ctx);
        MD5AddData(&ctx, ((u8*)head)+BYN_MD5_OFS_V6, size-BYN_MD5_OFS_V6);
        MD5Calculate(&ctx, md5res);
        if(memcmp(md5res, head->v6.md5chk, 16)) {
            return YERRMSG(YAPI_INVALID_ARGUMENT,"Invalid checksum");
        }
    }
    return res;
}
#endif

#ifdef CPU_BIG_ENDIAN

#define BSWAP_U16(NUM)  (((NUM )>> 8) | ((NUM) << 8))
#define BSWAP_U32(NUM) ((((NUM) >> 24) & 0xff) | (((NUM) << 8) & 0xff0000) | (((NUM) >> 8) & 0xff00) | (((NUM) << 24) & 0xff000000 ))

void decode_byn_head_multi(byn_head_multi *head)
{
    head->h.sign = BSWAP_U32(head->h.sign);
    head->h.rev = BSWAP_U16(head->h.rev);
    switch (head->h.rev) {
    case BYN_REV_V4:
        head->v4.nbzones = BSWAP_U32(head->v4.nbzones);
        head->v4.datasize = BSWAP_U32(head->v4.datasize);
        break;
    case BYN_REV_V5:
        head->v5.pad = BSWAP_U16(head->v5.pad);
        head->v5.nbzones = BSWAP_U32(head->v5.nbzones);
        head->v5.datasize = BSWAP_U32(head->v5.datasize);
        break;
    case BYN_REV_V6:
        head->v6.ROM_total_size = BSWAP_U32(head->v6.ROM_total_size);
        head->v6.FLA_total_size = BSWAP_U32(head->v6.FLA_total_size);
        break;
    default:
        break;
    }
}

void decode_byn_zone(byn_zone *zone)
{
    zone->addr_page = BSWAP_U32(zone->addr_page);
    zone->len = BSWAP_U32(zone->len);
}

#endif


#if !defined(MICROCHIP_API)
// Return 1 if the communication channel to the device is busy
// Return 0 if there is no ongoing transaction with the device
int ypIsSendBootloaderBusy(BootloaderSt *dev)
{
    return 0;
}


// Return 0 if there command was successfully queued for sending
// Return -1 if the output channel is busy and the command could not be sent
int ypSendBootloaderCmd(BootloaderSt *dev, const USB_Packet *pkt,char *errmsg)
{
	return yyySendPacket(&dev->iface,pkt,errmsg);
}

// Return 0 if a reply packet was available and returned
// Return -1 if there was no reply available or on error
int ypGetBootloaderReply(BootloaderSt *dev, USB_Packet *pkt,char *errmsg)
{
	pktItem *ptr;
    // clear the dest buffer to avoid any misinterpretation
    memset(pkt->prog.raw, 0, sizeof(USB_Packet));
	YPROPERR(yPktQueueWaitAndPopD2H(&dev->iface,&ptr,10,errmsg));
    if(ptr){
        yTracePtr(ptr);
        memcpy(pkt,&ptr->pkt,sizeof(USB_Packet));
        yFree(ptr);
        return 0;
    }
	return YAPI_TIMEOUT; // not a fatal error, handled by caller
}
#endif



#if !defined(MICROCHIP_API)
//pool a packet form usb for a specific device
int BlockingRead(BootloaderSt *dev,USB_Packet *pkt, int maxwait, char *errmsg)
{
	pktItem *ptr;
	YPROPERR(yPktQueueWaitAndPopD2H(&dev->iface,&ptr,maxwait,errmsg));
    if (ptr) {
	    yTracePtr(ptr);
		memcpy(pkt,&ptr->pkt,sizeof(USB_Packet));
		yFree(ptr);
        return YAPI_SUCCESS;
	}
	return YERR(YAPI_TIMEOUT);
}

int SendDataPacket( BootloaderSt *dev,int program, u32 address, u8 *data,int nbinstr,char *errmsg)
{

    USB_Packet  pkt;
    //USB_Prog_Packet *pkt = &dev->iface.txqueue->pkt.prog;
    memset(&pkt.prog,0,sizeof(USB_Prog_Packet));
    if(program){
        pkt.prog.pkt.type = PROG_PROG;
    }else{
        pkt.prog.pkt.type = PROG_VERIF;
    }
    pkt.prog.pkt.adress_low = address &0xffff;
    pkt.prog.pkt.addres_high = (address>>16)&0xff;
    if(nbinstr > MAX_INSTR_IN_PACKET){
        nbinstr = MAX_INSTR_IN_PACKET;
    }
    if(nbinstr){
        memcpy(pkt.prog.pkt.data,data,nbinstr*3);
        pkt.prog.pkt.size= nbinstr;
    }

    YPROPERR(ypSendBootloaderCmd(dev,&pkt,errmsg));
    return nbinstr;
}



int yUSBGetBooloader(const char *serial, const char * name,  yInterfaceSt *iface,char *errmsg)
{

    int             nbifaces=0;
    yInterfaceSt    *curif;
    yInterfaceSt    *runifaces=NULL;
    int             i;

    YPROPERR(yyyUSBGetInterfaces(&runifaces,&nbifaces,errmsg));
    //inspect all interfaces
    for(i=0, curif = runifaces ; i < nbifaces ; i++, curif++){
        // skip real devices
        if(curif->deviceid >YOCTO_DEVID_BOOTLOADER)
            continue;
#ifdef WINDOWS_API
        if(name !=NULL && YSTRICMP(curif->devicePath,name)==0){
            if (iface)
                memcpy(iface,curif,sizeof(yInterfaceSt));
            yFree(runifaces);
            return YAPI_SUCCESS;
        }else
#endif
        if(serial!=NULL && YSTRCMP(curif->serial,serial)==0){
            if (iface)
                memcpy(iface,curif,sizeof(yInterfaceSt));
            yFree(runifaces);
            return YAPI_SUCCESS;
        }
    }
    // free all tmp ifaces
    if(runifaces){
        yFree(runifaces);
    }
    return YERR(YAPI_DEVICE_NOT_FOUND);
}

#endif

#ifndef YAPI_IN_YDEVICE
static int yLoadFirmwareFile(const char * filename, u8 **buffer, char *errmsg)
{
    FILE *f = NULL;
    int  size;
    int  readed;
    u8   *ptr;

    *buffer = NULL;
    if (YFOPEN(&f, filename, "rb") != 0) {
        return YERRMSG(YAPI_IO_ERROR, "unable to access file");
    }
    fseek(f, 0, SEEK_END);
    size = (int)ftell(f);
    if (size > 0x100000 || size <= 0){
        fclose(f);
        return YERR(YAPI_IO_ERROR);
    }
    ptr = yMalloc(size);
    if (ptr == NULL) {
        fclose(f);
        return YERR(YAPI_IO_ERROR);
    }
    fseek(f, 0, SEEK_SET);
    readed = (int)fread(ptr, 1, size, f);
    fclose(f);
    if (readed != size) {
        yFree(ptr);
        return YERRMSG(YAPI_IO_ERROR, "short read");
    }
    *buffer = ptr;
    return size;
}


static void yGetFirmware(u32 ofs, u8 *dst, u16 size)
{
    YASSERT(fctx.firmware);
    YASSERT(ofs + size <= fctx.len);
    memcpy(dst, fctx.firmware + ofs, size);
}


#endif



#ifdef YAPI_IN_YDEVICE
    #define ulog ylog
    #define ulogU16 ylogU16
    #define ulogChar ylogChar
    #define uLogProgress(msg) yProgLogProgress(msg)


    // report progress for devices and vhub
    static void yProgLogProgress(const char *msg)
    {
        yEnterCriticalSection(&fctx.cs);
        YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN, msg);
        hProgLog(msg);
        yLeaveCriticalSection(&fctx.cs);
    }
#else
    #define ytime() ((u32) yapiGetTickCount())
    #define Flash_ready()  1
    #define ulog(str) dbglog("%s",str)
    #define ulogU16(val) dbglog("%x",val)
    #define ulogChar(val) dbglog("%c",val)

    // report progress for Yoctolib
    #define setOsGlobalProgress(prog, msg) osProgLogProgressEx(__FILE_ID__,__LINE__, prog, msg)
    #define uLogProgress(msg) yProgLogProgress(msg)


    // report progress for devices and vhub
    static void yProgLogProgress(const char *msg)
    {
        yEnterCriticalSection(&fctx.cs);
        YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN, msg);
        yLeaveCriticalSection(&fctx.cs);
    }


    static void osProgLogProgressEx(const char *fileid, int line, int prog, const char *msg)
    {
        yEnterCriticalSection(&fctx.cs);
        if (prog != 0){
           yContext->fuCtx.global_progress = prog;
        }
        if (msg != NULL && *msg != 0){
#ifdef DEBUG_FIRMWARE
            dbglog("%s:%d:(%d%%) %s\n", fileid, line, prog, msg);
            YSPRINTF(yContext->fuCtx.global_message, YOCTO_ERRMSG_LEN, "%s:%d:%s", fileid, line, msg);
#else
            YSTRCPY(yContext->fuCtx.global_message, YOCTO_ERRMSG_LEN, msg);
#endif
        }
        yLeaveCriticalSection(&fctx.cs);
    }

#endif

#ifdef MICROCHIP_API
#define uGetBootloader(serial,ifaceptr)   yGetBootloaderPort(serial,ifaceptr)
#else
#define uGetBootloader(serial,ifaceptr)   yUSBGetBooloader(serial, NULL, ifaceptr,NULL)
#endif




//-1 = error 0= retry 1= ok (the global fctx.stepA is allready updated)
static int uGetDeviceInfo(void)
{
    switch(fctx.stepB){
    case 0:
        fctx.stepB++;
        fctx.timeout = ytime() + PROG_GET_INFO_TIMEOUT;
        // no break on purpose;
    case 1:
        memset(&firm_pkt,0,sizeof(USB_Prog_Packet));
        firm_pkt.prog.pkt.type = PROG_INFO;
        if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
            if((s32)(fctx.timeout - ytime()) < 0) {
#ifdef DEBUG_FIRMWARE
                ulog("Cannot send GetInfo pkt\n");
#endif
                YSTRCPY(fctx.errmsg, FLASH_ERRMSG_LEN, "Cannot send GetInfo");
                return -1;
            }
            return 0;
        }
        fctx.stepB++;
        fctx.timeout = ytime() + PROG_GET_INFO_TIMEOUT;
        // no break on purpose;
    case 2:
        if(ypGetBootloaderReply(&firm_dev, &firm_pkt,NULL)<0){
            if((s32)(fctx.timeout - ytime()) < 0) {
#ifdef DEBUG_FIRMWARE
                ulog("Bootloader did not respond to GetInfo pkt\n");
#endif
                YSTRCPY(fctx.errmsg, FLASH_ERRMSG_LEN, "Cannot recv GetInfo");
                return -1;
            }
            return 0;
        }
        fctx.stepB++;
        // no break on purpose;
    case 3:
        if(firm_pkt.prog.pkt.type == PROG_INFO) {
#ifdef DEBUG_FIRMWARE
            ulog("PROG_INFO received\n");
#endif
            firm_dev.er_blk_size   = DECODE_U16(firm_pkt.prog.pktinfo.er_blk_size);
            firm_dev.pr_blk_size   = DECODE_U16(firm_pkt.prog.pktinfo.pr_blk_size);
            firm_dev.last_addr     = DECODE_U32(firm_pkt.prog.pktinfo.last_addr);
            firm_dev.settings_addr = DECODE_U32(firm_pkt.prog.pktinfo.settings_addr);
            firm_dev.devid_family  = DECODE_U16(firm_pkt.prog.pktinfo.devidl)>>8;
            firm_dev.devid_model   = DECODE_U16(firm_pkt.prog.pktinfo.devidl) & 0xff;
            firm_dev.devid_rev     = DECODE_U16(firm_pkt.prog.pktinfo.devidh);
            firm_dev.startconfig   = DECODE_U32(firm_pkt.prog.pktinfo.config_start);
            firm_dev.endofconfig   = DECODE_U32(firm_pkt.prog.pktinfo.config_stop);
#ifndef MICROCHIP_API
            firm_dev.ext_jedec_id    = 0xffff;
            firm_dev.ext_page_size   = 0xffff;
            firm_dev.ext_total_pages = 0;
            firm_dev.first_code_page = 0xffff;
            firm_dev.first_yfs3_page = 0xffff;
#endif
            uLogProgress("Device info retrieved");
            fctx.stepB = 0;
            fctx.stepA = FLASH_VALIDATE_BYN;
#ifndef MICROCHIP_API
        } else if(firm_pkt.prog.pkt.type == PROG_INFO_EXT) {
#ifdef DEBUG_FIRMWARE
            ulog("PROG_INFO_EXT received\n");
#endif
            firm_dev.er_blk_size   = DECODE_U16(firm_pkt.prog.pktinfo_ext.er_blk_size);
            firm_dev.pr_blk_size   = DECODE_U16(firm_pkt.prog.pktinfo_ext.pr_blk_size);
            firm_dev.last_addr     = DECODE_U32(firm_pkt.prog.pktinfo_ext.last_addr);
            firm_dev.settings_addr = DECODE_U32(firm_pkt.prog.pktinfo_ext.settings_addr);
            firm_dev.devid_family  = DECODE_U16(firm_pkt.prog.pktinfo_ext.devidl) >> 8;
            firm_dev.devid_model   = DECODE_U16(firm_pkt.prog.pktinfo_ext.devidl) & 0xff;
            firm_dev.devid_rev     = DECODE_U16(firm_pkt.prog.pktinfo_ext.devidh);
            firm_dev.startconfig   = DECODE_U32(firm_pkt.prog.pktinfo_ext.config_start);
            firm_dev.endofconfig   = DECODE_U32(firm_pkt.prog.pktinfo_ext.config_stop);
            firm_dev.ext_jedec_id    = DECODE_U16(firm_pkt.prog.pktinfo_ext.ext_jedec_id);
            firm_dev.ext_page_size   = DECODE_U16(firm_pkt.prog.pktinfo_ext.ext_page_size);
            firm_dev.ext_total_pages = DECODE_U16(firm_pkt.prog.pktinfo_ext.ext_total_pages);
            firm_dev.first_code_page = DECODE_U16(firm_pkt.prog.pktinfo_ext.first_code_page);
            firm_dev.first_yfs3_page = DECODE_U16(firm_pkt.prog.pktinfo_ext.first_yfs3_page);
            uLogProgress("Device info retrieved");
            fctx.stepB = 0;
            fctx.stepA = FLASH_VALIDATE_BYN;
#endif
        } else {
#ifdef DEBUG_FIRMWARE
            ulog("Not a PROG_INFO pkt\n");
#endif
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Invalid prog pkt");
            return -1;
        }
        return 1;
#ifdef DEBUG_FIRMWARE
    default:
        ulog("invalid step in uGetDeviceInfo\n");
        break;
#endif
    }
    return 0;
}




static int uSendCmd(u8 cmd,FLASH_DEVICE_STATE nextState)
{
    if(ypIsSendBootloaderBusy(&firm_dev)) {
        return 0;
    }
    memset(&firm_pkt,0,sizeof(USB_Packet));
    firm_pkt.prog.pkt.type = cmd;
    if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
        return -1;
    }
    fctx.stepA = nextState;
    return 1;
}

static int uFlashZone()
{
    u16         datasize;
    char msg[FLASH_ERRMSG_LEN];

    switch(fctx.zst){
        case FLASH_ZONE_START:
            if(fctx.currzone == fctx.bynHead.v6.ROM_nb_zone + fctx.bynHead.v6.FLA_nb_zone){
                fctx.stepA = FLASH_GET_INFO_BFOR_REBOOT;
                fctx.stepB = 0;
                fctx.zOfs = FLASH_NB_REBOOT_RETRY;
                return 0;
            }
            uGetFirmwareBynZone(fctx.zOfs, &fctx.bz);
            YSTRCPY(msg, FLASH_ERRMSG_LEN, "Flash zone");
#if defined(DEBUG_FIRMWARE)
#ifdef MICROCHIP_API
            {
                char *p = msg + 10;
                *p++ = ' ';
                u16toa(fctx.currzone, p);
                p += ystrlen(p);
                *p++ = ':';
                u32toa(fctx.zOfs, p);
                p += ystrlen(p);
            }
#else
            YSPRINTF(msg, FLASH_ERRMSG_LEN, "Flash zone %d:%d : %x(%x)", fctx.currzone, fctx.zOfs, fctx.bz.addr_page, fctx.bz.len);
            dbglog("Flash zone %d:%x : %x(%x)\n",fctx.currzone,fctx.zOfs,fctx.bz.addr_page,fctx.bz.len);
#endif
#endif
            uLogProgress(msg);
        if((fctx.bz.addr_page % (firm_dev.pr_blk_size*2)) !=0 ) {
                YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"ProgAlign");
                return -1;
            }
            fctx.zOfs += sizeof(byn_zone);
            fctx.zNbInstr = fctx.bz.len/3;
            fctx.stepB    = 0;
            if(fctx.zNbInstr < (u32)firm_dev.pr_blk_size){
                YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"ProgSmall");
                return -1;
            }
            fctx.zst = FLASH_ZONE_PROG;
            //no break on purpose
        case FLASH_ZONE_PROG:
            if(ypIsSendBootloaderBusy(&firm_dev)) {
                return 0;
            }
            memset(&firm_pkt,0,sizeof(USB_Packet));
            firm_pkt.prog.pkt.type = PROG_PROG;
            firm_pkt.prog.pkt.adress_low = DECODE_U16(fctx.bz.addr_page & 0xffff);
            firm_pkt.prog.pkt.addres_high = (fctx.bz.addr_page>>16) & 0xff;
            firm_pkt.prog.pkt.size = (u8) (fctx.zNbInstr < MAX_INSTR_IN_PACKET? fctx.zNbInstr : MAX_INSTR_IN_PACKET) ;

            datasize = firm_pkt.prog.pkt.size*3;
            uGetFirmware(fctx.zOfs, firm_pkt.prog.pkt.data, datasize);
            //dbglog("Flash zone %d:0x%x  0x%x(%d /%d)\n", fctx.currzone, fctx.zOfs, fctx.bz.addr_page, fctx.stepB, firm_dev.pr_blk_size);
            if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
                YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"ProgPkt");
                return -1;
            }

            fctx.zOfs     += datasize;
            fctx.zNbInstr -= firm_pkt.prog.pkt.size;
            fctx.stepB    += firm_pkt.prog.pkt.size;
            fctx.progress = (u16)(4 + 92*fctx.zOfs / fctx.len);

            if( fctx.stepB >= firm_dev.pr_blk_size){
                //look for confirmation
                fctx.timeout =ytime()+ BLOCK_FLASH_TIMEOUT;
                fctx.zst =  FLASH_ZONE_RECV_OK;
            }
            break;
        case FLASH_ZONE_RECV_OK:
            if(ypGetBootloaderReply(&firm_dev, &firm_pkt,NULL)<0){
                if((s32)(fctx.timeout - ytime()) < 0) {
#if defined(DEBUG_FIRMWARE) && !defined(MICROCHIP_API)
                    dbglog("Bootlaoder did not send confirmation for Zone %x Block %x\n",fctx.currzone,fctx.bz.addr_page);
#endif
                    YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"ProgDead");
                    return -1;
                }
                return 0;
            }
            if(firm_pkt.prog.pkt.type != PROG_PROG){
                YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"ProgReply");
                return -1;
            }else{
                u32 newblock = ((u32)firm_pkt.prog.pkt.addres_high <<16) | DECODE_U16(firm_pkt.prog.pkt.adress_low);
                //uLogProgress("Block %x to %x is done",fctx.zStartAddr,newblock);
                fctx.bz.addr_page = newblock;
            }
            fctx.stepB = fctx.stepB - firm_dev.pr_blk_size;
            if(fctx.zNbInstr==0){
                fctx.zst =  FLASH_ZONE_START;
                fctx.currzone++;
            }else{
                fctx.zst =  FLASH_ZONE_PROG;
            }
            break;
        default:
            YASSERT(0);
    }

    return 0;
}



#ifndef MICROCHIP_API

static void uSendReboot(u16 signature, FLASH_DEVICE_STATE nextState)
{
    if(ypIsSendBootloaderBusy(&firm_dev))
        return;
    memset(&firm_pkt,0,sizeof(USB_Packet));
    firm_pkt.prog.pkt_ext.type = PROG_REBOOT;
    firm_pkt.prog.pkt_ext.opt.btsign = DECODE_U16(signature);
    // do not check reboot packet on purpose (most of the time
    // the os generate an error because the device rebooted too quickly)
    ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL);
    fctx.stepA =  nextState;
    return;
}

static int uSendErase(u16 firstPage, u16 nPages, FLASH_DEVICE_STATE nextState)
{
    if(ypIsSendBootloaderBusy(&firm_dev))
        return 0;
    memset(&firm_pkt,0,sizeof(USB_Packet));
    firm_pkt.prog.pkt_ext.type = PROG_ERASE;
    SET_PROG_POS_PAGENO(firm_pkt.prog.pkt_ext, firstPage, 0);
    firm_pkt.prog.pkt_ext.opt.npages = DECODE_U16(nPages);
    if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
        return -1;
    }
    fctx.stepA =  nextState;
    return 0;
}

static int uFlashFlash()
{
    u32 addr, datasize;
    u8  buff[MAX_BYTE_IN_PACKET];
    char msg[FLASH_ERRMSG_LEN];
    u32 pos, pageno;

    switch(fctx.zst){
    case FLASH_ZONE_START:
        if(fctx.currzone == fctx.bynHead.v6.ROM_nb_zone + fctx.bynHead.v6.FLA_nb_zone){
            fctx.stepA = FLASH_AUTOFLASH;
            return 0;
        }
        uGetFirmwareBynZone(fctx.zOfs, &fctx.bz);
        if(fctx.currzone < fctx.bynHead.v6.ROM_nb_zone) {
            fctx.bz.addr_page = (u32)firm_dev.first_code_page * firm_dev.ext_page_size + 3*fctx.bz.addr_page/2;
        } else {
            fctx.bz.addr_page = (u32)firm_dev.first_yfs3_page * firm_dev.ext_page_size + fctx.bz.addr_page;
        }
#ifdef DEBUG_FIRMWARE
        dbglog("Flash zone %d:%x : %x(%x)\n",fctx.currzone,fctx.zOfs,fctx.bz.addr_page,fctx.bz.len);
#endif
        YSPRINTF(msg, FLASH_ERRMSG_LEN, "Flash zone %d:%x : %x(%x)",fctx.currzone,fctx.zOfs,fctx.bz.addr_page,fctx.bz.len);
        uLogProgress(msg);

        if((fctx.bz.addr_page & 1) != 0 || (fctx.bz.len & 1) != 0) {
            dbglog("Prog block not on a word boundary (%d+%d)\n", fctx.bz.addr_page, fctx.bz.len);
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Prog block not on a word boundary");
            return -1;
        }
        fctx.zOfs += sizeof(byn_zone);
        fctx.stepB = 0;
        fctx.zst = FLASH_ZONE_PROG;
        //no break on purpose
    case FLASH_ZONE_PROG:
        if(fctx.bz.len > 0 && fctx.currzone < fctx.bynHead.v6.ROM_nb_zone &&
           fctx.bz.addr_page >= (u32)firm_dev.first_yfs3_page * firm_dev.ext_page_size) {
            // skip end of ROM image past reserved flash zone
#ifdef DEBUG_FIRMWARE
            dbglog("Drop ROM data past firmware boundary (zone %d at offset %x)\n", fctx.currzone, fctx.zOfs);
#endif
            fctx.zOfs += fctx.bz.len;
            fctx.bz.len = 0;
            fctx.zst = FLASH_ZONE_START;
            fctx.currzone++;
            return 0;
        }
        addr = fctx.bz.addr_page + fctx.stepB;
        memset(&firm_pkt,0,sizeof(USB_Packet));

        SET_PROG_POS_PAGENO(firm_pkt.prog.pkt_ext, addr / firm_dev.ext_page_size,  addr >> 2);
        datasize = firm_dev.ext_page_size - (addr & (firm_dev.ext_page_size-1));
        if(datasize > MAX_BYTE_IN_PACKET) {
            datasize = MAX_BYTE_IN_PACKET;
        }
        if(fctx.stepB + datasize > fctx.bz.len) {
            datasize = fctx.bz.len - fctx.stepB;
        }
        YASSERT((datasize & 1) == 0);
        firm_pkt.prog.pkt_ext.size = (u8)(datasize / 2);
        firm_pkt.prog.pkt_ext.type = PROG_PROG;
#ifdef DEBUG_FIRMWARE
        {
            u32 page, pos;
            GET_PROG_POS_PAGENO(firm_pkt.prog.pkt_ext, page,  pos);
            pos *=4;
            dbglog("Flash at 0x%x:0x%x (0x%x bytes) found at 0x%x (0x%x more in zone)\n",page, pos,
              2*firm_pkt.prog.pkt_ext.size, fctx.zOfs, fctx.bz.len);
         }
#endif
        uGetFirmware(fctx.zOfs, firm_pkt.prog.pkt_ext.opt.data, 2*firm_pkt.prog.pkt_ext.size);
        if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
            dbglog("Unable to send prog pkt\n");
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Unable to send prog pkt");
            return -1;
        }
        fctx.zOfs  += datasize;
        fctx.stepB += datasize;

        // verify each time we finish a page or a zone
        if ((u16)((addr & (firm_dev.ext_page_size-1)) + datasize) >= firm_dev.ext_page_size || fctx.stepB >= fctx.bz.len) {
            fctx.zOfs -= fctx.stepB; // rewind to check
            fctx.zst = FLASH_ZONE_READ;
        }
        break;
    case FLASH_ZONE_READ:
        // pageno is already set properly
        addr = fctx.bz.addr_page;
        SET_PROG_POS_PAGENO(firm_pkt.prog.pkt_ext, addr / firm_dev.ext_page_size,  addr >> 2);
        firm_pkt.prog.pkt.type = PROG_VERIF;
        if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
            dbglog("Unable to send verif pkt\n");
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Unable to send verif pkt");
            return -1;
        }
        fctx.zst =  FLASH_ZONE_RECV_OK;
        fctx.timeout =ytime()+ ZONE_VERIF_TIMEOUT;
        //no break on purpose
    case FLASH_ZONE_RECV_OK:
        if(ypGetBootloaderReply(&firm_dev, &firm_pkt,NULL)<0){
            if((s32)(fctx.timeout - ytime()) < 0) {
#ifdef DEBUG_FIRMWARE
                dbglog("Bootlaoder did not send confirmation for Zone %x Block %x\n",fctx.currzone,fctx.bz.addr_page);
#endif
                YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Device did not respond to verif pkt");
                return -1;
            }
            return 0;
        }
        if(firm_pkt.prog.pkt.type != PROG_VERIF) {
            dbglog("Invalid verif pkt\n");
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Invalid verif pkt");
            return -1;
        }
        GET_PROG_POS_PAGENO(firm_pkt.prog.pkt_ext, pageno, pos);
#ifdef DEBUG_FIRMWARE
            dbglog("Verif at 0x%x:0x%x (up to 0x%x bytes)\n",pageno,
                  pos <<2,
                  2*firm_pkt.prog.pkt_ext.size);
#endif
        addr = pageno * firm_dev.ext_page_size + (pos << 2) ;
        YASSERT(addr >= fctx.bz.addr_page);
        if(addr < fctx.bz.addr_page + fctx.stepB) {
            // packet is in verification range
            datasize = 2 * firm_pkt.prog.pkt_ext.size;
            if(addr + datasize >= fctx.bz.addr_page + fctx.stepB) {
                datasize = fctx.bz.addr_page + fctx.stepB - addr;
            }
            uGetFirmware(fctx.zOfs + (addr-fctx.bz.addr_page), buff, (u16)datasize);
            if(memcmp(buff, firm_pkt.prog.pkt_ext.opt.data, datasize) != 0) {
                dbglog("Flash verification failed at %x (%x:%x)\n", addr, pageno, addr);
                YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Flash verification failed");
                return -1;
            }
#ifdef DEBUG_FIRMWARE
        } else {
            dbglog("Skip verification for block at addr 0x%x (block ends at %x)\n", addr, fctx.bz.addr_page + fctx.stepB);
#endif
        }
        if((addr & (firm_dev.ext_page_size-1)) + 2 * (u32)firm_pkt.prog.pkt_ext.size < (u32)firm_dev.ext_page_size) {
            // more packets expected (device will dump a whole flash page)
            return 0;
        }
        fctx.zOfs += fctx.stepB;
        fctx.progress = (u16)(20 + 76*fctx.zOfs / (BYN_HEAD_SIZE_V6 + fctx.bynHead.v6.ROM_total_size + fctx.bynHead.v6.FLA_total_size));
        fctx.bz.addr_page += fctx.stepB;
        fctx.bz.len -= fctx.stepB;
        if(fctx.bz.len > 0 && fctx.currzone < fctx.bynHead.v6.ROM_nb_zone &&
           fctx.bz.addr_page >= (u32)firm_dev.first_yfs3_page * firm_dev.ext_page_size) {
            // skip end of ROM image past reserved flash zone
#ifdef DEBUG_FIRMWARE
            dbglog("Drop ROM data past firmware boundary (zone %d at offset %x)\n", fctx.currzone, fctx.zOfs);
#endif
            fctx.zOfs += fctx.bz.len;
            fctx.bz.len = 0;
        }
        if(fctx.bz.len == 0){
            fctx.zst = FLASH_ZONE_START;
            fctx.currzone++;
#ifdef DEBUG_FIRMWARE
            dbglog("Switch to next zone (zone %d at offset %x)\n", fctx.currzone, fctx.zOfs);
#endif
        } else {
            fctx.zst = FLASH_ZONE_PROG;
            fctx.stepB = 0;
#ifdef DEBUG_FIRMWARE
            dbglog("Continue zone %d at offset %x for %x more bytes\n", fctx.currzone, fctx.zOfs, fctx.bz.len);
#endif
        }
    }

    return 0;
}
#endif


YPROG_RESULT uFlashDevice(void)
{
    byn_head_multi  head;
    int             res;

    if(fctx.stepA != FLASH_FIND_DEV && fctx.stepA != FLASH_DONE) {
        if(ypIsSendBootloaderBusy(&firm_dev)) {
            return YPROG_WAITING;
        }
        // ReSharper disable once CppUnreachableCode
        if(!Flash_ready()) {
            return YPROG_WAITING;
        }
    }

    switch(fctx.stepA){
    case FLASH_FIND_DEV:
        uLogProgress("Wait for device");
        if (uGetBootloader(fctx.bynHead.h.serial, &firm_dev.iface)<0){
#ifndef MICROCHIP_API
            if((s32)(fctx.timeout - ytime()) >= 0) {
                return YPROG_WAITING;
            }
 #endif
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"device not present");
#ifdef DEBUG_FIRMWARE
            ulog("device not present\n");
#endif
            return YPROG_DONE;
        }
        fctx.progress = 2;
        uLogProgress("Device detected");

#if defined(DEBUG_FIRMWARE) && defined(MICROCHIP_API)
        ulog("Bootloader ");
        ulog(fctx.bynHead.h.serial);
        ulog(" on port ");
#ifdef MICROCHIP_API
        ulogU16(firm_dev.iface);
#else
        ulogU16(firm_dev.iface.deviceid);
#endif
        ulog("\n");
#endif
#ifndef MICROCHIP_API
        fctx.stepA = FLASH_CONNECT;
        // no break on purpose
    case FLASH_CONNECT:
        if(YISERR(yyySetup(&firm_dev.iface,NULL))){
            YSTRCPY(fctx.errmsg,FLASH_ERRMSG_LEN,"Unable to open connection to the device");
            return YPROG_DONE;
        }
        uLogProgress("Device connected");
#endif
        fctx.stepA = FLASH_GET_INFO;
        fctx.stepB = 0;
        break;
    case FLASH_GET_INFO:
        if(uGetDeviceInfo()<0){
#ifdef DEBUG_FIRMWARE
            ulog("uGetDeviceInfo failed\n");
#endif
            fctx.stepA = FLASH_DISCONNECT;
        }
        fctx.progress = 2;
        break;
    case FLASH_VALIDATE_BYN:
#ifdef DEBUG_FIRMWARE
        ulog("PICDev ");
        ulogU16(firm_dev.devid_model);
        ulog(" detected\n");
#endif
        uGetFirmwareBynHead(&head);
        if (ValidateBynCompat(&head, fctx.len, fctx.bynHead.h.serial, fctx.flags, &firm_dev, fctx.errmsg) < 0) {
#ifdef DEBUG_FIRMWARE
            ulog("ValidateBynCompat failed:");
            ulog(fctx.errmsg);
            ulog("\n");
#endif
            fctx.stepA = FLASH_DISCONNECT;
            break;
        }

        switch(head.h.rev) {
            case BYN_REV_V4:
                fctx.bynHead.v6.ROM_nb_zone = (u8)head.v4.nbzones;
                fctx.bynHead.v6.FLA_nb_zone = 0;
                fctx.currzone = 0;
                fctx.zOfs = BYN_HEAD_SIZE_V4;
                break;
            case BYN_REV_V5:
                fctx.bynHead.v6.ROM_nb_zone = (u8)head.v5.nbzones;
                fctx.bynHead.v6.FLA_nb_zone = 0;
                fctx.currzone = 0;
                fctx.zOfs = BYN_HEAD_SIZE_V5;
                break;
            case BYN_REV_V6:
                fctx.bynHead.v6.ROM_nb_zone = (u8)head.v6.ROM_nb_zone;
                fctx.bynHead.v6.FLA_nb_zone = (u8)head.v6.FLA_nb_zone;
                fctx.currzone = 0;
                fctx.zOfs = BYN_HEAD_SIZE_V6;
                break;
            default:
#ifdef DEBUG_FIRMWARE
                ulog("Unsupported file format (upgrade our VirtualHub)\n");
#endif
                fctx.stepA  = FLASH_DISCONNECT;
                break;
            }
        fctx.progress = 3;
        fctx.stepA = FLASH_ERASE;
#ifndef MICROCHIP_API
        if(firm_dev.ext_total_pages) {
            fctx.flashPage = firm_dev.first_code_page;
        }
#endif
#ifdef DEBUG_FIRMWARE
        ulogU16(fctx.bynHead.v6.ROM_nb_zone);
        ulog(" ROM zones to flash\n");
#endif
        break;
    case FLASH_ERASE:
        fctx.zst = FLASH_ZONE_START;
        fctx.stepB = 0;
#ifdef MICROCHIP_API
        res = uSendCmd(PROG_ERASE,FLASH_WAIT_ERASE);
#else
        if(firm_dev.ext_total_pages) {
            int npages = firm_dev.ext_total_pages - fctx.flashPage;
            int maxpages = (firm_dev.ext_jedec_id == JEDEC_SPANSION_4MB || firm_dev.ext_jedec_id == JEDEC_SPANSION_8MB ? 16 : 128);
#ifdef DEBUG_FIRMWARE
            ulogU16(npages);
            ulog(" pages still to flash\n");
#endif
            if(npages > maxpages) npages = maxpages;
            res = uSendErase(fctx.flashPage, npages, FLASH_WAIT_ERASE);
            fctx.flashPage += npages;
        } else {
            res = uSendCmd(PROG_ERASE,FLASH_WAIT_ERASE);
        }
#endif
        if(res<0){
#ifdef DEBUG_FIRMWARE
            ulog("FlashErase failed\n");
#endif
            YSTRCPY(fctx.errmsg,sizeof(fctx.errmsg),"Unable to blank flash");
            fctx.stepA = FLASH_DISCONNECT;
        }
        break;
    case FLASH_WAIT_ERASE:
        if(fctx.stepB == 0) {
#ifndef MICROCHIP_API
            if(firm_dev.ext_total_pages) {
                if(ypIsSendBootloaderBusy(&firm_dev)) {
                    return YPROG_WAITING;
                }
                memset(&firm_pkt,0,sizeof(USB_Prog_Packet));
                firm_pkt.prog.pkt.type = PROG_INFO;
                if(ypSendBootloaderCmd(&firm_dev,&firm_pkt,NULL)<0){
                    return YPROG_WAITING;
                }
            }
#endif
            fctx.stepB = ytime();
        } else {
#ifndef MICROCHIP_API
            if(firm_dev.ext_total_pages) {
                if(ypGetBootloaderReply(&firm_dev, &firm_pkt,NULL)<0) {
                    if((u32)(ytime() - fctx.stepB) < 2000u) {
                        return YPROG_WAITING;
                    }
#ifdef DEBUG_FIRMWARE
                    ulog("FlashErase failed\n");
#endif
                    YSTRCPY(fctx.errmsg,sizeof(fctx.errmsg),"Timeout blanking flash");
                    fctx.stepA = FLASH_DISCONNECT;
                } else {
#ifdef DEBUG_FIRMWARE
                    ulog("clear time: ");
                    ulogU16((u16)(ytime() - fctx.stepB));
                    ulog("\n");
#endif
                    fctx.progress = 3+(18*fctx.flashPage/firm_dev.ext_total_pages);
                    uLogProgress("Erasing flash");
                    if(fctx.flashPage < firm_dev.ext_total_pages) {
                        fctx.stepA = FLASH_ERASE;
                        break;
                    }
                }
            } else
#endif
            {
                u32 delay = 1000 + (firm_dev.last_addr>>5);
                if((u32)(ytime() - fctx.stepB) < delay) {
                    return YPROG_WAITING;
                }
            }
            fctx.stepA = FLASH_DOFLASH;
            fctx.stepB = 0;
        }
        break;
    case FLASH_DOFLASH:
#ifdef MICROCHIP_API
        res = uFlashZone();
#else
        if(firm_dev.ext_total_pages) {
            res = uFlashFlash();
        } else {
            res = uFlashZone();
        }
#endif
        if(res<0){
#ifdef DEBUG_FIRMWARE
            ulog("Flash failed\n");
            ulog("errmsg=");
            ulog(fctx.errmsg);
            ulogChar('\n');
#endif
            fctx.stepA = FLASH_DISCONNECT;
        }
        break;

    case FLASH_GET_INFO_BFOR_REBOOT:
        res =uGetDeviceInfo();
        if(res <0){
#ifdef DEBUG_FIRMWARE
            ulog("uGetDeviceInfo failed\n");
#endif
            YSTRCPY(fctx.errmsg, FLASH_ERRMSG_LEN, "Last communication before reboot failed");
            fctx.stepA = FLASH_DISCONNECT;
        } else if(res == 1) {
            fctx.stepA = FLASH_REBOOT;
        }
        break;

    case FLASH_REBOOT:
        fctx.progress = 95;
#ifdef DEBUG_FIRMWARE
        ulog("Send reboot\n");
#endif

#ifdef MICROCHIP_API
        res = ypBootloaderShutdown(&firm_dev);
        if (res < 0) {
#ifdef DEBUG_FIRMWARE
            ulog("reboot failed\n");
#endif
            YSTRCPY(fctx.errmsg,sizeof(fctx.errmsg),"Unable to reboot bootloader");
            fctx.stepA = FLASH_DISCONNECT;
        }
#else
        uSendCmd(PROG_REBOOT, FLASH_REBOOT_VALIDATE);
        // do not check reboot packet on purpose (most of the time
        // the os generate an error because the device rebooted too quickly)
#endif
        fctx.stepA  = FLASH_REBOOT_VALIDATE;
        fctx.timeout = ytime() + YPROG_BOOTLOADER_TIMEOUT;
        break;
    case FLASH_REBOOT_VALIDATE:
        if(uGetBootloader(fctx.bynHead.h.serial,NULL)<0){
            fctx.progress = 98;
#ifdef DEBUG_FIRMWARE
            ulog("device not present\n");
#endif
            fctx.stepA  = FLASH_SUCCEEDED;
            break;
        } else {
            if((s32)(fctx.timeout - ytime()) >= 0) {
                return YPROG_WAITING;
            }
#if defined(DEBUG_FIRMWARE) && defined(MICROCHIP_API)
            ulog("Bootloader ");
            ulog(fctx.bynHead.h.serial);
            ulog(" on port ");
#ifdef MICROCHIP_API
            ulogU16(firm_dev.iface);
#else
            ulogU16(firm_dev.iface.deviceid);
#endif
            ulog("\n");
#endif
            if (fctx.zOfs == 0){
                uLogProgress("reboot failed try again...");
                fctx.stepA = FLASH_GET_INFO_BFOR_REBOOT;
                break;
            }
            uLogProgress("Device still in bootloader");
            fctx.zOfs--;
            uLogProgress("Device still in bootloader");
            // FIXME: could try to add a retry
            fctx.stepA = FLASH_DISCONNECT;
        }
        break;
#ifndef MICROCHIP_API
    case FLASH_AUTOFLASH:
        fctx.progress = 98;
        uSendReboot(START_AUTOFLASHER_SIGN, FLASH_SUCCEEDED);
        fctx.stepA  = FLASH_SUCCEEDED;
        break;
#endif
    case FLASH_SUCCEEDED:
#ifdef DEBUG_FIRMWARE
        ulog("Flash succeeded\n");
#endif
        YSTRCPY(fctx.errmsg,sizeof(fctx.errmsg),"Flash succeeded");
        fctx.progress = 100;
        fctx.stepA   = FLASH_DISCONNECT;
        // intentionally no break
    case FLASH_DISCONNECT:
#ifdef DEBUG_FIRMWARE
        ulog("Flash disconnect\n");
#endif
#ifndef MICROCHIP_API
        yyyPacketShutdown(&firm_dev.iface);
#endif
        fctx.stepA   = FLASH_DONE;
        // intentionally no break
    case FLASH_DONE:
        return YPROG_DONE;
    }
    return YPROG_WAITING;
}


#ifndef MICROCHIP_API

typedef int(*yprogTcpReqCb)(void *ctx, const char* buffer, u32 len, char *errmsg);

static int getTCPBootloaders(void *ctx, const char* buffer, u32 len, char *errmsg)
{
    int res = 0;
    yJsonStateMachine j;
    char *p = ctx;
    memset(p, 0, YOCTO_SERIAL_LEN * 4);

    // Parse HTTP header
    j.src = buffer;
    j.end = j.src + len;
    j.st = YJSON_HTTP_START;
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_CODE) {
        return YERRMSG(YAPI_IO_ERROR,"Failed to parse HTTP header");
    }
    if (YSTRCMP(j.token, "200")) {
        return YERRMSG(YAPI_IO_ERROR,"Unexpected HTTP return code");
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_MSG) {
        return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
    }
    while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
        if (!strcmp(j.token, "list")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_ARRAY) {
                return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
            }

            while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st != YJSON_PARSE_ARRAY) {
                if (res < 4) {
                    YSTRCPY(p + res * YOCTO_SERIAL_LEN, YOCTO_SERIAL_LEN, j.token);
                }
                res++;
            }
        }
        yJsonSkip(&j, 1);
    }
    return res;
}


// return the list of bootladers in a specific hub
// buffer must be an pointer to a buffer of min 4 * YOCTO_SERIAL_LEN
// return the number of booloader copyed to buffer
int yNetHubGetBootloaders(const char *hubserial, char *buffer, char *errmsg)
{
    const char * req = "GET /flash.json?a=list \r\n\r\n";
    YIOHDL  iohdl;
    YRETCODE res, subres;
    int replysize;
    char *reply;

    res = yapiHTTPRequestSyncStartEx_internal(&iohdl, 0, hubserial, req, YSTRLEN(req), &reply, &replysize, NULL, NULL, errmsg);
    if (YISERR(res)) {
        return res;
    }
    res = getTCPBootloaders(buffer, reply, replysize, errmsg);
    subres = yapiHTTPRequestSyncDone_internal(&iohdl, NULL);
    YASSERT(!YISERR(subres));
    return res;
}


#endif


#ifndef YAPI_IN_YDEVICE

static int  getBootloaderInfos(const char *devserial, char *out_hubserial, char *errmsg)
{
    int             i, res;


    if (yContext->detecttype & Y_DETECT_USB) {
        int             nbifaces = 0;
        yInterfaceSt    *iface;
        yInterfaceSt    *runifaces = NULL;

        if (YISERR(res = (YRETCODE)yyyUSBGetInterfaces(&runifaces, &nbifaces, errmsg))){
            return res;
        }

        for (i = 0, iface = runifaces; i < nbifaces; i++, iface++){
            if (iface->deviceid == YOCTO_DEVID_BOOTLOADER && YSTRCMP(devserial, iface->serial) == 0) {
                YSTRCPY(out_hubserial, YOCTO_SERIAL_LEN, "usb");
                return 1;
            }
        }
    }


    for (i = 0; i < NBMAX_NET_HUB; i++){
        if (yContext->nethub[i]){
            char bootloaders[4 * YOCTO_SERIAL_LEN];
            char hubserial[YOCTO_SERIAL_LEN];
            int j;
            char *serial;
            yHashGetStr(yContext->nethub[i]->serial, hubserial, YOCTO_SERIAL_LEN);
            res = yNetHubGetBootloaders(hubserial, bootloaders, errmsg);
            if (YISERR(res)) {
                return res;
            }
            for (j = 0, serial = bootloaders; j < res; j++, serial += YOCTO_SERIAL_LEN){
                if (YSTRCMP(devserial,serial) == 0) {
                    YSTRCPY(out_hubserial, YOCTO_SERIAL_LEN, hubserial);
                    return 1;
                }
            }
        }
    }
    return 0;
}

typedef enum
{
    FLASH_HUB_AVAIL = 0u,
    FLASH_HUB_STATE,
    FLASH_HUB_FLASH,
    FLASH_HUB_NOT_BUSY, // CHECK there is on pending fwupdate (cmd=state-> !uploading && !flashing)
    FLASH_HUB_NONE
} FLASH_HUB_CMD;


typedef struct {
    FLASH_HUB_CMD cmd;
    const char *devserial;
}ckReqHeadCtx;

static int checkRequestHeader(void *ctx_ptr, const char* buffer, u32 len, char *errmsg) {
    ckReqHeadCtx *ctx = ctx_ptr;
    yJsonStateMachine j;
    char lastmsg[YOCTO_ERRMSG_LEN] = "invalid";
    int count = 0, return_code = 0;;

    // Parse HTTP header
    j.src = buffer;
    j.end = j.src + len;
    j.st = YJSON_HTTP_START;
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_CODE) {
        return YERRMSG(YAPI_IO_ERROR,"Failed to parse HTTP header");
    }
    if (YSTRCMP(j.token, "200")) {
        return YERRMSG(YAPI_IO_ERROR,"Unexpected HTTP return code");
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_MSG) {
        return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
    }
    while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
        switch (ctx->cmd){
        case FLASH_HUB_STATE:
            if (!strcmp(j.token, "state")) {
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                    return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                }
                if (YSTRCMP(j.token, "valid")) {
                    YSTRCPY(lastmsg, YOCTO_ERRMSG_LEN, "Invalid firmware");
                    return_code = YAPI_IO_ERROR;
                } else {
                    count++;
                }
            } else  if (!strcmp(j.token, "firmware")) {
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                    return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                }
                if (YSTRNCMP(j.token, ctx->devserial,YOCTO_BASE_SERIAL_LEN)) {
                    YSTRCPY(lastmsg, YOCTO_ERRMSG_LEN, "Firmware not designed for this module");
                    return_code = YAPI_IO_ERROR;
                } else {
                    count++;
                }
            } else  if (!strcmp(j.token, "message")) {
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                    return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                }
                YSTRCPY(lastmsg, YOCTO_ERRMSG_LEN, j.token);
            } else {
                yJsonSkip(&j, 1);
            }
            break;
        case FLASH_HUB_NOT_BUSY:
            if (!strcmp(j.token, "state")) {
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                    return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                }
                if (YSTRCMP(j.token, "uploading") == 0 || YSTRCMP(j.token, "flashing")==0) {
                    YSTRCPY(lastmsg, YOCTO_ERRMSG_LEN, "Cannot start firmware update: busy (");
                    YSTRCAT(lastmsg, YOCTO_ERRMSG_LEN, j.token);
                    YSTRCAT(lastmsg, YOCTO_ERRMSG_LEN, ")");
                    return_code = YAPI_IO_ERROR;
                } else {
                    count++;
                }
            } else {
                yJsonSkip(&j, 1);
            }
            break;
        case FLASH_HUB_AVAIL:
            yJsonSkip(&j, 1);
            break;
        case FLASH_HUB_FLASH:
            if (!strcmp(j.token, "logs")) {
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_ARRAY) {
                    return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                }
                while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st != YJSON_PARSE_ARRAY) {
                    setOsGlobalProgress(0, j.token);
                    YSTRCPY(lastmsg, YOCTO_ERRMSG_LEN, j.token);
                }
            } else  if (!strcmp(j.token, "progress")) {
                int progress;
                if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                    return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                }
                progress = atoi(j.token);
                if (progress < 100) {
                    return_code = YAPI_IO_ERROR;
                }
            } else {
                yJsonSkip(&j, 1);
            }
            break;
        default:
            yJsonSkip(&j, 1);
            break;
        }
    }

    if (return_code < 0) {
        YSTRCPY(errmsg,YOCTO_ERRMSG_LEN, lastmsg);
        return return_code;
    }
    return count;
}

static int checkHTTPHeader(void *ctx, const char* buffer, u32 len, char *errmsg) {
    yJsonStateMachine j;
    // Parse HTTP header
    j.src = buffer;
    j.end = j.src + len;
    j.st = YJSON_HTTP_START;
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_CODE) {
        return YERRMSG(YAPI_IO_ERROR, "Failed to parse HTTP header");
    }
    if (YSTRCMP(j.token, "200")) {
        return YERRMSG(YAPI_IO_ERROR, "Unexpected HTTP return code");
    }

    return 0;
}


// Method used to upload a file to the device
static int upload(const char *hubserial, const char *subpath, const char *filename, u8 *data, u32 data_len, char *errmsg)
{

    char       *p;
    int         buffer_size = 1024 + data_len;
    char        *buffer = yMalloc(buffer_size);
    char        boundary[32];
    int         res;
    YIOHDL      iohdl;
    char    *reply = NULL;
    int     replysize = 0;

    do {
        YSPRINTF(boundary, 32, "Zz%06xzZ", rand() & 0xffffff);
    } while (ymemfind(data, data_len, (u8*)boundary, YSTRLEN(boundary)) >= 0);

    YSTRCPY(buffer, buffer_size, "POST ");
    YSTRCAT(buffer, buffer_size, subpath);
    YSTRCAT(buffer, buffer_size, "upload.html HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=");
    YSTRCAT(buffer, buffer_size, boundary);
    YSTRCAT(buffer, buffer_size,
        "\r\n\r\n"
        "--");
    YSTRCAT(buffer, buffer_size, boundary);
    YSTRCAT(buffer, buffer_size, "\r\nContent-Disposition: form-data; name=\"");
    YSTRCAT(buffer, buffer_size, filename);
    YSTRCAT(buffer, buffer_size, "\"; filename=\"api\"\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Transfer-Encoding: binary\r\n\r\n");
    p = buffer + YSTRLEN(buffer);
    memcpy(p, data, data_len);
    p += data_len;
    YASSERT(p - buffer < buffer_size);
    buffer_size -= (int)(p - buffer);
    YSTRCPY(p, buffer_size, "\r\n--");
    YSTRCAT(p, buffer_size, boundary);
    YSTRCAT(p, buffer_size, "--\r\n");
    buffer_size = (int)(p - buffer) + YSTRLEN(p);
    //todo: chose wisely callback and tpchan
    res = yapiHTTPRequestSyncStartEx_internal(&iohdl, 0, hubserial, buffer, buffer_size, &reply, &replysize, NULL, NULL,errmsg);
    if (res >= 0) {
        res = checkHTTPHeader(NULL, reply, replysize, errmsg);
        yapiHTTPRequestSyncDone_internal(&iohdl, errmsg);
    }
    yFree(buffer);
    return res;
}


typedef enum
{
    FLASH_USB = 0u,
    FLASH_NET_SELF,
    FLASH_NET_SUBDEV,
} FLASH_TYPE;


static int sendHubFlashCmd(const char *hubserial, const char *subpath, const char *devserial, FLASH_HUB_CMD cmd, const char *args, char *errmsg)
{
    char buffer[512];
    const char *cmd_str;
    ckReqHeadCtx ctx;
    int res;
    YIOHDL  iohdl;
    YRETCODE subres;
    int replysize;
    char *reply;

    switch (cmd){
    case FLASH_HUB_AVAIL:
    case FLASH_HUB_STATE:
    case FLASH_HUB_NOT_BUSY:
        cmd_str = "state";
        break;
    case FLASH_HUB_FLASH:
        cmd_str = "flash";
        break;
    default:
        return YERR(YAPI_INVALID_ARGUMENT);
    }
    YSPRINTF(buffer, 512, "GET %sflash.json?a=%s%s \r\n\r\n", subpath, cmd_str, args);
    ctx.cmd = cmd;
    ctx.devserial = devserial;
    res = yapiHTTPRequestSyncStartEx_internal(&iohdl, 0, hubserial, buffer, YSTRLEN(buffer), &reply, &replysize, NULL, NULL, errmsg);
    if (YISERR(res)) {
        return res;
    }
    res = checkRequestHeader(&ctx, reply, replysize, errmsg);
    subres = yapiHTTPRequestSyncDone_internal(&iohdl, NULL);
    YASSERT(!YISERR(subres));
    return res;
}

static int isWebPath(const char *path)
{
    if (YSTRNCMP(path, "http://", 7) == 0){
        return 7;
    } else if (YSTRNCMP(path, "www.yoctopuce.com",17) == 0){
        return 0;
    }
    return -1;
}

static int yDownloadFirmware(const char * url, u8 **out_buffer, char *errmsg)
{
    char host[256];
    u8 *buffer;
    int res, len, ofs, i;
    const char * http_ok = "HTTP/1.1 200 OK";


    for (i = 0; i < 255 && i < YSTRLEN(url) && url[i] != '/'; i++){
        host[i] = url[i];
    }

    if (url[i] != '/'){
        return YERRMSG(YAPI_INVALID_ARGUMENT, "invalid url");
    }
    host[i] = 0;

    //yFifoInit(&(hub->fifo), hub->buffer,sizeof(hub->buffer));
    res = yTcpDownload(host, url+i, &buffer, 10000, errmsg);
    if (res < 0){
        return res;
    }
    if (YSTRNCMP((char*)buffer, http_ok, YSTRLEN(http_ok))) {
        yFree(buffer);
        return YERRMSG(YAPI_IO_ERROR, "Unexpected HTTP return code");
    }

    ofs = ymemfind(buffer, res, (u8*)"\r\n\r\n", 4);
    if (ofs <0) {
        yFree(buffer);
        return YERRMSG(YAPI_IO_ERROR, "Invalid HTTP header");

    }
    ofs += 4;
    len = res - ofs;
    *out_buffer = yMalloc(len);
    memcpy(*out_buffer, buffer + ofs, len);
    yFree(buffer);
    return len;

}


static void* yFirmwareUpdate_thread(void* ctx)
{
    yThread     *thread = (yThread*)ctx;
    YAPI_DEVICE dev;
    int         res;
    char        errmsg[YOCTO_ERRMSG_LEN];
    char        buffer[256];
    char        subpath[256];
    char        bootloaders[YOCTO_SERIAL_LEN * 4];
    char        *p;
    char        replybuf[512];
    const char* reboot_req = "GET %sapi/module/rebootCountdown?rebootCountdown=-3 \r\n\r\n";
    const char* reboot_hub = "GET %sapi/module/rebootCountdown?rebootCountdown=-1003 \r\n\r\n";
    const char* get_api_fmt = "GET %sapi.json \r\n\r\n";
    char        hubserial[YOCTO_SERIAL_LEN];
    char        *reply = NULL;
    int         replysize = 0;
    int         ofs,i;
    u64         timeout;
    FLASH_TYPE  type = FLASH_USB;
    int         online, found;
    YPROG_RESULT u_flash_res;


    yThreadSignalStart(thread);

    //1% -> 5%
    setOsGlobalProgress(1, "Loading firmware");
    ofs = isWebPath(yContext->fuCtx.firmwarePath);
    if (ofs < 0){
        res = yLoadFirmwareFile(yContext->fuCtx.firmwarePath, &fctx.firmware, errmsg);
    } else {
        res = yDownloadFirmware(yContext->fuCtx.firmwarePath + ofs, &fctx.firmware, errmsg);
    }
    if (YISERR(res)) {
        setOsGlobalProgress(res, errmsg);
        goto exitthread;
    }
    fctx.len = res;
    //copy firmware header into context variable (to have same behaviour as a device)
    memcpy(&fctx.bynHead, fctx.firmware, sizeof(fctx.bynHead));
    YSTRCPY(fctx.bynHead.h.serial, YOCTO_SERIAL_LEN, yContext->fuCtx.serial);


    res = IsValidBynFile((const byn_head_multi *)fctx.firmware, fctx.len, yContext->fuCtx.serial, fctx.flags, errmsg);
    if (YISERR(res)) {
        setOsGlobalProgress(res, errmsg);
        goto exit_and_free;
    }

    //5% -> 10%
    setOsGlobalProgress(5, "Enter firmware update mode");
    dev = wpSearch(yContext->fuCtx.serial);
    if (dev != -1) {
        yUrlRef url;
        int urlres = wpGetDeviceUrl(dev, hubserial, subpath, 256, NULL);
        if (urlres < 0) {
            setOsGlobalProgress(YAPI_IO_ERROR, NULL);
            goto exit_and_free;
        }
        url = wpGetDeviceUrlRef(dev);
        if (yHashGetUrlPort(url, NULL, NULL, NULL, NULL, NULL, NULL) == USB_URL) {
            // USB connected device -> reboot it in bootloader
            type = FLASH_USB;
            YSPRINTF(buffer, sizeof(buffer), reboot_req, subpath);
            res = yapiHTTPRequest(hubserial, buffer, replybuf, sizeof(replybuf), NULL, errmsg);
            if (res < 0) {
                setOsGlobalProgress(res, errmsg);
                goto exit_and_free;
            }
        } else {
            res = sendHubFlashCmd(hubserial, subpath, yContext->fuCtx.serial, FLASH_HUB_AVAIL, "", NULL);
            if (res < 0 || YSTRNCMP(hubserial, "VIRTHUB", 7) == 0) {
                int is_shield = YSTRNCMP(yContext->fuCtx.serial, "YHUBSHL1", YOCTO_BASE_SERIAL_LEN)==0;
                res = yNetHubGetBootloaders(hubserial, bootloaders, errmsg);
                if (res < 0) {
                    setOsGlobalProgress(res, errmsg);
                    goto exit_and_free;
                }
                for (i = 0; i < res; i++) {
                    p = bootloaders + YOCTO_SERIAL_LEN * i;
                    if (YSTRCMP(yContext->fuCtx.serial, p) == 0) {
                        break;
                    }
                }
                if (i == res) {
                    // not in bootloader list...
                    //...check if list is allready full..
                    if (res == 4) {
                        setOsGlobalProgress(YAPI_IO_ERROR, "Too many devices in update mode");
                        goto exit_and_free;
                    }
                    if (is_shield) {
                        //...and that we do not already have a shield in bootlader..
                        for (i = 0; i < res; i++) {
                            p = bootloaders + YOCTO_SERIAL_LEN * i;
                            if (YSTRNCMP(p, "YHUBSHL1", YOCTO_BASE_SERIAL_LEN)==0) {
                                setOsGlobalProgress(YAPI_IO_ERROR, "Only one YoctoHub-Shield is allowed in update mode");
                                goto exit_and_free;
                            }
                        }
                    }

                    // ...must reboot in programtion
                    setOsGlobalProgress(8, "Reboot to firmware update mode");
                    YSPRINTF(buffer, sizeof(buffer), reboot_req, subpath);
                    res = yapiHTTPRequest(hubserial, buffer, replybuf, sizeof(replybuf), NULL, errmsg);
                    if (res < 0) {
                        setOsGlobalProgress(res, errmsg);
                        goto exit_and_free;
                    }
                    if (replybuf[0] != 'O' || replybuf[1] != 'K') {
                        dbglog("Reboot to firmware update mode:\n%s\n", replybuf);
                    }
                }
                type = FLASH_NET_SUBDEV;
            } else  {
                type = FLASH_NET_SELF;
            }
        }
    } else {
        //no known device -> check if device is in bootloader
        res = getBootloaderInfos(yContext->fuCtx.serial, hubserial, errmsg);
        if (res < 0) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }
        if (res == 0) {
            setOsGlobalProgress(YAPI_DEVICE_NOT_FOUND, "Bootloader not found");
            goto exit_and_free;
        }
        if (YSTRCMP(hubserial, "usb") == 0) {
            type = FLASH_USB;
        } else {
            type = FLASH_NET_SUBDEV;
        }
    }

    //10% -> 40%
    setOsGlobalProgress(10, "Send new firmware");
    if (type != FLASH_USB){
        // ensure flash engine is not busy
        res = sendHubFlashCmd(hubserial, type == FLASH_NET_SELF ? subpath : "/", yContext->fuCtx.serial, FLASH_HUB_NOT_BUSY, "", errmsg);
        if (res < 1) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }
        // start firmware upload
        // IP connected device -> upload the firmware to the Hub
        res = upload(hubserial, type == FLASH_NET_SELF ? subpath : "/", "firmware", fctx.firmware, fctx.len, errmsg);
        if (res < 0) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }
        // verify that firmware is correctly uploaded
        res = sendHubFlashCmd(hubserial, type == FLASH_NET_SELF ? subpath : "/", yContext->fuCtx.serial, FLASH_HUB_STATE, "", errmsg);
        if (res < 2) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }

        if (type == FLASH_NET_SELF) {
            const char *settingsOnly, *services;
            u8 *startupconf_data;
            int settings_len = yapiJsonGetPath_internal("api", (char*)yContext->fuCtx.settings, yContext->fuCtx.settings_len, 0, &settingsOnly, errmsg);
            int service_len = yapiJsonGetPath_internal("services", settingsOnly, settings_len, 0, &services, errmsg);
            int startupconf_data_len;
            if (service_len > 0) {
                int first_len = (services - settingsOnly) & 0xffffffff;
                int sec_len = ((settingsOnly + settings_len) - (services + service_len)) & 0xffffffff;
                startupconf_data = yMalloc(settings_len - service_len + 2);
                memcpy(startupconf_data, settingsOnly, first_len);
                startupconf_data[first_len] = '{';
                startupconf_data[first_len + 1] = '}';
                memcpy(startupconf_data + first_len + 2, services + service_len, sec_len);
                startupconf_data_len = first_len + sec_len;
            } else {
                startupconf_data_len = settings_len;
                startupconf_data = yMalloc(settings_len);
                memcpy(startupconf_data, settingsOnly, settings_len);
            }
            setOsGlobalProgress(20,"Save startupConf.json");
            // save settings
            res = upload(hubserial, subpath, "startupConf.json", startupconf_data, startupconf_data_len, errmsg);
            if (res < 0) {
                yFree(startupconf_data);
                setOsGlobalProgress(res, errmsg);
                goto exit_and_free;
            }
            setOsGlobalProgress(30,"Save firmwareConf");
            res = upload(hubserial, subpath, "firmwareConf", startupconf_data, startupconf_data_len, errmsg);
            yFree(startupconf_data);
            if (res < 0) {
                setOsGlobalProgress(res, errmsg);
                goto exit_and_free;
            }
        }
    }

    //40%-> 80%
    fctx.progress = 0 ;
    switch (type){
    case FLASH_USB:
        setOsGlobalProgress(40, "Flash firmware");
        fctx.timeout = ytime() + YPROG_BOOTLOADER_TIMEOUT;
        do {
            u_flash_res = uFlashDevice();
            if (u_flash_res != YPROG_DONE){
                setOsGlobalProgress(40 + fctx.progress/2, fctx.errmsg);
                yApproximateSleep(1);
            }
        } while (u_flash_res != YPROG_DONE);
        if (fctx.progress < 100) {
            setOsGlobalProgress(YAPI_IO_ERROR, fctx.errmsg);
            goto exit_and_free;
        }
        break;
    case FLASH_NET_SELF:
        setOsGlobalProgress(40, "Flash firmware");
        // the hub itself -> reboot in autoflash mode
        YSPRINTF(buffer, sizeof(buffer), reboot_hub, subpath);
        res = yapiHTTPRequest(hubserial, buffer, replybuf, sizeof(replybuf), NULL, errmsg);
        if (res < 0) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }
        for (i = 0; i < 8; i++){
            setOsGlobalProgress(50 + i*5, "Flash firmware");
            yApproximateSleep(1000);
        }
        break;
    case FLASH_NET_SUBDEV:
        // verify that the device is in bootloader
        setOsGlobalProgress(40, "Verify that the device is in update mode");
        timeout = yapiGetTickCount() + YPROG_BOOTLOADER_TIMEOUT;
        found = 0;
        while (!found && yapiGetTickCount()< timeout) {
            res = yNetHubGetBootloaders(hubserial, bootloaders, errmsg);
            if (res < 0) {
                setOsGlobalProgress(res, errmsg);
                goto exit_and_free;
            } else if (res > 0) {
                for (i = 0; i < res; i++) {
                    p = bootloaders + YOCTO_SERIAL_LEN * i;
                    if (YSTRCMP(yContext->fuCtx.serial, p) == 0) {
                        found = 1;
                        break;
                    }
                }
            }
            // device still rebooting
            yApproximateSleep(100);
        }
        if (!found) {
            setOsGlobalProgress(YAPI_IO_ERROR, "Hub did not detect bootloader");
            goto exit_and_free;
        }
        //start flash
        setOsGlobalProgress(50, "Flash firmware");
        YSPRINTF(buffer, sizeof(buffer), "&s=%s", yContext->fuCtx.serial);
        res = sendHubFlashCmd(hubserial, "/", yContext->fuCtx.serial,  FLASH_HUB_FLASH, buffer, errmsg);
        if (res < 0) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }
        break;
    }

    //90%-> 98%
    setOsGlobalProgress(90, "Wait for the device to restart");
    online = 0;
    timeout = yapiGetTickCount() + 60000;
    do {
        YIOHDL  iohdl;
        char tmp_errmsg[YOCTO_ERRMSG_LEN];
        res = yapiUpdateDeviceList(1, errmsg);
        if (res < 0 && type != FLASH_NET_SELF) {
            setOsGlobalProgress(res, errmsg);
            goto exit_and_free;
        }
        dev = wpSearch(yContext->fuCtx.serial);
        if (dev != -1) {
            wpGetDeviceUrl(dev, hubserial, subpath, 256, NULL);
            YSPRINTF(buffer, sizeof(buffer), get_api_fmt, subpath);
            res = yapiHTTPRequestSyncStartEx_internal(&iohdl, 0, hubserial, buffer, YSTRLEN(buffer), &reply, &replysize, NULL, NULL, tmp_errmsg);
            if (res >= 0) {
                if (checkHTTPHeader(NULL, reply, replysize, tmp_errmsg) >= 0) {
                    const char * real_fw;
                    int fw_len;
                    fw_len = yapiJsonGetPath_internal("module|firmwareRelease", (char*)reply, replysize, 1, &real_fw, errmsg);
                    online = 1;
                    if (fw_len > 2) {
                        const char *p = ((const byn_head_multi *)fctx.firmware)->h.firmware;
                        //remove quote
                        real_fw++;
                        fw_len -= 2;
                        if (YSTRNCMP(real_fw,p, fw_len)==0) {
                            online = 2;
                        }
                    }
                    yapiHTTPRequestSyncDone_internal(&iohdl, tmp_errmsg);
                    break;
                }
                yapiHTTPRequestSyncDone_internal(&iohdl, tmp_errmsg);
            }
        }
        // idle a bit
        yApproximateSleep(100);
    } while (!online && yapiGetTickCount()< timeout);

    if (online){
        if (online == 2) {
            setOsGlobalProgress(100, "Firmware updated");
        }else {
            setOsGlobalProgress(YAPI_VERSION_MISMATCH, "Unable to update firmware");
        }
    } else {
        setOsGlobalProgress(YAPI_DEVICE_NOT_FOUND, "Device did not reboot correctly");
    }

exit_and_free:

    if (fctx.firmware) {
        yFree(fctx.firmware);
        fctx.firmware = NULL;
    }

exitthread:
    yThreadSignalEnd(thread);
    return NULL;
}


static int yStartFirmwareUpdate(const char *serial, const char *firmwarePath, const char *settings, u16 flags, char *msg)
{

    if (yContext->fuCtx.serial)
        yFree(yContext->fuCtx.serial);
    yContext->fuCtx.serial = YSTRDUP(serial);
    if (yContext->fuCtx.firmwarePath)
        yFree(yContext->fuCtx.firmwarePath);
    if (yContext->fuCtx.settings)
        yFree(yContext->fuCtx.settings);
    yContext->fuCtx.firmwarePath = YSTRDUP(firmwarePath);
    yContext->fuCtx.settings = (u8*) YSTRDUP(settings);
    yContext->fuCtx.settings_len = YSTRLEN(settings);
    fctx.firmware = NULL;
    fctx.len = 0;
    fctx.flags = flags;
    fctx.stepA = FLASH_FIND_DEV;
    YSTRNCPY(fctx.bynHead.h.serial, YOCTO_SERIAL_LEN, serial, YOCTO_SERIAL_LEN - 1);
    yContext->fuCtx.global_progress = 0;
    YSTRCPY(msg, FLASH_ERRMSG_LEN, "Firmware update started");
    memset(&yContext->fuCtx.thread, 0, sizeof(yThread));
    //yThreadCreate will not create a new thread if there is already one running
    if (yThreadCreate(&yContext->fuCtx.thread, yFirmwareUpdate_thread, NULL)<0){
        yContext->fuCtx.serial = NULL;
        YSTRCPY(msg, FLASH_ERRMSG_LEN, "Unable to start helper thread");
        return YAPI_IO_ERROR;
    }
    return 0;

}



static YRETCODE yapiCheckFirmwareFile(const char *serial, int current_rev, u16 flags, const char *path, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    byn_head_multi *head;
    int size, res, file_rev;
    u8  *p;

    size = yLoadFirmwareFile(path, &p, errmsg);
    if (YISERR(size) || p == NULL){
        return YAPI_IO_ERROR;
    }
    head = (byn_head_multi*) p;
    res = IsValidBynFile(head, size, serial, flags, errmsg);
    if (YISERR(res)) {
        yFree(p);
        return res;
    }


    file_rev = atoi(head->h.firmware);
    if (file_rev > current_rev) {
        int pathsize = YSTRLEN(path) + 1;
        if (fullsize)
            *fullsize = YSTRLEN(path);
        if (pathsize <= buffersize) {
            YSTRCPY(buffer, buffersize, path);
        }
    } else{
        file_rev = 0;
    }
    yFree(p);
    return file_rev;
}


/***************************************************************************
 * new firmware upgrade API
 **************************************************************************/

static YRETCODE yapiCheckFirmware_r(const char *serial, int current_rev, u16 flags, const char *path, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    int best_rev = current_rev;
    int pathlen = YSTRLEN(path);
    char abspath[1024];
#ifdef WINDOWS_API
    WIN32_FIND_DATAA ffd;
    HANDLE hFind;
#else
    struct dirent *pDirent;
    DIR *pDir;
#endif

#ifdef WINDOWS_API
#else

    pDir = opendir(path);
    if (pDir == NULL) {
        return yapiCheckFirmwareFile(serial, current_rev, flags, path, buffer, buffersize, fullsize, errmsg);
    }
#endif

    if (pathlen == 0 || pathlen >= 1024 - 32) {
        return YERRMSG(YAPI_INVALID_ARGUMENT, "path too long");
    }

    YSTRCPY(abspath, 1024, path);
    if (abspath[pathlen - 1] != '/' && abspath[pathlen - 1] != '\\') {
#ifdef WINDOWS_API
        abspath[pathlen] = '\\';
#else
        abspath[pathlen] = '/';
#endif
        abspath[++pathlen] = 0;
    }


#ifdef WINDOWS_API
    // Find the first file in the directory.
    YSTRCAT(abspath, 1024, "*");
    hFind = FindFirstFileA(abspath, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return yapiCheckFirmwareFile(serial, current_rev, flags, path, buffer, buffersize, fullsize, errmsg);
    }
    do {
        char *name = ffd.cFileName;
#else
    while ((pDirent = readdir(pDir)) != NULL) {
        char *name = pDirent->d_name;
        struct stat buf;
#endif
        int isdir;
        int frev = 0;

        if (*name == '.')
            continue;
        abspath[pathlen] = 0;
        YSTRCAT(abspath, 1024, name);
#ifdef WINDOWS_API
        isdir = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
        stat(abspath, &buf);
        isdir = S_ISDIR(buf.st_mode);
#endif
        if (isdir)
        {
            frev = yapiCheckFirmware_r(serial, best_rev, flags, abspath, buffer, buffersize, fullsize, errmsg);
        } else {
            int len = YSTRLEN(name);
            if (len < 32 && 'b' == name[len - 3] && 'y' == name[len - 2] && 'n' == name[len - 1]) {
                frev = yapiCheckFirmwareFile(serial, best_rev, flags, abspath, buffer, buffersize, fullsize, errmsg);
            }
        }
        if (frev > 0){
            best_rev = frev;
        }

#ifdef WINDOWS_API
    } while (FindNextFileA(hFind, &ffd) != 0);
#else
    }
    closedir(pDir);
#endif
    return best_rev;
}


static int checkFirmwareFromWeb(const char * serial, char * out_url, int url_max_len, int *fullsize,  char * errmsg)
{
    char request[256];
    u8 *buffer;
    int res, len;
    yJsonStateMachine j;

    YSPRINTF(request, 256,"/FR/common/getLastFirmwareLink.php?serial=%s" , serial);
    res = yTcpDownload("www.yoctopuce.com", request, &buffer, 10000, errmsg);
    if (res<0){
        return res;
    }
    // Parse HTTP header
    j.src = (char*)buffer;
    j.end = j.src + res;
    j.st = YJSON_HTTP_START;
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_CODE) {
        yFree(buffer);
        return YERRMSG(YAPI_IO_ERROR,"Unexpected HTTP return code");
    }
    if (YSTRCMP(j.token, "200")) {
        yFree(buffer);
        return YERRMSG(YAPI_IO_ERROR,"Unexpected HTTP return code");
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_HTTP_READ_MSG) {
        yFree(buffer);
        return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
    }
    if (yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        yFree(buffer);
        return YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
    }
    res = 0;
    while (yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
        if (!strcmp(j.token, "link")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                res = YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                break;
            }
            len = YSTRLEN(j.token);
            if (fullsize){
                *fullsize = len;
            }
            if (url_max_len < len + 1){
                res = YERRMSG(YAPI_INVALID_ARGUMENT, "buffer too small");
                break;
            }
            if (out_url) {
                YSTRCPY(out_url,url_max_len,j.token);
            }
        } else  if (!strcmp(j.token, "version")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                res = YERRMSG(YAPI_IO_ERROR, "Unexpected JSON reply format");
                break;
            }
            res = atoi(j.token);
        } else {
            yJsonSkip(&j, 1);
        }
    }

    yFree(buffer);
    return res;
}

YRETCODE yapiCheckFirmware_internal(const char *serial, const char *rev, u32 flags, const char *path, char *buffer, int buffersize, int *fullsize, char *errmsg)
{
    int current_rev = 0;
    int best_rev;

    *buffer = 0;
    if (fullsize)
        *fullsize = 0;
    if (*rev!=0)
        current_rev = atoi(rev);

    if (isWebPath(path)>=0) {
        best_rev = checkFirmwareFromWeb(serial, buffer, buffersize, fullsize, errmsg);
    } else{
        best_rev = yapiCheckFirmware_r(serial, current_rev, (u16)flags, path, buffer, buffersize, fullsize, errmsg);
    }
    if (best_rev < 0){
        return best_rev;
    }
    if (best_rev <= current_rev) {
        buffer[0] = 0;
        if (fullsize){
            *fullsize = 0;
        }
        return 0;
    }
    return best_rev;
}

YRETCODE yapiUpdateFirmware_internal(const char *serial, const char *firmwarePath, const char *settings, int force, int startUpdate, char *msg)
{
    YRETCODE res;
    yEnterCriticalSection(&fctx.cs);
    if (startUpdate) {
        if (yContext->fuCtx.serial == NULL || yContext->fuCtx.firmwarePath == NULL) {
            res = yStartFirmwareUpdate(serial, firmwarePath, settings, force ? YPROG_FORCE_FW_UPDATE : 0, msg);
        }else if (yContext->fuCtx.global_progress < 0 || yContext->fuCtx.global_progress >= 100) {
            res = yStartFirmwareUpdate(serial, firmwarePath, settings, force ? YPROG_FORCE_FW_UPDATE : 0, msg);
        } else {
            YSTRCPY(msg, FLASH_ERRMSG_LEN, "Last firmware update is not finished");
            res = 0;
        }
    } else {
        if (yContext->fuCtx.serial == NULL || yContext->fuCtx.firmwarePath == NULL) {
            YSTRCPY(msg, FLASH_ERRMSG_LEN, "No firmware update pending");
            res = YAPI_INVALID_ARGUMENT;
        } else if (YSTRCMP(serial, yContext->fuCtx.serial) || YSTRCMP(firmwarePath, yContext->fuCtx.firmwarePath)){
            YSTRCPY(msg, FLASH_ERRMSG_LEN, "Last firmware update is not finished");
            res = YAPI_INVALID_ARGUMENT;
        } else {
            YSTRCPY(msg, FLASH_ERRMSG_LEN, yContext->fuCtx.global_message);
            res = yContext->fuCtx.global_progress;
        }
    }
    yLeaveCriticalSection(&fctx.cs);
    return res;
}

#endif
