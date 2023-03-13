/*********************************************************************
 *
 * $Id: yocto_powersupply.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindPowerSupply(), the high-level API for PowerSupply functions
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
#include "yocto_powersupply.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "powersupply"

YPowerSupply::YPowerSupply(const string& func): YFunction(func)
//--- (YPowerSupply initialization)
    ,_voltageSetPoint(VOLTAGESETPOINT_INVALID)
    ,_currentLimit(CURRENTLIMIT_INVALID)
    ,_powerOutput(POWEROUTPUT_INVALID)
    ,_voltageSense(VOLTAGESENSE_INVALID)
    ,_measuredVoltage(MEASUREDVOLTAGE_INVALID)
    ,_measuredCurrent(MEASUREDCURRENT_INVALID)
    ,_inputVoltage(INPUTVOLTAGE_INVALID)
    ,_vInt(VINT_INVALID)
    ,_ldoTemperature(LDOTEMPERATURE_INVALID)
    ,_voltageTransition(VOLTAGETRANSITION_INVALID)
    ,_voltageAtStartUp(VOLTAGEATSTARTUP_INVALID)
    ,_currentAtStartUp(CURRENTATSTARTUP_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackPowerSupply(NULL)
//--- (end of YPowerSupply initialization)
{
    _className="PowerSupply";
}

YPowerSupply::~YPowerSupply()
{
//--- (YPowerSupply cleanup)
//--- (end of YPowerSupply cleanup)
}
//--- (YPowerSupply implementation)
// static attributes
const double YPowerSupply::VOLTAGESETPOINT_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::CURRENTLIMIT_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::MEASUREDVOLTAGE_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::MEASUREDCURRENT_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::INPUTVOLTAGE_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::VINT_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::LDOTEMPERATURE_INVALID = YAPI_INVALID_DOUBLE;
const string YPowerSupply::VOLTAGETRANSITION_INVALID = YAPI_INVALID_STRING;
const double YPowerSupply::VOLTAGEATSTARTUP_INVALID = YAPI_INVALID_DOUBLE;
const double YPowerSupply::CURRENTATSTARTUP_INVALID = YAPI_INVALID_DOUBLE;
const string YPowerSupply::COMMAND_INVALID = YAPI_INVALID_STRING;

int YPowerSupply::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("voltageSetPoint")) {
        _voltageSetPoint =  floor(json_val->getDouble("voltageSetPoint") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("currentLimit")) {
        _currentLimit =  floor(json_val->getDouble("currentLimit") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("powerOutput")) {
        _powerOutput =  (Y_POWEROUTPUT_enum)json_val->getInt("powerOutput");
    }
    if(json_val->has("voltageSense")) {
        _voltageSense =  (Y_VOLTAGESENSE_enum)json_val->getInt("voltageSense");
    }
    if(json_val->has("measuredVoltage")) {
        _measuredVoltage =  floor(json_val->getDouble("measuredVoltage") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("measuredCurrent")) {
        _measuredCurrent =  floor(json_val->getDouble("measuredCurrent") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("inputVoltage")) {
        _inputVoltage =  floor(json_val->getDouble("inputVoltage") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("vInt")) {
        _vInt =  floor(json_val->getDouble("vInt") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("ldoTemperature")) {
        _ldoTemperature =  floor(json_val->getDouble("ldoTemperature") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("voltageTransition")) {
        _voltageTransition =  json_val->getString("voltageTransition");
    }
    if(json_val->has("voltageAtStartUp")) {
        _voltageAtStartUp =  floor(json_val->getDouble("voltageAtStartUp") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("currentAtStartUp")) {
        _currentAtStartUp =  floor(json_val->getDouble("currentAtStartUp") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Changes the voltage set point, in V.
 *
 * @param newval : a floating point number corresponding to the voltage set point, in V
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YPowerSupply::set_voltageSetPoint(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("voltageSetPoint", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the voltage set point, in V.
 *
 * @return a floating point number corresponding to the voltage set point, in V
 *
 * On failure, throws an exception or returns Y_VOLTAGESETPOINT_INVALID.
 */
