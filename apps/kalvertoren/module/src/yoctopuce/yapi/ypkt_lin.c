/*********************************************************************
 *
 * $Id: ypkt_lin.c 29340 2017-11-29 10:42:47Z seb $
 *
 * OS-specific USB packet layer, Linux version
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

#define __FILE_ID__  "ypkt_lin"
#include "yapi.h"
#ifdef LINUX_API
#include "yproto.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define yLinSetErr(intro, err,errmsg)  yLinSetErrEx(__LINE__,intro, err,errmsg)

static int yLinSetErrEx(u32 line,char *intro, int err,char *errmsg)
{
    const char *msg;
    if(errmsg==NULL)
        return YAPI_IO_ERROR;
     switch(err){
        case LIBUSB_SUCCESS:            msg="Success (no error)";break;
        case LIBUSB_ERROR_IO:           msg="Input/output error"; break;
        case LIBUSB_ERROR_INVALID_PARAM:msg="Invalid parameter"; break;
        case LIBUSB_ERROR_ACCESS:       msg="Access denied (insufficient permissions)"; break;
        case LIBUSB_ERROR_NO_DEVICE:    msg="No such device (it may have been disconnected)"; break;
        case LIBUSB_ERROR_NOT_FOUND:    msg="Entity not found"; break;
        case LIBUSB_ERROR_BUSY:         msg="Resource busy"; break;
        case LIBUSB_ERROR_TIMEOUT:      msg="Operation timed out"; break;
        case LIBUSB_ERROR_OVERFLOW:     msg="Overflow"; break;
        case LIBUSB_ERROR_PIPE:         msg="Pipe error"; break;
        case LIBUSB_ERROR_INTERRUPTED:  msg="System call interrupted (perhaps due to signal)"; break;
        case LIBUSB_ERROR_NO_MEM:       msg="Insufficient memory"; break;
        case LIBUSB_ERROR_NOT_SUPPORTED:msg="Operation not supported or unimplemented on this platform"; break;
        default:
        case LIBUSB_ERROR_OTHER:        msg="Other error"; break;
    }
    if (intro){
        YSPRINTF(errmsg,YOCTO_ERRMSG_LEN,"%s:%s",intro,msg);
    } else{
        YSPRINTF(errmsg,YOCTO_ERRMSG_LEN,"LIN(%d):%s",line,msg);
    }
    HALLOG("LIN(%d):%s\n",line,msg);
    return YAPI_IO_ERROR;
};


 /*****************************************************************
 * USB ENUMERATION
*****************************************************************/


#define YOCTO_LOCK_PIPE "/tmp/.yoctolock"

int pid_lock_fd = -1;


