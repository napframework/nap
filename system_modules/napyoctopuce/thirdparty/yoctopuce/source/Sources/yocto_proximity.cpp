/*********************************************************************
 *
 * $Id: yocto_proximity.cpp 29767 2018-01-26 08:53:27Z seb $
 *
 * Implements yFindProximity(), the high-level API for Proximity functions
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
#include "yocto_proximity.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "proximity"

YProximity::YProximity(const string& func): YSensor(func)
//--- (YProximity initialization)
    ,_signalValue(SIGNALVALUE_INVALID)
    ,_detectionThreshold(DETECTIONTHRESHOLD_INVALID)
    ,_detectionHysteresis(DETECTIONHYSTERESIS_INVALID)
    ,_presenceMinTime(PRESENCEMINTIME_INVALID)
    ,_removalMinTime(REMOVALMINTIME_INVALID)
    ,_isPresent(ISPRESENT_INVALID)
    ,_lastTimeApproached(LASTTIMEAPPROACHED_INVALID)
    ,_lastTimeRemoved(LASTTIMEREMOVED_INVALID)
    ,_pulseCounter(PULSECOUNTER_INVALID)
    ,_pulseTimer(PULSETIMER_INVALID)
    ,_proximityReportMode(PROXIMITYREPORTMODE_INVALID)
    ,_valueCallbackProximity(NULL)
    ,_timedReportCallbackProximity(NULL)
//--- (end of YProximity initialization)
{
    _className="Proximity";
}

YProximity::~YProximity()
{
//--- (YProximity cleanup)
//--- (end of YProximity cleanup)
}
//--- (YProximity implementation)
// static attributes
const double YProximity::SIGNALVALUE_INVALID = YAPI_INVALID_DOUBLE;

int YProximity::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("signalValue")) {
        _signalValue =  floor(json_val->getDouble("signalValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("detectionThreshold")) {
        _detectionThreshold =  json_val->getInt("detectionThreshold");
    }
    if(json_val->has("detectionHysteresis")) {
        _detectionHysteresis =  json_val->getInt("detectionHysteresis");
    }
    if(json_val->has("presenceMinTime")) {
        _presenceMinTime =  json_val->getInt("presenceMinTime");
    }
    if(json_val->has("removalMinTime")) {
        _removalMinTime =  json_val->getInt("removalMinTime");
    }
    if(json_val->has("isPresent")) {
        _isPresent =  (Y_ISPRESENT_enum)json_val->getInt("isPresent");
    }
    if(json_val->has("lastTimeApproached")) {
        _lastTimeApproached =  json_val->getLong("lastTimeApproached");
    }
    if(json_val->has("lastTimeRemoved")) {
        _lastTimeRemoved =  json_val->getLong("lastTimeRemoved");
    }
    if(json_val->has("pulseCounter")) {
        _pulseCounter =  json_val->getLong("pulseCounter");
    }
    if(json_val->has("pulseTimer")) {
        _pulseTimer =  json_val->getLong("pulseTimer");
    }
    if(json_val->has("proximityReportMode")) {
        _proximityReportMode =  (Y_PROXIMITYREPORTMODE_enum)json_val->getInt("proximityReportMode");
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Returns the current value of signal measured by the proximity sensor.
 *
 * @return a floating point number corresponding to the current value of signal measured by the proximity sensor
 *
 * On failure, throws an exception or returns Y_SIGNALVALUE_INVALID.
 */
double YProximity::get_signalValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::SIGNALVALUE_INVALID;
                }
            }
        }
        res = floor(_signalValue * 1000+0.5) / 1000;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the threshold used to determine the logical state of the proximity sensor, when considered
 * as a binary input (on/off).
 *
 * @return an integer corresponding to the threshold used to determine the logical state of the
 * proximity sensor, when considered
 *         as a binary input (on/off)
 *
 * On failure, throws an exception or returns Y_DETECTIONTHRESHOLD_INVALID.
 */
