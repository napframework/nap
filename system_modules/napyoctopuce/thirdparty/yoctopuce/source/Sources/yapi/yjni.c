/*********************************************************************
 *
 * $Id: yjni.c 21960 2015-11-06 15:59:59Z seb $
 *
 * Implementation of public entry points to the low-level API
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
#define __FILE_ID__  "yjni"

#include "yapi.h"
#include "yproto.h"
#include "yhash.h"
#include "yjson.h"
#include "yprog.h"

#ifdef YAPI_WITH_JNI

#include <jni.h>
#include <stdio.h>
#include "yjni.h"



 jint throwYAPI_Exception( JNIEnv *env, char *message )
{
    jclass exClass;
    char *className = "com/yoctopuce/YoctoAPI/YAPI_Exception" ;
    dbglog("Exception:%s\n", message);
    exClass = (*env)->FindClass( env, className );
    return (*env)->ThrowNew( env, exClass, message );
}


/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    getAPIVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_getAPIVersion(JNIEnv *env, jclass thisObj)
{
    const char *version;
    const char *apidate;
    yapiGetAPIVersion(&version, &apidate);

    return (*env)->NewStringUTF(env, version);
}


/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    reserveUSBAccess
 * Signature: ()V;
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_reserveUSBAccess(JNIEnv *env, jclass thisObj)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    YRETCODE res;

    res = yapiInitAPI(Y_DETECT_USB, errmsg);
    if(YISERR(res)) {
        throwYAPI_Exception(env, errmsg);
    }
}

/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    releaseUSBAccess
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_releaseUSBAccess(JNIEnv *env, jclass thisObj)
{
    yapiFreeAPI();
}


/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    getBootloaders
 * Signature: ()Ljava/util/String;
 */
JNIEXPORT jobject JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_getBootloaders(JNIEnv *env, jclass thisObj)
{
    char errmsg[YOCTO_ERRMSG_LEN];
    char buffer[1024];
    char *p;
    int fullsize;
    YRETCODE res;
    jobject result;


    res = yapiGetBootloaders(buffer, 1024, &fullsize, errmsg);
    if(YISERR(res)) {
        throwYAPI_Exception(env, errmsg);
        return NULL;
    }
    if (res == fullsize) {
        return (*env)->NewStringUTF(env, buffer);
    }

    p = yMalloc(fullsize+1);
    memset(p, 0, fullsize+1);
    res = yapiGetBootloaders(buffer, fullsize, &fullsize, errmsg);
    if(YISERR(res)) {
        yFree(p);
        throwYAPI_Exception(env, errmsg);
        return NULL;
    }
    result = (*env)->NewStringUTF(env, buffer);
    yFree(p);
    return result;
}


static jobject allocWPEntry(JNIEnv *env, yDeviceSt *infos)
{
    jstring logicalName;
    jstring productName;
    jint productId;
    jstring networkUrl;
    jint beacon;
    jstring serialNumber;
    jobject res;
    jmethodID constructor;


    jclass cls = (*env)->FindClass(env, "com/yoctopuce/YoctoAPI/WPEntry");
    if (cls == 0) {
        throwYAPI_Exception(env, "Unable to find class WPEntry");
        return NULL;
    }

    constructor = (*env)->GetMethodID(env, cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;)V");
    if (constructor == 0) {
        throwYAPI_Exception(env, "Unable to find constructor for WPEntry");
        return NULL;
    }

    logicalName = (*env)->NewStringUTF(env, infos->logicalname);
    productName = (*env)->NewStringUTF(env, infos->productname);
    networkUrl = (*env)->NewStringUTF(env, "");
    serialNumber = (*env)->NewStringUTF(env, infos->serial);
    productId = infos->vendorid;
    beacon = 0; //fixme : use real beacon vallue

    res = (*env)->NewObject(env, cls, constructor, logicalName, productName, productId, networkUrl, beacon, serialNumber);
    return res;

}