// return 1 if we can reserve access to the device 0 if the device
// is already reserved
static int yReserveGlobalAccess(yContextSt *ctx, char *errmsg)
{
    int fd;
    int chk_val, mypid, usedpid = 0;
    size_t res;
    mode_t mode = 0666;
    mode_t oldmode = umask(0000);
    char msg[YOCTO_ERRMSG_LEN];

    HALLOG("old mode (%#o)\n",oldmode);
    HALLOG("create fifo with (%#o)\n",mode);
    if(mkfifo(YOCTO_LOCK_PIPE,mode)<0) {
        HALLOG("unable to create lock fifo (%d:%s)\n",errno,strerror(errno));
    }
    umask(oldmode);
    fd = open(YOCTO_LOCK_PIPE,O_RDWR|O_NONBLOCK);
    if(fd<0){
        HALLOG("unable to open lock fifo (%d)\n",errno);
        if(errno==EACCES) {
            return YERRMSG(YAPI_DOUBLE_ACCES, "we do not have acces to lock fifo");
        }else{
            // we cannot open lock file so we cannot realy
            // check double instance so we asume that we are
            // alone
            return YAPI_SUCCESS;
        }
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
    res = write(fd, &chk_val, sizeof(chk_val));
    if(res != sizeof(chk_val)) {
        YSPRINTF(msg, YOCTO_ERRMSG_LEN, "Write to lock fifo failed (%d)", res);
        close(fd);
        return YERRMSG(YAPI_DOUBLE_ACCES, msg);
    }
    if (usedpid != 0) {
        if (usedpid == 1) {
            close(fd);
            // locked by api that not store the pid
            return YERRMSG(YAPI_DOUBLE_ACCES, "Another process is already using yAPI");
        } else {
            YSPRINTF(msg, YOCTO_ERRMSG_LEN, "Another process (pid %d) is already using yAPI", (u32) usedpid);
            close(fd);
            return YERRMSG(YAPI_DOUBLE_ACCES, msg);
        }
    }
    pid_lock_fd = fd;
    return YAPI_SUCCESS;
}


size_t dropwarning;

static void yReleaseGlobalAccess(yContextSt *ctx)
{
    int chk_val;
    if(pid_lock_fd >=0){
        dropwarning = read(pid_lock_fd,&chk_val,sizeof(chk_val));
        close(pid_lock_fd);
        pid_lock_fd = -1;
      }
}

typedef struct {
    libusb_device *dev;
    int desc_index;
    int len;
    char *string;
    u64 expiration;
} stringCacheSt;

#define STRING_CACHE_SIZE 16
#define STRING_CACHE_EXPIRATION 60000 //1 minutes
static stringCacheSt stringCache[STRING_CACHE_SIZE];


// on success data point to a null terminated string of max length-1 characters
static int getUsbStringASCII(yContextSt *ctx, libusb_device_handle *hdl, libusb_device *dev, u8 desc_index, char *data, u32 length)
{
    u8  buffer[512];
    u32 l,len;
    int res,i;
    stringCacheSt *c = stringCache;
    stringCacheSt *f = NULL;
    u64 now = yapiGetTickCount();

    yEnterCriticalSection(&ctx->string_cache_cs);

    for (i = 0; i < STRING_CACHE_SIZE; i++, c++) {
        if (c->expiration > now) {
            if(c->dev == dev && c->desc_index == desc_index) {
                if (c->len > 0 && c->string) {
                    len = c->len;
                    if (c->len >= length)
                        len = length-1;
                    memcpy(data, c->string,  len);
                    data[len] = 0;
                    HALENUMLOG("return string from cache (%p:%d->%s)\n",dev,desc_index,c->string);
                    yLeaveCriticalSection(&ctx->string_cache_cs);
                    return c->len;
                } else {
                    f = c;
                    break;
                }
            }
        } else {
            if (c->string) {
                yFree(c->string);
                c->string =NULL;
            }
            if (f == NULL) {
                f = c;
            }
        }
    }

    res=libusb_control_transfer(hdl, LIBUSB_ENDPOINT_IN,
        LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8) | desc_index,
        0, buffer, 512, 10000);
    if(res<0) {
        yLeaveCriticalSection(&ctx->string_cache_cs);
        return res;
    }

    len=(buffer[0]-2)/2;
    if (len >= length) {
        len = length - 1;
    }

    for (l = 0; l < len; l++){
        data[l] = (char) buffer[2+(l*2)];
    }
    data[len] = 0;

    if (f != NULL) {
        f->dev = dev;
        f->desc_index = desc_index;
        f->string = yMalloc(len+1);
        memcpy(f->string, data, len+1);
        f->len  = len;
        f->expiration = yapiGetTickCount() + STRING_CACHE_EXPIRATION;
        HALENUMLOG("add string to cache (%p:%d->%s)\n",dev,desc_index,f->string);
    }
    yLeaveCriticalSection(&ctx->string_cache_cs);

    return len;
}


int process_libusb_events(yContextSt *ctx, int ms, char * errmsg)
{
    int res;
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    res = libusb_handle_events_timeout(ctx->libusb, &tv);
    if (res < 0) {
        yLinSetErr("libusb_handle_events_timeout", res, errmsg);
    }
    return res;
}


static void *event_thread(void *param)
{
    yContextSt *ctx = (yContextSt*)param;
    char            errmsg[YOCTO_ERRMSG_LEN];
    ctx->usb_thread_state = USB_THREAD_RUNNING;
    /* Non-blocking. See if the OS has any reports to give. */
    HALLOG("Start event_thread run loop\n");
    while (ctx->usb_thread_state != USB_THREAD_MUST_STOP) {
        int res = process_libusb_events(ctx, 1000, errmsg);
        if (res < 0) {
            yLinSetErr("libusb_handle_events_timeout", res,errmsg);
            break;
        }
    }
    HALLOG("event_thread run loop stoped\n");
    ctx->usb_thread_state = USB_THREAD_STOPED;
    return NULL;
}


