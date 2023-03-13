/*********************************************************************
 *
 * $Id: yocto_currentloopoutput.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindCurrentLoopOutput(), the high-level API for CurrentLoopOutput functions
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
#include "yocto_currentloopoutput.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "currentloopoutput"

YCurrentLoopOutput::YCurrentLoopOutput(const string& func): YFunction(func)
//--- (YCurrentLoopOutput initialization)
    ,_current(CURRENT_INVALID)
    ,_currentTransition(CURRENTTRANSITION_INVALID)
    ,_currentAtStartUp(CURRENTATSTARTUP_INVALID)
    ,_loopPower(LOOPPOWER_INVALID)
    ,_valueCallbackCurrentLoopOutput(NULL)
//--- (end of YCurrentLoopOutput initialization)
{
    _className="CurrentLoopOutput";
}

YCurrentLoopOutput::~YCurrentLoopOutput()
{
//--- (YCurrentLoopOutput cleanup)
//--- (end of YCurrentLoopOutput cleanup)
}
//--- (YCurrentLoopOutput implementation)
// static attributes
const double YCurrentLoopOutput::CURRENT_INVALID = YAPI_INVALID_DOUBLE;
const string YCurrentLoopOutput::CURRENTTRANSITION_INVALID = YAPI_INVALID_STRING;
const double YCurrentLoopOutput::CURRENTATSTARTUP_INVALID = YAPI_INVALID_DOUBLE;

int YCurrentLoopOutput::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("current")) {
        _current =  floor(json_val->getDouble("current") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("currentTransition")) {
        _currentTransition =  json_val->getString("currentTransition");
    }
    if(json_val->has("currentAtStartUp")) {
        _currentAtStartUp =  floor(json_val->getDouble("currentAtStartUp") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("loopPower")) {
        _loopPower =  (Y_LOOPPOWER_enum)json_val->getInt("loopPower");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Changes the current loop, the valid range is from 3 to 21mA. If the loop is
 * not propely powered, the  target current is not reached and
 * loopPower is set to LOWPWR.
 *
 * @param newval : a floating point number corresponding to the current loop, the valid range is from 3 to 21mA
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCurrentLoopOutput::set_current(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("current", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the loop current set point in mA.
 *
 * @return a floating point number corresponding to the loop current set point in mA
 *
 * On failure, throws an exception or returns Y_CURRENT_INVALID.
 */
double YCurrentLoopOutput::get_current(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCurrentLoopOutput::CURRENT_INVALID;
                }
            }
        }
        res = _current;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YCurrentLoopOutput::get_currentTransition(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCurrentLoopOutput::CURRENTTRANSITION_INVALID;
                }
            }
        }
        res = _currentTransition;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YCurrentLoopOutput::set_currentTransition(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("currentTransition", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the loop current at device start up. Remember to call the matching
 * module saveToFlash() method, otherwise this call has no effect.
 *
 * @param newval : a floating point number corresponding to the loop current at device start up
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCurrentLoopOutput::set_currentAtStartUp(double newval)
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
 * Returns the current in the loop at device startup, in mA.
 *
 * @return a floating point number corresponding to the current in the loop at device startup, in mA
 *
 * On failure, throws an exception or returns Y_CURRENTATSTARTUP_INVALID.
 */
double YCurrentLoopOutput::get_currentAtStartUp(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCurrentLoopOutput::CURRENTATSTARTUP_INVALID;
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

/**
 * Returns the loop powerstate.  POWEROK: the loop
 * is powered. NOPWR: the loop in not powered. LOWPWR: the loop is not
 * powered enough to maintain the current required (insufficient voltage).
 *
 * @return a value among Y_LOOPPOWER_NOPWR, Y_LOOPPOWER_LOWPWR and Y_LOOPPOWER_POWEROK corresponding
 * to the loop powerstate
 *
 * On failure, throws an exception or returns Y_LOOPPOWER_INVALID.
 */
Y_LOOPPOWER_enum YCurrentLoopOutput::get_loopPower(void)
{
    Y_LOOPPOWER_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCurrentLoopOutput::LOOPPOWER_INVALID;
                }
            }
        }
        res = _loopPower;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a 4-20mA output for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the 4-20mA output is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCurrentLoopOutput.isOnline() to test if the 4-20mA output is
 * indeed online at a given time. In case of ambiguity when looking for
 * a 4-20mA output by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the 4-20mA output
 *
 * @return a YCurrentLoopOutput object allowing you to drive the 4-20mA output.
 */
YCurrentLoopOutput* YCurrentLoopOutput::FindCurrentLoopOutput(string func)
{
    YCurrentLoopOutput* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YCurrentLoopOutput*) YFunction::_FindFromCache("CurrentLoopOutput", func);
        if (obj == NULL) {
            obj = new YCurrentLoopOutput(func);
            YFunction::_AddToCache("CurrentLoopOutput", func, obj);
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
int YCurrentLoopOutput::registerValueCallback(YCurrentLoopOutputValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackCurrentLoopOutput = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YCurrentLoopOutput::_invokeValueCallback(string value)
{
    if (_valueCallbackCurrentLoopOutput != NULL) {
        _valueCallbackCurrentLoopOutput(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Performs a smooth transistion of current flowing in the loop. Any current explicit
 * change cancels any ongoing transition process.
 *
 * @param mA_target   : new current value at the end of the transition
 *         (floating-point number, representing the end current in mA)
 * @param ms_duration : total duration of the transition, in milliseconds
 *
 * @return YAPI_SUCCESS when the call succeeds.
 */
int YCurrentLoopOutput::currentMove(double mA_target,int ms_duration)
{
    string newval;
    if (mA_target < 3.0) {
        mA_target  = 3.0;
    }
    if (mA_target > 21.0) {
        mA_target = 21.0;
    }
    newval = YapiWrapper::ysprintf("%d:%d", (int) floor(mA_target*65536+0.5),ms_duration);

    return this->set_currentTransition(newval);
}

YCurrentLoopOutput *YCurrentLoopOutput::nextCurrentLoopOutput(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YCurrentLoopOutput::FindCurrentLoopOutput(hwid);
}

YCurrentLoopOutput* YCurrentLoopOutput::FirstCurrentLoopOutput(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("CurrentLoopOutput", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YCurrentLoopOutput::FindCurrentLoopOutput(serial+"."+funcId);
}

//--- (end of YCurrentLoopOutput implementation)

//--- (YCurrentLoopOutput functions)
//--- (end of YCurrentLoopOutput functions)
