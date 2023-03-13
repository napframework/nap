/*********************************************************************
 *
 * $Id: yocto_daisychain.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindDaisyChain(), the high-level API for DaisyChain functions
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
#include "yocto_daisychain.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "daisychain"

YDaisyChain::YDaisyChain(const string& func): YFunction(func)
//--- (YDaisyChain initialization)
    ,_daisyState(DAISYSTATE_INVALID)
    ,_childCount(CHILDCOUNT_INVALID)
    ,_requiredChildCount(REQUIREDCHILDCOUNT_INVALID)
    ,_valueCallbackDaisyChain(NULL)
//--- (end of YDaisyChain initialization)
{
    _className="DaisyChain";
}

YDaisyChain::~YDaisyChain()
{
//--- (YDaisyChain cleanup)
//--- (end of YDaisyChain cleanup)
}
//--- (YDaisyChain implementation)
// static attributes

int YDaisyChain::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("daisyState")) {
        _daisyState =  (Y_DAISYSTATE_enum)json_val->getInt("daisyState");
    }
    if(json_val->has("childCount")) {
        _childCount =  json_val->getInt("childCount");
    }
    if(json_val->has("requiredChildCount")) {
        _requiredChildCount =  json_val->getInt("requiredChildCount");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the state of the daisy-link between modules.
 *
 * @return a value among Y_DAISYSTATE_READY, Y_DAISYSTATE_IS_CHILD, Y_DAISYSTATE_FIRMWARE_MISMATCH,
 * Y_DAISYSTATE_CHILD_MISSING and Y_DAISYSTATE_CHILD_LOST corresponding to the state of the daisy-link
 * between modules
 *
 * On failure, throws an exception or returns Y_DAISYSTATE_INVALID.
 */
Y_DAISYSTATE_enum YDaisyChain::get_daisyState(void)
{
    Y_DAISYSTATE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDaisyChain::DAISYSTATE_INVALID;
                }
            }
        }
        res = _daisyState;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of child nodes currently detected.
 *
 * @return an integer corresponding to the number of child nodes currently detected
 *
 * On failure, throws an exception or returns Y_CHILDCOUNT_INVALID.
 */
int YDaisyChain::get_childCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDaisyChain::CHILDCOUNT_INVALID;
                }
            }
        }
        res = _childCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of child nodes expected in normal conditions.
 *
 * @return an integer corresponding to the number of child nodes expected in normal conditions
 *
 * On failure, throws an exception or returns Y_REQUIREDCHILDCOUNT_INVALID.
 */
int YDaisyChain::get_requiredChildCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDaisyChain::REQUIREDCHILDCOUNT_INVALID;
                }
            }
        }
        res = _requiredChildCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the number of child nodes expected in normal conditions.
 * If the value is zero, no check is performed. If it is non-zero, the number
 * child nodes is checked on startup and the status will change to error if
 * the count does not match.
 *
 * @param newval : an integer corresponding to the number of child nodes expected in normal conditions
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDaisyChain::set_requiredChildCount(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("requiredChildCount", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a module chain for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the module chain is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YDaisyChain.isOnline() to test if the module chain is
 * indeed online at a given time. In case of ambiguity when looking for
 * a module chain by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the module chain
 *
 * @return a YDaisyChain object allowing you to drive the module chain.
 */
YDaisyChain* YDaisyChain::FindDaisyChain(string func)
{
    YDaisyChain* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YDaisyChain*) YFunction::_FindFromCache("DaisyChain", func);
        if (obj == NULL) {
            obj = new YDaisyChain(func);
            YFunction::_AddToCache("DaisyChain", func, obj);
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
int YDaisyChain::registerValueCallback(YDaisyChainValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackDaisyChain = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YDaisyChain::_invokeValueCallback(string value)
{
    if (_valueCallbackDaisyChain != NULL) {
        _valueCallbackDaisyChain(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

YDaisyChain *YDaisyChain::nextDaisyChain(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YDaisyChain::FindDaisyChain(hwid);
}

YDaisyChain* YDaisyChain::FirstDaisyChain(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("DaisyChain", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YDaisyChain::FindDaisyChain(serial+"."+funcId);
}

//--- (end of YDaisyChain implementation)

//--- (YDaisyChain functions)
//--- (end of YDaisyChain functions)