int yyyUSB_init(yContextSt *ctx,char *errmsg)
{
    int res;

    YPROPERR(yReserveGlobalAccess(ctx, errmsg));
    memset(stringCache, 0, sizeof(stringCache));
    yInitializeCriticalSection(&ctx->string_cache_cs);
    res = libusb_init(&ctx->libusb);
    if(res !=0){
        return yLinSetErr("Unable to start lib USB", res,errmsg);
    }
#if 0
    {
        const struct libusb_version *libusb_v;
        libusb_v = libusb_get_version();
        dbglog("Use libUSB v%d.%d.%d.%d\n", libusb_v->major, libusb_v->minor, libusb_v->micro, libusb_v->nano);
    }
#endif
    ctx->usb_thread_state = USB_THREAD_NOT_STARTED;
    pthread_create(&ctx->usb_thread, NULL, event_thread, ctx);
    //wait thead start
    while(ctx->usb_thread_state != USB_THREAD_RUNNING){
        usleep(50000);
    }

    return YAPI_SUCCESS;
}


int yyyUSB_stop(yContextSt *ctx,char *errmsg)
{
    int i;
    stringCacheSt *c = stringCache;

    if(ctx->usb_thread_state == USB_THREAD_RUNNING){
        ctx->usb_thread_state = USB_THREAD_MUST_STOP;
        pthread_join(ctx->usb_thread,NULL);
    }
    YASSERT(ctx->usb_thread_state == USB_THREAD_STOPED);

    libusb_exit(ctx->libusb);
    yReleaseGlobalAccess(ctx);
    for (i = 0; i < STRING_CACHE_SIZE; i++, c++) {
        if(c->string) {
            yFree(c->string);
        }
    }
    yDeleteCriticalSection(&ctx->string_cache_cs);

    return YAPI_SUCCESS;
}



static int getDevConfig(libusb_device *dev, struct libusb_config_descriptor **config)
{

   int res = libusb_get_active_config_descriptor(dev,  config);
    if(res==LIBUSB_ERROR_NOT_FOUND){
        HALLOG("not yet configured\n");
        if(libusb_get_config_descriptor(dev, 0, config)!=0){
            return -1;
        }
    }else if(res!=0){
        HALLOG("unable to get active configuration %d\n",res);
        return -1;
    }
    return 0;

}


int yyyUSBGetInterfaces(yInterfaceSt **ifaces,int *nbifaceDetect,char *errmsg)
{
    libusb_device   **list;
    ssize_t         nbdev;
    int             returnval=YAPI_SUCCESS;
    int             i;
    int             alloc_size;
    yInterfaceSt    *iface;

    nbdev = libusb_get_device_list(yContext->libusb,&list);
    if (nbdev < 0)
        return yLinSetErr("Unable to get device list", nbdev, errmsg);
    HALENUMLOG("%d devices found\n",nbdev);

     // allocate buffer for detected interfaces
    *nbifaceDetect = 0;
    alloc_size = (nbdev + 1) * sizeof(yInterfaceSt);
    *ifaces = (yInterfaceSt*) yMalloc(alloc_size);
    memset(*ifaces, 0, alloc_size);

    for (i = 0; i < nbdev; i++) {
        int  res;
        struct libusb_device_descriptor desc;
        struct libusb_config_descriptor *config;
        libusb_device_handle *hdl;

        libusb_device  *dev = list[i];
        if ((res = libusb_get_device_descriptor(dev,&desc)) != 0){
            returnval = yLinSetErr("Unable to get device descriptor",res,errmsg);
            goto exit;
        }
        if (desc.idVendor != YOCTO_VENDORID) {
            continue;
        }
        HALENUMLOG("open device %x:%x\n", desc.idVendor, desc.idProduct);

        if(getDevConfig(dev, &config) < 0) {
            continue;
        }
        iface = (*ifaces) + (*nbifaceDetect);
        iface->vendorid = (u16)desc.idVendor;
        iface->deviceid = (u16)desc.idProduct;
        iface->ifaceno  = 0;
        iface->devref   = libusb_ref_device(dev);
        res = libusb_open(dev, &hdl);
        if (res == LIBUSB_ERROR_ACCESS) {
            returnval = YERRMSG(YAPI_IO_ERROR, "the user has insufficient permissions to access USB devices");
            goto exit;
        }
        if (res != 0){
            HALENUMLOG("unable to access device %x:%x\n", desc.idVendor, desc.idProduct);
            continue;
        }
        HALENUMLOG("try to get serial for %x:%x:%x (%p)\n", desc.idVendor, desc.idProduct, desc.iSerialNumber, dev);
        res = getUsbStringASCII(yContext, hdl, dev, desc.iSerialNumber, iface->serial, YOCTO_SERIAL_LEN);
        if (res < 0) {
            HALENUMLOG("unable to get serial for device %x:%x\n", desc.idVendor, desc.idProduct);
        }
        libusb_close(hdl);
        (*nbifaceDetect)++;
        HALENUMLOG("----Running Dev %x:%x:%d:%s ---\n", iface->vendorid, iface->deviceid, iface->ifaceno, iface->serial);
        libusb_free_config_descriptor(config);
    }

exit:
    libusb_free_device_list(list,1);
    return returnval;

}



