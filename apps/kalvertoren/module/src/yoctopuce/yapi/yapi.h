/*********************************************************************
 *
 * $Id: yapi.h 24575 2016-05-26 06:28:03Z seb $
 *
 * Declaration of public entry points to the low-level API
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

#ifndef YAPI_H
#define YAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ydef.h"

#if defined(WINDOWS_API) && defined(GENERATE_DLL)
#define YAPI_FUNCTION_EXPORT __declspec(dllexport)
#else
#define YAPI_FUNCTION_EXPORT
#endif

// Timeout for blocking requests to the devices, in milliseconds
#define YAPI_BLOCKING_USBOPEN_REQUEST_TIMEOUT    2000
#define YAPI_BLOCKING_USBREAD_REQUEST_TIMEOUT    8000
#define YAPI_BLOCKING_NET_REQUEST_TIMEOUT       30000

/*****************************************************************************
 CALLBACK TYPES
 ****************************************************************************/

// prototype of the Log callback
typedef void YAPI_FUNCTION_EXPORT(*yapiLogFunction)(const char *log,u32 loglen);

// prototype of the device arrival/update/removal callback
typedef void YAPI_FUNCTION_EXPORT(*yapiDeviceUpdateCallback)(YAPI_DEVICE devdescr);


// prototype of functions change callback
// value :
//       if null     : notify a new logical name
//       if not null : notify a new value, (a pointer to a  YOCTO_PUBVAL_LEN bytes null terminated string)
typedef void YAPI_FUNCTION_EXPORT(*yapiFunctionUpdateCallback)(YAPI_FUNCTION fundescr,const char *value);

// prototype of timed report callback
typedef void YAPI_FUNCTION_EXPORT(*yapiTimedReportCallback)(YAPI_FUNCTION fundesc, double timestamp, const u8 *bytes, u32 len);

// prototype of the ssdp hub discovery callback
typedef void YAPI_FUNCTION_EXPORT(*yapiHubDiscoveryCallback)(const char *serial, const char *url);

typedef void YAPI_FUNCTION_EXPORT(*yapiDeviceLogCallback)(YAPI_FUNCTION fundescr,const char *line);


/*****************************************************************************
 API FUNCTION DECLARATION
 ****************************************************************************/


void YAPI_FUNCTION_EXPORT yapiStartStopDeviceLogCallback(const char *serial,int start);





