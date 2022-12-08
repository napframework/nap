/*********************************************************************
 *
 * $Id: yocto_rangefinder.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindRangeFinder(), the high-level API for RangeFinder functions
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
#include "yocto_rangefinder.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "rangefinder"

YRangeFinder::YRangeFinder(const string& func): YSensor(func)
//--- (YRangeFinder initialization)
    ,_rangeFinderMode(RANGEFINDERMODE_INVALID)
    ,_hardwareCalibration(HARDWARECALIBRATION_INVALID)
    ,_currentTemperature(CURRENTTEMPERATURE_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackRangeFinder(NULL)
    ,_timedReportCallbackRangeFinder(NULL)
//--- (end of YRangeFinder initialization)
{
    _className="RangeFinder";
}

YRangeFinder::~YRangeFinder()
{
//--- (YRangeFinder cleanup)
//--- (end of YRangeFinder cleanup)
}
//--- (YRangeFinder implementation)
// static attributes
const string YRangeFinder::HARDWARECALIBRATION_INVALID = YAPI_INVALID_STRING;
const double YRangeFinder::CURRENTTEMPERATURE_INVALID = YAPI_INVALID_DOUBLE;
const string YRangeFinder::COMMAND_INVALID = YAPI_INVALID_STRING;

int YRangeFinder::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("rangeFinderMode")) {
        _rangeFinderMode =  (Y_RANGEFINDERMODE_enum)json_val->getInt("rangeFinderMode");
    }
    if(json_val->has("hardwareCalibration")) {
        _hardwareCalibration =  json_val->getString("hardwareCalibration");
    }
    if(json_val->has("currentTemperature")) {
        _currentTemperature =  floor(json_val->getDouble("currentTemperature") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Changes the measuring unit for the measured range. That unit is a string.
 * String value can be " or mm. Any other value is ignored.
 * Remember to call the saveToFlash() method of the module if the modification must be kept.
 * WARNING: if a specific calibration is defined for the rangeFinder function, a
 * unit system change will probably break it.
 *
 * @param newval : a string corresponding to the measuring unit for the measured range
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::set_unit(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("unit", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the range finder running mode. The rangefinder running mode
 * allows you to put priority on precision, speed or maximum range.
 *
 * @return a value among Y_RANGEFINDERMODE_DEFAULT, Y_RANGEFINDERMODE_LONG_RANGE,
 * Y_RANGEFINDERMODE_HIGH_ACCURACY and Y_RANGEFINDERMODE_HIGH_SPEED corresponding to the range finder running mode
 *
 * On failure, throws an exception or returns Y_RANGEFINDERMODE_INVALID.
 */
Y_RANGEFINDERMODE_enum YRangeFinder::get_rangeFinderMode(void)
{
    Y_RANGEFINDERMODE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRangeFinder::RANGEFINDERMODE_INVALID;
                }
            }
        }
        res = _rangeFinderMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the rangefinder running mode, allowing you to put priority on
 * precision, speed or maximum range.
 *
 * @param newval : a value among Y_RANGEFINDERMODE_DEFAULT, Y_RANGEFINDERMODE_LONG_RANGE,
 * Y_RANGEFINDERMODE_HIGH_ACCURACY and Y_RANGEFINDERMODE_HIGH_SPEED corresponding to the rangefinder
 * running mode, allowing you to put priority on
 *         precision, speed or maximum range
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::set_rangeFinderMode(Y_RANGEFINDERMODE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("rangeFinderMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YRangeFinder::get_hardwareCalibration(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRangeFinder::HARDWARECALIBRATION_INVALID;
                }
            }
        }
        res = _hardwareCalibration;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YRangeFinder::set_hardwareCalibration(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("hardwareCalibration", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current sensor temperature, as a floating point number.
 *
 * @return a floating point number corresponding to the current sensor temperature, as a floating point number
 *
 * On failure, throws an exception or returns Y_CURRENTTEMPERATURE_INVALID.
 */
double YRangeFinder::get_currentTemperature(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRangeFinder::CURRENTTEMPERATURE_INVALID;
                }
            }
        }
        res = _currentTemperature;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YRangeFinder::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRangeFinder::COMMAND_INVALID;
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

int YRangeFinder::set_command(const string& newval)
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
 * Retrieves a range finder for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the range finder is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YRangeFinder.isOnline() to test if the range finder is
 * indeed online at a given time. In case of ambiguity when looking for
 * a range finder by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the range finder
 *
 * @return a YRangeFinder object allowing you to drive the range finder.
 */
