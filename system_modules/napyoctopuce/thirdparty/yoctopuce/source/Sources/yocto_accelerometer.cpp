/*********************************************************************
 *
 * $Id: yocto_accelerometer.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindAccelerometer(), the high-level API for Accelerometer functions
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
#include "yocto_accelerometer.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "accelerometer"

YAccelerometer::YAccelerometer(const string& func): YSensor(func)
//--- (YAccelerometer initialization)
    ,_bandwidth(BANDWIDTH_INVALID)
    ,_xValue(XVALUE_INVALID)
    ,_yValue(YVALUE_INVALID)
    ,_zValue(ZVALUE_INVALID)
    ,_gravityCancellation(GRAVITYCANCELLATION_INVALID)
    ,_valueCallbackAccelerometer(NULL)
    ,_timedReportCallbackAccelerometer(NULL)
//--- (end of YAccelerometer initialization)
{
    _className="Accelerometer";
}

YAccelerometer::~YAccelerometer()
{
//--- (YAccelerometer cleanup)
//--- (end of YAccelerometer cleanup)
}
//--- (YAccelerometer implementation)
// static attributes
const double YAccelerometer::XVALUE_INVALID = YAPI_INVALID_DOUBLE;
const double YAccelerometer::YVALUE_INVALID = YAPI_INVALID_DOUBLE;
const double YAccelerometer::ZVALUE_INVALID = YAPI_INVALID_DOUBLE;

int YAccelerometer::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("bandwidth")) {
        _bandwidth =  json_val->getInt("bandwidth");
    }
    if(json_val->has("xValue")) {
        _xValue =  floor(json_val->getDouble("xValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("yValue")) {
        _yValue =  floor(json_val->getDouble("yValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("zValue")) {
        _zValue =  floor(json_val->getDouble("zValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("gravityCancellation")) {
        _gravityCancellation =  (Y_GRAVITYCANCELLATION_enum)json_val->getInt("gravityCancellation");
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Returns the measure update frequency, measured in Hz (Yocto-3D-V2 only).
 *
 * @return an integer corresponding to the measure update frequency, measured in Hz (Yocto-3D-V2 only)
 *
 * On failure, throws an exception or returns Y_BANDWIDTH_INVALID.
 */
int YAccelerometer::get_bandwidth(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YAccelerometer::BANDWIDTH_INVALID;
                }
            }
        }
        res = _bandwidth;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the measure update frequency, measured in Hz (Yocto-3D-V2 only). When the
 * frequency is lower, the device performs averaging.
 *
 * @param newval : an integer corresponding to the measure update frequency, measured in Hz (Yocto-3D-V2 only)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YAccelerometer::set_bandwidth(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("bandwidth", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the X component of the acceleration, as a floating point number.
 *
 * @return a floating point number corresponding to the X component of the acceleration, as a floating point number
 *
 * On failure, throws an exception or returns Y_XVALUE_INVALID.
 */
double YAccelerometer::get_xValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YAccelerometer::XVALUE_INVALID;
                }
            }
        }
        res = _xValue;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the Y component of the acceleration, as a floating point number.
 *
 * @return a floating point number corresponding to the Y component of the acceleration, as a floating point number
 *
 * On failure, throws an exception or returns Y_YVALUE_INVALID.
 */
double YAccelerometer::get_yValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YAccelerometer::YVALUE_INVALID;
                }
            }
        }
        res = _yValue;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the Z component of the acceleration, as a floating point number.
 *
 * @return a floating point number corresponding to the Z component of the acceleration, as a floating point number
 *
 * On failure, throws an exception or returns Y_ZVALUE_INVALID.
 */
double YAccelerometer::get_zValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YAccelerometer::ZVALUE_INVALID;
                }
            }
        }
        res = _zValue;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

Y_GRAVITYCANCELLATION_enum YAccelerometer::get_gravityCancellation(void)
{
    Y_GRAVITYCANCELLATION_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YAccelerometer::GRAVITYCANCELLATION_INVALID;
                }
            }
        }
        res = _gravityCancellation;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YAccelerometer::set_gravityCancellation(Y_GRAVITYCANCELLATION_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = (newval>0 ? "1" : "0");
        res = _setAttr("gravityCancellation", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves an accelerometer for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the accelerometer is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YAccelerometer.isOnline() to test if the accelerometer is
 * indeed online at a given time. In case of ambiguity when looking for
 * an accelerometer by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the accelerometer
 *
 * @return a YAccelerometer object allowing you to drive the accelerometer.
 */
YAccelerometer* YAccelerometer::FindAccelerometer(string func)
{
    YAccelerometer* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YAccelerometer*) YFunction::_FindFromCache("Accelerometer", func);
        if (obj == NULL) {
            obj = new YAccelerometer(func);
            YFunction::_AddToCache("Accelerometer", func, obj);
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
int YAccelerometer::registerValueCallback(YAccelerometerValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackAccelerometer = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YAccelerometer::_invokeValueCallback(string value)
{
    if (_valueCallbackAccelerometer != NULL) {
        _valueCallbackAccelerometer(this, value);
    } else {
        YSensor::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Registers the callback function that is invoked on every periodic timed notification.
 * The callback is invoked only during the execution of ySleep or yHandleEvents.
 * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
 * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
 *
 * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
 *         arguments: the function object of which the value has changed, and an YMeasure object describing
 *         the new advertised value.
 * @noreturn
 */
int YAccelerometer::registerTimedReportCallback(YAccelerometerTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackAccelerometer = callback;
    return 0;
}

int YAccelerometer::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackAccelerometer != NULL) {
        _timedReportCallbackAccelerometer(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

YAccelerometer *YAccelerometer::nextAccelerometer(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YAccelerometer::FindAccelerometer(hwid);
}

YAccelerometer* YAccelerometer::FirstAccelerometer(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Accelerometer", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YAccelerometer::FindAccelerometer(serial+"."+funcId);
}

//--- (end of YAccelerometer implementation)

//--- (YAccelerometer functions)
//--- (end of YAccelerometer functions)