/*****************************************************************************
  Function:
    YRETCODE yInitAPI(int type,char *errmsg)

  Description:
    Initializes  and Allocate structures needed for the YoctoAPI

  Parameters:
    type: Y_DETECT_USB will auto-detect only USB connnected devices
          Y_DETECT_NET will auto-detect only Network devices
          Y_DETECT_ALL will auto-detect devices on all usable protocol
    errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

  Returns:
    on ERROR  : error code
    on SUCCES : YAPI_SUCCESS

  Remarks:
    This function must be called first and only one time.
    Network devices can be used when Y_DETECT_NET is not specified when
    manually registering network hubs; auto-detection only applies to
    Bonjour-based and NBNS-based discovery.
  ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiInitAPI(int type,char *errmsg);

#define Y_DETECT_NONE           0
#define Y_DETECT_USB            1
#define Y_DETECT_NET            2
#define Y_RESEND_MISSING_PKT    4
#define Y_DETECT_ALL   (Y_DETECT_USB | Y_DETECT_NET)

#define Y_DEFAULT_PKT_RESEND_DELAY 50


/*****************************************************************************
  Function:
    void yFreeAPI(void)

  Description:
    Release Allocate structures needed for the YoctoAPI

  Parameters:
    None

  Returns:
    None

  Remarks:

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiFreeAPI(void);


/*****************************************************************************
  Function:
    void  yapiRegisterLogFunction(yapiLogFunction logfun);

  Description:
    Register log function for yapi. This function is used mainly for debug purpose.
    To unregister your callback you can call this function with a NULL pointer.

  Parameters:
    logfun : a function to register or NULL to unregister the callback

  Returns:
    None

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterLogFunction(yapiLogFunction logfun);

/*****************************************************************************


 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterDeviceLogCallback(yapiDeviceLogCallback logCallback);

/*****************************************************************************
  Function:
    void  yapiRegisterDeviceArrivalCallback(yapiDeviceUpdateCallback arrivalCallback);

  Description:
    Register a callback function for device arrival. This function is only used to
    notify event you should return as soon as possible of the callback. you should
    also not call yapiUpdateDeviceList from the callback.
    To unregister your callback you can call this function with a NULL pointer.

  Parameters:
    arrivalCallback : a function to register or NULL to unregister the callback

  Returns:
    None

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterDeviceArrivalCallback(yapiDeviceUpdateCallback arrivalCallback);

/*****************************************************************************
  Function:
    void  yapiRegisterDeviceRemovalCallback(yapiDeviceUpdateCallback removalCallback);

  Description:
    Register a callback function for device removal. This function is only used to
    notify event you should return as soon as possible of the callback. you should
    also not call yapiUpdateDeviceList from the callback.
    To unregister your callback you can call this function with a NULL pointer.

  Parameters:
    removalCallback : a function to register or NULL to unregister the callback

  Returns:
    None

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterDeviceRemovalCallback(yapiDeviceUpdateCallback removalCallback);

/*****************************************************************************
  Function:
    void  yapiRegisterDeviceChangeCallback(yapiDeviceUpdateCallback changeCallback);

  Description:
    Register a callback function for a device that change his logical name. This function
    is only used to notify event you should return as soon as possible of the callback.
    you should also not call yapiUpdateDeviceList from the callback. To unregister
    your callback you can call this function with a NULL pointer.

  Parameters:
    changeCallback : a function to register or NULL to unregister the callback

  Returns:
    None

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterDeviceChangeCallback(yapiDeviceUpdateCallback changeCallback);

/*****************************************************************************
  Function:
    void yapiRegisterFunctionUpdateCallback(yapiFunctionUpdateCallback updateCallback);

  Description:
    Register a callback function for device function notification (value change,
    configuration change...). This function is only used to notify event you should
    return as soon as possible of the callback. To unregister your callback you can
    call this function with a NULL pointer.

  Parameters:
    updateCallback : a function to register or NULL to unregister the callback

  Returns:
    None

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterFunctionUpdateCallback(yapiFunctionUpdateCallback updateCallback);

/*****************************************************************************
  Function:
      void YAPI_FUNCTION_EXPORT yapiRegisterTimedReportCallback(yapiTimedReportCallback timedReportCallback);

  Description:
    Register a callback function for device function timed report. This function
    is only used to notify event you should return as soon as possible of the
    callback. To unregister your callback you can call this function with a NULL
    pointer.

  Parameters:
    timedReportCallback : a function to register or NULL to unregister the callback

  Returns:
    None

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterTimedReportCallback(yapiTimedReportCallback timedReportCallback);

YRETCODE YAPI_FUNCTION_EXPORT yapiLockFunctionCallBack( char *errmsg);


YRETCODE YAPI_FUNCTION_EXPORT yapiUnlockFunctionCallBack(char *errmsg);

YRETCODE YAPI_FUNCTION_EXPORT yapiLockDeviceCallBack( char *errmsg);


YRETCODE YAPI_FUNCTION_EXPORT yapiUnlockDeviceCallBack(char *errmsg);




/*****************************************************************************
 Function:
   YRETCODE yapiTestHub(const char *rooturl, int mstimeout, char *errmsg)

 Description:
   Test if a network URL can be used for yapiRegisterHub and yapiPreregisterHub. This
   function will not register the hub but will test that the hub is reachable and that
   authentication parameters are correct. This function will return before  mstimeout (or
   2 * mstimeout if the hub use authentication)

 Parameters:
   rooturl: The network URL of the hub, for instance "http://192.168.2.34", or "usb"
   mstimeout: The number of ms that the function has to test the URL
   errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR  : error code
   on SUCCES : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiTestHub(const char *rooturl, int mstimeout, char *errmsg);


/*****************************************************************************
 Function:
   YRETCODE yRegisterHub(const char *rooturl,char *errmsg)

 Description:
   Register a network URL to be scanned for devices when yUpdateDeviceList
   is called. To enable USB detection you could use  "usb" as rooturl. This
   is will return an error (and not register the hub) the url is not reachable

 Parameters:
   rooturl: The network URL of the hub, for instance "http://192.168.2.34", or "usb"
   errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR  : error code
   on SUCCES : YAPI_SUCCESS

 Remarks:
    if you have initialized the API with Y_DETECT_USB it is not necessary to
    manually to call this function with "usb"

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiRegisterHub(const char *rooturl, char *errmsg);


/*****************************************************************************
 Function:
 YRETCODE yPreregisterHub(const char *rooturl,char *errmsg)

 Description:
 Register a network URL to be scanned for devices when yUpdateDeviceList
 is called. To enable USB detection you could use  "usb" as rooturl. it is
 not considered as an error to give an url that is not reachable.

 Parameters:
 rooturl: The network URL of the hub, for instance "http://192.168.2.34", or "usb"
 errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
 on ERROR  : error code
 on SUCCES : YAPI_SUCCESS

 Remarks:
 if you have initialized the API with Y_DETECT_USB it is not necessary to
 manually to call this function with "usb"

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiPreregisterHub(const char *rooturl, char *errmsg);




/*****************************************************************************
 Function:
 YRETCODE yUnregisterHub(const char *rooturl)

 Description:
 Unregister a network URL  that has been register by yapiPreRegisterHub or yapiRegisterHub

 Parameters:
 rooturl: The network URL of the hub, for instance "http://192.168.2.34", or "usb"

 Returns:

 Remarks:

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiUnregisterHub(const char *url);



/*****************************************************************************
  Function:
    int yUpdateDeviceList(char *errmsg)

  Description:
    Detect or redetect all Yocto devices. This function can be called multiples
    times to refresh the devices list.

  Parameters:
    errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

  Returns:
    on ERROR   : an error code
    on SUCCES : YAPI_SUCCESS

  Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiUpdateDeviceList(u32 forceupdate, char *errmsg);


/*****************************************************************************
 Function:
 YRETCODE yapiHandleEvents(char *errmsg)

 Description:
 Perform periodic polling tasks on USB queues, invoke callbacks, etc
 This function can be called either from application main loop (if any)
 or from a dedicated thread in a multithreaded application.

 Parameters:
 errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
 on ERROR   : error code
 on SUCCESS : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHandleEvents(char *errmsg);


/*****************************************************************************
 Function:
 YRETCODE yapiSleep(int duration_ms,char *errmsg)

 Description:
 Perform periodic polling tasks on USB queues, invoke callbacks, etc
 This function can be called either from application main loop (if any)
 or from a dedicated thread in a multithreaded application.

 Parameters:
 errmsg: a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
 on ERROR   : error code
 on SUCCESS : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiSleep(int duration_ms, char *errmsg);


/*****************************************************************************
 Function:
 u64 yGetTickCount()

 Description:
 Return a the current value of a monotone millisecond-based time counter

 Returns:
 Monotone millisecond-based time counter

 Remarks:
 ***************************************************************************/