int YProximity::get_detectionThreshold(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::DETECTIONTHRESHOLD_INVALID;
                }
            }
        }
        res = _detectionThreshold;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the threshold used to determine the logical state of the proximity sensor, when considered
 * as a binary input (on/off).
 *
 * @param newval : an integer corresponding to the threshold used to determine the logical state of
 * the proximity sensor, when considered
 *         as a binary input (on/off)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YProximity::set_detectionThreshold(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("detectionThreshold", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the hysteresis used to determine the logical state of the proximity sensor, when considered
 * as a binary input (on/off).
 *
 * @return an integer corresponding to the hysteresis used to determine the logical state of the
 * proximity sensor, when considered
 *         as a binary input (on/off)
 *
 * On failure, throws an exception or returns Y_DETECTIONHYSTERESIS_INVALID.
 */
int YProximity::get_detectionHysteresis(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::DETECTIONHYSTERESIS_INVALID;
                }
            }
        }
        res = _detectionHysteresis;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the hysteresis used to determine the logical state of the proximity sensor, when considered
 * as a binary input (on/off).
 *
 * @param newval : an integer corresponding to the hysteresis used to determine the logical state of
 * the proximity sensor, when considered
 *         as a binary input (on/off)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YProximity::set_detectionHysteresis(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("detectionHysteresis", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the minimal detection duration before signaling a presence event. Any shorter detection is
 * considered as noise or bounce (false positive) and filtered out.
 *
 * @return an integer corresponding to the minimal detection duration before signaling a presence event
 *
 * On failure, throws an exception or returns Y_PRESENCEMINTIME_INVALID.
 */
int YProximity::get_presenceMinTime(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::PRESENCEMINTIME_INVALID;
                }
            }
        }
        res = _presenceMinTime;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the minimal detection duration before signaling a presence event. Any shorter detection is
 * considered as noise or bounce (false positive) and filtered out.
 *
 * @param newval : an integer corresponding to the minimal detection duration before signaling a presence event
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YProximity::set_presenceMinTime(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("presenceMinTime", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the minimal detection duration before signaling a removal event. Any shorter detection is
 * considered as noise or bounce (false positive) and filtered out.
 *
 * @return an integer corresponding to the minimal detection duration before signaling a removal event
 *
 * On failure, throws an exception or returns Y_REMOVALMINTIME_INVALID.
 */
int YProximity::get_removalMinTime(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::REMOVALMINTIME_INVALID;
                }
            }
        }
        res = _removalMinTime;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the minimal detection duration before signaling a removal event. Any shorter detection is
 * considered as noise or bounce (false positive) and filtered out.
 *
 * @param newval : an integer corresponding to the minimal detection duration before signaling a removal event
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YProximity::set_removalMinTime(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("removalMinTime", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns true if the input (considered as binary) is active (detection value is smaller than the
 * specified threshold), and false otherwise.
 *
 * @return either Y_ISPRESENT_FALSE or Y_ISPRESENT_TRUE, according to true if the input (considered as
 * binary) is active (detection value is smaller than the specified threshold), and false otherwise
 *
 * On failure, throws an exception or returns Y_ISPRESENT_INVALID.
 */
Y_ISPRESENT_enum YProximity::get_isPresent(void)
{
    Y_ISPRESENT_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::ISPRESENT_INVALID;
                }
            }
        }
        res = _isPresent;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of elapsed milliseconds between the module power on and the last observed
 * detection (the input contact transitioned from absent to present).
 *
 * @return an integer corresponding to the number of elapsed milliseconds between the module power on
 * and the last observed
 *         detection (the input contact transitioned from absent to present)
 *
 * On failure, throws an exception or returns Y_LASTTIMEAPPROACHED_INVALID.
 */
s64 YProximity::get_lastTimeApproached(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::LASTTIMEAPPROACHED_INVALID;
                }
            }
        }
        res = _lastTimeApproached;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of elapsed milliseconds between the module power on and the last observed
 * detection (the input contact transitioned from present to absent).
 *
 * @return an integer corresponding to the number of elapsed milliseconds between the module power on
 * and the last observed
 *         detection (the input contact transitioned from present to absent)
 *
 * On failure, throws an exception or returns Y_LASTTIMEREMOVED_INVALID.
 */
