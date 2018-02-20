/*********************************************************************
 *
 * $Id: ystream.c 29651 2018-01-17 20:48:29Z mvuilleu $
 *
 * USB stream implementation
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

#define __FILE_ID__  "ystream"
#include "yproto.h"
#include "yhash.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#ifdef WINDOWS_API
#include <time.h>
#else
#include <sys/time.h>
#endif

/*****************************************************************************
  Global variables
 ***************************************************************************/

yContextSt *yContext=NULL;

#ifdef DEBUG_TRACE_FILE
char  ytracefile[512]=DEBUG_TRACE_FILE;
#else
char  ytracefile[512]="";
#endif

/*****************************************************************************
  Error and logs functions
 ***************************************************************************/


int ySetErr(int code, char *outmsg, const char *erreur,const char *file,u32 line)
{
    const char *msg;
    if(outmsg==NULL){
        // no output buffer only return code
        return code;
    }
    if(erreur==NULL){
        switch(code){
            case YAPI_SUCCESS:          msg="Success";break;
            case YAPI_NOT_INITIALIZED:  msg="API not initialized";break;
            case YAPI_INVALID_ARGUMENT: msg="Invalid argument";break;
            case YAPI_NOT_SUPPORTED:    msg="Not supported";break;
            case YAPI_DEVICE_NOT_FOUND: msg="Device not found";break;
            case YAPI_VERSION_MISMATCH: msg="Version mismatch"; break;
            case YAPI_DEVICE_BUSY:      msg="Device busy";break;
            case YAPI_TIMEOUT:          msg="Timeout";break;
            case YAPI_IO_ERROR:         msg="I/O error";break;
            case YAPI_NO_MORE_DATA:     msg="No more data";break;
            case YAPI_EXHAUSTED:        msg="Resource exhausted";break;
            case YAPI_DOUBLE_ACCES:     msg="double access to the same device";break;
            case YAPI_UNAUTHORIZED:     msg="unauthorized access";break;
            case YAPI_RTC_NOT_READY:    msg="real-time clock has not been initialized";break;
            case YAPI_FILE_NOT_FOUND:   msg="file is not found";break;
            default:                    msg="Unknown error";
            break;
        }
    }else{
        msg=erreur;
    }
    if (file) {
        YSPRINTF(outmsg, YOCTO_ERRMSG_LEN, "%s (%s:%d)", msg, file, line);
    } else {
        YSTRCPY(outmsg, YOCTO_ERRMSG_LEN, msg);
    }
    return code;
}



int FusionErrmsg(int code,char *errmsg, const char *generr, const char *detailerr)
{
    int len;
    if(YISERR(code) && errmsg){
        YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,generr);
        YSTRCAT(errmsg,YOCTO_ERRMSG_LEN,":");
        len=YSTRLEN(errmsg);
        YSTRNCAT(errmsg,YOCTO_ERRMSG_LEN,detailerr,YOCTO_ERRMSG_LEN-len);
    }
    return code;
}





#if !defined(_MSC_VER) ||  (_MSC_VER <= MSC_VS2003)

int YFOPEN(FILE** f, const char *filename, const char *mode)
{
    *f=fopen(filename,mode);
    if(*f==NULL){
        return -1;
    }
    return 0;
}

#endif



#define DBGLOGOUT(str) yContext->log(str,YSTRLEN(str))

static int dayofs[] = { 0,31,59,90,120,151,181,212,243,273,304,334 };

static void WriteTsamp(FILE *f)
{
    u32  year, month, day, h, m, s;

#ifdef WINCE
    SYSTEMTIME time;
    GetLocalTime( &time);
    year  = time.wYear;
    month = time.wMonth;
    day   = time.wDay;
    h     = time.wHour;
    m     = time.wMinute;
    s     = time.wSecond;
#else

    u32 secs, days,y;
    time_t ltime;

    time(&ltime);
    secs = (u32)(ltime % 86400);
    days = (u32)(ltime / 86400);
    h = secs / 3600;
    m = (secs % 3600) / 60;
    s = secs % 60;
    y = days / 365; /* relative to 1970 */
    if (((y + 1) >> 2) + 365 * (u32)y > days) y--;
    days -= ((y + 1) >> 2) + 365 * (u32)y;
    if ((y & 3) == 2 && days == 59) {
        month = 2;
    } else {
        if((y & 3) == 2 && days >= 59)
        days--;
        for (month = 1; month < 12; (month)++)
            if (dayofs[month] > (u16)days) break;
    }
    year = y + 1970;
    day = days - dayofs[month - 1] + 1;
#endif
    fprintf(f,"[%04d-%02d-%02d/%02d:%02d:%02d]:",year,month,day,h,m,s);
}


int vdbglogf(const char *fileid,int line,const char *fmt,va_list args)
{
    char buffer[2048];
    int len;
    int  threadIdx;
    threadIdx = yThreadIndex();
    len=YSPRINTF(buffer,2048,"[%d]%s:% 4d: ",threadIdx,fileid,line);
    if(len<0 || len>=2028 || (len = YVSPRINTF(buffer+len,2048-len,fmt,args))<0){
        YSTRCPY(buffer,2048,"dbglogf failed\n");
        return -1;
    }
    len =YSTRLEN(buffer);

    if(yContext && yContext->log)
        yContext->log(buffer,len);

#if 0
    buffer[len] = 0;
    printf("%s", buffer);
#endif

    if(ytracefile[0]){
        FILE *f;
        if(YFOPEN(&f,ytracefile,"a+")!=0){
            return -1;
        }
        WriteTsamp(f);
        fwrite(buffer,1,len,f);
        fclose(f);
    }
    return len;
}

int dbglogf(const char *fileid,int line,const char *fmt,...)
{
    int len;
    va_list args;

    va_start( args, fmt );
    len =vdbglogf(fileid,line,fmt,args);
    va_end(args);
    return len;
}


#ifdef __BORLANDC__
#pragma argsused
int sprintf_s(char *buffer,size_t sizeOfBuffer,const char *format,...)
{
    va_list args;
    va_start( args, format);
    return vsprintf(buffer,format,args);
}
#pragma argsused
int wcstombs_s(size_t *pReturnValue, char *mbstr, size_t sizeInBytes, const wchar_t *wcstr, size_t count)
{
    *pReturnValue = wcstombs(mbstr, wcstr, count);
    return 0;
}
#endif


/*****************************************************************************
 Whitepage and Yellowpage wrapper for USB devices
 ****************************************************************************/

void ypUpdateUSB(const char *serial, const char *funcid, const char *funcname, int funclass, int funydx, const char *funcval)
{
    char    funcid_cstr[YOCTO_FUNCTION_LEN];
    char    categ[YOCTO_FUNCTION_LEN];
    yStrRef serialref, funcidref, funcnameref = INVALID_HASH_IDX;
    u16     i;

    funcid_cstr[0] = funcid[0];
    categ[0] = (funcid[0] & ~0x20); // aka to_upper()
    for(i = 1; i < YOCTO_FUNCTION_LEN-1; i++) {
        char c = funcid[i];
        if(!c || (c <= '9' && c >= '0')) break;
        funcid_cstr[i] = c;
        categ[i] = c;
    }
    categ[i] = 0;
    for(; i < YOCTO_FUNCTION_LEN-1; i++) {
        char c = funcid[i];
        if(!c) break;
        funcid_cstr[i] = c;
    }
    funcid_cstr[i] = 0;
    serialref = yHashPutStr(serial);
    funcidref = yHashPutStr(funcid_cstr);
    if(funcname) funcnameref = yHashPutStr(funcname);
    if(ypRegister(yHashPutStr(categ), serialref, funcidref, funcnameref, funclass, funydx, funcval)){
        // Forward high-level notification to API user
        yFunctionUpdate(((s32)funcidref << 16) | serialref, funcval);
    }
}

void ypUpdateYdx(int devydx, Notification_funydx funInfo, const char *funcval)
{
    YAPI_FUNCTION fundesc;

    if(ypRegisterByYdx(devydx, funInfo, funcval, &fundesc)){
        // Forward high-level notification to API user
        if (funcval) {
            char buffer[YOCTO_PUBVAL_LEN];
            decodePubVal(funInfo, funcval, buffer);
            yFunctionUpdate(fundesc,buffer);
        }
    }
}

void ypUpdateHybrid(const char *serial, Notification_funydx funInfo, const char *funcval)
{
    int     devydx;
    yStrRef serialref;

    serialref = yHashPutStr(serial);
    devydx = wpGetDevYdx(serialref);
    if(devydx >= 0) {
        ypUpdateYdx(devydx, funInfo, funcval);
    }
}

/*****************************************************************************
  THEAD / CONCURENCY RELATED FUNCTIONS
 ***************************************************************************/
//#define DEBUG_DEVICE_LOCK
#ifdef DEBUG_DEVICE_LOCK
#define LOCATION int line,
#define PUSH_LOCATION __LINE__,
#define PUSH_LOCATION __LINE__,
#else
#define LOCATION
#define PUSH_LOCATION
#endif

 static void devInitAccces(LOCATION yPrivDeviceSt *dev)
 {
    yInitializeCriticalSection(&dev->acces_state);
  }

 static void devDeleteAccces(LOCATION yPrivDeviceSt *dev)
 {
     yDeleteCriticalSection(&dev->acces_state);
 }