// return 1 if OS hdl are identicals
//        0 if any of the interface has changed
int yyyOShdlCompare( yPrivDeviceSt *dev, yInterfaceSt *newiface)
{
    if(dev->infos.nbinbterfaces != 1){
        HALLOG("bad number of inteface for %s (%d)\n",dev->infos.serial,dev->infos.nbinbterfaces);
        return 0;
    }
    if(dev->iface.devref != newiface->devref){
        HALLOG("devref has changed for %s (%X)\n",dev->infos.serial,dev->iface.devref);
        return 0;
    }

    return 1;
}

static void rd_callback(struct libusb_transfer *transfer);
static void wr_callback(struct libusb_transfer *transfer);


static int sendNextPkt(yInterfaceSt *iface, char *errmsg)
{
    pktItem *pktitem;
    yPktQueuePeekH2D(iface, &pktitem);
    if (pktitem != NULL) {
        int res;
        memcpy(&iface->wrTr->tmppkt, &pktitem->pkt, sizeof(USB_Packet));
        libusb_fill_interrupt_transfer( iface->wrTr->tr,
                                iface->hdl,
                                iface->wrendp,
                                (u8*)&iface->wrTr->tmppkt,
                                sizeof(USB_Packet),
                                wr_callback,
                                iface->wrTr,
                                2000);
        res = libusb_submit_transfer(iface->wrTr->tr);
        if (res < 0) {
            return yLinSetErr("libusb_submit_transfer(WR) failed", res, errmsg);
        }
    }
    return YAPI_SUCCESS;
}


static int submitReadPkt(yInterfaceSt *iface, char *errmsg)
{
    int res;
    libusb_fill_interrupt_transfer( iface->rdTr->tr,
                                    iface->hdl,
                                    iface->rdendp,
                                    (u8*)&iface->rdTr->tmppkt,
                                    sizeof(USB_Packet),
                                    rd_callback,
                                    iface->rdTr,
                                    0);
    res = libusb_submit_transfer(iface->rdTr->tr);
    if (res < 0) {
        return yLinSetErr("libusb_submit_transfer(RD) failed", res, errmsg);
    }
    return YAPI_SUCCESS;
}


static void rd_callback(struct libusb_transfer *transfer)
{
    int res;
    linRdTr      *lintr = (linRdTr*)transfer->user_data;
    yInterfaceSt *iface = lintr->iface;
    char          errmsg[YOCTO_ERRMSG_LEN];

    if (lintr == NULL){
        HALLOG("CBrd:drop invalid ypkt rd_callback (lintr is null)\n");
        return;
    }
    if (iface == NULL){
        HALLOG("CBrd:drop invalid ypkt rd_callback (iface is null)\n");
        return;
    }

    switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:
        //HALLOG("%s:%d pkt_arrived (len=%d)\n",iface->serial,iface->ifaceno,transfer->actual_length);
        yPktQueuePushD2H(iface,&lintr->tmppkt,NULL);
        break;
    case LIBUSB_TRANSFER_ERROR:
        iface->ioError++;
        HALLOG("CBrd:%s pkt error (len=%d nbError:%d)\n",iface->serial, transfer->actual_length,  iface->ioError);
        break;
    case LIBUSB_TRANSFER_TIMED_OUT :
        HALLOG("CBrd:%s pkt timeout\n",iface->serial);
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        HALLOG("CBrd:%s pkt_cancelled (len=%d) \n",iface->serial, transfer->actual_length);
        if (iface->flags.yyySetupDone && transfer->actual_length == 64) {
            yPktQueuePushD2H(iface, &lintr->tmppkt, NULL);
        }
        return;
    case LIBUSB_TRANSFER_STALL:
        HALLOG("CBrd:%s pkt stall\n",iface->serial );
        res = libusb_cancel_transfer(lintr->tr);
        HALLOG("CBrd:%s libusb_cancel_transfer returned %d\n",iface->serial, res);
        res = libusb_clear_halt(iface->hdl, iface->rdendp);
        HALLOG("CBrd:%s libusb_clear_hal returned %d\n",iface->serial, res);
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        HALLOG("CBrd:%s no_device (len=%d)\n",iface->serial, transfer->actual_length);
        return;
    case LIBUSB_TRANSFER_OVERFLOW:
        HALLOG("CBrd:%s pkt_overflow (len=%d)\n",iface->serial, transfer->actual_length);
        return;
    default:
        HALLOG("CBrd:%s unknown state %X\n",iface->serial, transfer->status);
        return;
    }

    if (iface->flags.yyySetupDone) {
        res = submitReadPkt(iface, errmsg);
        if (res < 0) {
            HALLOG("CBrd:%s libusb_submit_transfer errror %X\n", iface->serial, res);
        }
    }

}