u64 YAPI_FUNCTION_EXPORT yapiGetTickCount(void);


/*****************************************************************************
 Function:
 int yCheckLogicalName(const char *name)

 Description:
 Verifies if a given logical name is valid or not for this API

 Parameters:
 name: the logicalName to verify

 Returns:
 true (1) or false (0)

 Remarks:

 *****************************************************************************/
int YAPI_FUNCTION_EXPORT yapiCheckLogicalName(const char *name);

/*****************************************************************************
 Function:
 u16 yGetAPIVersion(char **version,char **subversion)

 Description:
 Return API Version

 Parameters:
 version: if not NULL this pointer will be updated with the version of the api
 build: if not NULL this pointer will be updated with the subversion of the api
 date : if not NULL this pointer will be updated with the date of the api

 Returns:
 Return the BCD encoded version number of the API

 Remarks:
 ***************************************************************************/
u16 YAPI_FUNCTION_EXPORT yapiGetAPIVersion(const char **version,const char **apidate);


//
/*****************************************************************************
 Function:
 YRETCODE SetTraceFile(const char *file)

 Description:
 Enable low-level traces of the API into a specified file

 Parameters:
 file: the full path of the log file to use

 Remarks:
 This function is typically called even before yInitAPI.
 It is optional, and should only be called when low-level logs
 are desirable.
 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiSetTraceFile(const char *file);


/*****************************************************************************
 Function:
   YAPI_DEVICE yGetDevice(const char *device_str,char *errmsg)

 Description:
   return a device descriptor identifying a device provided by serial number,
   logical name or URL. The descriptor can later be used with yGetDeviceInfo
   to retrieve all properties of the device. The function will fail if there
   is no connected device matching the requested device string.

 Parameters:
   device_str : string referring to a Yocto device (serial number, logical name or URL)
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   check the result with the YISERR(retcode)
   on ERROR   : error code
   on SUCCESS : a valid YAPI_DEVICE descriptor

 Example:
   char      errmsg[YOCTO_ERRMSG_LEN];
   YAPI_DEVICE   mydevice = yGetDevice("logicalname");
   yDeviceSt infos;
   if(mydevice < 0 || yGetDeviceInfo(mydevice, &infos, errmsg) < 0) {
      // handle error
   }

 ***************************************************************************/
