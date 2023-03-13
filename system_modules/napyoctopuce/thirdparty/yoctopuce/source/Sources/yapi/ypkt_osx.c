/*********************************************************************
 *
 * $Id: ypkt_osx.c 28024 2017-07-10 08:50:02Z mvuilleu $
 *
 * OS-specific USB packet layer, Mac OS X version
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

#define __FILE_ID__ "ypkt_osx"
#include "yapi.h"
#ifdef OSX_API
#include "yproto.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <fcntl.h>
#include <unistd.h>

/*****************************************************************
 * USB ENUMERATION
 *****************************************************************/


#define YOCTO_LOCK_PIPE "/tmp/.yoctolock"

// return YAPI_SUCCESS if we can reserve access to the device return
// an error if the device is already reserved
static int yReserveGlobalAccess(yContextSt *ctx, char *errmsg)
{
    int fd;
    int chk_val, mypid, usedpid = 0;
    size_t res;

    mkfifo(YOCTO_LOCK_PIPE, 0600);
    fd = open(YOCTO_LOCK_PIPE, O_RDWR|O_NONBLOCK);
    if (fd < 0) {
        // we cannot open lock file so we cannot realy
        // check double instance so we asume that we are
        // alone
        return YAPI_SUCCESS;
    }
    chk_val = 0;
    mypid = (int) getpid();
    res = read(fd, &chk_val, sizeof(chk_val));
    if (res == sizeof(chk_val)) {
        //there is allready someone
        usedpid = chk_val;
    } else{
        // nobody there -> store my PID
        chk_val = mypid;
    }
    write(fd, &chk_val, sizeof(chk_val));
    if (usedpid != 0) {
        if (usedpid == 1) {
            // locked by api that not store the pid
            return YERRMSG(YAPI_DOUBLE_ACCES, "Another process is already using yAPI");
        } else {
            char msg[YOCTO_ERRMSG_LEN];
            YSPRINTF(msg, YOCTO_ERRMSG_LEN, "Another process (pid %d) is already using yAPI", (u32) usedpid);
            return YERRMSG(YAPI_DOUBLE_ACCES, msg);
        }
    }
    return YAPI_SUCCESS;
}



static void yReleaseGlobalAccess(yContextSt *ctx)
{
    int chk_val;
    int fd = open(YOCTO_LOCK_PIPE,O_RDWR|O_NONBLOCK);
    if(fd>=0){
        read(fd,&chk_val,sizeof(chk_val));
    }
}

static void *event_thread(void *param)
{
    yContextSt  *ctx=param;

    ctx->usb_run_loop     = CFRunLoopGetCurrent();
    ctx->usb_thread_state = USB_THREAD_RUNNING;
    /* Non-blocking. See if the OS has any reports to give. */
    HALLOG("Start event_thread run loop\n");
    while (ctx->usb_thread_state != USB_THREAD_MUST_STOP) {
        CFRunLoopRunInMode( kCFRunLoopDefaultMode, 10, FALSE);
    }

    HALLOG("event_thread run loop stoped\n");
    ctx->usb_thread_state = USB_THREAD_STOPED;
    return NULL;
}


static int setupHIDManager(yContextSt *ctx, OSX_HID_REF *hid, char *errmsg)
{
    int             c_vendorid = YOCTO_VENDORID;
    CFMutableDictionaryRef dictionary;
    CFNumberRef     Vendorid;
    IOReturn        tIOReturn;

    yInitializeCriticalSection(&hid->hidMCS);
    // Initialize HID Manager
    hid->manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    // create dictionary to match Yocto devices
    dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault,1,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
    Vendorid = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &c_vendorid );
    CFDictionarySetValue( dictionary, CFSTR( kIOHIDVendorIDKey ), Vendorid );
    CFRelease(Vendorid);
    // register the dictionary
    IOHIDManagerSetDeviceMatching(hid->manager, dictionary );
    // now we can release the dictionary
    CFRelease(dictionary);
    // sechedulle the HID Manager with our global run loop
    IOHIDManagerScheduleWithRunLoop(hid->manager, ctx->usb_run_loop, kCFRunLoopDefaultMode);

    // Now open the IO HID Manager reference
    tIOReturn = IOHIDManagerOpen(hid->manager, kIOHIDOptionsTypeNone );
    if(kIOReturnSuccess != tIOReturn ||CFGetTypeID(hid->manager) != IOHIDManagerGetTypeID()){
        HALLOG("Unable to Open HID Manager");
        return YERRMSG(YAPI_NOT_SUPPORTED,"Unable to Open HID Manager");
    }
    return YAPI_SUCCESS;

}