static jobject allocYPEntry(JNIEnv *env, const char *classname, const char *serial, const char *funcId, const char *logicalName, const char *advertisedValue, int baseType, int funIdx)
{
    jstring j_classname;
    jstring j_serial;
    jstring j_funcId;
    jstring j_logicalName;
    jstring j_advertisedValue;
    jmethodID constructor;

    jclass cls = (*env)->FindClass(env, "com/yoctopuce/YoctoAPI/YPEntry");
    if (cls == 0) {
        throwYAPI_Exception(env, "Unable to find class WPEntry");
        return NULL;
    }
    constructor = (*env)->GetMethodID(env, cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)V");
    if (constructor == 0) {
        throwYAPI_Exception(env, "Unable to find constructor for YPEntry");
        return NULL;
    }
    j_classname = (*env)->NewStringUTF(env, classname);
    j_serial = (*env)->NewStringUTF(env, serial);
    j_funcId = (*env)->NewStringUTF(env, funcId);
    j_logicalName = (*env)->NewStringUTF(env, logicalName);
    j_advertisedValue = (*env)->NewStringUTF(env, advertisedValue);
    return (*env)->NewObject(env, cls, constructor, j_classname, j_serial, j_funcId, j_logicalName, j_advertisedValue, baseType, funIdx);
}




/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    updateDeviceList
 * Signature: (Ljava/util/ArrayList;Ljava/util/HashMap;)V
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_updateDeviceList(JNIEnv *env, jclass thisObj, jobject wpArray, jobject ypArray)
{
    char    errmsg[YOCTO_ERRMSG_LEN];
    YAPI_DEVICE *buffer, *dev_ptr;
    int     nbdev, buffsize, i;
    yBlkHdl categ;

    jobject wpEntry, ypEntry;
    jclass arrayList_class;
    jmethodID arrayList_add;

    if (yapiUpdateDeviceList(1, errmsg) < 0) {
        throwYAPI_Exception(env, errmsg);
        return;
    }

    // populate white pages
    if (yapiGetAllDevices(NULL, 0, &buffsize, errmsg) < 0) {
        throwYAPI_Exception(env, errmsg);
        return;
    }
    buffer = (YAPI_DEVICE *)yMalloc(buffsize);
    nbdev = yapiGetAllDevices(buffer, buffsize, &buffsize, errmsg);

    arrayList_class = (*env)->FindClass(env, "java/util/ArrayList");
    if (arrayList_class == 0) {
        throwYAPI_Exception(env, "Unable to find class ArrayList");
        return;
    }

    arrayList_add = (*env)->GetMethodID(env, arrayList_class, "add", "(Ljava/lang/Object;)Z");
    if (arrayList_add == 0) {
        throwYAPI_Exception(env, "Unable to find add method of ArrayList");
        return;
    }

    dev_ptr = buffer;
    for (i = 0 ; i < nbdev; i++, dev_ptr++) {
        yDeviceSt dev_info;
        if (yapiGetDeviceInfo(*dev_ptr, &dev_info, errmsg) < 0) {
            throwYAPI_Exception(env, errmsg);
            return;
        }
        wpEntry = allocWPEntry(env, &dev_info);
        if (wpEntry ==NULL) {
            return;
        }
        (*env)->CallBooleanMethod(env, wpArray, arrayList_add, wpEntry);
    }
    yFree(buffer);

    // populate Yellow pages
    categ = yYpListHead;
    for (categ = yYpListHead; categ != INVALID_BLK_HDL; categ = yBlkListSeek(categ, 1)) {
        char categname[YOCTO_FUNCTION_LEN];
        yBlkHdl entry;

        ypGetCategory(categ, categname, &entry);
        if (YSTRCMP(categname,"Module")==0){
            continue;
        }

        // add all Yellow pages
        for (; entry != INVALID_BLK_HDL; entry = yBlkListSeek(entry, 1)) {
            yStrRef serial, funcId, funcName;
            Notification_funydx funcInfo;
            int  yidx, baseType;
            char pubRaw[YOCTO_PUBVAL_SIZE];
            char pubDecoded[YOCTO_PUBVAL_LEN];
            yidx =  ypGetAttributes(entry, &serial, &funcId, &funcName, &funcInfo, pubRaw);
            baseType = ypGetType(entry);
            decodePubVal(funcInfo, pubRaw, pubDecoded);
            ypEntry = allocYPEntry(env, categname, yHashGetStrPtr(serial), yHashGetStrPtr(funcId), yHashGetStrPtr(funcName),
                pubDecoded, baseType, yidx);
            if (ypEntry == NULL) {
                return;
            }
            (*env)->CallBooleanMethod(env, ypArray, arrayList_add, ypEntry);
        }
    }

}