s64 YProximity::get_lastTimeRemoved(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::LASTTIMEREMOVED_INVALID;
                }
            }
        }
        res = _lastTimeRemoved;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the pulse counter value. The value is a 32 bit integer. In case
 * of overflow (>=2^32), the counter will wrap. To reset the counter, just
 * call the resetCounter() method.
 *
 * @return an integer corresponding to the pulse counter value
 *
 * On failure, throws an exception or returns Y_PULSECOUNTER_INVALID.
 */
s64 YProximity::get_pulseCounter(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::PULSECOUNTER_INVALID;
                }
            }
        }
        res = _pulseCounter;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YProximity::set_pulseCounter(s64 newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%u", (u32)newval); rest_val = string(buf);
        res = _setAttr("pulseCounter", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the timer of the pulse counter (ms).
 *
 * @return an integer corresponding to the timer of the pulse counter (ms)
 *
 * On failure, throws an exception or returns Y_PULSETIMER_INVALID.
 */
s64 YProximity::get_pulseTimer(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::PULSETIMER_INVALID;
                }
            }
        }
        res = _pulseTimer;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the parameter (sensor value, presence or pulse count) returned by the get_currentValue
 * function and callbacks.
 *
 * @return a value among Y_PROXIMITYREPORTMODE_NUMERIC, Y_PROXIMITYREPORTMODE_PRESENCE and
 * Y_PROXIMITYREPORTMODE_PULSECOUNT corresponding to the parameter (sensor value, presence or pulse
 * count) returned by the get_currentValue function and callbacks
 *
 * On failure, throws an exception or returns Y_PROXIMITYREPORTMODE_INVALID.
 */
Y_PROXIMITYREPORTMODE_enum YProximity::get_proximityReportMode(void)
{
    Y_PROXIMITYREPORTMODE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YProximity::PROXIMITYREPORTMODE_INVALID;
                }
            }
        }
        res = _proximityReportMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the  parameter  type (sensor value, presence or pulse count) returned by the
 * get_currentValue function and callbacks.
 * The edge count value is limited to the 6 lowest digits. For values greater than one million, use
 * get_pulseCounter().
 *
 * @param newval : a value among Y_PROXIMITYREPORTMODE_NUMERIC, Y_PROXIMITYREPORTMODE_PRESENCE and
 * Y_PROXIMITYREPORTMODE_PULSECOUNT corresponding to the  parameter  type (sensor value, presence or
 * pulse count) returned by the get_currentValue function and callbacks
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YProximity::set_proximityReportMode(Y_PROXIMITYREPORTMODE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("proximityReportMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a proximity sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the proximity sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YProximity.isOnline() to test if the proximity sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a proximity sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the proximity sensor
 *
 * @return a YProximity object allowing you to drive the proximity sensor.
 */
YProximity* YProximity::FindProximity(string func)
{
    YProximity* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YProximity*) YFunction::_FindFromCache("Proximity", func);
        if (obj == NULL) {
            obj = new YProximity(func);
            YFunction::_AddToCache("Proximity", func, obj);
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
int YProximity::registerValueCallback(YProximityValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackProximity = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YProximity::_invokeValueCallback(string value)
{
    if (_valueCallbackProximity != NULL) {
        _valueCallbackProximity(this, value);
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
int YProximity::registerTimedReportCallback(YProximityTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackProximity = callback;
    return 0;
}

int YProximity::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackProximity != NULL) {
        _timedReportCallbackProximity(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

/**
 * Resets the pulse counter value as well as its timer.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YProximity::resetCounter(void)
{
    return this->set_pulseCounter(0);
}

YProximity *YProximity::nextProximity(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YProximity::FindProximity(hwid);
}

YProximity* YProximity::FirstProximity(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Proximity", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YProximity::FindProximity(serial+"."+funcId);
}

//--- (end of YProximity implementation)

//--- (YProximity functions)
//--- (end of YProximity functions)