static void stopHIDManager(OSX_HID_REF *hid)
{
    if (hid->manager) {
        IOHIDManagerClose(hid->manager, kIOHIDOptionsTypeNone );
        CFRelease( hid->manager);
        hid->manager=NULL;
        yDeleteCriticalSection(&hid->hidMCS);
    }
}


int yyyUSB_init(yContextSt *ctx,char *errmsg)
{
    char str[256];
    size_t size = sizeof(str);
    YPROPERR(yReserveGlobalAccess(ctx, errmsg));

    if (sysctlbyname("kern.osrelease", str, &size, NULL, 0) ==0){
        int numver;
        //15.x.x  OS X 10.11.x El Capitan
        //14.x.x  OS X 10.10.x Yosemite
        //13.x.x  OS X 10.9.x Mavericks
        //12.x.x  OS X 10.8.x Mountain Lion
        //11.x.x  OS X 10.7.x Lion
        //10.x.x  OS X 10.6.x Snow Leopard
        str[2]=0;
        numver = atoi(str);
        if (numver >= 13 && numver < 15){
            ctx->osx_flags |= YCTX_OSX_MULTIPLES_HID;
        }
    }

    ctx->usb_thread_state = USB_THREAD_NOT_STARTED;
    pthread_create(&ctx->usb_thread, NULL, event_thread, ctx);
    while(ctx->usb_thread_state != USB_THREAD_RUNNING){
        usleep(50000);
    }

    if (YISERR(setupHIDManager(ctx, &ctx->hid,errmsg))) {
        return YAPI_IO_ERROR;
    }
    return YAPI_SUCCESS;
}


int yyyUSB_stop(yContextSt *ctx,char *errmsg)
{
    stopHIDManager(&ctx->hid);

    if(ctx->usb_thread_state == USB_THREAD_RUNNING){
        ctx->usb_thread_state = USB_THREAD_MUST_STOP;
        CFRunLoopStop(ctx->usb_run_loop);
    }
    pthread_join(ctx->usb_thread,NULL);
    YASSERT(ctx->usb_thread_state == USB_THREAD_STOPED);

    yReleaseGlobalAccess(ctx);

    return 0;
}




static u32 get_int_property(IOHIDDeviceRef device, CFStringRef key)
{
    CFTypeRef ref;
    u32 value;

    ref = IOHIDDeviceGetProperty(device, key);
    if (ref) {
        if (CFGetTypeID(ref) == CFNumberGetTypeID() && CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value)) {
            return value;
        }
    }
    return 0;
}


static void get_txt_property(IOHIDDeviceRef device,char *buffer,u32 maxlen, CFStringRef key)
{
    CFTypeRef ref;
    size_t  len;

    ref = IOHIDDeviceGetProperty(device, key);
    if (ref) {
        if (CFGetTypeID(ref) == CFStringGetTypeID()) {
            const char *str;
            CFStringEncoding encodingMethod;
            encodingMethod = CFStringGetSystemEncoding();
            // 1st try for English system
            str = CFStringGetCStringPtr(ref, encodingMethod);
            //str = NULL;
            if ( str == NULL ) {
                // 2nd try
                encodingMethod = kCFStringEncodingUTF8;
                str = CFStringGetCStringPtr(ref, encodingMethod);
            }
            if( str == NULL ) {
                //3rd try
                CFIndex cflength = CFStringGetLength(ref)*2+2;
                char *tmp_str = yMalloc( (u32)cflength);
                if (!CFStringGetCString(ref, tmp_str, cflength, kCFStringEncodingUTF8 )) {
                    yFree( tmp_str );
                    *buffer=0;
                    return;
                }
                if(cflength>maxlen-1){
                    cflength=maxlen-1;
                }
                memcpy(buffer,tmp_str,cflength);
                buffer[cflength]=0;
                yFree( tmp_str );
                return;
            }
            len=strlen(str);
            if(len>maxlen-1){
                len=maxlen-1;
            }
            memcpy(buffer,str,len);
            buffer[len]=0;
            return;
        }
    }
    *buffer=0;
}