YAPI_DEVICE YAPI_FUNCTION_EXPORT yapiGetDevice(const char *device_str,char *errmsg);


/*****************************************************************************
  Function:
    int yGetAllDevices(YAPI_DEVICE *buffer,int maxsize,int *neededsize,char *errmsg)

  Description:
    fill buffer with device descriptors that can be used to access device details.
    If buffer == NULL the function will only set neededsize so that the caller can
    allocate a buffer of the right size and call again the function with a buffer
    of correct size.

  Parameters:
    buffer     : buffer to be filled with devices descriptors
    maxize     : size in byte of buffer
    neededsize : size in byte of buffer to pass to this function to get all devices
    errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

  Returns:
   check the ressult with the YISERR(retcode)
    on ERROR   : error code
    on SUCCESS : nb of devices descriptors written into buffer

  Example:
    char    errmsg[YOCTO_ERRMSG_LEN];
    YAPI_DEVICE *buffer;
    int     nbdev, buffsize;
    if(yGetAllDevices(NULL,0,&buffsize,errmsg) >= 0) {
        buffer = (YAPI_DEVICE *)malloc(buffsize);
        nbdev = yGetAllDevices(buffer,buffsize,&buffsize,errmsg);
    }

 ***************************************************************************/
int YAPI_FUNCTION_EXPORT yapiGetAllDevices(YAPI_DEVICE *buffer,int maxsize,int *neededsize,char *errmsg);


/*****************************************************************************
  Function:
   YRETCODE yGetDeviceInfo(YAPI_DEVICE devhdl, yDeviceSt *infos, char *errmsg)

  Description:
    Get all information about a single device given by its descriptor,
    as returned by yGetDevice or yGetAllDevices.

  Parameters:
    devdesc: device object returned by yGetDevice or yGetAllDevices
    errmsg:  a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

  Returns:
    on ERROR   : error code
    on SUCCESS : YAPI_SUCCESS

 Remarks:
    the call may fail if the device pointed by devhdl has been disconnected

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiGetDeviceInfo(YAPI_DEVICE devdesc,yDeviceSt *infos,char *errmsg);


/*****************************************************************************
 Function:
   YRETCODE yapiGetDevicePath(YAPI_DEVICE devdesc, char *rootdevice, char *path, int pathsize, int *neededsize, char *errmsg);

 Description:
   return the rootdevice and the path to access a device.

 Parameters:
   devdesc     : device descriptor returned by yapiGetAllDevices or yapiGetDevice
   rootdevice  : a pointer to a buffer of YOCTO_SERIAL_LEN characters for receiving the rootdevice
   path        : a pointer to a buffer of for storing the base path to add to your HTTP request
   pathtsize   : size of the path buffer
   neededsize  : size of the path buffer needed to be complete (may be NULL)
   errmsg      : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR   : error code
   on SUCCESS : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/

YRETCODE YAPI_FUNCTION_EXPORT yapiGetDevicePath(YAPI_DEVICE devdesc, char *rootdevice, char *path, int pathsize, int *neededsize, char *errmsg);


/*****************************************************************************
 Function:
   YRETCODE yapiGetDevicePathEx(const char * serial, char *rootdevice, char *path, int pathsize, int *neededsize, char *errmsg);

 Description:
   return the rootdevice and the path to access a device.

 Parameters:
   serila      : the serial number of the devcie
   rootdevice  : a pointer to a buffer of YOCTO_SERIAL_LEN characters for receiving the rootdevice
   path        : a pointer to a buffer of for storing the base path to add to your HTTP request
   pathtsize   : size of the path buffer
   neededsize  : size of the path buffer needed to be complete (may be NULL)
   errmsg      : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR   : error code
   on SUCCESS : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiGetDevicePathEx(const char * serial, char *rootdevice, char *request, int requestsize, int *neededsize, char *errmsg);


/*****************************************************************************
 Function:
   YAPI_FUNCTION yGetFunction(const char *class_str,const char *func_str,char *errmsg)

 Description:
   return a function descriptor identifying a function provided by full hardware id,
   logical name or mixed. The descriptor can later be used with yGetFunctionInfo
   to retrieve all yellow-page information or to access the function. This function
   fails if there is no connected device with the requested function identification.

 Parameters:
   class_str  : string refering to the function class
   func_str   : string refering to a Yocto function (hardware id, logical name, etc.)
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   check the ressult with the YISERR(retcode)
   on ERROR   : error code
   on SUCCESS : a valid YAPI_FUNCTION descriptor

 Example:
   char      errmsg[YOCTO_ERRMSG_LEN];
   YAPI_FUNCTION myfunction = yGetFunction("functionname");
   if(myfunction < 0 || yGetFunctionInfo(mydevice, &infos, errmsg) < 0) {
       // handle error
   }

 ***************************************************************************/
