/*********************************************************************
 *
 * $Id: yocto_voltageoutput.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindVoltageOutput(), the high-level API for VoltageOutput functions
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
#include "yocto_voltageoutput.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "voltageoutput"

YVoltageOutput::YVoltageOutput(const string& func): YFunction(func)
//--- (YVoltageOutput initialization)
    ,_currentVoltage(CURRENTVOLTAGE_INVALID)
    ,_voltageTransition(VOLTAGETRANSITION_INVALID)
    ,_voltageAtStartUp(VOLTAGEATSTARTUP_INVALID)
    ,_valueCallbackVoltageOutput(NULL)
//--- (end of YVoltageOutput initialization)
{
    _className="VoltageOutput";
}

YVoltageOutput::~YVoltageOutput()
{
//--- (YVoltageOutput cleanup)
//--- (end of YVoltageOutput cleanup)
}
//--- (YVoltageOutput implementation)
// static attributes
const double YVoltageOutput::CURRENTVOLTAGE_INVALID = YAPI_INVALID_DOUBLE;
const string YVoltageOutput::VOLTAGETRANSITION_INVALID = YAPI_INVALID_STRING;
const double YVoltageOutput::VOLTAGEATSTARTUP_INVALID = YAPI_INVALID_DOUBLE;

int YVoltageOutput::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("currentVoltage")) {
        _currentVoltage =  floor(json_val->getDouble("currentVoltage") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("voltageTransition")) {
        _voltageTransition =  json_val->getString("voltageTransition");
    }
    if(json_val->has("voltageAtStartUp")) {
        _voltageAtStartUp =  floor(json_val->getDouble("voltageAtStartUp") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Changes the output voltage, in V. Valid range is from 0 to 10V.
 *
 * @param newval : a floating point number corresponding to the output voltage, in V
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YVoltageOutput::set_currentVoltage(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("currentVoltage", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the output voltage set point, in V.
 *
 * @return a floating point number corresponding to the output voltage set point, in V
 *
 * On failure, throws an exception or returns Y_CURRENTVOLTAGE_INVALID.
 */
double YVoltageOutput::get_currentVoltage(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YVoltageOutput::CURRENTVOLTAGE_INVALID;
                }
            }
        }
        res = _currentVoltage;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YVoltageOutput::get_voltageTransition(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YVoltageOutput::VOLTAGETRANSITION_INVALID;
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

int YVoltageOutput::set_voltageTransition(const string& newval)
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
 * Changes the output voltage at device start up. Remember to call the matching
 * module saveToFlash() method, otherwise this call has no effect.
 *
 * @param newval : a floating point number corresponding to the output voltage at device start up
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YVoltageOutput::set_voltageAtStartUp(double newval)
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
 * Returns the selected voltage output at device startup, in V.
 *
 * @return a floating point number corresponding to the selected voltage output at device startup, in V
 *
 * On failure, throws an exception or returns Y_VOLTAGEATSTARTUP_INVALID.
 */
double YVoltageOutput::get_voltageAtStartUp(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YVoltageOutput::VOLTAGEATSTARTUP_INVALID;
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
 * Retrieves a voltage output for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the voltage output is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YVoltageOutput.isOnline() to test if the voltage output is
 * indeed online at a given time. In case of ambiguity when looking for
 * a voltage output by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the voltage output
 *
 * @return a YVoltageOutput object allowing you to drive the voltage output.
 */
YVoltageOutput* YVoltageOutput::FindVoltageOutput(string func)
{
    YVoltageOutput* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YVoltageOutput*) YFunction::_FindFromCache("VoltageOutput", func);
        if (obj == NULL) {
            obj = new YVoltageOutput(func);
            YFunction::_AddToCache("VoltageOutput", func, obj);
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
int YVoltageOutput::registerValueCallback(YVoltageOutputValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackVoltageOutput = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YVoltageOutput::_invokeValueCallback(string value)
{
    if (_valueCallbackVoltageOutput != NULL) {
        _valueCallbackVoltageOutput(this, value);
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
int YVoltageOutput::voltageMove(double V_target,int ms_duration)
{
    string newval;
    if (V_target < 0.0) {
        V_target  = 0.0;
    }
    if (V_target > 10.0) {
        V_target = 10.0;
    }
    newval = YapiWrapper::ysprintf("%d:%d", (int) floor(V_target*65536+0.5),ms_duration);

    return this->set_voltageTransition(newval);
}

YVoltageOutput *YVoltageOutput::nextVoltageOutput(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YVoltageOutput::FindVoltageOutput(hwid);
}

YVoltageOutput* YVoltageOutput::FirstVoltageOutput(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("VoltageOutput", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YVoltageOutput::FindVoltageOutput(serial+"."+funcId);
}

//--- (end of YVoltageOutput implementation)

//--- (YVoltageOutput functions)
//--- (end of YVoltageOutput functions)