static IOHIDDeviceRef* getDevRef(OSX_HID_REF *hid, CFIndex *deviceCount)
{

    CFSetRef        deviceCFSetRef;
    IOHIDDeviceRef  *dev_refs=NULL;

    *deviceCount = 0;

    yEnterCriticalSection(&hid->hidMCS);
    deviceCFSetRef = IOHIDManagerCopyDevices(hid->manager);
    yLeaveCriticalSection(&hid->hidMCS);
    if (deviceCFSetRef!= NULL) {
        // how many devices in the set?
        *deviceCount = CFSetGetCount( deviceCFSetRef );
        dev_refs = yMalloc( sizeof(IOHIDDeviceRef) * (u32)*deviceCount );
        // now extract the device ref's from the set
        CFSetGetValues( deviceCFSetRef, (const void **) dev_refs );
    }
    return dev_refs;
}


int yyyUSBGetInterfaces(yInterfaceSt **ifaces,int *nbifaceDetect,char *errmsg)
{
    int             nbifaceAlloc;
    int             deviceIndex;
    CFIndex         deviceCount;
    IOHIDDeviceRef  *dev_refs;

    // get all device detected by the OSX
    dev_refs = getDevRef(&yContext->hid, &deviceCount);
    if(dev_refs == NULL) {
        return 0;
    }

    // allocate buffer for detected interfaces
    *nbifaceDetect = 0;
    nbifaceAlloc  = 8;
    *ifaces =yMalloc(nbifaceAlloc * sizeof(yInterfaceSt));
    memset(*ifaces, 0 ,nbifaceAlloc * sizeof(yInterfaceSt));
    for(deviceIndex=0 ; deviceIndex < deviceCount ;deviceIndex++){
        u16 vendorid;
        u16 deviceid;
        IOHIDDeviceRef dev = dev_refs[deviceIndex];
        yInterfaceSt    *iface;
        vendorid = get_int_property(dev,CFSTR(kIOHIDVendorIDKey));
        deviceid = get_int_property(dev,CFSTR(kIOHIDProductIDKey));
        //ensure the buffer of detected interface is big enought
        if(*nbifaceDetect == nbifaceAlloc){
            yInterfaceSt    *tmp;
            tmp = (yInterfaceSt*) yMalloc(nbifaceAlloc*2 * sizeof(yInterfaceSt));
            memset(tmp,0,nbifaceAlloc*2 * sizeof(yInterfaceSt));
            yMemcpy(tmp,*ifaces, nbifaceAlloc * sizeof(yInterfaceSt) );
            yFree(*ifaces);
            *ifaces = tmp;
            nbifaceAlloc    *=2;
        }
        iface = *ifaces + *nbifaceDetect;
        //iface->devref   = dev;
        iface->vendorid = vendorid;
        iface->deviceid = deviceid;
        get_txt_property(dev,iface->serial,YOCTO_SERIAL_LEN*2, CFSTR(kIOHIDSerialNumberKey));
        HALENUMLOG("work on interface %d (%x:%x:%s)\n",deviceIndex,vendorid,deviceid,iface->serial);
        (*nbifaceDetect)++;
    }
    yFree(dev_refs);
    return YAPI_SUCCESS;
}


/*****************************************************************
 * OSX implementation of yyypacket functions
 *****************************************************************/