YAPI_FUNCTION YAPI_FUNCTION_EXPORT yapiGetFunction(const char *class_str, const char *function_str,char *errmsg);


/*****************************************************************************
 Function:
   int yGetFunctionsByClass(const char *class_str, YAPI_FUNCTION prevfundesc,
                            YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg);
   int yGetFunctionsByDevice(YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
                             YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg);

 Description:
   fill buffer with function descriptors that can be used to access functions.
   If buffer == NULL the function will only set neededsize so that the caller can
   allocate a buffer of the right size and call again the function with a buffer
   of correct size.

 Parameters:
   class_str  : string referring to the function class
 or
   devdesc    : device descriptor returned by yGetDevice or yGetAllDevices
   prevfundesc: 0 when calling for the first time, or last function descriptor previously received
   buffer     : buffer to be filled with function descriptors
   maxize     : size in byte of buffer
   neededsize : size in byte of buffer to pass to this function to get all functions
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   check the result with the YISERR(retcode)
   on ERROR   : error code
   on SUCCESS : nb of function descriptors written into buffer

 Example:
   char    errmsg[YOCTO_ERRMSG_LEN];
   YAPI_FUNCTION *buffer;
   int     nbfunc, buffsize;
   if(yGetFunctionsByClass("Relay",0,NULL,0,&buffsize,errmsg) >= 0) {
       buffer = (YAPI_FUNCTION *)malloc(buffsize);
       nbfunc = yGetFunctionsByClass("Relay",0,buffer,buffsize,&buffsize,errmsg);
   }

 ***************************************************************************/
int YAPI_FUNCTION_EXPORT yapiGetFunctionsByClass(const char *class_str, YAPI_FUNCTION prevfundesc,
                                          YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg);
int YAPI_FUNCTION_EXPORT yapiGetFunctionsByDevice(YAPI_DEVICE devdesc, YAPI_FUNCTION prevfundesc,
                                           YAPI_FUNCTION *buffer,int maxsize,int *neededsize,char *errmsg);


