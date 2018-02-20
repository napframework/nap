/*********************************************************************
 *
 * $Id: ymemory.c 24731 2016-06-06 09:09:06Z seb $
 *
 * Basic memory check function to prevent memory leak
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

#define __FILE_ID__  "ymemory"
 // do not use microsoft secure string
#define _CRT_SECURE_NO_DEPRECATE
#define YMEMORY_ALLOW_MALLOC
#include "yproto.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef WINDOWS_API
#include <time.h>
#else
#include <sys/time.h>
#endif


#ifdef YSAFE_MEMORY
typedef  enum{
    YMEM_NOT_USED=0,
    YMEM_MALLOCED,
    YMEM_FREED
} YMEM_STATE;

typedef struct{
    YMEM_STATE  state;
    const char *malloc_file;
    u32         malloc_line;
    u32         malloc_size;
    const char *free_file;
    u32         free_line;
    void        *ptr;
} YMEM_ENTRY;

YMEM_ENTRY          *yMap    = NULL;
u32                 yMapSize = 0;
u32                 yMapUsed = 0;
yCRITICAL_SECTION   yMapCS;



static void ymemdumpentry(YMEM_ENTRY *entry,const char* prefix)
{
    dbglog("%s %lX: alloc %s:%d of %db, free %s:%d\n",prefix,entry->ptr,entry->malloc_file,entry->malloc_line,entry->malloc_size,entry->free_file,entry->free_line);
}


static void  ymemdump(void)
{
    u32 i;
    YMEM_ENTRY *entry;
    u32 total,count;

    dbglog("ySafeMemoryDump: %d/%d entry\n\n", yMapUsed, yMapSize);
    dbglog("Malloc:\n");
    total=count=0;
    for(i=0, entry=yMap; i< yMapUsed ; i++,entry++){
        if(entry->state == YMEM_MALLOCED){
            ymemdumpentry(entry,"");
            dbglog("%s : %d of %db (0x%x)\n", entry->malloc_file, entry->malloc_line, entry->malloc_size, entry->ptr);
            total+= entry->malloc_size;
            count++;
        }
    }
    dbglog("total: %db (%d Kb) on %d entry\n\n",total,(int)(total/1024),count);
#if 0
    dbglog("Free:\n");
    for(i=0, entry=yMap; i< yMapUsed ; i++,entry++){
        if(entry->state == YMEM_FREED){
            dbglog("alloc %s:%d of %db, free %s:%d\n",entry->malloc_file,entry->malloc_line,entry->malloc_size,entry->free_file,entry->free_line);
        }
    }
#endif
}



void ySafeMemoryInit(u32 nbentry)
{
    YASSERT(yMap==NULL);
    YASSERT(yMapSize==0);
    yInitializeCriticalSection(&yMapCS);
    yEnterCriticalSection(&yMapCS);
    yMap = malloc(nbentry *sizeof(YMEM_ENTRY));
    if(yMap){
        yMapSize = nbentry;
        memset(yMap,0,nbentry *sizeof(YMEM_ENTRY));
        yMapUsed=0;
    }
    yLeaveCriticalSection(&yMapCS);
}

void *ySafeMalloc(const char *file,u32 line,u32 size)
{
    u32 i;
    YMEM_ENTRY *entry;
    void *ptr;

    yEnterCriticalSection(&yMapCS);
    if(yMapUsed < yMapSize){
        //use a new one
        entry=yMap+yMapUsed;
    }else{
        // find a freed entry
        for(i=0; i< yMapSize;i++){
            if(yMap[i].state == YMEM_FREED)
                break;
        }
        if(i==yMapSize){
            dbglog("No more entry available for ySafeMalloc\n\n");
            ymemdump();
            yLeaveCriticalSection(&yMapCS);
            return NULL;
        }
        entry = yMap+i;
    }

    ptr=malloc(size);
    if(!ptr){
        dbglog("No more memory available (unable to allocate %d bytes)\n\n",size);
        ymemdump();
        yLeaveCriticalSection(&yMapCS);
        return NULL;
    }

    memset(entry,0,sizeof(YMEM_ENTRY));
    entry->state = YMEM_MALLOCED;
    entry->malloc_file = file;
    entry->malloc_line = line;
    entry->ptr  = ptr;
    entry->malloc_size = size;
    if(yMapUsed < yMapSize)
        yMapUsed++;
    yLeaveCriticalSection(&yMapCS);

    return ptr;
}

void  ySafeFree(const char *file,u32 line,void *ptr)
{
    u32 i;
    YMEM_ENTRY *entry;

    yEnterCriticalSection(&yMapCS);
    for(i=0, entry=yMap; i< yMapUsed ; i++,entry++){
        YASSERT(entry->state != YMEM_NOT_USED);
        if(entry->ptr == ptr)
            break;
    }
    if(i == yMapUsed){
        dbglog("Free of unallocated pointer 0x%x at %s:%d\n\n",ptr,file,line);
        ymemdump();
        YASSERT(0);
    }
    if(entry->state == YMEM_FREED){
        dbglog("Free of allready freed pointer (0x%x) at %s:%d\n",ptr,file,line);
        dbglog("was allocated at %s:%d size =%d freed at %s:%d\n\n",
            entry->malloc_file, entry->malloc_line, entry->malloc_size, entry->free_file,entry->free_line);
        ymemdump();
        YASSERT(0);
    }
    free(entry->ptr);
    entry->free_file = file;
    entry->free_line = line;
    entry->state = YMEM_FREED;
    entry->ptr=NULL;

    yLeaveCriticalSection(&yMapCS);
}

void  ySafeTrace(const char *file,u32 line,void *ptr)
{
    u32 i;
    YMEM_ENTRY *entry;

    yEnterCriticalSection(&yMapCS);
    for(i=0, entry=yMap; i< yMapUsed ; i++,entry++){
        YASSERT(entry->state != YMEM_NOT_USED);
        if(entry->ptr == ptr)
            break;
    }
    if(i == yMapUsed){
        dbglog("Update trace of unallocated pointer 0x%x at %s:%d\n\n",ptr,file,line);
        ymemdump();
        YASSERT(0);
    }
    if(entry->state == YMEM_FREED){
        dbglog("Update trace of allready freed pointer (0x%x) at %s:%d\n",ptr,file,line);
        dbglog("was allocated at %s:%d size =%d freed at %s:%d\n\n",
               entry->malloc_file, entry->malloc_line, entry->malloc_size, entry->free_file,entry->free_line);
        ymemdump();
        YASSERT(0);
    }
    ymemdumpentry(entry,"trace");
    entry->malloc_file = file;
    entry->malloc_line = line;
    yLeaveCriticalSection(&yMapCS);
}

void  ySafeMemoryDump(void *discard)
{
    u32 i;
    YMEM_ENTRY *entry;

    yEnterCriticalSection(&yMapCS);
    for(i=0, entry=yMap; i< yMapUsed ; i++,entry++){
        if(entry->state == YMEM_MALLOCED && entry->ptr!=discard){
            break;
        }
    }
    if(i< yMapUsed){
        ymemdump();
    } else {
        dbglog("No memory leak detected\n");
    }
    yLeaveCriticalSection(&yMapCS);
}


void  ySafeMemoryStop(void)
{
    yDeleteCriticalSection(&yMapCS);
    free(yMap);
    yMap=NULL;
    yMapSize = yMapUsed = 0;
}

#endif


// return the min of strlen and maxlen
static unsigned ystrnlen(const char *src,unsigned maxlen)
{
    unsigned len;
    for (len=0 ; *src && len < maxlen ;len++,src++);
    return len;
}

YRETCODE ystrcpy_s(char *dst, unsigned dstsize,const char *src)
{

    return ystrncpy_s(dst,dstsize,src,dstsize);
}


char* ystrdup_s(const char *src)
{
    int len = YSTRLEN(src);
    char *tmp = yMalloc(len+1);
    memcpy(tmp, src, len + 1);
    return tmp;
}


YRETCODE ystrcat_s(char *dst, unsigned dstsize,const char *src)
{
    return ystrncat_s(dst, dstsize, src, dstsize);
}

int ysprintf_s(char *dst, unsigned dstsize,const char *fmt ,...)
{
    int len;
    va_list args;
    va_start( args, fmt );
    len = yvsprintf_s(dst,dstsize,fmt,args);
    va_end(args);
    return len;
}

YRETCODE ystrncpy_s(char *dst,unsigned dstsize,const char *src,unsigned arglen)
{
    unsigned len;

    if (dst==NULL){
        YPANIC;
        return YAPI_INVALID_ARGUMENT;
    }
    if (src==NULL){
        YPANIC;
        return YAPI_INVALID_ARGUMENT;
    }
    if(dstsize ==0){
        YPANIC;
        return YAPI_INVALID_ARGUMENT;
    }
    len = ystrnlen(src,arglen);
    if(len+1 > dstsize){
        YPANIC;
        dst[0]=0;
        return YAPI_INVALID_ARGUMENT;
    }else{
        memcpy(dst,src,len);
        dst[len]=0;
    }
    return YAPI_SUCCESS;
}


YRETCODE ystrncat_s(char *dst, unsigned dstsize,const char *src,unsigned len)
{
    unsigned dstlen;
    if (dst==NULL){
        YPANIC;
        return YAPI_INVALID_ARGUMENT;
    }
    if (src==NULL){
        YPANIC;
        return YAPI_INVALID_ARGUMENT;
    }
    dstlen = ystrnlen(dst, dstsize);
    if(dstlen+1 > dstsize){
        YPANIC;
        return YAPI_INVALID_ARGUMENT;
    }
    return ystrncpy_s(dst+dstlen, dstsize-dstlen, src, len);
}


int yvsprintf_s (char *dst, unsigned dstsize, const char * fmt, va_list arg )
{
    int len;
#if defined(_MSC_VER) && (_MSC_VER <= MSC_VS2003)
    len = _vsnprintf(dst,dstsize,fmt,arg);
#else
    len = vsnprintf(dst,dstsize,fmt,arg);
#endif
    if(len <0 || len >=(long)dstsize){
        YPANIC;
        dst[dstsize-1]=0;
        return YAPI_INVALID_ARGUMENT;
    }
    return len;
}

int ymemfind(const u8 *haystack, u32 haystack_len, const u8 *needle, u32 needle_len)
{
    u32 abspos = 0;
    u32 needle_pos = 0;

    do {
        while (needle_pos < needle_len && (abspos + needle_pos)<haystack_len && needle[needle_pos] == haystack[abspos + needle_pos]) {
            needle_pos++;
        }
        if (needle_pos == needle_len) {
            return abspos;
        } else {
            abspos++;
            needle_pos = 0;
        }
    } while (abspos + needle_len < haystack_len);
    return -1;
}


