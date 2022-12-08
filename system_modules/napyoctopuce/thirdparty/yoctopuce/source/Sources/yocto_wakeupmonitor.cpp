/*********************************************************************
 *
 * $Id: yocto_wakeupmonitor.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindWakeUpMonitor(), the high-level API for WakeUpMonitor functions
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
#include "yocto_wakeupmonitor.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "wakeupmonitor"

YWakeUpMonitor::YWakeUpMonitor(const string& func): YFunction(func)
//--- (YWakeUpMonitor initialization)
    ,_powerDuration(POWERDURATION_INVALID)
    ,_sleepCountdown(SLEEPCOUNTDOWN_INVALID)
    ,_nextWakeUp(NEXTWAKEUP_INVALID)
    ,_wakeUpReason(WAKEUPREASON_INVALID)
    ,_wakeUpState(WAKEUPSTATE_INVALID)
    ,_rtcTime(RTCTIME_INVALID)
    ,_endOfTime(2145960000)
    ,_valueCallbackWakeUpMonitor(NULL)
//--- (end of YWakeUpMonitor initialization)
{
    _className="WakeUpMonitor";
}

YWakeUpMonitor::~YWakeUpMonitor()
{
//--- (YWakeUpMonitor cleanup)
//--- (end of YWakeUpMonitor cleanup)
}
//--- (YWakeUpMonitor implementation)
// static attributes

int YWakeUpMonitor::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("powerDuration")) {
        _powerDuration =  json_val->getInt("powerDuration");
    }
    if(json_val->has("sleepCountdown")) {
        _sleepCountdown =  json_val->getInt("sleepCountdown");
    }
    if(json_val->has("nextWakeUp")) {
        _nextWakeUp =  json_val->getLong("nextWakeUp");
    }
    if(json_val->has("wakeUpReason")) {
        _wakeUpReason =  (Y_WAKEUPREASON_enum)json_val->getInt("wakeUpReason");
    }
    if(json_val->has("wakeUpState")) {
        _wakeUpState =  (Y_WAKEUPSTATE_enum)json_val->getInt("wakeUpState");
    }
    if(json_val->has("rtcTime")) {
        _rtcTime =  json_val->getLong("rtcTime");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the maximal wake up time (in seconds) before automatically going to sleep.
 *
 * @return an integer corresponding to the maximal wake up time (in seconds) before automatically going to sleep
 *
 * On failure, throws an exception or returns Y_POWERDURATION_INVALID.
 */
int YWakeUpMonitor::get_powerDuration(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpMonitor::POWERDURATION_INVALID;
                }
            }
        }
        res = _powerDuration;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the maximal wake up time (seconds) before automatically going to sleep.
 *
 * @param newval : an integer corresponding to the maximal wake up time (seconds) before automatically
 * going to sleep
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::set_powerDuration(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("powerDuration", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the delay before the  next sleep period.
 *
 * @return an integer corresponding to the delay before the  next sleep period
 *
 * On failure, throws an exception or returns Y_SLEEPCOUNTDOWN_INVALID.
 */
int YWakeUpMonitor::get_sleepCountdown(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpMonitor::SLEEPCOUNTDOWN_INVALID;
                }
            }
        }
        res = _sleepCountdown;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the delay before the next sleep period.
 *
 * @param newval : an integer corresponding to the delay before the next sleep period
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::set_sleepCountdown(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("sleepCountdown", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the next scheduled wake up date/time (UNIX format).
 *
 * @return an integer corresponding to the next scheduled wake up date/time (UNIX format)
 *
 * On failure, throws an exception or returns Y_NEXTWAKEUP_INVALID.
 */
s64 YWakeUpMonitor::get_nextWakeUp(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpMonitor::NEXTWAKEUP_INVALID;
                }
            }
        }
        res = _nextWakeUp;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the days of the week when a wake up must take place.
 *
 * @param newval : an integer corresponding to the days of the week when a wake up must take place
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::set_nextWakeUp(s64 newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%u", (u32)newval); rest_val = string(buf);
        res = _setAttr("nextWakeUp", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the latest wake up reason.
 *
 * @return a value among Y_WAKEUPREASON_USBPOWER, Y_WAKEUPREASON_EXTPOWER, Y_WAKEUPREASON_ENDOFSLEEP,
 * Y_WAKEUPREASON_EXTSIG1, Y_WAKEUPREASON_SCHEDULE1 and Y_WAKEUPREASON_SCHEDULE2 corresponding to the
 * latest wake up reason
 *
 * On failure, throws an exception or returns Y_WAKEUPREASON_INVALID.
 */
Y_WAKEUPREASON_enum YWakeUpMonitor::get_wakeUpReason(void)
{
    Y_WAKEUPREASON_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpMonitor::WAKEUPREASON_INVALID;
                }
            }
        }
        res = _wakeUpReason;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns  the current state of the monitor.
 *
 * @return either Y_WAKEUPSTATE_SLEEPING or Y_WAKEUPSTATE_AWAKE, according to  the current state of the monitor
 *
 * On failure, throws an exception or returns Y_WAKEUPSTATE_INVALID.
 */
