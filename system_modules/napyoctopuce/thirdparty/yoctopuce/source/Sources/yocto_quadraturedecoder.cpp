/*********************************************************************
 *
 * $Id: yocto_quadraturedecoder.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindQuadratureDecoder(), the high-level API for QuadratureDecoder functions
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
#include "yocto_quadraturedecoder.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "quadraturedecoder"

YQuadratureDecoder::YQuadratureDecoder(const string& func): YSensor(func)
//--- (YQuadratureDecoder initialization)
    ,_speed(SPEED_INVALID)
    ,_decoding(DECODING_INVALID)
    ,_valueCallbackQuadratureDecoder(NULL)
    ,_timedReportCallbackQuadratureDecoder(NULL)
//--- (end of YQuadratureDecoder initialization)
{
    _className="QuadratureDecoder";
}

YQuadratureDecoder::~YQuadratureDecoder()
{
//--- (YQuadratureDecoder cleanup)
//--- (end of YQuadratureDecoder cleanup)
}
//--- (YQuadratureDecoder implementation)
// static attributes
const double YQuadratureDecoder::SPEED_INVALID = YAPI_INVALID_DOUBLE;

int YQuadratureDecoder::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("speed")) {
        _speed =  floor(json_val->getDouble("speed") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("decoding")) {
        _decoding =  (Y_DECODING_enum)json_val->getInt("decoding");
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Changes the current expected position of the quadrature decoder.
 * Invoking this function implicitely activates the quadrature decoder.
 *
 * @param newval : a floating point number corresponding to the current expected position of the quadrature decoder
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YQuadratureDecoder::set_currentValue(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("currentValue", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the increments frequency, in Hz.
 *
 * @return a floating point number corresponding to the increments frequency, in Hz
 *
 * On failure, throws an exception or returns Y_SPEED_INVALID.
 */
double YQuadratureDecoder::get_speed(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YQuadratureDecoder::SPEED_INVALID;
                }
            }
        }
        res = _speed;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current activation state of the quadrature decoder.
 *
 * @return either Y_DECODING_OFF or Y_DECODING_ON, according to the current activation state of the
 * quadrature decoder
 *
 * On failure, throws an exception or returns Y_DECODING_INVALID.
 */
Y_DECODING_enum YQuadratureDecoder::get_decoding(void)
{
    Y_DECODING_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YQuadratureDecoder::DECODING_INVALID;
                }
            }
        }
        res = _decoding;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the activation state of the quadrature decoder.
 *
 * @param newval : either Y_DECODING_OFF or Y_DECODING_ON, according to the activation state of the
 * quadrature decoder
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YQuadratureDecoder::set_decoding(Y_DECODING_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = (newval>0 ? "1" : "0");
        res = _setAttr("decoding", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a quadrature decoder for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the quadrature decoder is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YQuadratureDecoder.isOnline() to test if the quadrature decoder is
 * indeed online at a given time. In case of ambiguity when looking for
 * a quadrature decoder by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the quadrature decoder
 *
 * @return a YQuadratureDecoder object allowing you to drive the quadrature decoder.
 */
YQuadratureDecoder* YQuadratureDecoder::FindQuadratureDecoder(string func)
{
    YQuadratureDecoder* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YQuadratureDecoder*) YFunction::_FindFromCache("QuadratureDecoder", func);
        if (obj == NULL) {
            obj = new YQuadratureDecoder(func);
            YFunction::_AddToCache("QuadratureDecoder", func, obj);
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
int YQuadratureDecoder::registerValueCallback(YQuadratureDecoderValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackQuadratureDecoder = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YQuadratureDecoder::_invokeValueCallback(string value)
{
    if (_valueCallbackQuadratureDecoder != NULL) {
        _valueCallbackQuadratureDecoder(this, value);
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
int YQuadratureDecoder::registerTimedReportCallback(YQuadratureDecoderTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackQuadratureDecoder = callback;
    return 0;
}

int YQuadratureDecoder::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackQuadratureDecoder != NULL) {
        _timedReportCallbackQuadratureDecoder(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

YQuadratureDecoder *YQuadratureDecoder::nextQuadratureDecoder(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YQuadratureDecoder::FindQuadratureDecoder(hwid);
}

YQuadratureDecoder* YQuadratureDecoder::FirstQuadratureDecoder(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("QuadratureDecoder", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YQuadratureDecoder::FindQuadratureDecoder(serial+"."+funcId);
}

//--- (end of YQuadratureDecoder implementation)

//--- (YQuadratureDecoder functions)
//--- (end of YQuadratureDecoder functions)