/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    devRequestSync
 * Signature: (Ljava/lang/String;[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_devRequestSync(JNIEnv *env, jclass thisObj, jstring serial_java, jbyteArray request_java)
{
    char        errmsg[YOCTO_ERRMSG_LEN];
    YRETCODE    res;
    YIOHDL      iohdl;
    char        *reply;
    int         replysize = 0;
    const char  *serial = NULL;
    jbyte       *request_bytes = NULL;
    jsize       length;
    jbyteArray  result = NULL;

    // get serial
    serial = (*env)->GetStringUTFChars(env, serial_java, NULL);
    if (NULL == serial) {
        throwYAPI_Exception(env, "Invalid String");
        goto exit;
    }

    // get request
    request_bytes = (*env)->GetByteArrayElements(env, request_java, NULL);
    if (NULL == request_bytes) {
        throwYAPI_Exception(env, "Invalid Byte Array");
        goto exit;
    }
    length = (*env)->GetArrayLength(env, request_java);

    if(YISERR(res = yapiHTTPRequestSyncStartEx(&iohdl, serial, (const char *)request_bytes, length, &reply, &replysize, errmsg))) {
        throwYAPI_Exception(env, errmsg);
        goto exit;
    }
    if (replysize < 0 || reply == NULL) {
        replysize = 0;
    }

   // compute return value
   result = (*env)->NewByteArray(env, replysize);  // allocate
   if (NULL == result) {
        throwYAPI_Exception(env, "Unable to allocate bytes array");
        goto exit;
    }

    if (replysize > 0) {
        (*env)->SetByteArrayRegion(env, result, 0 , replysize, (jbyte*)reply); // copy
    }

    if(YISERR(res=yapiHTTPRequestSyncDone(&iohdl, errmsg))) {
        throwYAPI_Exception(env, errmsg);
    }

exit:
    if (serial != NULL) {
        (*env)->ReleaseStringUTFChars(env, serial_java, serial);  // release resources
    }

    if (request_bytes != NULL) {
       (*env)->ReleaseByteArrayElements(env, request_java, request_bytes, 0); // release resources
    }
    return result;
}


/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    devRequestAsync
 * Signature: (Ljava/lang/String;[BLcom/yoctopuce/YoctoAPI/YGenericHub/RequestAsyncResult;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_devRequestAsync(JNIEnv *env, jclass thisObj, jstring serial_java, jbyteArray request_java, jobject callback, jobject context)
{
    char        errmsg[YOCTO_ERRMSG_LEN];
    YRETCODE    res;
    const char  *serial = NULL;
    jbyte       *request_bytes = NULL;
    jsize       length;

    // get serial
    serial = (*env)->GetStringUTFChars(env, serial_java, NULL);
    if (NULL == serial) {
        throwYAPI_Exception(env, "Invalid String");
        goto exit;
    }

    // get request
    request_bytes = (*env)->GetByteArrayElements(env, request_java, NULL);
    if (NULL == request_bytes) {
        throwYAPI_Exception(env, "Invalid Byte Array");
        goto exit;
    }
    length = (*env)->GetArrayLength(env, request_java);


    if(YISERR(res=yapiHTTPRequestAsyncEx(serial, (const char *)request_bytes, length, NULL, NULL, errmsg))) {
        throwYAPI_Exception(env, errmsg);
        goto exit;
    }
    //Todo: handle correctly callback

exit:
    if (serial != NULL) {
        (*env)->ReleaseStringUTFChars(env, serial_java, serial);  // release resources
    }

    if (request_bytes != NULL) {
       (*env)->ReleaseByteArrayElements(env, request_java, request_bytes, 0); // release resources
    }
}


static JavaVM *jvm;

static jobject jObj;

static JNIEnv* getThreadEnv()
{
    JNIEnv *env;
    // double check it's all ok
    int getEnvStat = (*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        dbglog("GetEnv: not attached\n");
        if ((*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL) != 0) {
             dbglog("Failed to attach\n");
             return NULL;
        }
    } else if (getEnvStat == JNI_OK) {
        //dbglog("attached\n");
    } else if (getEnvStat == JNI_EVERSION) {
         dbglog("GetEnv: version not supported\n");
         return NULL;
    }
    return env;
}


static void jFunctionUpdateCallbackFwd(YAPI_FUNCTION fundesc,const char *value)
{
    char serial[YOCTO_SERIAL_LEN];
    char funcId[YOCTO_FUNCTION_LEN];
    jstring j_serial;
    jstring j_funcId;
    jstring j_value;
    jclass yUSBHub_class;
    jmethodID yUSBHub_handleValueNotification;
    JNIEnv *env;

    if (value==NULL){
        return;
    }

    env = getThreadEnv();
    if (env == NULL){
        return;
    }

    ypGetFunctionInfo(fundesc, serial, funcId,NULL, NULL, NULL);
    j_serial = (*env)->NewStringUTF(env, serial);
    j_funcId = (*env)->NewStringUTF(env, funcId);
    j_value = (*env)->NewStringUTF(env, value);

    yUSBHub_class = (*env)->FindClass(env, "com/yoctopuce/YoctoAPI/YUSBHub");
    if (yUSBHub_class == 0) {
        dbglog("Unable to find class YUSBHub\n");
        return;
    }


    yUSBHub_handleValueNotification = (*env)->GetMethodID(env, yUSBHub_class, "handleValueNotification", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (yUSBHub_handleValueNotification == 0) {
        dbglog("Unable to find add method of handleValueNotification\n");
        return;
    }

    (*env)->CallVoidMethod(env, jObj, yUSBHub_handleValueNotification, j_serial, j_funcId, j_value);
}

static void jFunctionTimedReportCallbackFwd(YAPI_FUNCTION fundesc, double timestamp, const u8 *bytes, u32 len)
{
    char serial[YOCTO_SERIAL_LEN];
    char funcId[YOCTO_FUNCTION_LEN];
    jstring j_serial;
    jstring j_funcId;
    jbyteArray  result = NULL;
    jclass yUSBHub_class;
    jmethodID YUSBHub_handleTimedNotification;
    JNIEnv *env;

    env = getThreadEnv();
    if (env == NULL){
        return;
    }

    ypGetFunctionInfo(fundesc, serial, funcId, NULL, NULL, NULL);
    j_serial = (*env)->NewStringUTF(env, serial);
    j_funcId = (*env)->NewStringUTF(env, funcId);

    yUSBHub_class = (*env)->FindClass(env, "com/yoctopuce/YoctoAPI/YUSBHub");
    if (yUSBHub_class == 0) {
        dbglog("Unable to find class YUSBHub\n");
        return;
    }

    YUSBHub_handleTimedNotification = (*env)->GetMethodID(env, yUSBHub_class, "handleTimedNotification", "(Ljava/lang/String;Ljava/lang/String;D[B)V");
    if (YUSBHub_handleTimedNotification == 0) {
        dbglog("Unable to find add method of handleTimedNotification\n");
        return;
    }


   // compute return value
   result = (*env)->NewByteArray(env, len);  // allocate
   if (NULL == result) {
        dbglog("Unable to allocate bytes array");
        return;
    }

    (*env)->SetByteArrayRegion(env, result, 0 , len, (jbyte*) bytes);  // copy

    (*env)->CallVoidMethod(env, jObj, YUSBHub_handleTimedNotification, j_serial, j_funcId, timestamp, result);
}



/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    startNotifications
 * Signature: (Lcom/yoctopuce/YoctoAPI/YUSBHub;)V
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_startNotifications(JNIEnv * env, jclass thisObj, jobject yUSBHubRef)
{

    if ((*env)->GetJavaVM(env, &jvm) != 0) {
        throwYAPI_Exception(env, "GetJavaVM: Unable to get VM");
        return;
    }
    jObj = (*env)->NewGlobalRef(env, yUSBHubRef);
    yapiRegisterFunctionUpdateCallback(jFunctionUpdateCallbackFwd);
    yapiRegisterTimedReportCallback(jFunctionTimedReportCallbackFwd);
}

/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    stopNotifications
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_stopNotifications(JNIEnv * env, jclass thisObj)
{
    yapiRegisterFunctionUpdateCallback(NULL);
    yapiRegisterTimedReportCallback(NULL);
    (*env)->DeleteGlobalRef(env, jObj);
    jObj = NULL;
    jvm = NULL;
}

/*
 * Class:     com_yoctopuce_YoctoAPI_YJniWrapper
 * Method:    usbProcess
 * Signature: (Lcom/yoctopuce/YoctoAPI/YUSBHub;)V
 */
JNIEXPORT void JNICALL Java_com_yoctopuce_YoctoAPI_YJniWrapper_usbProcess(JNIEnv *env, jclass thisObj, jobject yUSBHubRef)
{
    char        errmsg[YOCTO_ERRMSG_LEN];
    YRETCODE res;

    if(YISERR(res=yapiHandleEvents(errmsg))) {
        throwYAPI_Exception(env, errmsg);
    }
}



#endif