static int devStartIdle(LOCATION yPrivDeviceSt *dev,char *errmsg)
{
    int res =YAPI_DEVICE_BUSY;
    //get access
    if (yTryEnterCriticalSection(&dev->acces_state)==0) {
        return YERR(YAPI_DEVICE_BUSY);
    }

    if (dev->dStatus!=YDEV_WORKING){
        yLeaveCriticalSection(&dev->acces_state);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    switch(dev->rstatus){
        case YRUN_STOPED:
        case YRUN_ERROR:
            res= YERRMSG(YAPI_DEVICE_NOT_FOUND,"This device is not available");
            break;
        case YRUN_REQUEST:
        case YRUN_BUSY:
            res = YERR(YAPI_DEVICE_BUSY);
            break;
        case YRUN_AVAIL:
            dev->rstatus = YRUN_IDLE;
            // do not release the mutex on purpose!
#ifdef DEBUG_DEVICE_LOCK
            dbglog("start Idle on %s (line %d)\n",dev->infos.serial,line);
#endif
            return YAPI_SUCCESS;
        case YRUN_IDLE:
#ifdef DEBUG_DEVICE_LOCK
            dbglog("doube Idle on %s (line %d)\n",dev->infos.serial,line);
#endif
            res = YERR(YAPI_DEVICE_BUSY);
            break;
    }

    yLeaveCriticalSection(&dev->acces_state);
    return res;
}


static void devStopIdle(LOCATION yPrivDeviceSt *dev)
{

    switch(dev->rstatus){
    case YRUN_STOPED:
        break;
    case YRUN_ERROR:
    case YRUN_REQUEST:
    case YRUN_BUSY:
    case YRUN_AVAIL:
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        break;
    case YRUN_IDLE:
        dev->rstatus = YRUN_AVAIL;
#ifdef DEBUG_DEVICE_LOCK
        dbglog("stop Idle on %s (line %d)\n",dev->infos.serial,line);
#endif
        break;
    }

    yLeaveCriticalSection(&dev->acces_state);
}



static void devStartEnum(LOCATION yPrivDeviceSt *dev)
{
    u64  timeref;
    //get access
    yEnterCriticalSection(&dev->acces_state);

    timeref = yapiGetTickCount();
    while((dev->rstatus == YRUN_IDLE || dev->rstatus == YRUN_BUSY ) && (u64)(yapiGetTickCount() - timeref) < 2000){
        // if someone is doing IO release the mutex and give him 2 second to quit
        yLeaveCriticalSection(&dev->acces_state);
        yPktQueueSetError(&dev->iface.rxQueue, YAPI_DEVICE_NOT_FOUND, "Device need to be stoped");
        yApproximateSleep(100);
        yEnterCriticalSection(&dev->acces_state);
    }
    dev->rstatus = YRUN_STOPED;
    // keep the Mutex on purpose
}


static void devStopEnum(LOCATION yPrivDeviceSt *dev)
{
    yLeaveCriticalSection(&dev->acces_state);
}




static void devReportErrorFromIdle(LOCATION yPrivDeviceSt *dev, char *error_to_set)
{
    //get access
    dbglog("Error from idle %s(%d) : %s\n",dev->infos.serial,dev->rstatus,error_to_set);
    switch(dev->rstatus){
    case YRUN_STOPED:
        break;
    case YRUN_ERROR:
    case YRUN_REQUEST:
    case YRUN_BUSY:
    case YRUN_AVAIL:
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        break;
    case YRUN_IDLE:
        dev->rstatus = YRUN_ERROR;
        YSTRCPY(dev->errmsg,YOCTO_ERRMSG_LEN,error_to_set);
        break;
    }
    yLeaveCriticalSection(&dev->acces_state);
}




static int devStartIO(LOCATION yPrivDeviceSt *dev, char *errmsg)
{
    int res =YAPI_DEVICE_BUSY;
    //get access
    yEnterCriticalSection(&dev->acces_state);

    if (dev->dStatus!=YDEV_WORKING){
        yLeaveCriticalSection(&dev->acces_state);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    switch(dev->rstatus){
    case YRUN_ERROR:
        res = YERRMSG(YAPI_IO_ERROR,dev->errmsg);
        break;
    case YRUN_STOPED:
        res = YERRMSG(YAPI_DEVICE_NOT_FOUND,"This device is not available");
        break;
    case YRUN_REQUEST:
    case YRUN_BUSY:
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    case YRUN_AVAIL:
        dev->rstatus = YRUN_BUSY;
        res = YAPI_SUCCESS;
#ifdef DEBUG_DEVICE_LOCK
        dbglog("start IO on %s (line %d)\n",dev->infos.serial,line);
#endif
        break;
    case YRUN_IDLE:
        //should never occure since we keep the mutex during idlle
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    }
    yLeaveCriticalSection(&dev->acces_state);
    return res;
}


static int devPauseIO(LOCATION yPrivDeviceSt *dev,char *errmsg)
{
    int res = YAPI_SUCCESS;

    yEnterCriticalSection(&dev->acces_state);
    switch(dev->rstatus){
    case YRUN_ERROR:
        res = YERRMSG(YAPI_IO_ERROR,dev->errmsg);
        break;
    case YRUN_STOPED:
        res = YERRMSG(YAPI_DEVICE_NOT_FOUND,"This device is not available");
        break;
    case YRUN_REQUEST:
        //should never ocuure
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    case YRUN_BUSY:
        dev->rstatus = YRUN_REQUEST;
#ifdef DEBUG_DEVICE_LOCK
        dbglog("pause IO on %s (line %d)\n",dev->infos.serial,line);
#endif
        break;
    case YRUN_AVAIL:
        res = YERRMSG(YAPI_INVALID_ARGUMENT,"No IO started");
        break;
    case YRUN_IDLE:
        //should never occure since we keep the mutex during idlle
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    }

    yLeaveCriticalSection(&dev->acces_state);
    return res;
}



static int devCheckIO(LOCATION yPrivDeviceSt *dev, YIOHDL_internal *iohdl,char *errmsg)
{
    int res = YAPI_SUCCESS;

    yEnterCriticalSection(&dev->acces_state);

    if (dev->dStatus!=YDEV_WORKING){
        yLeaveCriticalSection(&dev->acces_state);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    switch(dev->rstatus){
    case YRUN_ERROR:
        res = YERRMSG(YAPI_IO_ERROR,dev->errmsg);
        break;
    case YRUN_STOPED:
        res = YERRMSG(YAPI_DEVICE_NOT_FOUND,"This device is not available");
        break;
    case YRUN_REQUEST:
        if(YIO_USB != iohdl->type || dev->pendingIO.hdl != iohdl->hdl){
            res=YERRMSG(YAPI_INVALID_ARGUMENT,"Invalid  IO Handle");
        }else{
            dev->rstatus = YRUN_BUSY;
#ifdef DEBUG_DEVICE_LOCK
            dbglog("restart IO on %s (line %d)\n",dev->infos.serial,line);
#endif
        }
        break;
   case YRUN_BUSY:
        res=YERRMSG(YAPI_INVALID_ARGUMENT,"Access violation");
        break;
    case YRUN_AVAIL:
        res = YERRMSG(YAPI_INVALID_ARGUMENT,"No IO started");
        break;
    case YRUN_IDLE:
        //should never occure since we keep the mutex during idlle
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    }

    yLeaveCriticalSection(&dev->acces_state);
    return res;
}

static int devCheckAsyncIO(LOCATION yPrivDeviceSt *dev, char *errmsg)
{
    int res = YAPI_SUCCESS;

    yEnterCriticalSection(&dev->acces_state);

    if (dev->dStatus!=YDEV_WORKING){
        yLeaveCriticalSection(&dev->acces_state);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    switch(dev->rstatus){
    case YRUN_ERROR:
        res = YERRMSG(YAPI_IO_ERROR,dev->errmsg);
        break;
    case YRUN_STOPED:
        res = YERRMSG(YAPI_DEVICE_NOT_FOUND,"This device is not available");
        break;
    case YRUN_REQUEST:
        if (dev->pendingIO.callback == NULL) {
            res=YERRMSG(YAPI_INVALID_ARGUMENT,"Invalid IO Handle");
        } else {
            dev->rstatus = YRUN_BUSY;
#ifdef DEBUG_DEVICE_LOCK
            dbglog("Async IO bg process on %s (line %d)\n",dev->infos.serial,line);
#endif
        }
        break;
   case YRUN_BUSY:
        res=YERRMSG(YAPI_INVALID_ARGUMENT,"Access violation");
        break;
    case YRUN_AVAIL:
        res = YERRMSG(YAPI_INVALID_ARGUMENT,"No IO started");
        break;
    case YRUN_IDLE:
        //should never occure since we keep the mutex during idlle
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    }

    yLeaveCriticalSection(&dev->acces_state);
    return res;
}


static int devStopIO(LOCATION yPrivDeviceSt *dev, char *errmsg)
{
    int res = YAPI_SUCCESS;

    yEnterCriticalSection(&dev->acces_state);
    switch(dev->rstatus){
    case YRUN_ERROR:
        res = YERRMSG(YAPI_IO_ERROR,dev->errmsg);
        break;
    case YRUN_STOPED:
        res = YERRMSG(YAPI_DEVICE_NOT_FOUND,"This device is not available");
        break;
    case YRUN_REQUEST:
        res=YERRMSG(YAPI_INVALID_ARGUMENT,"Access violation");
        break;
   case YRUN_BUSY:
        dev->rstatus = YRUN_AVAIL;
#ifdef DEBUG_DEVICE_LOCK
        dbglog("Stop IO on %s (line %d)\n",dev->infos.serial,line);
#endif
        break;
    case YRUN_AVAIL:
        res = YERRMSG(YAPI_INVALID_ARGUMENT,"No IO started");
        break;
    case YRUN_IDLE:
        //should never occure since we keep the mutex during idlle
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        res = YERR(YAPI_DEVICE_BUSY);
        break;
    }

    yLeaveCriticalSection(&dev->acces_state);
    return res;

}

static void devReportError(LOCATION yPrivDeviceSt *dev, char *error_to_set)
{
    //get access
    yEnterCriticalSection(&dev->acces_state);
    switch(dev->rstatus){
    case YRUN_STOPED:
    case YRUN_ERROR:
    case YRUN_REQUEST:
    case YRUN_BUSY:
    case YRUN_AVAIL:
        dbglog("Error %s(%d) : %s\n",dev->infos.serial,dev->rstatus,error_to_set);
        dev->rstatus = YRUN_ERROR;
        YSTRCPY(dev->errmsg,YOCTO_ERRMSG_LEN,error_to_set);
        break;
    case YRUN_IDLE:
        //should never occure since we keep the mutex during idlle
#ifdef DEBUG_DEVICE_LOCK
        dbglog("panic on %s (line %d)\n",dev->infos.serial,line);
#endif
        YPANIC;
        break;
    }

    yLeaveCriticalSection(&dev->acces_state);
}



/*****************************************************************************
  Dump functions
 ***************************************************************************/



#ifdef DEBUG_DUMP_PKT

static void dumpAnyStream(char *prefix,int iface,u8 pkt,u8 stream,u8 size,u8 *data,u8 pktno)
{

    USB_Notify_Pkt *notif= (USB_Notify_Pkt*) data;
    int pos, j;
    char buff[17];
    buff[16] = 0;
    dbglog("%s:%d: head: type=%d stream/cmd=%d size=%d (pktno=%d)\n",
           prefix,iface ,pkt ,stream,size,pktno);

    if(pkt== YPKT_CONF){
        switch(stream){
            case USB_CONF_RESET:
                dbglog("%s: USB_CONF_RESET\n",prefix);
                break;
            case USB_CONF_START:
                dbglog("%s: USB_CONF_START\n",prefix);
                break;
            default:
                dbglog("%s: CONF PKT INVALID!\n",prefix);
                break;
        }
    }else if(pkt == YPKT_STREAM){
        switch(stream){
            case YSTREAM_EMPTY:
                dbglog("%s: Stream Empty\n",prefix);
                break;
            case YSTREAM_NOTICE:
            case YSTREAM_NOTICE_V2:
                if(notif->firstByte <= NOTIFY_1STBYTE_MAXTINY) {
                    dbglog("%s: TINY FUNCTION VALUE\n",prefix);
                } else if(notif->firstByte >= NOTIFY_1STBYTE_MINSMALL) {
                    dbglog("%s: SMALL FUNCTION VALUE\n",prefix);
                } else switch(notif->head.type){
                    case NOTIFY_PKT_NAME:
                        dbglog("%s: NOTIFY LOGICAL NAME\n",prefix);
                        break;
                    case NOTIFY_PKT_PRODNAME:
                        dbglog("%s: NOTIFY PRODNAME\n",prefix);
                        break;
                    case NOTIFY_PKT_CHILD:
                        dbglog("%s: NOTIFY CHILD\n",prefix);
                        break;
                    case NOTIFY_PKT_FIRMWARE:
                        dbglog("%s: NOTIFY FIRMWARE\n",prefix);
                        break;
                    case NOTIFY_PKT_FUNCNAME:
                        dbglog("%s: NOTIFY FUNCTION NAME\n",prefix);
                        break;
                    case NOTIFY_PKT_FUNCVAL:
                        dbglog("%s: NOTIFY FUNCTION VALUE\n",prefix);
                        break;
                    case NOTIFY_PKT_STREAMREADY:
                        dbglog("%s: NOTIFY DEVICE READY\n",prefix);
                        break;
                    case NOTIFY_PKT_LOG:
                        dbglog("%s: NOTIFY DEVICE LOG\n",prefix);
                        break;
                    default:
                        dbglog("%s: Unknown NOTIFY\n",prefix);
                        break;
                }
                break;
            case YSTREAM_TCP:
                dbglog("%s: Stream tcp\n",prefix);
                for(pos = 0; pos < size; pos += 16) {
                    memset(buff, '.', 16);
                    for(j = 0; j < 16; j++)
                        if(data[pos+j] >= ' ')
                            buff[j] = data[pos+j];
                    dbglog("   %02x.%02x.%02x.%02x %02x.%02x.%02x.%02x %02x.%02x.%02x.%02x %02x.%02x.%02x.%02x   %s\n",
                           data[pos+0], data[pos+1], data[pos+2], data[pos+3],
                           data[pos+4], data[pos+5], data[pos+6], data[pos+7],
                           data[pos+8], data[pos+9], data[pos+10], data[pos+11],
                           data[pos+12], data[pos+13], data[pos+14], data[pos+15],
                           buff);
                }
                break;
            case YSTREAM_TCP_CLOSE:
                dbglog("%s: Stream tcp close\n",prefix);
                for(pos = 0; pos < size; pos += 16) {
                    memset(buff, '.', 16);
                    for(j = 0; j < 16; j++)
                        if(data[pos+j] >= ' ')
                            buff[j] = data[pos+j];
                    dbglog("   %02x.%02x.%02x.%02x %02x.%02x.%02x.%02x %02x.%02x.%02x.%02x %02x.%02x.%02x.%02x   %s\n",
                           data[pos+0], data[pos+1], data[pos+2], data[pos+3],
                           data[pos+4], data[pos+5], data[pos+6], data[pos+7],
                           data[pos+8], data[pos+9], data[pos+10], data[pos+11],
                           data[pos+12], data[pos+13], data[pos+14], data[pos+15],
                           buff);
                }
                break;
            default:
                dbglog("%s: Stream invalid !\n",prefix);
                break;
        }

    }
}
static u32 dumpAnyStreamHead(char *prefix,int iface,YSTREAM_Head *head,int dumpContent)
{
    u8 *data =((u8*)head)+sizeof(YSTREAM_Head);
    dumpAnyStream(prefix,iface,head->pkt , head->stream, head->size, data, head->pktno);
    return head->size;
}

void dumpAnyPacket(char *prefix, int iface,USB_Packet *pkt)
{
    u32 pos=0;
    //dbglog("%s:%d: DUMP FULL PACKET!\n",prefix,iface);
    while(pos < USB_PKT_SIZE-sizeof(YSTREAM_Head)){
        YSTREAM_Head *head =(YSTREAM_Head*) (pkt->data+pos);
        pos+=sizeof(YSTREAM_Head)+dumpAnyStreamHead(prefix,iface,head,1);
    }

}

#endif



/*****************************************************************************
  PACKET QUEUING FUNCTIONS
  ***************************************************************************/
//#define DUMP_USB_PKT_SHORT
#ifdef DUMP_USB_PKT_SHORT
static void dumpPktSummary(char *prefix, int iface,int isInput,const USB_Packet *pkt)
{
    u32 pos=0;
    char buffer[512];
    u32 len;

    len =YSPRINTF(buffer,512,"%s%s:%d",(isInput?"<-":"->"),prefix,iface);
#if 0
    while(pos < USB_PKT_SIZE-sizeof(YSTREAM_Head)){
        YSTREAM_Head *head =(YSTREAM_Head*) (pkt->data+pos);
        char ty,st;
        switch(head->pkt){
        case YPKT_STREAM :
            ty='S';
            switch(head->stream){
            case YSTREAM_EMPTY:
                st ='e';
                break;
            case YSTREAM_TCP:
                st ='T';
                break;
            case YSTREAM_TCP_CLOSE:
                st ='C';
                break;
            case YSTREAM_NOTICE:
                st ='N';
                break;
            case YSTREAM_REPORT:
                st ='X';
                break;
            case YSTREAM_META:
                st ='M';
                break;
            case YSTREAM_REPORT_V2:
                st ='V';
                break;
            case YSTREAM_NOTICE_V2:
                st ='U';
                break;
            default:
                st='?';
                break;
            }
            break;
        case YPKT_CONF:
            ty='C';
            switch(head->stream){
            case USB_CONF_RESET:
                st ='R';
                break;
            case USB_CONF_START:
                st ='S';
                break;
            default:
                st='?';
                break;
            }
            break;
        default:
            ty='?';
            st='!';
            break;
        }
        len += YSPRINTF(buffer+len,512-len," /ty=%c st=%c sz=%d pno=%d",
           ty ,st,head->size,head->pktno);
        pos+=sizeof(YSTREAM_Head)+head->size;
    }
#endif
    dbglog("%s\n",buffer);

}
#endif

void yPktQueueInit(pktQueue *q)
{
    memset(q,0,sizeof(pktQueue));
    q->status = YAPI_SUCCESS;
    yInitializeCriticalSection(&q->cs);
    yCreateManualEvent(&q->notEmptyEvent,0);
    yCreateManualEvent(&q->emptyEvent,0);
}

void yPktQueueFree(pktQueue *q)
{
    pktItem *p,*t;

    p=q->first;
    while(p){
        t=p;
        p=p->next;
        yFree(t);
    }
    yDeleteCriticalSection(&q->cs);
    yCloseEvent(&q->notEmptyEvent);
    yCloseEvent(&q->emptyEvent);
    memset(q,0xca,sizeof(pktQueue));
}

static YRETCODE  yPktQueuePushEx(pktQueue *q,const USB_Packet *pkt, char * errmsg)
{
    pktItem *newpkt;
    YRETCODE res;
    yEnterCriticalSection(&q->cs);

    if (q->status != YAPI_SUCCESS) {
        res = q->status;
        if(errmsg)
            YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,q->errmsg);
        //dbglog("%X:yPktQueuePush drop pkt\n",q);
    } else {
        res = YAPI_SUCCESS;
         // allocate new buffer
        newpkt= ( pktItem *) yMalloc(sizeof(pktItem));
        memcpy(&newpkt->pkt,pkt,sizeof(USB_Packet));
#ifdef DEBUG_PKT_TIMING
        newpkt->time = yapiGetTickCount();
        newpkt->ospktno = q->totalPush;
#endif
        newpkt->next = NULL;
        //lock the queue acces
        if (q->first == NULL) {
            //empty queue
            q->first = newpkt;
            q->last = newpkt;
            yResetEvent(&q->emptyEvent);
            //dbglog("%X:yPktQueuePush First pkt\n",q);
        } else {
            q->last->next = newpkt;
            q->last = newpkt;
            //dbglog("%X:yPktQueuePush a pkt\n",q);
        }
        q->count++;
        q->totalPush++;
    }
        ySetEvent(&q->notEmptyEvent);
        yLeaveCriticalSection(&q->cs);
    return res;
}

void  yPktQueueSetError(pktQueue *q, YRETCODE code, const char * msg)
{
    //lock the queue acces
    yEnterCriticalSection(&q->cs);
    //dbglog("PKTSetErr %d:%s\n",code,msg);
    YSTRCPY(q->errmsg,YOCTO_ERRMSG_LEN,msg);
    q->status = code;
    ySetEvent(&q->emptyEvent);
    ySetEvent(&q->notEmptyEvent);
    yLeaveCriticalSection(&q->cs);
}


static int yPktQueueIsEmpty(pktQueue *q, char * errmsg)
{
    int retval;

    yEnterCriticalSection(&q->cs);
    retval = q->status;
    if(retval !=YAPI_SUCCESS){
        //dbglog("%X:yPktQueuePop error %d:%s\n",q,q->status,q->errmsg);
        if(errmsg)
            YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,q->errmsg);
    }else{
        if(q->first==NULL)
            retval=1;
    }
    yLeaveCriticalSection(&q->cs);
    return retval;
}

static YRETCODE yPktQueuePeek(pktQueue *q, pktItem **pkt, char * errmsg)
{
    YRETCODE retval;

    yEnterCriticalSection(&q->cs);
    retval = q->status;
    if(retval !=YAPI_SUCCESS){
        //dbglog("%X:yPktQueuePop error %d:%s\n",q,q->status,q->errmsg);
        *pkt = NULL;
        if(errmsg)
            YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,q->errmsg);
    }else{
        *pkt = q->first;
    }
    yLeaveCriticalSection(&q->cs);
    return retval;
}



static YRETCODE yPktQueuePop(pktQueue *q, pktItem **pkt, char * errmsg)
{
    YRETCODE retval;

    yEnterCriticalSection(&q->cs);
    retval = q->status;
    if(retval !=YAPI_SUCCESS){
        //dbglog("%X:yPktQueuePop error %d:%s\n",q,q->status,q->errmsg);
        *pkt = NULL;
        if(errmsg)
            YSTRCPY(errmsg,YOCTO_ERRMSG_LEN,q->errmsg);
    }else{
        *pkt = q->first;
        if(q->first!=NULL){
           if(q->first == q->last){
                q->first=NULL;
                q->last=NULL;
                //dbglog("%X:yPktQueuePop last pkt\n",q);
                yResetEvent(&q->notEmptyEvent);
                ySetEvent(&q->emptyEvent);
            }else{
                //dbglog("%X:yPktQueuePop a pkt\n",q);
                q->first = q->first->next;
            }
            q->count--;
            q->totalPop++;
        }
    }
    yLeaveCriticalSection(&q->cs);
    return retval;
}


static void yPktQueueDup(pktQueue *q, int expected_pkt_no, const char *file, int line)
{
    int verifcount = 0;
    pktItem *pkt;

    yEnterCriticalSection(&q->cs);
    dbglogf(file, line, "PKTs: %dpkts (%lld in / %lld out)\n", q->count, q->totalPush, q->totalPop);
    dbglogf(file, line, "PKTs: start %x stop =%X\n", q->first, q->last);
    if (q->status != YAPI_SUCCESS) {
        dbglogf(file, line, "PKTs: state = %s\n", q->status, q->errmsg);
    }
    pkt = q->first;
    while (pkt != NULL){
        if (expected_pkt_no != pkt->pkt.first_stream.pktno) {
            dbglogf(file, line, "PKTs: invalid pkt %d (no=%d should be %d\n", verifcount, pkt->pkt.first_stream.pktno, expected_pkt_no);
        }

        verifcount++;
        expected_pkt_no = NEXT_YPKT_NO(expected_pkt_no);
        pkt = pkt->next;
    }
    if (verifcount != q->count) {
        dbglogf(file, line, "PKTs: invalid pkt count has %d report %d\n", verifcount, q->count);
    }
    yLeaveCriticalSection(&q->cs);
}


YRETCODE  yPktQueuePushD2H(yInterfaceSt *iface,const USB_Packet *pkt, char * errmsg)
{
#ifdef DUMP_USB_PKT_SHORT
    dumpPktSummary(iface->serial, iface->ifaceno,1,pkt);
#endif
#ifdef DEBUG_MISSING_PACKET
    {
        int mustdump = 0;
        yEnterCriticalSection(&iface->rxQueue.cs);
        if (pkt->first_stream.pkt != YPKT_CONF) {
            pktItem *p = iface->rxQueue.last;
            if (p != NULL && p->pkt.first_stream.pkt == YPKT_CONF) {
                int pktno = p->pkt.first_stream.pktno + 1;
                if (pktno > 7)
                    pktno = 0;
                if (pkt->first_stream.pktno != pktno) {
                    dbglog("mssing packet on push (need %d received %d)\n", pktno, pkt->first_stream.pktno );
                    mustdump++;
                }
            }
        }
        yLeaveCriticalSection(&iface->rxQueue.cs);
        if (mustdump) {
            yPktQueueDup(&iface->rxQueue, __FILE_ID__, __LINE__);
        }
    }
#endif

    return yPktQueuePushEx(&iface->rxQueue,pkt,errmsg);
}

YRETCODE yPktQueueWaitAndPopD2H(yInterfaceSt *iface,pktItem **pkt, int ms, char * errmsg)
{

    YRETCODE res;
    *pkt = NULL;
    res= yPktQueuePop(&iface->rxQueue,pkt,errmsg);
    if(res != YAPI_SUCCESS || ms == 0){
        return  res;
    }
    if (*pkt == NULL) {
        yWaitForEvent(&iface->rxQueue.notEmptyEvent, ms);
        return  yPktQueuePop(&iface->rxQueue,pkt, errmsg);
    }
    return res;
}


YRETCODE  yPktQueuePushH2D(yInterfaceSt *iface,const USB_Packet *pkt, char * errmsg)
{
    return yPktQueuePushEx(&iface->txQueue,pkt,errmsg);
}

// return 1 if empty, 0 if not empty, or an error code
static int yPktQueueWaitEmptyH2D(yInterfaceSt *iface,int ms, char * errmsg)
{
    if (ms > 0) {
        yWaitForEvent(&iface->txQueue.emptyEvent, ms);
    }
    return yPktQueueIsEmpty(&iface->txQueue,errmsg);
}


YRETCODE yPktQueuePeekH2D(yInterfaceSt *iface,pktItem **pkt)
{

#ifdef DUMP_USB_PKT_SHORT
    pktItem *tmp;
    YRETCODE res = yPktQueuePeek(&iface->txQueue,&tmp,NULL);
    if(tmp!=NULL){
        dumpPktSummary(iface->serial, iface->ifaceno, 0, &tmp->pkt);
    }
    *pkt=tmp;
    return res;
#else
    return yPktQueuePeek(&iface->txQueue,pkt,NULL);
#endif
}

YRETCODE yPktQueuePopH2D(yInterfaceSt *iface,pktItem **pkt)
{

#ifdef DUMP_USB_PKT_SHORT
    pktItem *tmp;
    YRETCODE res = yPktQueuePop(&iface->txQueue,&tmp,NULL);
    if(tmp!=NULL){
        dumpPktSummary(iface->serial, iface->ifaceno,0,&tmp->pkt);
    }
    *pkt=tmp;
    return res;
#else
    return yPktQueuePop(&iface->txQueue,pkt,NULL);
#endif
}


/*****************************************************************************
  yyPACKET ioFUNCTIONS
  ***************************************************************************/


// this function copy the pkt into the interface out queue and send the packet
YRETCODE yyySendPacket(yInterfaceSt *iface, const USB_Packet *pkt, char *errmsg)
{
    int res;
    res = yPktQueuePushH2D(iface,pkt,errmsg);
    if (YISERR(res)) {
        return (YRETCODE) res;
    }
    res = yyySignalOutPkt(iface, errmsg);
    if (YISERR(res)) {
        return res;
    }
    res = yPktQueueWaitEmptyH2D(iface,5000,errmsg);
    if (YISERR(res)) {
        return (YRETCODE) res;
    }else if(res > 0){
        return YAPI_SUCCESS;
    }
    return YERRMSG(YAPI_TIMEOUT,"Unable to send packet to the device");
}

static YRETCODE yAckPkt(yInterfaceSt *iface, int pktno, char *errmsg)
{

    pktItem qpkt;

    YSTREAM_Head  *yshead = (YSTREAM_Head*)(qpkt.pkt.data);
    u8 *data = qpkt.pkt.data + sizeof(YSTREAM_Head);
    yshead->pktno = 0;
    yshead->pkt = YPKT_STREAM;
    yshead->stream = YSTREAM_META;
    yshead->size = 2;
    data[0] = USB_META_ACK_D2H_PACKET;
    data[1] = pktno & 0xff;
    yshead = (YSTREAM_Head*)(qpkt.pkt.data + 2 + sizeof(YSTREAM_Head));
    yshead->pkt = YPKT_STREAM;
    yshead->stream = YSTREAM_EMPTY;
    yshead->size = USB_PKT_SIZE - (2 * sizeof(YSTREAM_Head) + 2);
    return yyySendPacket(iface, &qpkt.pkt, errmsg);
}




// wait for the next Configuration packet to come on a particular interface
// ALL OTHER PACKET OF THIS INTERFACE ARE DROPED
static int yyWaitOnlyConfPkt(yInterfaceSt *iface, u8 cmdtowait,pktItem **rpkt,u32 s_timeout,char *errmsg)
{
    u64 timeout = yapiGetTickCount() + s_timeout * 1000;
    pktItem *tmp;
    u32 dropcount = 0;
    YRETCODE  error;

    *rpkt = NULL;
    do{
        error = yPktQueueWaitAndPopD2H(iface, &tmp, 1000, errmsg);
        if(error!=YAPI_SUCCESS){
            return error;
        }
        if (tmp != NULL){
            if(tmp->pkt.confpkt.head.pkt == YPKT_CONF && tmp->pkt.confpkt.head.stream == cmdtowait){
                //conf packet has bee received
                YASSERT(tmp->pkt.confpkt.head.size >= sizeof(USB_Conf_Pkt));
                *rpkt=tmp;
                if(dropcount)
                    dbglog("drop %d pkt on iface %d\n",dropcount, iface->ifaceno);
                return YAPI_SUCCESS;
            }
#ifdef DEBUG_DUMP_PKT
            else{
                dumpAnyPacket("Drop non-config pkt",(u8)iface->ifaceno, &tmp->pkt);
            }
#endif
            dropcount++;
            yFree(tmp);
        }
    } while(timeout> yapiGetTickCount());

    return YERR(YAPI_TIMEOUT);
}


static void yyFormatConfPkt(pktItem *pkt, u8 conftype)
{
    pkt->pkt.confpkt.head.pkt    = YPKT_CONF;
    pkt->pkt.confpkt.head.stream = conftype;
    pkt->pkt.confpkt.head.size   = USB_PKT_SIZE - sizeof(pkt->pkt.confpkt.head);
    pkt->pkt.confpkt.head.pktno  = 0;
}


// check procol version compatibility
// compatiblewithout limitation -> return 1
// compatible with limitations -> return 0;
// incompatible -> return YAPI_IO_ERROR
static int CheckVersionCompatibility(u16 version,const char *serial, char *errmsg)
{
    if ((version & 0xff00) != (YPKT_USB_VERSION_BCD & 0xff00)) {
        // major version change
        if ((version & 0xff00) > (YPKT_USB_VERSION_BCD & 0xff00)) {
            dbglog("Yoctopuce library is too old (using 0x%x, need 0x%x) to handle device %s, please upgrade your Yoctopuce library\n",YPKT_USB_VERSION_BCD,version,serial);
            return YERRMSG(YAPI_IO_ERROR,"Library is too old to handle this device");
        } else {
            // implement backward compatibility when implementing a new protocol
            YPANIC;
            return 1;
        }
    } else if (version != YPKT_USB_VERSION_BCD) {
        if (version == YPKT_USB_VERSION_NO_RETRY_BCD && (yContext->detecttype & Y_RESEND_MISSING_PKT) == 0) {
            // do prompt for firmware update since the Y_RESEND_MISSING_PKT feature is not used
            return 1;
        }
        if (version > YPKT_USB_VERSION_BCD) {
            dbglog("Device %s is using a newer protocol, consider upgrading your Yoctopuce library\n",serial);
        } else {
            dbglog("Device %s is using an older protocol, consider upgrading the device firmware\n",serial);
        }
        return 0;
    }
    return 1;
}


static int ySendStart(yPrivDeviceSt *dev,char *errmsg)
{
    int nextiface;
    pktItem  qpkt,*rpkt;

    yyFormatConfPkt(&qpkt,USB_CONF_START);
    if (yContext->detecttype & Y_RESEND_MISSING_PKT && dev->iface.pkt_version >= YPKT_USB_VERSION_BCD) {
        dev->pktAckDelay = Y_DEFAULT_PKT_RESEND_DELAY;
    } else{
        dev->pktAckDelay = 0;
    }
    qpkt.pkt.confpkt.conf.start.nbifaces = 1;
    qpkt.pkt.confpkt.conf.start.ack_delay = dev->pktAckDelay;
    YPROPERR(yyySendPacket(&dev->iface, &qpkt.pkt, errmsg));
    YPROPERR(yyWaitOnlyConfPkt(&dev->iface, USB_CONF_START, &rpkt, 5, errmsg));
    nextiface = rpkt->pkt.confpkt.conf.start.nbifaces;
    if (dev->pktAckDelay) {
        // update ack delay with the one from the device (in case device does not implement pkt ack)
        dev->pktAckDelay = rpkt->pkt.confpkt.conf.start.ack_delay;
        dbglog("Activate USB pkt ack (%dms)\n", dev->pktAckDelay);
    }
    dev->lastpktno = rpkt->pkt.first_stream.pktno;
    yFree(rpkt);
    if(nextiface!=0 ){
        return YERRMSG(YAPI_VERSION_MISMATCH,"Device has not been started correctly");
    }
    return YAPI_SUCCESS;
}


static int yPacketSetup(yPrivDeviceSt *dev,char *errmsg)
{
    pktItem qpkt, *rpkt = NULL;
    int res;

    YPROPERR(yyySetup(&dev->iface, errmsg));

    //first try to send a connection reset
    yyFormatConfPkt(&qpkt, USB_CONF_RESET);
    qpkt.pkt.confpkt.conf.reset.ok = 1;
    TO_SAFE_U16(qpkt.pkt.confpkt.conf.reset.api, YPKT_USB_VERSION_BCD);
    YPROPERR(yyySendPacket(&dev->iface, &qpkt.pkt, errmsg));

    if (YISERR(yyWaitOnlyConfPkt(&dev->iface, USB_CONF_RESET, &rpkt, 5, errmsg)) || rpkt == NULL) {
        res = YERRMSG(YAPI_VERSION_MISMATCH, "Device does not respond to reset");
        goto error;
    }
    FROM_SAFE_U16(rpkt->pkt.confpkt.conf.reset.api, dev->iface.pkt_version);
    if (CheckVersionCompatibility(dev->iface.pkt_version, dev->iface.serial, errmsg)<0) {
        res = YAPI_VERSION_MISMATCH;
        goto error;
    }
    YASSERT(rpkt->pkt.confpkt.conf.reset.ifaceno < NBMAX_INTERFACE_PER_DEV);
    if (rpkt->pkt.confpkt.conf.reset.nbifaces != 1) {
        res = YERRMSG(YAPI_VERSION_MISMATCH, "Multiples USB interface are no more supported");
        goto error;
    }
    dev->iface.ifaceno = 0;
    yFree(rpkt);
    rpkt = NULL;

    if(!YISERR(res=ySendStart(dev,errmsg))){
        return YAPI_SUCCESS;
     }
error:
    if (rpkt) {
        yFree(rpkt);
    }
    //shutdown all previously started interfaces;
    dbglog("Closing partially opened device %s\n",dev->infos.serial);
    yyyPacketShutdown(&dev->iface);
    return res;
}

// Trigger a non blocking read
static int yGetNextPktEx(yPrivDeviceSt *dev, pktItem **pkt_out, u64 blockUntilTime, char *errmsg)
{
    int             dropcount=0;
    u8              nextpktno;
    YRETCODE        res;
    pktItem         *item;
    u64             wait;
    u64             now;
     yInterfaceSt   *iface;

    *pkt_out = NULL;
    iface = &dev->iface;
again:
    now = yapiGetTickCount();
    if (blockUntilTime > now)
        wait = blockUntilTime - now;
    else
        wait = 0;



    // ptr is set to null by yPktQueueWaitAndPop
    res = yPktQueueWaitAndPopD2H(iface, &item, (int)wait, errmsg);
    if (YISERR(res))
        return res;
    nextpktno = NEXT_YPKT_NO(dev->lastpktno);
    if (item != NULL) {
        if (dev->pktAckDelay > 0) {
            res = yAckPkt(iface, item->pkt.first_stream.pktno, errmsg);
            if (YISERR(res)){
                yFree(item);
                return res;
            }
        }
        // verfiy the packet
        if (item->pkt.first_stream.pkt == YPKT_CONF) {
#ifdef DEBUG_DUMP_PKT
            dumpAnyPacket("Drop Late config pkt",iface->ifaceno,&item->pkt);
#endif
            yFree(item);
            dropcount++;
            if(dropcount >10){
                dbglog("Too many packets dropped, disable %s\n",dev->infos.serial);
                return YERRMSG(YAPI_IO_ERROR,"Too many packets dropped");
            }
            goto again;
        }
        if (item->pkt.first_stream.pktno == dev->lastpktno) {
            //late retry : drop it since we allready have the packet.
            yFree(item);
            goto again;
        }

        if(item->pkt.first_stream.pktno == nextpktno){
            *pkt_out = item;
            dev->lastpktno = nextpktno;
#ifdef DEBUG_DUMP_PKT
            dumpAnyPacket("POP Pkt",iface->ifaceno,&item->pkt);
#endif
            return YAPI_SUCCESS;
        } else {
            yPktQueueDup(&iface->rxQueue, nextpktno, __FILE_ID__, __LINE__);
            yFree(item);
            return YERRMSG(YAPI_IO_ERROR, "Missing Packet");
        }
    }
    return YAPI_SUCCESS;
}


/*****************************************************************
 * yStream API with cycling logic of the inside of the packet
*****************************************************************/
static int yStreamGetTxBuff(yPrivDeviceSt *dev, u8 **data, u8 *maxsize);
static int yStreamTransmit(yPrivDeviceSt *dev, u8 proto, u8 size, char *errmsg);
static int yStreamFlush(yPrivDeviceSt *dev, char *errmsg);

static int yStreamSetup(yPrivDeviceSt *dev,char *errmsg)
{
    u32 currUtcTime;
    YPROPERR(yPacketSetup(dev,errmsg));
    // now we have all setup packet sent and received
    dev->currxpkt=NULL;
    dev->curxofs=0xff;
    dev->curtxpkt = &dev->tmptxpkt;
    dev->tmptxpkt.next=NULL;
    dev->curtxofs=0;
    dev->devYdxMap=NULL;
    dev->lastUtcUpdate=0;
    // send UTC time to the device
    currUtcTime = (u32)time(NULL);
    if (currUtcTime > (u32)0x51f151f1){ // timestamp appears to be valid
        u8  *pktdata;
        u8  maxpktlen;
        // send updated UTC timestamp to keep datalogger on time
        if (yStreamGetTxBuff(dev, &pktdata, &maxpktlen) && maxpktlen >= 5) {
            dev->lastUtcUpdate = currUtcTime;
            pktdata[0] = USB_META_UTCTIME;
            pktdata[1] = currUtcTime & 0xff;
            pktdata[2] = (currUtcTime >> 8) & 0xff;
            pktdata[3] = (currUtcTime >> 16) & 0xff;
            pktdata[4] = (currUtcTime >> 24) & 0xff;
            YPROPERR(yStreamTransmit(dev, YSTREAM_META, 5, errmsg));
            YPROPERR(yStreamFlush(dev, errmsg));
        }
    }
    return YAPI_SUCCESS;
}


static int yStreamReceived(yPrivDeviceSt *dev, u8 *stream, u8 **data, u8 *size, u64 blockUntilTime, char *errmsg)
{
    YSTREAM_Head  *yshead;
    int res;

    //check if we have processed the full usb packet
    if (dev->curxofs >= USB_PKT_SIZE - sizeof(YSTREAM_Head)) {
        // look if we have the next packet on a interface
        if (dev->currxpkt) {
            yFree(dev->currxpkt);
            dev->currxpkt=NULL;
        }
        res = yGetNextPktEx(dev, &dev->currxpkt, blockUntilTime, errmsg);
        YPROPERR(res);
        if (dev->currxpkt != NULL) {
            dev->curxofs=0;
        } else {
            // no pkt avail
            return 0;
        }
    }

    yshead= (YSTREAM_Head*) &dev->currxpkt->pkt.data[dev->curxofs];
    YASSERT(dev->curxofs + sizeof(YSTREAM_Head) + yshead->size <= USB_PKT_SIZE);
    *stream = yshead->stream;
    *size = yshead->size;
    *data = &dev->currxpkt->pkt.data[ dev->curxofs + sizeof(YSTREAM_Head) ];

    return 1;
}

static int yStreamReceptionDone(yPrivDeviceSt *dev,char *errmsg)
{
    YSTREAM_Head * yshead= (YSTREAM_Head*) &dev->currxpkt->pkt.data[dev->curxofs];
    dev->curxofs += sizeof(YSTREAM_Head) + yshead->size;
    return YAPI_SUCCESS;
}
// return 0 when no more buff is available
static int yStreamGetTxBuff(yPrivDeviceSt *dev, u8 **data, u8 *maxsize)
{
    u8  avail;
    avail = USB_PKT_SIZE - dev->curtxofs;
    if(avail <= sizeof(YSTREAM_Head)){
        //we may want to allocate an new buffer
        return 0;
    }

    *maxsize = avail - sizeof(YSTREAM_Head);
    *data = &dev->curtxpkt->pkt.data[dev->curtxofs+sizeof(YSTREAM_Head)];
    return 1;
}

static int yStreamFlush(yPrivDeviceSt *dev,char *errmsg)
{
    u8 avail;

    if(dev->curtxofs==0){
        return YAPI_SUCCESS;
    }

    avail = USB_PKT_SIZE - dev->curtxofs;

    if(avail >=sizeof(YSTREAM_Head) ){
        YSTREAM_Head  *yshead = (YSTREAM_Head*) (dev->curtxpkt->pkt.data + dev->curtxofs);
        yshead->pkt    = YPKT_STREAM;
        yshead->stream = YSTREAM_EMPTY;
        yshead->size   = avail - sizeof(YSTREAM_Head);
        dev->curtxofs  += sizeof(YSTREAM_Head)+yshead->size;
    }
    YPROPERR( yyySendPacket(&dev->iface,&dev->curtxpkt->pkt, errmsg));
    dev->curtxofs =0;
    return YAPI_SUCCESS;
}


static int yStreamTransmit(yPrivDeviceSt *dev, u8 proto,u8 size,char *errmsg)
{
    //dbglog("yPacketTransmit\n");

    YSTREAM_Head  *yshead;
    yshead = (YSTREAM_Head*) dev->curtxpkt->pkt.data + dev->curtxofs ;
    yshead->pktno   =0;
    yshead->pkt     = YPKT_STREAM;
    yshead->stream  = proto;
    yshead->size    = size;
    if(!(dev->curtxofs  + sizeof(YSTREAM_Head) + size <=USB_PKT_SIZE)){
        dbglog("Buffer overrun detected on (%s)\n",dev->infos.serial);
        dbglog("unable to append data proto=%d (%d + %d + %d <= %d) (%s)\n",
                proto,dev->curtxofs,sizeof(YSTREAM_Head),size,USB_PKT_SIZE);
        //dumpAnyPacket("ystream buffer",0,&dev->curtxpkt->pkt);
        return YERRMSG(YAPI_IO_ERROR,"buffer overrun");
    }
    dev->curtxofs += sizeof(YSTREAM_Head)+size;
    if( dev->curtxofs >= USB_PKT_SIZE-sizeof(YSTREAM_Head))
        return yStreamFlush(dev,errmsg);
    return YAPI_SUCCESS;
}


static void yStreamShutdown(yPrivDeviceSt *dev)
{
    if(dev->devYdxMap) {
        yFree(dev->devYdxMap);
        dev->devYdxMap = NULL;
    }
    yyyPacketShutdown(&dev->iface);
}


// Notification packet dispatcher
//
static void yDispatchNotice(yPrivDeviceSt *dev, USB_Notify_Pkt *notify, int pktsize, int isV2)
{
    yPrivDeviceSt *notDev;
    u16 vendorid,deviceid;

    if(isV2 || notify->firstByte <= NOTIFY_1STBYTE_MAXTINY || notify->firstByte >= NOTIFY_1STBYTE_MINSMALL) {
        // Tiny or small pubval notification:
        // create a new null-terminated small notification that we can use and forward
        char buff[sizeof(Notification_small)+YOCTO_PUBVAL_SIZE+2];
        Notification_small *smallnot = (Notification_small *)buff;
        memset(smallnot->pubval,0,YOCTO_PUBVAL_SIZE+2);

        if (notify->smallpubvalnot.funInfo.v2.isSmall == 0) {
            // convert tiny notification to samll notification
            memcpy(smallnot->pubval, notify->tinypubvalnot.pubval,pktsize - sizeof(Notification_tiny));
            smallnot->funInfo.v2.funydx = notify->tinypubvalnot.funInfo.v2.funydx;
            smallnot->funInfo.v2.typeV2 = notify->tinypubvalnot.funInfo.v2.typeV2;
            smallnot->funInfo.v2.isSmall = 1;
            smallnot->devydx = wpGetDevYdx(yHashPutStr(dev->infos.serial));
        } else {
#ifndef __BORLANDC__
            YASSERT(0);
#endif
            // Assert added on 2015-02-25, remove code below when confirmed dead code
            memcpy(smallnot->pubval,notify->smallpubvalnot.pubval,pktsize - sizeof(Notification_small));
            smallnot->funInfo.raw = notify->smallpubvalnot.funInfo.raw;
            if(dev->devYdxMap) {
                smallnot->devydx = dev->devYdxMap[notify->smallpubvalnot.devydx];
            } else {
                smallnot->devydx = 255;
            }
        }
#ifdef DEBUG_NOTIFICATION
        if(smallnot->funInfo.v2.typeV2 == NOTIFY_V2_LEGACY) {
            dbglog("notifysmall %d %d %s\n",smallnot->devydx,smallnot->funInfo.v2.funydx,smallnot->pubval);
        } else {
            u8 *tmpbuff = (u8 *)smallnot->pubval;
            dbglog("notifysmall %d %d %d:%02x.%02x.%02x.%02x.%02x.%02x\n",smallnot->devydx,smallnot->funInfo.v2.funydx,smallnot->funInfo.v2.typeV2,
                   tmpbuff[0],tmpbuff[1],tmpbuff[2],tmpbuff[3],tmpbuff[4],tmpbuff[5]);
        }
#endif
        if (smallnot->devydx < 255 && smallnot->funInfo.v2.typeV2 != NOTIFY_V2_FLUSHGROUP) {
            ypUpdateYdx(smallnot->devydx,smallnot->funInfo,smallnot->pubval);
            if(yContext->rawNotificationCb){
                yContext->rawNotificationCb((USB_Notify_Pkt *)smallnot);
            }
        }
        return;
    }

    notDev=findDev(notify->head.serial,FIND_FROM_SERIAL);
    if(notDev==NULL){
        dbglog("drop Notification %d for %s received (device missing)\n", notify->head.type,notify->head.serial);
        return;
    }

#ifdef DEBUG_NOTIFICATION
    dbglog("Notification %d received\n", notify->head.type);
#endif
    switch(notify->head.type){
    case NOTIFY_PKT_NAME:
        YSTRCPY(notDev->infos.logicalname,YOCTO_LOGICAL_LEN,notify->namenot.name);
#ifdef DEBUG_NOTIFICATION
        dbglog("new name is \"%s\"\n", notify->namenot.name);
#endif
        notDev->infos.beacon=notify->namenot.beacon;
#ifdef DEBUG_NOTIFICATION
        dbglog("new beacon %x\n",notify->namenot.beacon);
#endif
        {
            yStrRef serialref = yHashPutStr(notify->head.serial);
            yStrRef lnameref = yHashPutStr(notify->namenot.name);
            wpSafeUpdate(NULL, MAX_YDX_PER_HUB,serialref,lnameref,yHashUrlUSB(serialref),notify->namenot.beacon);
            if(yContext->rawNotificationCb){
                yContext->rawNotificationCb(notify);
            }
        }
        break;
    case NOTIFY_PKT_PRODNAME:
        YSTRCPY(notDev->infos.productname,YOCTO_PRODUCTNAME_LEN,notify->productname);
#ifdef DEBUG_NOTIFICATION
        dbglog("Product name update %s\n",notify->productname);
#endif
        break;
    case NOTIFY_PKT_CHILD:
#ifdef DEBUG_NOTIFICATION
        dbglog("child update %s %d %u\n",notify->childserial.childserial,notify->childserial.onoff,notify->childserial.devydx);
#endif
        if(notDev == dev) {
            // build devYdx mapping for immediate child hubs
            if(dev->devYdxMap == NULL) {
                dev->devYdxMap = (u8*) yMalloc(ALLOC_YDX_PER_HUB);
                memset(dev->devYdxMap, 255, ALLOC_YDX_PER_HUB);
            }
            dev->devYdxMap[notify->childserial.devydx] = wpGetDevYdx(yHashPutStr(notify->childserial.childserial));
        }
        break;
    case NOTIFY_PKT_FIRMWARE:
#ifdef DEBUG_NOTIFICATION
        dbglog("notify firmware %s\n",notify->firmwarenot.firmware);
#endif
        YSTRCPY(notDev->infos.firmware,YOCTO_FIRMWARE_LEN,notify->firmwarenot.firmware);
        FROM_SAFE_U16(notify->firmwarenot.deviceid,deviceid);
        FROM_SAFE_U16(notify->firmwarenot.vendorid,vendorid);
        YASSERT(notDev->infos.deviceid == deviceid);
        YASSERT(notDev->infos.vendorid == vendorid);
        break;
    case NOTIFY_PKT_FUNCNAME:
        notify->funcnameydxnot.funydx = -1;
        // common code below
    case NOTIFY_PKT_FUNCNAMEYDX:
        if(notify->funcnameydxnot.funclass >= YOCTO_N_BASECLASSES) {
            // Unknown subclass, use YFunction instead
            notify->funcnameydxnot.funclass = YOCTO_AKA_YFUNCTION;
        }
#ifdef DEBUG_NOTIFICATION
        if(notify->funcnameydxnot.funydx >= 0) {
            dbglog("notify functnameydx %s %s %d %d\n",notify->funcnamenot.funcid, notify->funcnamenot.funcname,
                   notify->funcnameydxnot.funydx, notify->funcnameydxnot.funclass);
        } else {
            dbglog("notify functname %s %s\n",notify->funcnamenot.funcid, notify->funcnamenot.funcname);
        }
#endif
        ypUpdateUSB(notDev->infos.serial,notify->funcnamenot.funcid,notify->funcnamenot.funcname,notify->funcnameydxnot.funclass,notify->funcnameydxnot.funydx,NULL);
        if(yContext->rawNotificationCb){
            yContext->rawNotificationCb(notify);
        }
        break;
    case NOTIFY_PKT_FUNCVAL:
        {
            char buff[YOCTO_PUBVAL_SIZE+2];
            memset(buff,0,YOCTO_PUBVAL_SIZE+2);
            memcpy(buff,notify->pubvalnot.pubval,YOCTO_PUBVAL_SIZE);
#ifdef DEBUG_NOTIFICATION
            dbglog("notify funcval %s %s\n",notify->pubvalnot.funcid, buff);
#endif
            ypUpdateUSB(notDev->infos.serial,notify->pubvalnot.funcid,NULL,-1,-1,buff);
            if(yContext->rawNotificationCb){
                yContext->rawNotificationCb(notify);
            }
        }
        break;
    case NOTIFY_PKT_STREAMREADY:
#ifdef DEBUG_NOTIFICATION
        dbglog("notify steam ready (%x)\n",notify->raw);
#endif
        notDev->rstatus = YRUN_AVAIL;
        break;
    case NOTIFY_PKT_LOG:
        {
            if (!strncmp(notify->head.serial, dev->infos.serial, YOCTO_SERIAL_LEN)) {
                yStrRef serialref = yHashPutStr(notify->head.serial);
                int devydx = wpGetDevYdx(serialref);
                if (devydx >=0 ) {
                    yEnterCriticalSection(&yContext->generic_cs);
                    if (yContext->generic_infos[devydx].flags & DEVGEN_LOG_ACTIVATED) {
                        yContext->generic_infos[devydx].flags |= DEVGEN_LOG_PENDING;
#ifdef DEBUG_NOTIFICATION
                        dbglog("notify device log for %s\n",dev->infos.serial);
#endif
                    }
#ifdef DEBUG_NOTIFICATION
                    else {
                        dbglog("notify device log for %s dropped\n",dev->infos.serial);
                    }
#endif
                    yLeaveCriticalSection(&yContext->generic_cs);
                }

            }
            if(yContext->rawNotificationCb){
                yContext->rawNotificationCb(notify);
            }
        }
    default:
        break;
    }
}

// Timed report packet dispatcher
//
static void yDispatchReportV1(yPrivDeviceSt *dev, u8 *data, int pktsize)
{
    yStrRef serialref = yHashPutStr(dev->infos.serial);
#ifdef DEBUG_NOTIFICATION
    {
        USB_Report_Pkt_V1 *report = (USB_Report_Pkt_V1*)data;
        dbglog("timed report (v1) for %d %d\n", wpGetDevYdx(serialref), report->funYdx);
    }
#endif
    if(yContext->rawReportCb) {
        yContext->rawReportCb(serialref, (USB_Report_Pkt_V1*) data, pktsize);
    }
    if (yContext->timedReportCallback) {
        int  devydx = wpGetDevYdx(serialref);
        if (devydx < 0)
            return;
        while (pktsize > 0) {
            USB_Report_Pkt_V1 *report = (USB_Report_Pkt_V1*) data;
            int  len = report->extraLen + 1;
            if (report->funYdx == 0xf) {
                u32 t = data[1] + 0x100u * data[2] + 0x10000u * data[3] + 0x1000000u * data[4];
                yEnterCriticalSection(&yContext->generic_cs);
                yContext->generic_infos[devydx].deviceTime = (double)t + data[5] / 250.0;
                yLeaveCriticalSection(&yContext->generic_cs);
            } else {
                YAPI_FUNCTION fundesc;
                double devtime;
                Notification_funydx funInfo;
                funInfo.raw = report->funYdx;
                ypRegisterByYdx(devydx, funInfo, NULL, &fundesc);
                data[0] = report->isAvg ? 1 : 0;
                yEnterCriticalSection(&yContext->generic_cs);
                devtime = yContext->generic_infos[devydx].deviceTime;
                yLeaveCriticalSection(&yContext->generic_cs);
                yFunctionTimedUpdate(fundesc, devtime, data, len + 1);
            }
            pktsize -= 1 + len;
            data += 1 + len;
        }
    }
}

// Timed report packet dispatcher
//
static void yDispatchReportV2(yPrivDeviceSt *dev, u8 *data, int pktsize)
{
    yStrRef serialref = yHashPutStr(dev->infos.serial);
#ifdef DEBUG_NOTIFICATION
    {
        USB_Report_Pkt_V2 *report = (USB_Report_Pkt_V2*)data;
        dbglog("timed report (v2) for %d %d\n", wpGetDevYdx(serialref), report->funYdx);
    }
#endif
    if(yContext->rawReportV2Cb) {
        yContext->rawReportV2Cb(serialref, (USB_Report_Pkt_V2*) data, pktsize);
    }
    if (yContext->timedReportCallback) {
        int  devydx = wpGetDevYdx(serialref);
        if (devydx < 0)
            return;
        while (pktsize > 0) {
            USB_Report_Pkt_V2 *report = (USB_Report_Pkt_V2*) data;
            int  len = report->extraLen + 1;
            if (report->funYdx == 0xf) {
                u32 t = data[1] + 0x100u * data[2] + 0x10000u * data[3] + 0x1000000u * data[4];
                yEnterCriticalSection(&yContext->generic_cs);
                yContext->generic_infos[devydx].deviceTime = (double)t + data[5] / 250.0;
                yLeaveCriticalSection(&yContext->generic_cs);
            } else {
                YAPI_FUNCTION fundesc;
                double devtime;
                Notification_funydx funInfo;
                funInfo.raw = report->funYdx;
                ypRegisterByYdx(devydx, funInfo, NULL, &fundesc);
                data[0] = 2;
                yEnterCriticalSection(&yContext->generic_cs);
                devtime = yContext->generic_infos[devydx].deviceTime;
                yLeaveCriticalSection(&yContext->generic_cs);
                yFunctionTimedUpdate(fundesc, devtime, data, len + 1);
            }
            pktsize -= 1 + len;
            data += 1 + len;
        }
    }
}

// blockUntilTime:
//    0 -> only check pending (non blocking)
//    >0 -> wait util yapiGetTickCount is >= blockUntilTime
static int yDispatchReceive(yPrivDeviceSt *dev, u64 blockUntilTime, char *errmsg)
{
    u8 stream;
    u8 size;
    u8 *data;
#ifdef DEBUG_USB_TRAFIC
    char dump[64];
#endif
    int pktavail;

    pktavail = yStreamReceived(dev, &stream, &data, &size, blockUntilTime,errmsg);
    YPROPERR(pktavail);
    while (pktavail && yFifoGetFree(&dev->http_fifo) > size) {
        switch (stream) {
            case YSTREAM_TCP_CLOSE:
#ifdef DEBUG_USB_TRAFIC
                dbglog("YSTREAM : TCP_CLOSE %d (%d:%d)\n",size,dev->pendingIO.hdl,dev->httpstate);
                memcpy(dump,data,size);
                dump[size]=0;
                dbglog("---------------------------\n");
                dbglog("%s\n",dump);
                dbglog("---------------------------\n");
#endif
                if(dev->httpstate == YHTTP_OPENED || dev->httpstate == YHTTP_INREQUEST) {
                    //handle new received packet
                    if(size != yPushFifo(&dev->http_fifo, data, size)){
                        return YERRMSG(YAPI_IO_ERROR,"FIFO overrun");
                    }
                    dev->httpstate = YHTTP_CLOSE_BY_DEV;
                } else if(dev->httpstate == YHTTP_CLOSE_BY_API) {
                    dev->httpstate = YHTTP_CLOSED;
                }
                break;
            case YSTREAM_TCP:
#ifdef DEBUG_USB_TRAFIC
                dbglog("YSTREAM : TCP %d (%d:%d)\n",size,dev->pendingIO.hdl,dev->httpstate);
                memcpy(dump,data,size);
                dump[size]='\0';
                dbglog("---------------------------\n");
                dbglog("%s\n",dump);
                dbglog("---------------------------\n");
#endif
                if (dev->httpstate == YHTTP_OPENED || dev->httpstate == YHTTP_INREQUEST) {
                    //handle new received packet
                    if(size!=yPushFifo(&dev->http_fifo, data, size)){
                        return YERRMSG(YAPI_IO_ERROR,"FIFO overrun");
                    }
                }
                break;
            case YSTREAM_NOTICE:
                yDispatchNotice(dev, (USB_Notify_Pkt*)data, size, 0);
                break;
            case YSTREAM_NOTICE_V2:
                yDispatchNotice(dev, (USB_Notify_Pkt*)data, size, 1);
                break;
            case YSTREAM_REPORT:
                yDispatchReportV1(dev, data, size);
                break;
            case YSTREAM_REPORT_V2:
                yDispatchReportV2(dev, data, size);
                break;
            case YSTREAM_EMPTY:
            default:
                //packet droped
                break;
        }
        //listen again
        YPROPERR(yStreamReceptionDone(dev,errmsg));
        // do not block on second atempt
        pktavail=yStreamReceived(dev,&stream, &data, &size, 0, errmsg);
        YPROPERR(pktavail);
    }
    return YAPI_SUCCESS;
}





/*****************************************************************************
  ENUMERATION RELATED FUNCTION
  ***************************************************************************/
#ifdef DEBUG_DEV_ENUM
const char *YDEV_STATUS_TXT[] =
{
    "YDEV_UNPLUGED",              // device has been plugged by the past but is no more
                                  // -> YDEV_ARRIVAL
    "YDEV_WORKING",               // device is plugged and running
                                  // -> YDEV->UNPLUGET, YDEV_ALLREADY_THERE
    "YDEV_NOTRESPONDING"          // device has not repsond to StartDevice and we will never try to speak with it
                                  // -> none
} ;
#endif

static void enuResetDStatus(void)
{

    yPrivDeviceSt *p=yContext->devs;

    while(p){
#ifdef DEBUG_DEV_ENUM_VERBOSE
        dbglog("Initial State of %s is %s\n",p->infos.serial,YDEV_STATUS_TXT[p->dStatus]);
#endif
        if( p->dStatus== YDEV_WORKING){
            p->enumAction=YENU_STOP;
        }else{
            p->enumAction=YENU_NONE;
        }
        p=p->next;
    }
}


static yPrivDeviceSt* enuFindDevSlot(yInterfaceSt *iface)
{
    yPrivDeviceSt  *p;

    for(p=yContext->devs ; p ; p=p->next ){
        if(p->infos.vendorid == iface->vendorid && p->infos.deviceid == iface->deviceid
            && strncmp(p->infos.serial,iface->serial,YOCTO_SERIAL_LEN)==0){
            return p;
        }
    }
    return NULL;
}

//allocate a  new device structure
static yPrivDeviceSt* AllocateDevice(void)
{
    yPrivDeviceSt *dev;
    dev  = (yPrivDeviceSt*) yMalloc(sizeof(yPrivDeviceSt));
    yMemset(dev,0,sizeof(yPrivDeviceSt));
    dev->http_raw_buf =  (u8*) yMalloc(HTTP_RAW_BUFF_SIZE);
    yFifoInit(&dev->http_fifo, dev->http_raw_buf, HTTP_RAW_BUFF_SIZE);
    devInitAccces(PUSH_LOCATION dev);
    return dev;
}

static void FreeDevice(yPrivDeviceSt *dev)
{
    devDeleteAccces(PUSH_LOCATION dev);
    yFree(dev->http_raw_buf);
    yFifoCleanup(&dev->http_fifo);
    yFree(dev);
}



// Start an interface
static int StartDevice(yPrivDeviceSt *dev, char *errmsg)
{
    unsigned delay = 10;
    int nb_try;
    int res = YERRMSG(YAPI_IO_ERROR, "Negotiation failed");

    for (nb_try = 0; nb_try < 4; nb_try++, delay *= 4, dbglog("retrying StartDevice (%s)\n", errmsg)) {
        u64 timeout;
        int res = yStreamSetup(dev, errmsg);
        if (YISERR(res)) {
            continue;
        }
        timeout = yapiGetTickCount() + 10000;
        do {
            res = yDispatchReceive(dev, timeout, errmsg);
            if (dev->iface.pkt_version == YPKT_VERSION_ORIGINAL_RELEASE && !dev->infos.productname[0]) {
                dev->rstatus = YRUN_AVAIL;
            }
            if (yapiGetTickCount() >= timeout) {
                yStreamShutdown(dev);
                return YERRMSG(YAPI_TIMEOUT, "Negotiation failed (device did not respond for 10 secs");
            }
        } while (res == YAPI_SUCCESS && dev->rstatus != YRUN_AVAIL);
        if (res == YAPI_SUCCESS && dev->rstatus == YRUN_AVAIL) {
            return YAPI_SUCCESS;
        }
        yStreamShutdown(dev);
    }
    return res;
}


static int StopDevice(yPrivDeviceSt *dev,char *errmsg)

{
    dev->rstatus=YRUN_STOPED;
    yStreamShutdown(dev);
    return YAPI_SUCCESS;
}

//thread safe because only modified only by yDetectDevices which is not reentrant
static void enuUpdateDStatus(void)
{
    yPrivDeviceSt *p=yContext->devs;
    char errmsg[YOCTO_ERRMSG_LEN];
    int res, updateWP;
    yStrRef lnameref, prodref;
    yUrlRef usb;
    u8 beacon;
    u16 deviceid;

    while(p){
        yStrRef serialref = yHashPutStr(p->infos.serial);
        switch(p->enumAction){
        case YENU_STOP:
            devStartEnum(p);
#ifdef DEBUG_DEV_ENUM
            dbglog("ENU:stop %s(%d)->YDEV_UNPLUGED\n",p->infos.serial,p->infos.nbinbterfaces);
#endif
            p->dStatus = YDEV_UNPLUGGED;
            if(YISERR(StopDevice(p,errmsg))){
                dbglog("Unable to stop the device %s correctly:(%s)\n",p->infos.serial,errmsg);
            }
            dbglog("Device %s unplugged\n",p->infos.serial);
            devStopEnum(p);
            wpSafeUnregister(serialref);
            break;

        case YENU_RESTART:
            devStartEnum(p);
            if(YISERR(StopDevice(p,errmsg))){
                dbglog("Unable to stop the device %s correctly:(%s)\n",p->infos.serial,errmsg);
            }
            p->dStatus = YDEV_WORKING; //we need to put the device in working to start device (safe because we alread have the mutex)
            res = StartDevice(p, errmsg);
            if(YISERR(res)){
                // we are unable to restart the device -> unplug it and follow the traditional process (white page update etc...)
#ifdef DEBUG_DEV_ENUM
                dbglog("ENU:Restart %s(%d)->YDEV_UNPLUGED (restart failed)\n",p->infos.serial,p->infos.nbinbterfaces);
#endif

                p->dStatus = YDEV_UNPLUGGED;
            }else{
#ifdef DEBUG_DEV_ENUM
                dbglog("ENU:restart %s(%d)->YDEV_WORKING(restart)\n",p->infos.serial,p->infos.nbinbterfaces);
#endif
            }
            devStopEnum(p);
            if (YISERR(res)) {
                wpSafeUnregister(serialref);
            }
            break;
        case YENU_START:
            if( p->next_startup_attempt <= yapiGetTickCount()) {
                devStartEnum(p);
                updateWP = 0;
                p->dStatus = YDEV_WORKING; //we need to put the device in working to start device (safe because we alread have the mutex)
                res = StartDevice(p, errmsg);
                if(YISERR(res)){
                    if (res !=YAPI_TIMEOUT && p->nb_startup_retry < NB_MAX_STARTUP_RETRY) {
                        dbglog("Unable to start the device %s correctly (%s). retry later\n", p->infos.serial, errmsg);
#ifdef DEBUG_DEV_ENUM
                        dbglog("ENU:start %s(%d)->YDEV_UNPLUGED\n", p->infos.serial, p->infos.nbinbterfaces);
#endif
                        p->dStatus = YDEV_UNPLUGGED;
                        p->next_startup_attempt = yapiGetTickCount() + 1000;
                        p->nb_startup_retry++;
                    } else {
#ifdef DEBUG_DEV_ENUM
                        dbglog("ENU:start %s(%d)->YDEV_NOTRESPONDING\n", p->infos.serial, p->infos.nbinbterfaces);
#endif
                        dbglog("Disable device %s (reason:%s)\n",p->infos.serial,errmsg);
                        p->dStatus = YDEV_NOTRESPONDING;
                        updateWP = 1;
                    }
                    devStopEnum(p);
                    if (updateWP) {
                        wpSafeUnregister(serialref);
                    }
                } else {
#ifdef DEBUG_DEV_ENUM
                    dbglog("ENU:start %s(%d)->YDEV_WORKING\n",p->infos.serial,p->infos.nbinbterfaces);
#endif
                    p->yhdl    = yContext->devhdlcount++;
                    dbglog("Device %s plugged\n",p->infos.serial);
                    lnameref = yHashPutStr(p->infos.logicalname);
                    prodref = yHashPutStr(p->infos.productname);
                    beacon = p->infos.beacon;
                    deviceid = p->infos.deviceid;
                    usb = yHashUrlUSB(serialref);
                    devStopEnum(p);
                    wpSafeRegister(NULL, MAX_YDX_PER_HUB, serialref, lnameref, prodref, deviceid, usb, beacon);
                }
            } else {
#ifdef DEBUG_DEV_ENUM_VERBOSE
                dbglog("enum : %s (%d ifaces) waiting for next attempt\n",p->infos.serial,p->infos.nbinbterfaces);
#endif
            }
            break;
        case YENU_NONE:
            break;
        }
        p=p->next;
    }
}

void yUSBReleaseAllDevices(void)
{
    yEnterCriticalSection(&yContext->enum_cs);
    enuResetDStatus();
    enuUpdateDStatus();
    yLeaveCriticalSection(&yContext->enum_cs);

}

YRETCODE yUSBUpdateDeviceList(char *errmsg)
{
    int             nbifaces=0;
    yInterfaceSt *iface;
    int             j;
    yInterfaceSt    *runifaces=NULL;

    YPROPERR(yyyUSBGetInterfaces(&runifaces,&nbifaces,errmsg));

    yEnterCriticalSection(&yContext->enum_cs);
    enuResetDStatus();

    for (j = 0, iface = runifaces; j < nbifaces; j++, iface++){
        yPrivDeviceSt *dev;
        if (iface->deviceid <= YOCTO_DEVID_BOOTLOADER)
            continue;

        dev =enuFindDevSlot(iface);
        if(dev){
            //device already allocated
            if(dev->dStatus == YDEV_WORKING ){
                if(!yyyOShdlCompare(dev, iface)){
                    ENUMLOG("%s was already there but OS handles are no more valid\n",dev->infos.serial);
                    dev->enumAction =  YENU_RESTART;
                } else if (dev->rstatus==YRUN_ERROR){
                    ENUMLOG("%s was already there but need to be reset due to runtime error\n",dev->infos.serial);
                    dev->enumAction =  YENU_RESTART;
                } else {
                    // device is working correctly
                    dev->enumAction = YENU_NONE;
                }
            }else if(dev->dStatus == YDEV_UNPLUGGED) {
                ENUMLOG("%s replug of a previously detected device\n",dev->infos.serial);
                dev->enumAction =  YENU_START;
                // to be safe we update infos with fresh data form last enumeration
                dev->infos.nbinbterfaces = 1;
                memcpy(&dev->iface, iface, sizeof(yInterfaceSt));
            } else if(dev->dStatus == YDEV_NOTRESPONDING && !yyyOShdlCompare(dev, iface) ){
                ENUMLOG("%s replug of a previously detected device that was not responding\n",dev->infos.serial);
                dev->enumAction =  YENU_START;
                // to be safe we update infos with fresh data form last enumeration
                dev->infos.nbinbterfaces = 1;
                memcpy(&dev->iface, iface, sizeof(yInterfaceSt));
            }
        }else{
            ENUMLOG("%s newly plugged device \n",iface->serial);
            //ALLOCATE A  NEW DEVICE STUCTURE
            dev = AllocateDevice();
            dev->enumAction =  YENU_START;
            //mark device a stopped
            dev->rstatus = YRUN_STOPED;
            dev->infos.vendorid = iface->vendorid;
            dev->infos.deviceid = iface->deviceid;
            YSTRNCPY(dev->infos.serial, YOCTO_SERIAL_LEN, iface->serial, YOCTO_SERIAL_LEN - 1);
            dev->infos.nbinbterfaces = 1;
            memcpy(&dev->iface, iface, sizeof(yInterfaceSt));
            dev->next = yContext->devs;
            yContext->devs=dev;
        }
    }
    enuUpdateDStatus();
    yLeaveCriticalSection(&yContext->enum_cs);

    // free all tmp ifaces
    if(runifaces){
        yFree(runifaces);
    }
    return YAPI_SUCCESS;
}




/*****************************************************************************
  GENERIC DEVICE LIST FUNCTION
  ***************************************************************************/

// return the YHANDLE from a matching string (serial or name)
yPrivDeviceSt *findDev(const char *str,u32 flags)
{
    yPrivDeviceSt *p;

    if(flags& FIND_FROM_SERIAL){
        for( p=yContext->devs ; p ; p=p->next){
            if(p->dStatus == YDEV_UNPLUGGED){
                continue;
            }
            if(strncmp(str,p->infos.serial,YOCTO_SERIAL_LEN)==0){
                return p;
            }
        }
    }
    if (flags & FIND_FROM_NAME) {
        for( p=yContext->devs ; p ; p=p->next){
            if(p->dStatus == YDEV_UNPLUGGED){
                continue;
            }
            if(strncmp(str,p->infos.logicalname,YOCTO_LOGICAL_LEN)==0){
                return p;
            }
        }
    }

    return NULL;
}


// return the YHANDLE from a matching string (serial or name)
YUSBDEV findDevHdlFromStr(const char *str)
{
    yPrivDeviceSt *p = findDev(str,FIND_FROM_ANY);

    if(p != NULL) {
        return p->yhdl;
    }
    return INVALID_YHANDLE;
}


yPrivDeviceSt *findDevFromIOHdl(YIOHDL_internal *iohdl)
{
    yPrivDeviceSt *p;
    if(iohdl->type!=YIO_USB)
        return NULL;
    for( p=yContext->devs ; p ; p=p->next){
        if(p->pendingIO.hdl ==iohdl->hdl)
            return p;
    }
    return NULL;
}

// find a device from his YHANDLE
static yPrivDeviceSt *findDevFromDevHdl(YUSBDEV hdl)
{
    yPrivDeviceSt *p;

    for( p=yContext->devs ; p ; p=p->next){
        if(p->yhdl ==hdl)
            return p;
    }
    return NULL;
}

void devHdlInfo(YUSBDEV hdl,yDeviceSt *infos)
{
    yPrivDeviceSt *p;

    p=findDevFromDevHdl(hdl);
    if(p!=NULL){
        *infos = p->infos;
    } else {
        memset(infos, 0, sizeof(yDeviceSt));
    }
}



/*****************************************************************************
  USB REQUEST FUNCTIONS
  ***************************************************************************/


void  dumpYPerfEntry(yPerfMon *entry,const char *name)
{
    dbglog("%s count %lld(%lld) totaltime=%lld (avg =%lld)\n",name,entry->count,entry->leave,entry->totaltime,(entry->count>0?entry->totaltime/entry->count:0));
}



//#define PERF_YHUB_FUNCTIONS
#ifdef PERF_YHUB_FUNCTIONS


typedef struct {
    yPerfMon  yUsbIdle;
    yPerfMon  yUsbTrafficPending;
    yPerfMon  yUsbOpen;
    yPerfMon  yUsbSetIOAsync;
    yPerfMon  yUsbWrite;
    yPerfMon  yUsbReadNonBlock;
    yPerfMon  yUsbReadBlock;
    yPerfMon  yUsbEOF;
    yPerfMon  yUsbClose;
} yUsbPerfMonSt;

yUsbPerfMonSt yUsbPerf;


#define YPERF_ENTER(NAME) {yUsbPerf.NAME.count++;yUsbPerf.NAME.tmp=yapiGetTickCount();}
#define YPERF_LEAVE(NAME) {yUsbPerf.NAME.leave++;yUsbPerf.NAME.totaltime += yapiGetTickCount()- yUsbPerf.NAME.tmp;}



void dumpYUSBPerf(void)
{
    dumpYPerfEntry(&yUsbPerf.yUsbIdle,"yUsbIdle");
    dumpYPerfEntry(&yUsbPerf.yUsbTrafficPending,"yUsbTrafficPending");
    dumpYPerfEntry(&yUsbPerf.yUsbOpen,"yUsbOpen");
    dumpYPerfEntry(&yUsbPerf.yUsbSetIOAsync,"yUsbSetIOAsync");
    dumpYPerfEntry(&yUsbPerf.yUsbWrite,"yUsbWrite");
    dumpYPerfEntry(&yUsbPerf.yUsbReadNonBlock,"yUsbReadNonBlock");
    dumpYPerfEntry(&yUsbPerf.yUsbReadBlock,"yUsbReadBlock");
    dumpYPerfEntry(&yUsbPerf.yUsbEOF,"yUsbEOF");
    dumpYPerfEntry(&yUsbPerf.yUsbClose,"yUsbClose");
}
#else
#define YPERF_ENTER(NAME)
#define YPERF_LEAVE(NAME)
#endif

int yUsbInit(yContextSt *ctx,char *errmsg)
{
#ifdef PERF_YHUB_FUNCTIONS
    memset(&yUsbPerf,0,sizeof(yUsbPerfMonSt));
#endif
    return yyyUSB_init(ctx,errmsg);
}


int yUsbFree(yContextSt *ctx,char *errmsg)
{

    yPrivDeviceSt   *p,*next;

#ifdef PERF_YHUB_FUNCTIONS
    dumpYUSBPerf();
#endif
    p = ctx->devs;
    ctx->devs = NULL;
    while(p) {
        if(p->dStatus == YDEV_WORKING){
            int csTaken = yTryEnterCriticalSection(&p->acces_state);
            yStrRef serialref = yHashTestStr(p->infos.serial);
            p->dStatus = YDEV_UNPLUGGED;
            StopDevice(p,NULL);
            wpSafeUnregister(serialref);
            if(csTaken)
                yLeaveCriticalSection(&p->acces_state);
        }
        if(p->replybuf) {
            yFree(p->replybuf);
            p->replybuf = NULL;
        }
        next = p->next;
        FreeDevice(p);
        p = next;
    }
    return yyyUSB_stop(yContext,errmsg);
}

int yUsbIdle(void)
{
    yPrivDeviceSt   *p;
    int             res;
    char            errmsg[YOCTO_ERRMSG_LEN];

    YPERF_ENTER(yUsbIdle);
    for( p=yContext->devs ; p ; p=p->next){

        if(p->dStatus != YDEV_WORKING){
            continue;
        }

        res = devStartIdle(PUSH_LOCATION p,errmsg);
        if (res == YAPI_SUCCESS) {
            u32 currUtcTime;
            if(YISERR(yDispatchReceive(p,0,errmsg))){
                dbglog("yPacketDispatchReceive error:%s\n",errmsg);
                devReportErrorFromIdle(PUSH_LOCATION p,errmsg);
                continue;
            }
            currUtcTime = (u32)time(NULL);
            if(currUtcTime > (u32)0x51f151f1 && // timestamp appears to be valid
               (!p->lastUtcUpdate || currUtcTime < p->lastUtcUpdate || currUtcTime >= p->lastUtcUpdate+60u)) {
                u8  *pktdata;
                u8  maxpktlen;
                // send updated UTC timestamp to keep datalogger on time
                if(yStreamGetTxBuff(p,&pktdata, &maxpktlen) && maxpktlen >= 5){
                    p->lastUtcUpdate = currUtcTime;
                    pktdata[0] = USB_META_UTCTIME;
                    pktdata[1] = currUtcTime & 0xff;
                    pktdata[2] = (currUtcTime>>8) & 0xff;
                    pktdata[3] = (currUtcTime>>16) & 0xff;
                    pktdata[4] = (currUtcTime>>24) & 0xff;
                    if(YISERR(yStreamTransmit(p,YSTREAM_META,5,errmsg))){
                        dbglog("Unable to send UTC timestamp\n");
                    } else if(YISERR(yStreamFlush(p,errmsg))) {
                        dbglog("Unable to flush UTC timestamp\n");
                    }
                }
            }
            devStopIdle(PUSH_LOCATION p);
            yapiPullDeviceLog(p->infos.serial);
        } else if(res == YAPI_DEVICE_BUSY){
            if (p->httpstate != YHTTP_CLOSED && p->pendingIO.callback) {
                // if we have an async IO on this device
                // simulate read from users
                if (!YISERR(devCheckAsyncIO(PUSH_LOCATION p,errmsg))) {
                    int sendClose=0;
                    if(YISERR(yDispatchReceive(p,0,errmsg))){
                        dbglog("yPacketDispatchReceive error:%s\n",errmsg);
                        devReportError(PUSH_LOCATION p,errmsg);
                        continue;
                    }
                    if(p->httpstate == YHTTP_CLOSE_BY_DEV) {
                        sendClose=1;
                    }else if(p->pendingIO.timeout<yapiGetTickCount()){
                        dbglog("Last async request did not complete (%X:%d)\n",p->pendingIO.hdl,p->httpstate);
                        sendClose=1;
                    }
                    if (sendClose) {
                        u8  *pktdata;
                        u8  maxpktlen;
                        // send connection close
                        if(yStreamGetTxBuff(p,&pktdata, &maxpktlen)){
                            u8 * ptr;
                            u16 len;
                            if(YISERR(yStreamTransmit(p,YSTREAM_TCP_CLOSE,0,errmsg))){
                                dbglog("Unable to send async connection close\n");
                            } else if(YISERR(yStreamFlush(p,errmsg))) {
                                dbglog("Unable to flush async connection close\n");
                            }
                            // since we empty the fifo at each request we can use yPeekContinuousFifo
                            len = yPeekContinuousFifo(&p->http_fifo, &ptr, 0);
                            p->pendingIO.callback(p->pendingIO.context, ptr, len, YAPI_SUCCESS, NULL);
                            yFifoEmpty(&p->http_fifo);
                            p->httpstate = YHTTP_CLOSED;
                        }
                    }
                    if(p->httpstate == YHTTP_CLOSED) {
                        if (YISERR(res =devStopIO(PUSH_LOCATION p,errmsg))) {
                            dbglog("Idle : devStopIO err %s : %X:%s\n",p->infos.serial,res,errmsg);
                        }
                    } else {
                        devPauseIO(PUSH_LOCATION p,NULL);
                    }
                }
            }
        }
    }
    YPERF_LEAVE(yUsbIdle);
    return YAPI_SUCCESS;
}

int yUsbTrafficPending(void)
{
    yPrivDeviceSt   *p;
    YPERF_ENTER(yUsbTrafficPending);
    for( p=yContext->devs ; p ; p=p->next){
        if(p->dStatus != YDEV_WORKING){
            continue;
        }
        if(p->httpstate != YHTTP_CLOSED && p->pendingIO.callback) {
            YPERF_LEAVE(yUsbTrafficPending);
            return 1;
        }
    }
    YPERF_LEAVE(yUsbTrafficPending);
    return 0;
}


int yUsbOpenDevDescr(YIOHDL_internal *ioghdl, yStrRef devdescr, char *errmsg)
{
    char    serialBuf[YOCTO_SERIAL_LEN];
    int     res;

    YPERF_ENTER(yUsbOpenDevDescr);
    yHashGetStr(devdescr, serialBuf, YOCTO_SERIAL_LEN);
    res = yUsbOpen(ioghdl, serialBuf, errmsg);
    YPERF_LEAVE(yUsbOpenDevDescr);

    return res;
}

int yUsbOpen(YIOHDL_internal *ioghdl, const char *device, char *errmsg)
{
    int           res;
    yPrivDeviceSt *p;

    YPERF_ENTER(yUsbOpen);
    p=findDev(device,FIND_FROM_ANY);
    if(p==NULL){
        YPERF_LEAVE(yUsbOpen);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    memset(ioghdl, 0, sizeof(YIOHDL_internal));
    res = devStartIO(PUSH_LOCATION p,errmsg);
    if(YISERR(res)){
        YPERF_LEAVE(yUsbOpen);
        return res;
    }
    //process some packet
    if(YISERR(res=yDispatchReceive(p,0,errmsg))){
        devReportError(PUSH_LOCATION p,errmsg);
        YPERF_LEAVE(yUsbOpen);
        return res;
    }
    p->httpstate = YHTTP_OPENED;
    ioghdl->type = YIO_USB;
    memset(&p->pendingIO,0,sizeof(USB_HDL));
    yEnterCriticalSection(&yContext->io_cs);
    p->pendingIO.hdl = ioghdl->hdl = ++(yContext->io_counter);
    yLeaveCriticalSection(&yContext->io_cs);
    p->pendingIO.timeout = YIO_DEFAULT_USB_TIMEOUT+yapiGetTickCount();
    res = devPauseIO(PUSH_LOCATION p,errmsg);
    YPERF_LEAVE(yUsbOpen);
    return res;
}

int yUsbSetIOAsync(YIOHDL_internal *ioghdl, yapiRequestAsyncCallback callback, void *context, char *errmsg)
{
    int res;
    yPrivDeviceSt *p;

    YPERF_ENTER(yUsbSetIOAsync);
    p = findDevFromIOHdl(ioghdl);
    if(p == NULL){
        YPERF_LEAVE(yUsbSetIOAsync);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    res = devCheckIO(PUSH_LOCATION p,ioghdl,errmsg);
    if(YISERR(res)){
        YPERF_LEAVE(yUsbSetIOAsync);
        return res;
    }
    p->pendingIO.callback = callback;
    p->pendingIO.context = context;
    res = devPauseIO(PUSH_LOCATION p,errmsg);
    YPERF_LEAVE(yUsbSetIOAsync);

    return res;
}



int  yUsbWrite(YIOHDL_internal *ioghdl, const char *buffer, int writelen,char *errmsg)
{
    yPrivDeviceSt *p;
    int totalsend=0;
    u8  *pktdata;
    u8  maxpktlen;
    int res;

    YPERF_ENTER(yUsbWrite);
    p=findDevFromIOHdl(ioghdl);
    if(p==NULL){
        YPERF_LEAVE(yUsbWrite);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    YPROPERR(devCheckIO(PUSH_LOCATION p,ioghdl,errmsg));
    if(YISERR(res=yDispatchReceive(p,0,errmsg))){
        devReportError(PUSH_LOCATION p,errmsg);
        YPERF_LEAVE(yUsbWrite);
        return res;
    }

    if(p->httpstate != YHTTP_OPENED && p->httpstate != YHTTP_INREQUEST) {
        devPauseIO(PUSH_LOCATION p,NULL);
        YPERF_LEAVE(yUsbWrite);
        return YERRMSG(YAPI_IO_ERROR,"Connection closed");
    }
    p->httpstate = YHTTP_INREQUEST;
    while (writelen){
        while(writelen && yStreamGetTxBuff(p,&pktdata, &maxpktlen)==1) {
            u8 pktlen =(maxpktlen < writelen ? maxpktlen: writelen);
            memcpy(pktdata,buffer,pktlen);
            if(YISERR(res=yStreamTransmit(p,YSTREAM_TCP,pktlen,errmsg))) {
                devReportError(PUSH_LOCATION p,errmsg);
                YPERF_LEAVE(yUsbWrite);
                return res;
            }
            buffer    += pktlen;
            writelen  -= pktlen;
            totalsend += pktlen;
        }
        if(YISERR(res=yStreamFlush(p,errmsg))) {
            devReportError(PUSH_LOCATION p,errmsg);
            YPERF_LEAVE(yUsbWrite);
            return res;
        }
    }

    res = devPauseIO(PUSH_LOCATION p,errmsg);
    if(res==YAPI_SUCCESS)
        res = totalsend;
    YPERF_LEAVE(yUsbWrite);
    return res;
}



int  yUsbReadNonBlock(YIOHDL_internal *ioghdl, char *buffer, int len,char *errmsg)
{
    yPrivDeviceSt *p;
    u16 readed;
    int res;

    YPERF_ENTER(yUsbReadNonBlock);

    p=findDevFromIOHdl(ioghdl);
    if(p==NULL){
        YPERF_LEAVE(yUsbReadNonBlock);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    res =devCheckIO(PUSH_LOCATION p,ioghdl,errmsg);
    if(YISERR(res)){
        YPERF_LEAVE(yUsbReadNonBlock);
        return res ;
    }
    if(p->pendingIO.callback){
        YPROPERR(devPauseIO(PUSH_LOCATION p,errmsg));
        YPERF_LEAVE(yUsbReadNonBlock);
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Operation not supported on async IO");
    }

    if(YISERR(res=yDispatchReceive(p, 0, errmsg))){
        devReportError(PUSH_LOCATION p,errmsg);
        YPERF_LEAVE(yUsbReadNonBlock);
        return res;
    }
    if(len > HTTP_RAW_BUFF_SIZE){
        len = HTTP_RAW_BUFF_SIZE;
    }
    //get all available data
    readed=yPopFifo(&p->http_fifo,(u8*)buffer,(u16)len);
    YPROPERR(devPauseIO(PUSH_LOCATION p,errmsg));
    YPERF_LEAVE(yUsbReadNonBlock);
    return readed;
}





int  yUsbReadBlock(YIOHDL_internal *ioghdl, char *buffer, int len,u64 blockUntil,char *errmsg)
{
    yPrivDeviceSt *p;
    u16 readed, avail;
    int res;

    YPERF_ENTER(yUsbReadBlock);

    p=findDevFromIOHdl(ioghdl);
    if(p==NULL){
        YPERF_LEAVE(yUsbReadBlock);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    res =devCheckIO(PUSH_LOCATION p,ioghdl,errmsg);
    if(YISERR(res)){
        YPERF_LEAVE(yUsbReadBlock);
        return res ;
    }
    if(p->pendingIO.callback){
        YPROPERR(devPauseIO(PUSH_LOCATION p,errmsg));
        YPERF_LEAVE(yUsbReadBlock);
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Operation not supported on async IO");
    }

    avail = yFifoGetUsed(&p->http_fifo);
    if(YISERR(res=yDispatchReceive(p,(avail==0 ? blockUntil : 0) ,errmsg))){
        devReportError(PUSH_LOCATION p,errmsg);
        YPERF_LEAVE(yUsbReadBlock);
        return res;
    }
    if(len > HTTP_RAW_BUFF_SIZE){
        len = HTTP_RAW_BUFF_SIZE;
    }
    //get all available data
    readed=yPopFifo(&p->http_fifo,(u8*)buffer,(u16)len);
    YPROPERR(devPauseIO(PUSH_LOCATION p,errmsg));
    YPERF_LEAVE(yUsbReadBlock);
    return readed;
}




int  yUsbEOF(YIOHDL_internal *ioghdl,char *errmsg)
{
    yPrivDeviceSt *p;
    int res;

    YPERF_ENTER(yUsbEOF);

    p=findDevFromIOHdl(ioghdl);
    if(p==NULL){
        YPERF_LEAVE(yUsbEOF);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    res =devCheckIO(PUSH_LOCATION p,ioghdl,errmsg);
    if(YISERR(res)){
        YPERF_LEAVE(yUsbEOF);
        return res;
    }
    if(p->pendingIO.callback){
        YPROPERR(devPauseIO(PUSH_LOCATION p,errmsg));
        YPERF_LEAVE(yUsbEOF);
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Operation not supported on async IO");
    }
    if(YISERR(res=yDispatchReceive(p,0,errmsg))){
        devReportError(PUSH_LOCATION p,errmsg);
        YPERF_LEAVE(yUsbEOF);
        return res;
    }
    res=0;
    if(yFifoGetUsed(&p->http_fifo)==0 && p->httpstate == YHTTP_CLOSE_BY_DEV){
        //dbglog("* yUsbEOF for %d is TRUE\n",p->iohdl);
        res =1;
        p->pendingIO.flags |= YIO_REMOTE_CLOSE;
    }
    devPauseIO(PUSH_LOCATION p,NULL);
    YPERF_LEAVE(yUsbEOF);
    return res;
}



int  yUsbClose(YIOHDL_internal *ioghdl,char *errmsg)
{
    yPrivDeviceSt *p;
    u8  *pktdata;
    u8  maxpktlen;
    u16 deviceDead = 0;
    int res;

    YPERF_ENTER(yUsbClose);

    p = findDevFromIOHdl(ioghdl);
    if (p == NULL) {
        YPERF_LEAVE(yUsbClose);
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }
    //dbglog("* yUsbClose for %d, httpstate=%d\n",p->iohdl,p->httpstate);
    if(p->httpstate == YHTTP_CLOSED || p->httpstate == YHTTP_CLOSE_BY_API) {
        dbglog("yUsb double-close");
        YPERF_LEAVE(yUsbClose);
        return YAPI_SUCCESS;
    }
    res = devCheckIO(PUSH_LOCATION p,ioghdl,errmsg);
    if(YISERR(res)) {
        YPERF_LEAVE(yUsbClose);
        return res;
    }
    if(p->pendingIO.callback){
        YPROPERR(devPauseIO(PUSH_LOCATION p,errmsg));
        YPERF_LEAVE(yUsbClose);
        return YERRMSG(YAPI_INVALID_ARGUMENT,"Operation not supported on async IO");
    }

    // send connection close
    if(!yStreamGetTxBuff(p,&pktdata, &maxpktlen)) {
        if(YISERR(yStreamFlush(p,errmsg))) {
            dbglog("Unable to flush pending data");
            deviceDead = 1;
        }
        yStreamGetTxBuff(p,&pktdata, &maxpktlen);
    }
    if (!deviceDead && p->httpstate >= YHTTP_INREQUEST) {
        if(YISERR(yStreamTransmit(p,YSTREAM_TCP_CLOSE,0,errmsg))) {
            dbglog("Unable to send connection close");
            deviceDead = 1;
        } else if(YISERR(yStreamFlush(p,errmsg))) {
            dbglog("Unable to flush connection close");
            deviceDead = 1;
        }
    }
    if (p->httpstate == YHTTP_OPENED || p->httpstate == YHTTP_CLOSE_BY_DEV || deviceDead) {
        p->httpstate = YHTTP_CLOSED;
    } else {
        //wait for the device close packet
        u64 timeout = yapiGetTickCount() + 100;
        p->httpstate = YHTTP_CLOSE_BY_API;
        while (!YISERR(yDispatchReceive(p, 5, errmsg))) {
            if(p->httpstate == YHTTP_CLOSED) {
                // received close from device
                break;
            }
            if(timeout<yapiGetTickCount()) {
                dbglog("yUSBClose without device ack\n");
                break;
            }
        }
    }
    yFifoEmpty(&p->http_fifo);
    memset(&p->pendingIO, 0, sizeof(USB_HDL));
    ioghdl->type=YIO_INVALID;
    res =devStopIO(PUSH_LOCATION p,errmsg);
    yapiPullDeviceLog(p->infos.serial);
    YPERF_LEAVE(yUsbClose);
    return res;
}
