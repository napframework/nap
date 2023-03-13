/*********************************************************************
 *
 * $Id: yocto_oscontrol.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindOsControl(), the high-level API for OsControl functions
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
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED 'AS IS' WITHOUT
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


#define _CRT_SECURE_NO_DEPRECATE //do not use windows secure crt
#include "yocto_oscontrol.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "oscontrol"

YOsControl::YOsControl(const string& func): YFunction(func)
//--- (YOsControl initialization)
    ,_shutdownCountdown(SHUTDOWNCOUNTDOWN_INVALID)
    ,_valueCallbackOsControl(NULL)
//--- (end of YOsControl initialization)
{
    _className="OsControl";
}

YOsControl::~YOsControl()
{
//--- (YOsControl cleanup)
//--- (end of YOsControl cleanup)
}
//--- (YOsControl implementation)
// static attributes

int YOsControl::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("shutdownCountdown")) {
        _shutdownCountdown =  json_val->getInt("shutdownCountdown");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the remaining number of seconds before the OS shutdown, or zero when no
 * shutdown has been scheduled.
 *
 * @return an integer corresponding to the remaining number of seconds before the OS shutdown, or zero when no
 *         shutdown has been scheduled
 *
 * On failure, throws an exception or returns Y_SHUTDOWNCOUNTDOWN_INVALID.
 */
int YOsControl::get_shutdownCountdown(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YOsControl::SHUTDOWNCOUNTDOWN_INVALID;
                }
            }
        }
        res = _shutdownCountdown;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YOsControl::set_shutdownCountdown(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("shutdownCountdown", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves OS control for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the OS control is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YOsControl.isOnline() to test if the OS control is
 * indeed online at a given time. In case of ambiguity when looking for
 * OS control by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the OS control
 *
 * @return a YOsControl object allowing you to drive the OS control.
 */
YOsControl* YOsControl::FindOsControl(string func)
{
    YOsControl* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YOsControl*) YFunction::_FindFromCache("OsControl", func);
        if (obj == NULL) {
            obj = new YOsControl(func);
            YFunction::_AddToCache("OsControl", func, obj);
        }
    } catch (std::exception) {
        if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
        throw;
    }
    if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
    return obj;
}

/**
 * Registers the callback function that is invoked on every change of advertised value.
 * The callback is invoked only during the execution of ySleep or yHandleEvents.
 * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
 * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
 *
 * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
 *         arguments: the function object of which the value has changed, and the character string describing
 *         the new advertised value.
 * @noreturn
 */
int YOsControl::registerValueCallback(YOsControlValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackOsControl = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YOsControl::_invokeValueCallback(string value)
{
    if (_valueCallbackOsControl != NULL) {
        _valueCallbackOsControl(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Schedules an OS shutdown after a given number of seconds.
 *
 * @param secBeforeShutDown : number of seconds before shutdown
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YOsControl::shutdown(int secBeforeShutDown)
{
    return this->set_shutdownCountdown(secBeforeShutDown);
}

YOsControl *YOsControl::nextOsControl(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YOsControl::FindOsControl(hwid);
}

YOsControl* YOsControl::FirstOsControl(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("OsControl", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YOsControl::FindOsControl(serial+"."+funcId);
}

//--- (end of YOsControl implementation)

//--- (YOsControl functions)
//--- (end of YOsControl functions)