Y_WAKEUPSTATE_enum YWakeUpMonitor::get_wakeUpState(void)
{
    Y_WAKEUPSTATE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpMonitor::WAKEUPSTATE_INVALID;
                }
            }
        }
        res = _wakeUpState;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YWakeUpMonitor::set_wakeUpState(Y_WAKEUPSTATE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("wakeUpState", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

s64 YWakeUpMonitor::get_rtcTime(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpMonitor::RTCTIME_INVALID;
                }
            }
        }
        res = _rtcTime;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a monitor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the monitor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YWakeUpMonitor.isOnline() to test if the monitor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a monitor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the monitor
 *
 * @return a YWakeUpMonitor object allowing you to drive the monitor.
 */
YWakeUpMonitor* YWakeUpMonitor::FindWakeUpMonitor(string func)
{
    YWakeUpMonitor* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YWakeUpMonitor*) YFunction::_FindFromCache("WakeUpMonitor", func);
        if (obj == NULL) {
            obj = new YWakeUpMonitor(func);
            YFunction::_AddToCache("WakeUpMonitor", func, obj);
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
int YWakeUpMonitor::registerValueCallback(YWakeUpMonitorValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackWakeUpMonitor = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YWakeUpMonitor::_invokeValueCallback(string value)
{
    if (_valueCallbackWakeUpMonitor != NULL) {
        _valueCallbackWakeUpMonitor(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Forces a wake up.
 */
int YWakeUpMonitor::wakeUp(void)
{
    return this->set_wakeUpState(Y_WAKEUPSTATE_AWAKE);
}

/**
 * Goes to sleep until the next wake up condition is met,  the
 * RTC time must have been set before calling this function.
 *
 * @param secBeforeSleep : number of seconds before going into sleep mode,
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::sleep(int secBeforeSleep)
{
    int currTime = 0;
    currTime = (int)(this->get_rtcTime());
    if (!(currTime != 0)) {
        _throw(YAPI_RTC_NOT_READY,"RTC time not set");
        return YAPI_RTC_NOT_READY;
    }
    this->set_nextWakeUp(_endOfTime);
    this->set_sleepCountdown(secBeforeSleep);
    return YAPI_SUCCESS;
}

/**
 * Goes to sleep for a specific duration or until the next wake up condition is met, the
 * RTC time must have been set before calling this function. The count down before sleep
 * can be canceled with resetSleepCountDown.
 *
 * @param secUntilWakeUp : number of seconds before next wake up
 * @param secBeforeSleep : number of seconds before going into sleep mode
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::sleepFor(int secUntilWakeUp,int secBeforeSleep)
{
    int currTime = 0;
    currTime = (int)(this->get_rtcTime());
    if (!(currTime != 0)) {
        _throw(YAPI_RTC_NOT_READY,"RTC time not set");
        return YAPI_RTC_NOT_READY;
    }
    this->set_nextWakeUp(currTime+secUntilWakeUp);
    this->set_sleepCountdown(secBeforeSleep);
    return YAPI_SUCCESS;
}

/**
 * Go to sleep until a specific date is reached or until the next wake up condition is met, the
 * RTC time must have been set before calling this function. The count down before sleep
 * can be canceled with resetSleepCountDown.
 *
 * @param wakeUpTime : wake-up datetime (UNIX format)
 * @param secBeforeSleep : number of seconds before going into sleep mode
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::sleepUntil(int wakeUpTime,int secBeforeSleep)
{
    int currTime = 0;
    currTime = (int)(this->get_rtcTime());
    if (!(currTime != 0)) {
        _throw(YAPI_RTC_NOT_READY,"RTC time not set");
        return YAPI_RTC_NOT_READY;
    }
    this->set_nextWakeUp(wakeUpTime);
    this->set_sleepCountdown(secBeforeSleep);
    return YAPI_SUCCESS;
}

/**
 * Resets the sleep countdown.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YWakeUpMonitor::resetSleepCountDown(void)
{
    this->set_sleepCountdown(0);
    this->set_nextWakeUp(0);
    return YAPI_SUCCESS;
}

YWakeUpMonitor *YWakeUpMonitor::nextWakeUpMonitor(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YWakeUpMonitor::FindWakeUpMonitor(hwid);
}

YWakeUpMonitor* YWakeUpMonitor::FirstWakeUpMonitor(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("WakeUpMonitor", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YWakeUpMonitor::FindWakeUpMonitor(serial+"."+funcId);
}

//--- (end of YWakeUpMonitor implementation)

//--- (YWakeUpMonitor functions)
//--- (end of YWakeUpMonitor functions)
