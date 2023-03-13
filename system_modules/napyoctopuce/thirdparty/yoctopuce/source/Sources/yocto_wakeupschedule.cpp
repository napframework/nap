/*********************************************************************
 *
 * $Id: yocto_wakeupschedule.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindWakeUpSchedule(), the high-level API for WakeUpSchedule functions
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
#include "yocto_wakeupschedule.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "wakeupschedule"

YWakeUpSchedule::YWakeUpSchedule(const string& func): YFunction(func)
//--- (YWakeUpSchedule initialization)
    ,_minutesA(MINUTESA_INVALID)
    ,_minutesB(MINUTESB_INVALID)
    ,_hours(HOURS_INVALID)
    ,_weekDays(WEEKDAYS_INVALID)
    ,_monthDays(MONTHDAYS_INVALID)
    ,_months(MONTHS_INVALID)
    ,_nextOccurence(NEXTOCCURENCE_INVALID)
    ,_valueCallbackWakeUpSchedule(NULL)
//--- (end of YWakeUpSchedule initialization)
{
    _className="WakeUpSchedule";
}

YWakeUpSchedule::~YWakeUpSchedule()
{
//--- (YWakeUpSchedule cleanup)
//--- (end of YWakeUpSchedule cleanup)
}
//--- (YWakeUpSchedule implementation)
// static attributes

int YWakeUpSchedule::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("minutesA")) {
        _minutesA =  json_val->getInt("minutesA");
    }
    if(json_val->has("minutesB")) {
        _minutesB =  json_val->getInt("minutesB");
    }
    if(json_val->has("hours")) {
        _hours =  json_val->getInt("hours");
    }
    if(json_val->has("weekDays")) {
        _weekDays =  json_val->getInt("weekDays");
    }
    if(json_val->has("monthDays")) {
        _monthDays =  json_val->getInt("monthDays");
    }
    if(json_val->has("months")) {
        _months =  json_val->getInt("months");
    }
    if(json_val->has("nextOccurence")) {
        _nextOccurence =  json_val->getLong("nextOccurence");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the minutes in the 00-29 interval of each hour scheduled for wake up.
 *
 * @return an integer corresponding to the minutes in the 00-29 interval of each hour scheduled for wake up
 *
 * On failure, throws an exception or returns Y_MINUTESA_INVALID.
 */