/*****************************************************************************
 Function:
   YRETCODE yGetFunctionInfo(YAPI_FUNCTION fundesc,YAPI_DEVICE *devdesc,char *serial,char *funcId,char *funcName,char *funcVal,char *errmsg)

 Description:
   Get all yellow-page information about a single function given by its descriptor,
   as returned by yGetFunction, yGetFunctionsByClass or yGetFunctionsByDevice.

 Parameters:
   fundesc : function descriptor returned by yGetFunction or yGetFunctionsByXXX
   devdesc : a pointer to a device descriptor to be filled with the device hosting the function
   serial  : a pointer to a buffer of YOCTO_SERIAL_LEN characters, or NULL
   funcId  : a pointer to a buffer of YOCTO_FUNCTION_LEN characters, or NULL
   funcName: a pointer to a buffer of YOCTO_LOGICAL_LEN characters, or NULL
   funcVal : a pointer to a buffer of 7 characters, or NULL
   errmsg  : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR   : error code
   on SUCCESS : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiGetFunctionInfo(YAPI_FUNCTION fundesc,YAPI_DEVICE *devdesc,char *serial,char *funcId,char *funcName,char *funcVal,char *errmsg);

/*****************************************************************************
 Function:
   YRETCODE yGetFunctionInfo(YAPI_FUNCTION fundesc,YAPI_DEVICE *devdesc,char *serial,char *funcId,char *funcName,char *funcVal,char *errmsg)

 Description:
   Get all yellow-page information about a single function given by its descriptor,
   as returned by yGetFunction, yGetFunctionsByClass or yGetFunctionsByDevice.

 Parameters:
   fundesc : function descriptor returned by yGetFunction or yGetFunctionsByXXX
   devdesc : a pointer to a device descriptor to be filled with the device hosting the function
   serial  : a pointer to a buffer of YOCTO_SERIAL_LEN characters, or NULL
   funcId  : a pointer to a buffer of YOCTO_FUNCTION_LEN characters, or NULL
   baseType: a pointer to a buffer of YOCTO_FUNCTION_LEN characters, or NULL
   funcName: a pointer to a buffer of YOCTO_LOGICAL_LEN characters, or NULL
   funcVal : a pointer to a buffer of 7 characters, or NULL
   errmsg  : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR   : error code
   on SUCCESS : YAPI_SUCCESS

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiGetFunctionInfoEx(YAPI_FUNCTION fundesc, YAPI_DEVICE *devdesc, char *serial, char *funcId, char *baseType, char *funcName, char *funcVal, char *errmsg);


 /*****************************************************************************
  Function:
    int yapiHTTPRequestSyncStartEx(YIOHDL *iohdl, const char *device, const char *request, int requestsize, char **reply, int *replysize, char *errmsg);

  Description:
    Open a HTTP request to a given device, send a query and receive the HTTP header and
    page content into the buffer. The buffer with result will be returned by reference,
    so that the caller can use it or copy it. Do not free reply buffer manually, but
    always call yapiHTTPRequestSyncDone when finished.

  Parameters:
    iohdl      : the request handle that will be initialized
    device     : a string that contain one of the flowing value: serial, logicalname, url
    request    : the HTTP request (HTTP header + body, in case of POST) of the page/file to retreive
    requestsize: the length of the HTTP request
    reply      : a pointer to the reply buffer, returned by reference
    replysize  : the length of the reply buffer, returned by reference
    errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

  Returns:
    on SUCCESS : YAPI_SUCCESS
    on ERROR   : return the YRETCODE

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncStartEx(YIOHDL *iohdl, const char *device, const char *request, int requestsize, char **reply, int *replysize, char *errmsg);

/*****************************************************************************
Function:
int yapiHTTPRequestSyncStartOutOfBand(YIOHDL *iohdl, const char *device, const char *request, int requestsize, char **reply, int *replysize, yapiRequestProgressCallback progress_cb, void *progress_ctx, char *errmsg);

Description:
Open a HTTP request to a given device, send a query and receive the HTTP header and
page content into the buffer. The buffer with result will be returned by reference,
so that the caller can use it or copy it. Do not free reply buffer manually, but
always call yapiHTTPRequestSyncDone when finished. 

Parameters:
iohdl        : the request handle that will be initialized
channel      : channel to use for the request
device       : a string that contain one of the flowing value: serial, logicalname, url
request      : the HTTP request (HTTP header + body, in case of POST) of the page/file to retreive
requestsize  : the length of the HTTP request
reply        : a pointer to the reply buffer, returned by reference
replysize    : the length of the reply buffer, returned by reference
progress_cb  : a callback that is called to report progress
progress_ctx : context passed to progress_cb 
errmsg       : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

Returns:
on SUCCESS : YAPI_SUCCESS
on ERROR   : return the YRETCODE

***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncStartOutOfBand(YIOHDL *iohdl, int channel, const char *device, const char *request, int requestsize, char **reply, int *replysize, yapiRequestProgressCallback progress_cb, void *progress_ctx, char *errmsg);


/*****************************************************************************
 Function:
 int yapiHTTPRequestSyncStart(YIOHDL *iohdl, const char *device, const char *request, char **reply, int *replysize, char *errmsg);

 Description:
 Open a HTTP request to a given device, send a query and receive the HTTP header and
 page content into the buffer. The buffer with result will be returned by reference,
 so that the caller can use it or copy it. Do not free reply buffer manually, but
 always call yapiHTTPRequestSyncDone when finished.

 Parameters:
 iohdl      : the request handle that will be initialized
 device     : a string that contain one of the flowing value: serial, logicalname, url
 request    : the HTTP request (HTTP header) of the page/file to retreive
 reply      : a pointer to the reply buffer, returned by reference
 replysize  : the length of the reply buffer, returned by reference
 errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
 on SUCCESS : YAPI_SUCCESS
 on ERROR   : return the YRETCODE

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncStart(YIOHDL *iohdl, const char *device, const char *request, char **reply, int *replysize, char *errmsg);


/*****************************************************************************
 Function:
   int yapiHTTPRequestSyncDone(YIOHDL *iohdl, char *errmsg)

 Description:
   Terminate a call to yapiHTTPRequestSyncStart and free corresponding ressources.
   No other request can take place to the device until this function is called.

 Parameters:
   iohdl      : the request handle returned by yapiHTTPRequestSyncStart
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on SUCCESS : YAPI_SUCCESS
   on ERROR   : return the YRETCODE

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestSyncDone(YIOHDL *iohdl, char *errmsg);


/*****************************************************************************
 Function:
   YRETCODE yapiHTTPRequestAsync(const char *device, const char *request, yapiRequestAsyncCallback callback, void *context, char *errmsg);

 Description:
   Execute a HTTP request to a given device, and leave it to the API to complete the request.

 Parameters:
   device     : a string that contain one of the flowing value: serial, logicalname, url
   request    : the HTTP request (HTTP hreader) of the page/file to retreive
   callback   : RESERVED FOR FUTURE USE
   context    : RESERVED FOR FUTURE USE
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR   : an error code
   on SUCCESS : YAPI_SUCCESS

 Remarks:
   we match the device string in this order: serial,logicalname,url

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestAsync(const char *device, const char *request, yapiRequestAsyncCallback callback, void *context, char *errmsg);

/*****************************************************************************
 Function:
   YRETCODE yapiHTTPRequestAsync(const char *device, const char *request, yapiRequestAsyncCallback callback, void *context, char *errmsg);

 Description:
   Execute a HTTP request to a given device, and leave it to the API to complete the request.

 Parameters:
   device     : a string that contain one of the flowing value: serial, logicalname, url
   request    : the HTTP request (HTTP header + body, in case of POST) of the page/file to retreive
   requestsize: the length of the HTTP request
   callback   : RESERVED FOR FUTURE USE
   context    : RESERVED FOR FUTURE USE
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   on ERROR   : an error code
   on SUCCESS : YAPI_SUCCESS

 Remarks:
   we match the device string in this order: serial,logicalname,url

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestAsyncEx(const char *device, const char *request, int requestsize, yapiRequestAsyncCallback callback, void *context, char *errmsg);


/*****************************************************************************
Function:
YRETCODE yapiHTTPRequestAsync(const char *device, const char *request, yapiRequestAsyncCallback callback, void *context, char *errmsg);

Description:
Execute a HTTP request to a given device, and leave it to the API to complete the request.

Parameters:
channel    : channel to use for the request
device     : a string that contain one of the flowing value: serial, logicalname, url
request    : the HTTP request (HTTP header + body, in case of POST) of the page/file to retreive
requestsize: the length of the HTTP request
callback   : RESERVED FOR FUTURE USE
context    : RESERVED FOR FUTURE USE
errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

Returns:
on ERROR   : an error code
on SUCCESS : YAPI_SUCCESS

Remarks:
we match the device string in this order: serial,logicalname,url

***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiHTTPRequestAsyncOutOfBand(int channel, const char *device, const char *request, int requestsize, yapiRequestAsyncCallback callback, void *context, char *errmsg);




/*****************************************************************************
 Function:
   int yHTTPRequest(char *device,char *request, char* buffer,int buffsize,int *fullsize,char *errmsg)

 Description:
   Open a HTTP request to a given device, send a query and receive the HTTP header and
   page content into the buffer. If the content is bigger than buffsize, buffer will contain
   no more than buffsize. If not NULL fullsize will be updated with the size of the full content.

 Parameters:
   device     : a string that contain one of the flowing value: serial, logicalname, url
   request    : the HTTP request (HTTP hreader) of the page/file to retreive
   buffer     : a buffer to fill with result
   buffsize   : the size of the buffer
   fullsize   : (optional) an integer to update with the full size of the data
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   check the result with the YISERR(retcode)
   on SUCCESS : return the number of byte written into buffer (excluding the terminating '\0' character)
   on ERROR   : return the YRETCODE

 Remarks:
   we always null terminate the returning buffer

 ***************************************************************************/
