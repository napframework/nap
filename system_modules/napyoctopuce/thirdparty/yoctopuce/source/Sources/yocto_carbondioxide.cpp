/*********************************************************************
 *
 * $Id: yocto_carbondioxide.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindCarbonDioxide(), the high-level API for CarbonDioxide functions
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
#include "yocto_carbondioxide.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "carbondioxide"

YCarbonDioxide::YCarbonDioxide(const string& func): YSensor(func)
//--- (YCarbonDioxide initialization)
    ,_abcPeriod(ABCPERIOD_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackCarbonDioxide(NULL)
    ,_timedReportCallbackCarbonDioxide(NULL)
//--- (end of YCarbonDioxide initialization)
{
    _className="CarbonDioxide";
}

YCarbonDioxide::~YCarbonDioxide()
{
//--- (YCarbonDioxide cleanup)
//--- (end of YCarbonDioxide cleanup)
}
//--- (YCarbonDioxide implementation)
// static attributes
const string YCarbonDioxide::COMMAND_INVALID = YAPI_INVALID_STRING;

int YCarbonDioxide::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("abcPeriod")) {
        _abcPeriod =  json_val->getInt("abcPeriod");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Returns the Automatic Baseline Calibration period, in hours. A negative value
 * means that automatic baseline calibration is disabled.
 *
 * @return an integer corresponding to the Automatic Baseline Calibration period, in hours
 *
 * On failure, throws an exception or returns Y_ABCPERIOD_INVALID.
 */
int YCarbonDioxide::get_abcPeriod(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCarbonDioxide::ABCPERIOD_INVALID;
                }
            }
        }
        res = _abcPeriod;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes Automatic Baseline Calibration period, in hours. If you need
 * to disable automatic baseline calibration (for instance when using the
 * sensor in an environment that is constantly above 400ppm CO2), set the
 * period to -1. Remember to call the saveToFlash() method of the
 * module if the modification must be kept.
 *
 * @param newval : an integer corresponding to Automatic Baseline Calibration period, in hours
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCarbonDioxide::set_abcPeriod(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("abcPeriod", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YCarbonDioxide::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCarbonDioxide::COMMAND_INVALID;
                }
            }
        }
        res = _command;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YCarbonDioxide::set_command(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("command", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a CO2 sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the CO2 sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCarbonDioxide.isOnline() to test if the CO2 sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a CO2 sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the CO2 sensor
 *
 * @return a YCarbonDioxide object allowing you to drive the CO2 sensor.
 */
YCarbonDioxide* YCarbonDioxide::FindCarbonDioxide(string func)
{
    YCarbonDioxide* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YCarbonDioxide*) YFunction::_FindFromCache("CarbonDioxide", func);
        if (obj == NULL) {
            obj = new YCarbonDioxide(func);
            YFunction::_AddToCache("CarbonDioxide", func, obj);
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
int YCarbonDioxide::registerValueCallback(YCarbonDioxideValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackCarbonDioxide = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YCarbonDioxide::_invokeValueCallback(string value)
{
    if (_valueCallbackCarbonDioxide != NULL) {
        _valueCallbackCarbonDioxide(this, value);
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
int YCarbonDioxide::registerTimedReportCallback(YCarbonDioxideTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackCarbonDioxide = callback;
    return 0;
}

int YCarbonDioxide::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackCarbonDioxide != NULL) {
        _timedReportCallbackCarbonDioxide(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

/**
 * Triggers a baseline calibration at standard CO2 ambiant level (400ppm).
 * It is normally not necessary to manually calibrate the sensor, because
 * the built-in automatic baseline calibration procedure will automatically
 * fix any long-term drift based on the lowest level of CO2 observed over the
 * automatic calibration period. However, if you disable automatic baseline
 * calibration, you may want to manually trigger a calibration from time to
 * time. Before starting a baseline calibration, make sure to put the sensor
 * in a standard environment (e.g. outside in fresh air) at around 400ppm.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCarbonDioxide::triggerBaselineCalibration(void)
{
    return this->set_command("BC");
}

int YCarbonDioxide::triggetBaselineCalibration(void)
{
    return this->triggerBaselineCalibration();
}

/**
 * Triggers a zero calibration of the sensor on carbon dioxide-free air.
 * It is normally not necessary to manually calibrate the sensor, because
 * the built-in automatic baseline calibration procedure will automatically
 * fix any long-term drift based on the lowest level of CO2 observed over the
 * automatic calibration period. However, if you disable automatic baseline
 * calibration, you may want to manually trigger a calibration from time to
 * time. Before starting a zero calibration, you should circulate carbon
 * dioxide-free air within the sensor for a minute or two, using a small pipe
 * connected to the sensor. Please contact support@yoctopuce.com for more details
 * on the zero calibration procedure.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCarbonDioxide::triggerZeroCalibration(void)
{
    return this->set_command("ZC");
}

int YCarbonDioxide::triggetZeroCalibration(void)
{
    return this->triggerZeroCalibration();
}

YCarbonDioxide *YCarbonDioxide::nextCarbonDioxide(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YCarbonDioxide::FindCarbonDioxide(hwid);
}

YCarbonDioxide* YCarbonDioxide::FirstCarbonDioxide(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("CarbonDioxide", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YCarbonDioxide::FindCarbonDioxide(serial+"."+funcId);
}

//--- (end of YCarbonDioxide implementation)

//--- (YCarbonDioxide functions)
//--- (end of YCarbonDioxide functions)