int YWakeUpSchedule::get_minutesA(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::MINUTESA_INVALID;
                }
            }
        }
        res = _minutesA;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the minutes in the 00-29 interval when a wake up must take place.
 *
 * @param newval : an integer corresponding to the minutes in the 00-29 interval when a wake up must take place
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpSchedule::set_minutesA(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("minutesA", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the minutes in the 30-59 intervalof each hour scheduled for wake up.
 *
 * @return an integer corresponding to the minutes in the 30-59 intervalof each hour scheduled for wake up
 *
 * On failure, throws an exception or returns Y_MINUTESB_INVALID.
 */
int YWakeUpSchedule::get_minutesB(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::MINUTESB_INVALID;
                }
            }
        }
        res = _minutesB;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the minutes in the 30-59 interval when a wake up must take place.
 *
 * @param newval : an integer corresponding to the minutes in the 30-59 interval when a wake up must take place
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpSchedule::set_minutesB(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("minutesB", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the hours scheduled for wake up.
 *
 * @return an integer corresponding to the hours scheduled for wake up
 *
 * On failure, throws an exception or returns Y_HOURS_INVALID.
 */
int YWakeUpSchedule::get_hours(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::HOURS_INVALID;
                }
            }
        }
        res = _hours;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the hours when a wake up must take place.
 *
 * @param newval : an integer corresponding to the hours when a wake up must take place
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpSchedule::set_hours(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("hours", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the days of the week scheduled for wake up.
 *
 * @return an integer corresponding to the days of the week scheduled for wake up
 *
 * On failure, throws an exception or returns Y_WEEKDAYS_INVALID.
 */
int YWakeUpSchedule::get_weekDays(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::WEEKDAYS_INVALID;
                }
            }
        }
        res = _weekDays;
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
int YWakeUpSchedule::set_weekDays(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("weekDays", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the days of the month scheduled for wake up.
 *
 * @return an integer corresponding to the days of the month scheduled for wake up
 *
 * On failure, throws an exception or returns Y_MONTHDAYS_INVALID.
 */
int YWakeUpSchedule::get_monthDays(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::MONTHDAYS_INVALID;
                }
            }
        }
        res = _monthDays;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the days of the month when a wake up must take place.
 *
 * @param newval : an integer corresponding to the days of the month when a wake up must take place
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpSchedule::set_monthDays(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("monthDays", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the months scheduled for wake up.
 *
 * @return an integer corresponding to the months scheduled for wake up
 *
 * On failure, throws an exception or returns Y_MONTHS_INVALID.
 */
int YWakeUpSchedule::get_months(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::MONTHS_INVALID;
                }
            }
        }
        res = _months;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the months when a wake up must take place.
 *
 * @param newval : an integer corresponding to the months when a wake up must take place
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpSchedule::set_months(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("months", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the date/time (seconds) of the next wake up occurence.
 *
 * @return an integer corresponding to the date/time (seconds) of the next wake up occurence
 *
 * On failure, throws an exception or returns Y_NEXTOCCURENCE_INVALID.
 */
s64 YWakeUpSchedule::get_nextOccurence(void)
{
    s64 res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWakeUpSchedule::NEXTOCCURENCE_INVALID;
                }
            }
        }
        res = _nextOccurence;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a wake up schedule for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the wake up schedule is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YWakeUpSchedule.isOnline() to test if the wake up schedule is
 * indeed online at a given time. In case of ambiguity when looking for
 * a wake up schedule by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the wake up schedule
 *
 * @return a YWakeUpSchedule object allowing you to drive the wake up schedule.
 */
YWakeUpSchedule* YWakeUpSchedule::FindWakeUpSchedule(string func)
{
    YWakeUpSchedule* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YWakeUpSchedule*) YFunction::_FindFromCache("WakeUpSchedule", func);
        if (obj == NULL) {
            obj = new YWakeUpSchedule(func);
            YFunction::_AddToCache("WakeUpSchedule", func, obj);
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
int YWakeUpSchedule::registerValueCallback(YWakeUpScheduleValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackWakeUpSchedule = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YWakeUpSchedule::_invokeValueCallback(string value)
{
    if (_valueCallbackWakeUpSchedule != NULL) {
        _valueCallbackWakeUpSchedule(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Returns all the minutes of each hour that are scheduled for wake up.
 */
s64 YWakeUpSchedule::get_minutes(void)
{
    s64 res = 0;

    res = this->get_minutesB();
    res = ((res) << (30));
    res = res + this->get_minutesA();
    return res;
}

/**
 * Changes all the minutes where a wake up must take place.
 *
 * @param bitmap : Minutes 00-59 of each hour scheduled for wake up.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWakeUpSchedule::set_minutes(s64 bitmap)
{
    this->set_minutesA((int)(((bitmap) & (0x3fffffff))));
    bitmap = ((bitmap) >> (30));
    return this->set_minutesB((int)(((bitmap) & (0x3fffffff))));
}

YWakeUpSchedule *YWakeUpSchedule::nextWakeUpSchedule(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YWakeUpSchedule::FindWakeUpSchedule(hwid);
}

YWakeUpSchedule* YWakeUpSchedule::FirstWakeUpSchedule(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("WakeUpSchedule", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YWakeUpSchedule::FindWakeUpSchedule(serial+"."+funcId);
}

//--- (end of YWakeUpSchedule implementation)

//--- (YWakeUpSchedule functions)
//--- (end of YWakeUpSchedule functions)