static void wr_callback(struct libusb_transfer *transfer)
{
    linRdTr      *lintr = (linRdTr*)transfer->user_data;
    yInterfaceSt *iface = lintr->iface;
    char          errmsg[YOCTO_ERRMSG_LEN];
    pktItem *pktitem;
    int res;

    if (lintr == NULL) {
        HALLOG("CBwr:drop invalid ypkt wr_callback (lintr is null)\n");
        return;
    }

    if (iface == NULL){
        HALLOG("CBwr:drop invalid ypkt wr_callback (iface is null)\n");
        return;
    }
    YASSERT(transfer == lintr->tr);

    switch(transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
        //HALLOG("CBwr:%s pkt_sent (len=%d)\n",iface->serial, transfer->actual_length);
        // remove sent packet
        yPktQueuePopH2D(iface, &pktitem);
        yFree(pktitem);
        res = sendNextPkt(iface, errmsg);
        if (res < 0) {
            HALLOG("send of next pkt item failed:%d:%s\n", res, errmsg);
        }
        return;
    case LIBUSB_TRANSFER_ERROR:
        iface->ioError++;
        HALLOG("CBwr:%s pkt error (len=%d nbError:%d)\n",iface->serial, transfer->actual_length,  iface->ioError);
        break;
    case LIBUSB_TRANSFER_TIMED_OUT :
        HALLOG("CBwr:%s pkt timeout\n",iface->serial);
        res = sendNextPkt(iface, errmsg);
        if (res < 0) {
            HALLOG("retry of next pkt item failed:%d:%s\n", res, errmsg);
        }
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        HALLOG("CBwr:%s pkt_cancelled (len=%d) \n",iface->serial, transfer->actual_length);
        break;
    case LIBUSB_TRANSFER_STALL:
        HALLOG("CBwr:%s pkt stall\n",iface->serial );
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        HALLOG("CBwr:%s pkt_cancelled (len=%d)\n",iface->serial, transfer->actual_length);
        return;
    case LIBUSB_TRANSFER_OVERFLOW:
        HALLOG("CBwr:%s pkt_overflow (len=%d)\n",iface->serial, transfer->actual_length);
        break;
    default:
        HALLOG("CBwr:%s unknown state %X\n",iface->serial, transfer->status);
        break;
    }
}