int YAPI_FUNCTION_EXPORT yapiHTTPRequest(const char *device, const char *request, char* buffer,int buffsize,int *fullsize, char *errmsg);

/*****************************************************************************
 Function:
   void yapiRegisterHubDiscoveryCallback(yapiHubDiscoveryCallback hubDiscoveryCallback);

 Description:
   register a callback function that will be called on every network hub  (or VirtualHub)
   that send an SSDP annouce or respond to a SSDP search

 Parameters:
   hubDiscoveryCallback : the function to call when an network hub his detected by ssdp or null to
                          unregister the prévious callback

 Returns:

 Remarks:

 ***************************************************************************/
void YAPI_FUNCTION_EXPORT yapiRegisterHubDiscoveryCallback(yapiHubDiscoveryCallback hubDiscoveryCallback);

/*****************************************************************************
 Function:
   YRETCODE YAPI_FUNCTION_EXPORT yapiTriggerHubDiscovery(char *errmsg);

 Description:
   Send an SSDP Msearch Request to force all online hub to annouce itself again.

 Parameters:
   errmsg     : a pointer to a buffer of YOCTO_ERRMSG_LEN bytes to store any error message

 Returns:
   check the result with the YISERR(retcode)
   on ERROR   : return the YRETCODE

 Remarks:

 ***************************************************************************/
