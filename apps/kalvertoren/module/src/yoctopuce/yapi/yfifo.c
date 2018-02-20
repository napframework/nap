/*********************************************************************
 *
 * $Id: yfifo.c 23630 2016-03-29 21:05:54Z mvuilleu $
 *
 * Implementation of a generic fifo queue
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
#define __FILE_ID__  "yfifo"

#include "ydef.h"

#if defined(MICROCHIP_API)
#include "api.h"
#else
#include "yfifo.h"
#include "yproto.h"
#endif

void yFifoInitEx(
#ifdef DEBUG_FIFO
    const char* fileid, int line,
#endif
    yFifoBuf *buf, u8 *buffer, u16 bufflen)
{
    memset(buf,0,sizeof(yFifoBuf));
    buf->buff = buffer;
    buf->buffsize = bufflen;
    buf->head = buf->tail = buffer;
#ifdef DEBUG_FIFO
    buf->line   = line;
    buf->fileid = fileid;
#endif
#ifdef YFIFO_USE_MUTEX
    yInitializeCriticalSection(&(buf->cs));
#endif
}

#ifndef MICROCHIP_API
void yFifoCleanup(yFifoBuf *buf)
{
#ifdef YFIFO_USE_MUTEX
    yDeleteCriticalSection(&(buf->cs));
#endif
    memset(buf,0,sizeof(yFifoBuf));
}
#endif

#ifdef YFIFO_USE_MUTEX

void yFifoEnterCS(yFifoBuf *buf)
{
    yEnterCriticalSection(&(buf->cs));
}

void yFifoLeaveCS(yFifoBuf *buf)
{
    yLeaveCriticalSection(&(buf->cs));
}
#endif

void yFifoEmptyEx(yFifoBuf *buf)
{
    buf->datasize = 0;
    buf->head = buf->tail =  buf->buff;
}

#ifdef YFIFO_USE_MUTEX

void yFifoEmpty(yFifoBuf *buf)
{
    yFifoEnterCS(buf);
    yFifoEmptyEx(buf);
    yFifoLeaveCS(buf);
}
#endif

u16 yPushFifoEx(yFifoBuf *buf, const u8 *data, u16 datalen)
{
    u16 freespace = buf->buffsize - buf->datasize;

    if (datalen > freespace) {
        // not enough space available in
        // buffer to handle correctly this packet
        // (we do not handle partial packet since usb packet are
        // max 64byte long)
        return 0;
    }
    //append data to our fifo buffer
    if (buf->tail + datalen <= YFIFOEND(buf)) {
        memcpy(buf->tail, data, datalen);
        buf->tail += datalen;
        if (buf->tail == YFIFOEND(buf))
            buf->tail = buf->buff;
    } else {
        u16 cplen = (u16)(YFIFOEND(buf) - buf->tail);
        memcpy(buf->tail, data, cplen);
        memcpy(buf->buff, data + cplen, datalen - cplen);
        buf->tail = buf->buff + (datalen - cplen);
    }
    // This must remain very last, so that datasize always remain
    // the number of bytes really available in buffer (may be polled
    // from within interrupt handlers)
    buf->datasize += datalen;
#ifdef DEBUG_FIFO
    buf->totalPushed += datalen;
#endif
    return datalen;
}

#ifdef YFIFO_USE_MUTEX

u16 yPushFifo(yFifoBuf *buf, const u8 *data, u16 datalen)
{
    u16 res;
    yFifoEnterCS(buf);
    res = yPushFifoEx(buf,data,datalen);
    yFifoLeaveCS(buf);
    return res;
}
#endif

u16 yPopFifoEx(yFifoBuf *buf, u8 *data, u16 datalen)
{
    if (datalen > buf->datasize)
        datalen = buf->datasize;
    if (buf->head + datalen > YFIFOEND(buf)) {
        //need to handle wrap
        u16 firstpart = (u16)(YFIFOEND(buf) - buf->head);
        if (data) {
            memcpy(data, buf->head, firstpart);
            memcpy(data + firstpart, buf->buff, datalen - firstpart);
        }

        buf->head = buf->buff + (datalen - firstpart);
    } else {
        if (data) {
            memcpy(data, buf->head, datalen);
        }
        buf->head += datalen;
        if (buf->head == YFIFOEND(buf))
            buf->head -= buf->buffsize;
    }
    // This must remain very last, so that buffsize-datasize always
    // remain the number of bytes really free in buffer (may be polled
    // from within interrupt handlers)
    buf->datasize -= datalen;
#ifdef DEBUG_FIFO
    buf->totalPopded += datalen;
#endif
    return datalen;
}


#ifdef YFIFO_USE_MUTEX

u16 yPopFifo(yFifoBuf *buf, u8 *data, u16 datalen)
{
    u16 res;
    yFifoEnterCS(buf);
    res = yPopFifoEx(buf,data,datalen);
    yFifoLeaveCS(buf);
    return res;
}
#endif


static u16 yForceFifoEx(yFifoBuf *buf, const u8 *data, u16 datalen)
{
    u16 freespace;
    freespace = buf->buffsize - buf->datasize;

    if(datalen > buf->buffsize) {
        return 0;
    }

    if (datalen > freespace) {
        // not enough space, drop oldest data
        yPopFifoEx(buf, NULL, datalen - freespace);
    }
    return yPushFifoEx(buf, data, datalen);
}

u16 yForceFifo(yFifoBuf *buf, const u8 *data, u16 datalen, u32 *absCounter)
{
    u16 res;

#ifndef MICROCHIP_API
    yFifoEnterCS(buf);
#endif

    res = yForceFifoEx(buf,data,datalen);
    *absCounter += res;

#ifndef MICROCHIP_API
    yFifoLeaveCS(buf);
#endif

    return res;
}

u16 yPeekFifoEx(yFifoBuf *buf, u8 *data, u16 datalen, u16 startofs)
{
    u8 *ptr ;
    if(startofs > buf->datasize){
        return 0;
    }

    if (datalen + startofs > buf->datasize)
        datalen = buf->datasize - startofs;

    ptr=buf->head+startofs;
    if(ptr >= YFIFOEND(buf)){
        ptr -= buf->buffsize;
    }
    if (ptr + datalen > YFIFOEND(buf)) {
        //need to handle wrap
        u16 firstpart = (u16)(YFIFOEND(buf) - ptr);
        if (data) {
            memcpy(data, ptr, firstpart);
            memcpy(data + firstpart, buf->buff, datalen - firstpart);
        }
    } else {
        if (data) {
            memcpy(data, ptr, datalen);
        }
    }
    return datalen;
}

#ifdef YFIFO_USE_MUTEX

u16 yPeekFifo(yFifoBuf *buf, u8 *data, u16 datalen, u16 startofs)
{
    u16 res;
    yFifoEnterCS(buf);
    res = yPeekFifoEx(buf,data,datalen,startofs);
    yFifoLeaveCS(buf);
    return res;
}
#endif


u16 yPeekContinuousFifoEx(yFifoBuf *buf, u8 **ptr, u16 startofs)
{
    u8 *lptr;

    if(startofs >= buf->datasize) {
        return 0;
    }

    lptr = buf->head + startofs;
    if(lptr >= YFIFOEND(buf)) {
        // wrap
        if(ptr) {
            *ptr = lptr - buf->buffsize;
        }
        return buf->datasize - startofs;
    } else {
        // no wrap
        u16 toend = (u16)(YFIFOEND(buf) - lptr);
        if(ptr) {
            *ptr = lptr;
        }
        return (toend < buf->datasize ? toend : buf->datasize);
    }

}


#ifdef YFIFO_USE_MUTEX

u16 yPeekContinuousFifo(yFifoBuf *buf, u8 **ptr, u16 startofs)
{
    u16 res;
    yFifoEnterCS(buf);
    res = yPeekContinuousFifoEx(buf,ptr,startofs);
    yFifoLeaveCS(buf);
    return res;
}
#endif


u16 ySeekFifoEx(yFifoBuf *buf, const u8* pattern, u16 patlen,  u16 startofs, u16 searchlen, u8 bTextCompare)
{
    u8 *ptr;
    u16 patidx;
    u16 firstmatch = 0xffff;

    // pattern bigger than our buffer size -> not found
    if (startofs + patlen > buf->datasize) {
        return 0xffff;
    }
    // ajust searchlen and ptr to our buffer
    // size and position
    if (searchlen == 0 || searchlen > buf->datasize - startofs)
        searchlen = buf->datasize - startofs;
    ptr = buf->head + startofs;
    if (ptr >= YFIFOEND(buf))
        ptr -= buf->buffsize;

    patidx = 0;
    while (searchlen > 0 && patidx < patlen) {
        u16 bletter = *ptr;
        u16 pletter = pattern[patidx];

        if (bTextCompare && pletter >= 'A' && bletter >= 'A' && pletter <= 'z' && bletter <= 'z') {
            pletter &= ~32;
            bletter &= ~32;
        }
        if (pletter == bletter) {
            if(patidx == 0) {
                firstmatch = startofs;
            }
            patidx++;
        } else if(patidx > 0) {
            // rescan this character as first pattern character
            patidx = 0;
            continue;
        }
        startofs++;
        searchlen--;
        ptr++;
        if (ptr >= YFIFOEND(buf))
            ptr -= buf->buffsize;

    }
    if (patidx == patlen) {
        return firstmatch;
    }
    return 0xffff;
}


#ifdef YFIFO_USE_MUTEX

u16 ySeekFifo(yFifoBuf *buf, const u8* pattern, u16 patlen,  u16 startofs, u16 searchlen, u8 bTextCompare)
{
    u16 res;

    yFifoEnterCS(buf);
    res = ySeekFifoEx(buf,pattern,patlen,startofs,searchlen,bTextCompare);
    yFifoLeaveCS(buf);
    return res;
}
#endif

u16 yFifoGetUsedEx(yFifoBuf *buf)
{
    return buf->datasize;
}

#ifdef YFIFO_USE_MUTEX

u16 yFifoGetUsed(yFifoBuf *buf)
{
    u16 res;
    yFifoEnterCS(buf);
    res = yFifoGetUsedEx(buf);
    yFifoLeaveCS(buf);
    return res;
}
#endif

u16 yFifoGetFreeEx(yFifoBuf *buf)
{
    return buf->buffsize-buf->datasize;
}

#ifdef YFIFO_USE_MUTEX
u16 yFifoGetFree(yFifoBuf *buf)
{
    u16 res;
    yFifoEnterCS(buf);
    res = yFifoGetFreeEx(buf);
    yFifoLeaveCS(buf);
    return res;
}

#endif

#ifndef REDUCE_COMMON_CODE
void yxtoa(u32 x, char *buf, u16 len)
{
    buf[len] = 0;
    while(len > 0) {
        unsigned b = x & 0xf;
        buf[--len] = (b>9u) ? b+'a'-10 : b+'0';
        x >>= 4;
    }
}
#endif

#if defined(USE_TYPED_NOTIFICATIONS) || !defined(MICROCHIP_API)

// Decode a standard (V1) or typed notification (V2), possibly not null terminated,
// to its text representation (always null terminated)
//
void decodePubVal(Notification_funydx funInfo, const char *funcval, char *buffer)
{
    const unsigned char *p = (const unsigned char *)funcval;
    u16     funcValType;
    s32     numVal;
    float   floatVal;
    int     i;

    if(funInfo.v2.typeV2 == NOTIFY_V2_6RAWBYTES || funInfo.v2.typeV2 == NOTIFY_V2_TYPEDDATA) {
        if(funInfo.v2.typeV2 == NOTIFY_V2_6RAWBYTES) {
            funcValType = PUBVAL_6RAWBYTES;
        } else {
            funcValType = *p++;
        }
        switch(funcValType) {
            case PUBVAL_LEGACY:
                // fallback to legacy handling, just in case
                break;
            case PUBVAL_1RAWBYTE:
            case PUBVAL_2RAWBYTES:
            case PUBVAL_3RAWBYTES:
            case PUBVAL_4RAWBYTES:
            case PUBVAL_5RAWBYTES:
            case PUBVAL_6RAWBYTES:
                // 1..6 hex bytes
                for(i = 0; i < funcValType; i++) {
                    unsigned c = *p++;
                    unsigned b = c >> 4;
                    buffer[2*i]   = (b>9u) ? b+'a'-10 : b+'0';
                    b = c & 0xf;
                    buffer[2*i+1] = (b>9u) ? b+'a'-10 : b+'0';
                }
                buffer[2*i] = 0;
                return;
            case PUBVAL_C_LONG:
            case PUBVAL_YOCTO_FLOAT_E3:
                // 32bit integer in little endian format or Yoctopuce 10-3 format
                numVal = *p++;
                numVal += (s32)*p++ << 8;
                numVal += (s32)*p++ << 16;
                numVal += (s32)*p++ << 24;
#ifdef MICROCHIP_API
                if(funcValType == PUBVAL_C_LONG) {
                    s32toa(numVal, buffer);
                } else {
                    dectoa(numVal, buffer, YOCTO_PUBVAL_LEN-1, 1);
                }
#else
                if(funcValType == PUBVAL_C_LONG) {
                    YSPRINTF(buffer, YOCTO_PUBVAL_LEN, "%d", numVal);
                } else {
                    char *endp;
                    YSPRINTF(buffer, YOCTO_PUBVAL_LEN, "%.3f", numVal/1000.0);
                    endp = buffer + strlen(buffer);
                    while(endp > buffer && endp[-1] == '0') {
                        *--endp = 0;
                    }
                    if(endp > buffer && endp[-1] == '.') {
                        *--endp = 0;
                    }
                }
#endif
                return;
            case PUBVAL_C_FLOAT:
                // 32bit (short) float
                memcpy(&floatVal, p, sizeof(floatVal));
#ifdef MICROCHIP_API
                dectoa(floatVal*1000.0, buffer, YOCTO_PUBVAL_LEN-1, 1);
#else
                {
                    char largeBuffer[64];
                    char *endp;
                    YSPRINTF(largeBuffer, 64, "%.6f", floatVal);
                    endp = largeBuffer + strlen(largeBuffer);
                    while(endp > largeBuffer && endp[-1] == '0') {
                        *--endp = 0;
                    }
                    if(endp > largeBuffer && endp[-1] == '.') {
                        *--endp = 0;
                    }
                    YSTRCPY(buffer, YOCTO_PUBVAL_LEN, largeBuffer);
                }
#endif
                return;
            default:
                buffer[0] = '?';
                buffer[1] = 0;
                return;
        }
    }

    // Legacy handling: just pad with NUL up to 7 chars
    for(i = 0; i < 6; i++,p++) {
        u8 c = *p;
        if(!c) break;
        buffer[i] = c;
    }
    buffer[i] = 0;
}

#endif