// return 1 if OS hdl are identicals
//        0 if any of the interface has changed
int yyyOShdlCompare( yPrivDeviceSt *dev, yInterfaceSt *newdev)
{
    if(dev->infos.nbinbterfaces != 1){
        HALLOG("bad number of inteface for %s (%d)\n",dev->infos.serial,dev->infos.nbinbterfaces);
        return 0;
    }
    return 1;
}




static void Handle_IOHIDDeviceIOHIDReportCallback(
                                                  void *          inContext,          // context from IOHIDDeviceRegisterInputReportCallback
                                                  IOReturn        inResult,           // completion result for the input report operation
                                                  void *          inSender,           // IOHIDDeviceRef of the device this report is from
                                                  IOHIDReportType inType,             // the report type
                                                  uint32_t        inReportID,         // the report ID
                                                  uint8_t *       inReport,           // pointer to the report data
                                                  CFIndex         InReportLength)     // the actual size of the input report
{
    yInterfaceSt *iface= (yInterfaceSt*) inContext;
    yPktQueuePushD2H(iface,&iface->tmprxpkt,NULL);
    memset(&iface->tmprxpkt,0xff,sizeof(USB_Packet));
}


int yyySetup(yInterfaceSt *iface,char *errmsg)
{
    char str[32];
    int i;
    CFIndex deviceCount;
    IOHIDDeviceRef *dev_refs;


    if (yContext->osx_flags & YCTX_OSX_MULTIPLES_HID) {
        if (YISERR(setupHIDManager(yContext, &iface->hid,errmsg))) {
            return YAPI_IO_ERROR;
        }
        // get all device detected by the OSX
        dev_refs = getDevRef(&iface->hid, &deviceCount);
    } else {
        dev_refs = getDevRef(&yContext->hid, &deviceCount);
    }
    if(dev_refs == NULL) {
        return YERRMSG(YAPI_IO_ERROR,"Device disapear before yyySetup");
    }


    for(i=0 ; i < deviceCount ;i++){
        u16 vendorid;
        u16 deviceid;
        IOHIDDeviceRef dev = dev_refs[i];
        vendorid = get_int_property(dev,CFSTR(kIOHIDVendorIDKey));
        deviceid = get_int_property(dev,CFSTR(kIOHIDProductIDKey));
        if (iface->vendorid == vendorid && iface->deviceid == deviceid){
            char serial[YOCTO_SERIAL_LEN * 2];
            memset(serial, 0, YOCTO_SERIAL_LEN * 2);
            get_txt_property(dev,serial,YOCTO_SERIAL_LEN * 2, CFSTR(kIOHIDSerialNumberKey));
            if (YSTRCMP(serial, iface->serial) == 0){
                HALLOG("right Interface detected (%x:%x:%s)\n",vendorid,deviceid,iface->serial);
                iface->devref = dev;
                break;
            }
        }
    }
    yFree(dev_refs);
    if (i == deviceCount) {
        return YERRMSG(YAPI_IO_ERROR,"Unable to match device detected");
    }

    IOReturn ret = IOHIDDeviceOpen(iface->devref, kIOHIDOptionsTypeNone);
    if (ret != kIOReturnSuccess) {
        YSPRINTF(str,32,"Unable to open device (0x%x)",ret);
        return YERRMSG(YAPI_IO_ERROR,str);
    }

    yPktQueueInit(&iface->rxQueue);
	yPktQueueInit(&iface->txQueue);


    /* Create the Run Loop Mode for this device. printing the reference seems to work. */
    sprintf(str, "yocto_%p", iface->devref);
    iface->run_loop_mode = CFStringCreateWithCString(NULL, str, kCFStringEncodingASCII);
    /* Attach the device to a Run Loop */
    IOHIDDeviceScheduleWithRunLoop(iface->devref, yContext->usb_run_loop, iface->run_loop_mode);
    IOHIDDeviceRegisterInputReportCallback( iface->devref,              // IOHIDDeviceRef for the HID device
                                           (u8*) &iface->tmprxpkt,     // pointer to the report data
                                           USB_PKT_SIZE,               // number of bytes in the report (CFIndex)
                                           &Handle_IOHIDDeviceIOHIDReportCallback,   // the callback routine
                                           iface);                     // context passed to callback

    // save setuped iface pointer in context in order
    // to retreive it durring unplugcallback
    for (i=0; i< SETUPED_IFACE_CACHE_SIZE ; i++) {
        if(yContext->setupedIfaceCache[i]==NULL){
            yContext->setupedIfaceCache[i] = iface;
            break;
        }
    }
    if (i==SETUPED_IFACE_CACHE_SIZE) {
        return YERRMSG(YAPI_IO_ERROR,"Too many setuped USB interfaces");
    }
    iface->flags.yyySetupDone = 1;
    return 0;
}