double YPowerSupply::get_voltageSetPoint(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::VOLTAGESETPOINT_INVALID;
                }
            }
        }
        res = _voltageSetPoint;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the current limit, in mA.
 *
 * @param newval : a floating point number corresponding to the current limit, in mA
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YPowerSupply::set_currentLimit(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("currentLimit", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current limit, in mA.
 *
 * @return a floating point number corresponding to the current limit, in mA
 *
 * On failure, throws an exception or returns Y_CURRENTLIMIT_INVALID.
 */
double YPowerSupply::get_currentLimit(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::CURRENTLIMIT_INVALID;
                }
            }
        }
        res = _currentLimit;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the power supply output switch state.
 *
 * @return either Y_POWEROUTPUT_OFF or Y_POWEROUTPUT_ON, according to the power supply output switch state
 *
 * On failure, throws an exception or returns Y_POWEROUTPUT_INVALID.
 */
Y_POWEROUTPUT_enum YPowerSupply::get_powerOutput(void)
{
    Y_POWEROUTPUT_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::POWEROUTPUT_INVALID;
                }
            }
        }
        res = _powerOutput;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the power supply output switch state.
 *
 * @param newval : either Y_POWEROUTPUT_OFF or Y_POWEROUTPUT_ON, according to the power supply output switch state
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YPowerSupply::set_powerOutput(Y_POWEROUTPUT_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = (newval>0 ? "1" : "0");
        res = _setAttr("powerOutput", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the output voltage control point.
 *
 * @return either Y_VOLTAGESENSE_INT or Y_VOLTAGESENSE_EXT, according to the output voltage control point
 *
 * On failure, throws an exception or returns Y_VOLTAGESENSE_INVALID.
 */
Y_VOLTAGESENSE_enum YPowerSupply::get_voltageSense(void)
{
    Y_VOLTAGESENSE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::VOLTAGESENSE_INVALID;
                }
            }
        }
        res = _voltageSense;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the voltage control point.
 *
 * @param newval : either Y_VOLTAGESENSE_INT or Y_VOLTAGESENSE_EXT, according to the voltage control point
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YPowerSupply::set_voltageSense(Y_VOLTAGESENSE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("voltageSense", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the measured output voltage, in V.
 *
 * @return a floating point number corresponding to the measured output voltage, in V
 *
 * On failure, throws an exception or returns Y_MEASUREDVOLTAGE_INVALID.
 */
double YPowerSupply::get_measuredVoltage(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::MEASUREDVOLTAGE_INVALID;
                }
            }
        }
        res = _measuredVoltage;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the measured output current, in mA.
 *
 * @return a floating point number corresponding to the measured output current, in mA
 *
 * On failure, throws an exception or returns Y_MEASUREDCURRENT_INVALID.
 */
double YPowerSupply::get_measuredCurrent(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::MEASUREDCURRENT_INVALID;
                }
            }
        }
        res = _measuredCurrent;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the measured input voltage, in V.
 *
 * @return a floating point number corresponding to the measured input voltage, in V
 *
 * On failure, throws an exception or returns Y_INPUTVOLTAGE_INVALID.
 */
double YPowerSupply::get_inputVoltage(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::INPUTVOLTAGE_INVALID;
                }
            }
        }
        res = _inputVoltage;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the internal voltage, in V.
 *
 * @return a floating point number corresponding to the internal voltage, in V
 *
 * On failure, throws an exception or returns Y_VINT_INVALID.
 */
double YPowerSupply::get_vInt(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::VINT_INVALID;
                }
            }
        }
        res = _vInt;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the LDO temperature, in Celsius.
 *
 * @return a floating point number corresponding to the LDO temperature, in Celsius
 *
 * On failure, throws an exception or returns Y_LDOTEMPERATURE_INVALID.
 */
