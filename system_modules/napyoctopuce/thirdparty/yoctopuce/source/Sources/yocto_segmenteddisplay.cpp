/*********************************************************************
 *
 * $Id: yocto_segmenteddisplay.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindSegmentedDisplay(), the high-level API for SegmentedDisplay functions
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
#include "yocto_segmenteddisplay.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "segmenteddisplay"

YSegmentedDisplay::YSegmentedDisplay(const string& func): YFunction(func)
//--- (YSegmentedDisplay initialization)
    ,_displayedText(DISPLAYEDTEXT_INVALID)
    ,_displayMode(DISPLAYMODE_INVALID)
    ,_valueCallbackSegmentedDisplay(NULL)
//--- (end of YSegmentedDisplay initialization)
{
    _className="SegmentedDisplay";
}

YSegmentedDisplay::~YSegmentedDisplay()
{
//--- (YSegmentedDisplay cleanup)
//--- (end of YSegmentedDisplay cleanup)
}
//--- (YSegmentedDisplay implementation)
// static attributes
const string YSegmentedDisplay::DISPLAYEDTEXT_INVALID = YAPI_INVALID_STRING;

int YSegmentedDisplay::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("displayedText")) {
        _displayedText =  json_val->getString("displayedText");
    }
    if(json_val->has("displayMode")) {
        _displayMode =  (Y_DISPLAYMODE_enum)json_val->getInt("displayMode");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the text currently displayed on the screen.
 *
 * @return a string corresponding to the text currently displayed on the screen
 *
 * On failure, throws an exception or returns Y_DISPLAYEDTEXT_INVALID.
 */
string YSegmentedDisplay::get_displayedText(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSegmentedDisplay::DISPLAYEDTEXT_INVALID;
                }
            }
        }
        res = _displayedText;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the text currently displayed on the screen.
 *
 * @param newval : a string corresponding to the text currently displayed on the screen
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSegmentedDisplay::set_displayedText(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("displayedText", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

Y_DISPLAYMODE_enum YSegmentedDisplay::get_displayMode(void)
{
    Y_DISPLAYMODE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSegmentedDisplay::DISPLAYMODE_INVALID;
                }
            }
        }
        res = _displayMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YSegmentedDisplay::set_displayMode(Y_DISPLAYMODE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("displayMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a segmented display for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the segmented displays is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YSegmentedDisplay.isOnline() to test if the segmented displays is
 * indeed online at a given time. In case of ambiguity when looking for
 * a segmented display by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the segmented displays
 *
 * @return a YSegmentedDisplay object allowing you to drive the segmented displays.
 */
YSegmentedDisplay* YSegmentedDisplay::FindSegmentedDisplay(string func)
{
    YSegmentedDisplay* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YSegmentedDisplay*) YFunction::_FindFromCache("SegmentedDisplay", func);
        if (obj == NULL) {
            obj = new YSegmentedDisplay(func);
            YFunction::_AddToCache("SegmentedDisplay", func, obj);
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
int YSegmentedDisplay::registerValueCallback(YSegmentedDisplayValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackSegmentedDisplay = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YSegmentedDisplay::_invokeValueCallback(string value)
{
    if (_valueCallbackSegmentedDisplay != NULL) {
        _valueCallbackSegmentedDisplay(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

YSegmentedDisplay *YSegmentedDisplay::nextSegmentedDisplay(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YSegmentedDisplay::FindSegmentedDisplay(hwid);
}

YSegmentedDisplay* YSegmentedDisplay::FirstSegmentedDisplay(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("SegmentedDisplay", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YSegmentedDisplay::FindSegmentedDisplay(serial+"."+funcId);
}

//--- (end of YSegmentedDisplay implementation)

//--- (YSegmentedDisplay functions)
//--- (end of YSegmentedDisplay functions)