int yyySignalOutPkt(yInterfaceSt *iface, char *errmsg)
{
    int res =YAPI_SUCCESS;
    pktItem *pktitem;

    yPktQueuePopH2D(iface, &pktitem);
    while (pktitem!=NULL){
        if(iface->devref==NULL){
            yFree(pktitem);
            return YERR(YAPI_IO_ERROR);
        }
        res = IOHIDDeviceSetReport(iface->devref,
                                   kIOHIDReportTypeOutput,
                                   0, /* Report ID*/
                                   (u8*)&pktitem->pkt, sizeof(USB_Packet));
        yFree(pktitem);
        if (res != kIOReturnSuccess) {
            dbglog("IOHIDDeviceSetReport failed with 0x%x\n", res);
            return YERRMSG(YAPI_IO_ERROR,"IOHIDDeviceSetReport failed");;
        }
        yPktQueuePopH2D(iface, &pktitem);
    }
	return YAPI_SUCCESS;
}



void yyyPacketShutdown(yInterfaceSt  *iface)
{
    int i;

    // remove iface from setuped ifaces
    for (i=0; i< SETUPED_IFACE_CACHE_SIZE ; i++) {
        if(yContext->setupedIfaceCache[i]==iface){
            yContext->setupedIfaceCache[i] = NULL;
            break;
        }
    }
    YASSERT(i<SETUPED_IFACE_CACHE_SIZE);
    if(iface->devref!=NULL){
        IOHIDDeviceRegisterInputReportCallback(iface->devref,              // IOHIDDeviceRef for the HID device
                                               (u8*) &iface->tmprxpkt,   // pointer to the report data (uint8_t's)
                                               USB_PKT_SIZE,              // number of bytes in the report (CFIndex)
                                               NULL,   // the callback routine
                                               iface);                      // context passed to callback
        IOHIDDeviceClose(iface->devref, kIOHIDOptionsTypeNone);
        iface->devref=NULL;
    }
    yPktQueueFree(&iface->rxQueue);
	yPktQueueFree(&iface->txQueue);
    iface->flags.yyySetupDone = 0;
    CFRelease(iface->run_loop_mode);
    if (yContext->osx_flags & YCTX_OSX_MULTIPLES_HID) {
        stopHIDManager(&iface->hid);
    }
}

#endif

#ifdef IOS_API
#include "yproto.h"

int yyyUSB_init(yContextSt *ctx,char *errmsg)
{
    return YAPI_SUCCESS;
}

int yyyUSB_stop(yContextSt *ctx,char *errmsg)
{
    return 0;
}

int yyyUSBGetInterfaces(yInterfaceSt **ifaces,int *nbifaceDetect,char *errmsg)
{
    *nbifaceDetect = 0;
    return 0;
}

int yyyOShdlCompare( yPrivDeviceSt *dev, yInterfaceSt *newdev)
{
    return 1;
}

int yyySetup(yInterfaceSt *iface,char *errmsg)
{
    return YERR(YAPI_NOT_SUPPORTED);
}

int yyySignalOutPkt(yInterfaceSt *iface, char *errmsg)
{
    return -1;
}

void yyyPacketShutdown(yInterfaceSt  *iface)
{}

#endif