YRETCODE YAPI_FUNCTION_EXPORT yapiTriggerHubDiscovery(char *errmsg);



YRETCODE YAPI_FUNCTION_EXPORT yapiGetSubdevices(const char *serial, char *buffer, int buffersize, int *fullsize, char *errmsg);

/*****************************************************************************
  Flash API
 ***************************************************************************/

YRETCODE YAPI_FUNCTION_EXPORT yapiGetAllJsonKeys(const char *jsonbuffer, char *out_buffer, int out_buffersize, int *fullsize, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT yapiCheckFirmware(const char *serial, const char *rev, const char *path, char *buffer, int buffersize, int *fullsize, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT yapiGetBootloaders(char *buffer, int buffersize, int *fullsize, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT yapiUpdateFirmware(const char *serial, const char *firmwarePath, const char *settings, int startUpdate, char *errmsg);
YRETCODE YAPI_FUNCTION_EXPORT yapiUpdateFirmwareEx(const char *serial, const char *firmwarePath, const char *settings, int force, int startUpdate, char *errmsg);

int YAPI_FUNCTION_EXPORT yapiJsonDecodeString(const char *json_string, char *output);
int YAPI_FUNCTION_EXPORT yapiJsonGetPath(const char *path, const char *json_data, int json_size, const char  **result, char *errmsg);


/*****************************************************************************
  helper for delphi
 ***************************************************************************/
YAPI_FUNCTION_EXPORT void* yapiGetMem(int size);
YAPI_FUNCTION_EXPORT void yapiFreeMem(void *ptr);


typedef  void (*yRawNotificationCb)(USB_Notify_Pkt*);
typedef  void (*yRawReportCb)(YAPI_DEVICE serialref, USB_Report_Pkt_V1 *report, int pktsize);
typedef  void (*yRawReportV2Cb)(YAPI_DEVICE serialref, USB_Report_Pkt_V2 *report, int pktsize);
void yapiRegisterRawNotificationCb(yRawNotificationCb callback);
void yapiRegisterRawReportCb(yRawReportCb callback);
void yapiRegisterRawReportV2Cb(yRawReportV2Cb callback);




#ifdef  __cplusplus
}
#endif
#endif