YRangeFinder* YRangeFinder::FindRangeFinder(string func)
{
    YRangeFinder* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YRangeFinder*) YFunction::_FindFromCache("RangeFinder", func);
        if (obj == NULL) {
            obj = new YRangeFinder(func);
            YFunction::_AddToCache("RangeFinder", func, obj);
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
int YRangeFinder::registerValueCallback(YRangeFinderValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackRangeFinder = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YRangeFinder::_invokeValueCallback(string value)
{
    if (_valueCallbackRangeFinder != NULL) {
        _valueCallbackRangeFinder(this, value);
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
int YRangeFinder::registerTimedReportCallback(YRangeFinderTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackRangeFinder = callback;
    return 0;
}

int YRangeFinder::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackRangeFinder != NULL) {
        _timedReportCallbackRangeFinder(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

/**
 * Returns the temperature at the time when the latest calibration was performed.
 * This function can be used to determine if a new calibration for ambient temperature
 * is required.
 *
 * @return a temperature, as a floating point number.
 *         On failure, throws an exception or return YAPI_INVALID_DOUBLE.
 */
double YRangeFinder::get_hardwareCalibrationTemperature(void)
{
    string hwcal;
    hwcal = this->get_hardwareCalibration();
    if (!((hwcal).substr(0, 1) == "@")) {
        return YAPI_INVALID_DOUBLE;
    }
    return atoi(((hwcal).substr(1, (int)(hwcal).length())).c_str());
}

/**
 * Triggers a sensor calibration according to the current ambient temperature. That
 * calibration process needs no physical interaction with the sensor. It is performed
 * automatically at device startup, but it is recommended to start it again when the
 * temperature delta since the latest calibration exceeds 8°C.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::triggerTemperatureCalibration(void)
{
    return this->set_command("T");
}

/**
 * Triggers the photon detector hardware calibration.
 * This function is part of the calibration procedure to compensate for the the effect
 * of a cover glass. Make sure to read the chapter about hardware calibration for details
 * on the calibration procedure for proper results.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::triggerSpadCalibration(void)
{
    return this->set_command("S");
}

/**
 * Triggers the hardware offset calibration of the distance sensor.
 * This function is part of the calibration procedure to compensate for the the effect
 * of a cover glass. Make sure to read the chapter about hardware calibration for details
 * on the calibration procedure for proper results.
 *
 * @param targetDist : true distance of the calibration target, in mm or inches, depending
 *         on the unit selected in the device
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::triggerOffsetCalibration(double targetDist)
{
    int distmm = 0;
    if (this->get_unit() == "\"") {
        distmm = (int) floor(targetDist * 25.4+0.5);
    } else {
        distmm = (int) floor(targetDist+0.5);
    }
    return this->set_command(YapiWrapper::ysprintf("O%d",distmm));
}

/**
 * Triggers the hardware cross-talk calibration of the distance sensor.
 * This function is part of the calibration procedure to compensate for the the effect
 * of a cover glass. Make sure to read the chapter about hardware calibration for details
 * on the calibration procedure for proper results.
 *
 * @param targetDist : true distance of the calibration target, in mm or inches, depending
 *         on the unit selected in the device
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::triggerXTalkCalibration(double targetDist)
{
    int distmm = 0;
    if (this->get_unit() == "\"") {
        distmm = (int) floor(targetDist * 25.4+0.5);
    } else {
        distmm = (int) floor(targetDist+0.5);
    }
    return this->set_command(YapiWrapper::ysprintf("X%d",distmm));
}

/**
 * Cancels the effect of previous hardware calibration procedures to compensate
 * for cover glass, and restores factory settings.
 * Remember to call the saveToFlash() method of the module if the modification must be kept.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YRangeFinder::cancelCoverGlassCalibrations(void)
{
    return this->set_hardwareCalibration("");
}

YRangeFinder *YRangeFinder::nextRangeFinder(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YRangeFinder::FindRangeFinder(hwid);
}

YRangeFinder* YRangeFinder::FirstRangeFinder(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("RangeFinder", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YRangeFinder::FindRangeFinder(serial+"."+funcId);
}

//--- (end of YRangeFinder implementation)

//--- (YRangeFinder functions)
//--- (end of YRangeFinder functions)