int yyySetup(yInterfaceSt *iface,char *errmsg)
{
    int res,j;
    int error;
    struct libusb_config_descriptor *config;
    const struct libusb_interface_descriptor* ifd;



    HALLOG("%s yyySetup %X:%X\n",iface->serial,iface->vendorid,iface->deviceid);
    if(iface->devref==NULL){
        return YERR(YAPI_DEVICE_NOT_FOUND);
    }

    // we need to do this as it is possible that the device was not closed properly in a previous session
    // if we don't do this and the device wasn't closed properly odd behavior results.
    // thanks to Rob Krakora who find this solution
    if((res=libusb_open(iface->devref,&iface->hdl))!=0){
        return yLinSetErr("libusb_open", res,errmsg);
    } else {
        libusb_reset_device(iface->hdl);
        libusb_close(iface->hdl);
        usleep(200);
    }

    if((res=libusb_open(iface->devref,&iface->hdl))!=0){
        return yLinSetErr("libusb_open", res,errmsg);
    }

    if((res=libusb_kernel_driver_active(iface->hdl,iface->ifaceno))<0){
        error= yLinSetErr("libusb_kernel_driver_active",res,errmsg);
        goto error;
    }
    if (res) {
        HALLOG("%s need to detach kernel driver\n",iface->serial);
        if((res = libusb_detach_kernel_driver(iface->hdl,iface->ifaceno))<0){
            error= yLinSetErr("libusb_detach_kernel_driver",res,errmsg);
            goto error;
        }
    }

    HALLOG("%s Claim interface\n",iface->serial);
    if((res = libusb_claim_interface(iface->hdl,iface->ifaceno))<0){
        error= yLinSetErr("libusb_claim_interface", res,errmsg);
        goto error;
    }


    res=getDevConfig(iface->devref,&config);
    if(res<0){
        error=YERRMSG(YAPI_IO_ERROR,"unable to get configuration descriptor");
        goto error;
    }


    ifd = &config->interface[iface->ifaceno].altsetting[0];
    for (j = 0; j < ifd->bNumEndpoints; j++) {
        //HALLOG("endpoint %X size=%d \n",ifd->endpoint[j].bEndpointAddress,ifd->endpoint[j].wMaxPacketSize);
        if((ifd->endpoint[j].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN){
            iface->rdendp = ifd->endpoint[j].bEndpointAddress;
        }else{
            iface->wrendp = ifd->endpoint[j].bEndpointAddress;
        }
    }
    HALLOG("ednpoints are rd=%d wr=%d\n", iface->rdendp,iface->wrendp);

    yPktQueueInit(&iface->rxQueue);
    yPktQueueInit(&iface->txQueue);
    iface->rdTr = yMalloc(sizeof(linRdTr));
    iface->wrTr = yMalloc(sizeof(linRdTr));
    HALLOG("allocate linRdTr=%p linWrTr\n", iface->rdTr, iface->wrTr);
    iface->wrTr->iface = iface;
    iface->wrTr->tr = libusb_alloc_transfer(0);
    iface->rdTr->iface = iface;
    iface->rdTr->tr = libusb_alloc_transfer(0);
    iface->flags.yyySetupDone = 1;
    HALLOG("%s both libusbTR allocated (%p /%p)\n",iface->serial, iface->rdTr->tr, iface->wrTr->tr);
    res = submitReadPkt(iface, errmsg);
    if (res < 0) {
        return res;
    }
    HALLOG("%s yyySetup done\n",iface->serial);

    return YAPI_SUCCESS;
error:
    libusb_close(iface->hdl);
    return error;
}




int yyySignalOutPkt(yInterfaceSt *iface, char *errmsg)
{
    return sendNextPkt(iface, errmsg);
}



void yyyPacketShutdown(yInterfaceSt  *iface)
{
    if (iface && iface->hdl) {
        int res;
        iface->flags.yyySetupDone = 0;
        HALLOG("%s:%d cancel all transfer\n",iface->serial,iface->ifaceno);
        if (iface->rdTr->tr) {
            int count = 10;
            int res =libusb_cancel_transfer(iface->rdTr->tr);
            if(res == 0){
                while(count && iface->rdTr->tr->status != LIBUSB_TRANSFER_CANCELLED){
                        usleep(1000);
                    count--;
                }
            }
        }
        HALLOG("%s:%d libusb relase iface\n",iface->serial,iface->ifaceno);
        res = libusb_release_interface(iface->hdl,iface->ifaceno);
        if(res != 0 && res!=LIBUSB_ERROR_NOT_FOUND && res!=LIBUSB_ERROR_NO_DEVICE){
            HALLOG("%s:%dlibusb_release_interface error\n",iface->serial,iface->ifaceno);
        }

        res = libusb_attach_kernel_driver(iface->hdl,iface->ifaceno);
        if(res<0 && res!=LIBUSB_ERROR_NO_DEVICE){
            HALLOG("%s:%d libusb_attach_kernel_driver error\n",iface->serial,iface->ifaceno);
        }
        libusb_close(iface->hdl);
        iface->hdl = NULL;

        if (iface->rdTr->tr) {
            HALLOG("%s:%d libusb_TR free\n", iface->serial, iface->ifaceno);
            libusb_free_transfer(iface->rdTr->tr);
            iface->rdTr->tr = NULL;
        }
        yFree(iface->rdTr);
        yPktQueueFree(&iface->rxQueue);
        yPktQueueFree(&iface->txQueue);
    }
}

#endif