double YPowerSupply::get_ldoTemperature(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::LDOTEMPERATURE_INVALID;
                }
            }
        }
        res = _ldoTemperature;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YPowerSupply::get_voltageTransition(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::VOLTAGETRANSITION_INVALID;
                }
            }
        }
        res = _voltageTransition;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YPowerSupply::set_voltageTransition(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("voltageTransition", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the voltage set point at device start up. Remember to call the matching
 * module saveToFlash() method, otherwise this call has no effect.
 *
 * @param newval : a floating point number corresponding to the voltage set point at device start up
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YPowerSupply::set_voltageAtStartUp(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("voltageAtStartUp", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the selected voltage set point at device startup, in V.
 *
 * @return a floating point number corresponding to the selected voltage set point at device startup, in V
 *
 * On failure, throws an exception or returns Y_VOLTAGEATSTARTUP_INVALID.
 */
double YPowerSupply::get_voltageAtStartUp(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::VOLTAGEATSTARTUP_INVALID;
                }
            }
        }
        res = _voltageAtStartUp;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the current limit at device start up. Remember to call the matching
 * module saveToFlash() method, otherwise this call has no effect.
 *
 * @param newval : a floating point number corresponding to the current limit at device start up
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YPowerSupply::set_currentAtStartUp(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("currentAtStartUp", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the selected current limit at device startup, in mA.
 *
 * @return a floating point number corresponding to the selected current limit at device startup, in mA
 *
 * On failure, throws an exception or returns Y_CURRENTATSTARTUP_INVALID.
 */
double YPowerSupply::get_currentAtStartUp(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::CURRENTATSTARTUP_INVALID;
                }
            }
        }
        res = _currentAtStartUp;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YPowerSupply::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YPowerSupply::COMMAND_INVALID;
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

int YPowerSupply::set_command(const string& newval)
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
 * Retrieves a regulated power supply for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the regulated power supply is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YPowerSupply.isOnline() to test if the regulated power supply is
 * indeed online at a given time. In case of ambiguity when looking for
 * a regulated power supply by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the regulated power supply
 *
 * @return a YPowerSupply object allowing you to drive the regulated power supply.
 */
YPowerSupply* YPowerSupply::FindPowerSupply(string func)
{
    YPowerSupply* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YPowerSupply*) YFunction::_FindFromCache("PowerSupply", func);
        if (obj == NULL) {
            obj = new YPowerSupply(func);
            YFunction::_AddToCache("PowerSupply", func, obj);
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
int YPowerSupply::registerValueCallback(YPowerSupplyValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackPowerSupply = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YPowerSupply::_invokeValueCallback(string value)
{
    if (_valueCallbackPowerSupply != NULL) {
        _valueCallbackPowerSupply(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Performs a smooth transistion of output voltage. Any explicit voltage
 * change cancels any ongoing transition process.
 *
 * @param V_target   : new output voltage value at the end of the transition
 *         (floating-point number, representing the end voltage in V)
 * @param ms_duration : total duration of the transition, in milliseconds
 *
 * @return YAPI_SUCCESS when the call succeeds.
 */
int YPowerSupply::voltageMove(double V_target,int ms_duration)
{
    string newval;
    if (V_target < 0.0) {
        V_target  = 0.0;
    }
    newval = YapiWrapper::ysprintf("%d:%d", (int) floor(V_target*65536+0.5),ms_duration);

    return this->set_voltageTransition(newval);
}

YPowerSupply *YPowerSupply::nextPowerSupply(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YPowerSupply::FindPowerSupply(hwid);
}

YPowerSupply* YPowerSupply::FirstPowerSupply(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("PowerSupply", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YPowerSupply::FindPowerSupply(serial+"."+funcId);
}

//--- (end of YPowerSupply implementation)

//--- (YPowerSupply functions)
//--- (end of YPowerSupply functions)
