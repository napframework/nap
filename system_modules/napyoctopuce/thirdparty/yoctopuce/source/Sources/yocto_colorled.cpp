/*********************************************************************
 *
 * $Id: yocto_colorled.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindColorLed(), the high-level API for ColorLed functions
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
#include "yocto_colorled.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "colorled"

YColorLed::YColorLed(const string& func): YFunction(func)
//--- (YColorLed initialization)
    ,_rgbColor(RGBCOLOR_INVALID)
    ,_hslColor(HSLCOLOR_INVALID)
    ,_rgbMove(RGBMOVE_INVALID)
    ,_hslMove(HSLMOVE_INVALID)
    ,_rgbColorAtPowerOn(RGBCOLORATPOWERON_INVALID)
    ,_blinkSeqSize(BLINKSEQSIZE_INVALID)
    ,_blinkSeqMaxSize(BLINKSEQMAXSIZE_INVALID)
    ,_blinkSeqSignature(BLINKSEQSIGNATURE_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackColorLed(NULL)
//--- (end of YColorLed initialization)
{
    _className="ColorLed";
}

YColorLed::~YColorLed()
{
//--- (YColorLed cleanup)
//--- (end of YColorLed cleanup)
}
//--- (YColorLed implementation)
// static attributes
const YMove YColorLed::RGBMOVE_INVALID = YMove();
const YMove YColorLed::HSLMOVE_INVALID = YMove();
const string YColorLed::COMMAND_INVALID = YAPI_INVALID_STRING;

int YColorLed::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("rgbColor")) {
        _rgbColor =  json_val->getInt("rgbColor");
    }
    if(json_val->has("hslColor")) {
        _hslColor =  json_val->getInt("hslColor");
    }
    if(json_val->has("rgbMove")) {
        YJSONObject* subjson = json_val->getYJSONObject("rgbMove");
        if (subjson->has("moving")) {
            _rgbMove.moving = subjson->getInt("moving");
        }
        if (subjson->has("target")) {
            _rgbMove.target = subjson->getInt("target");
        }
        if (subjson->has("ms")) {
            _rgbMove.ms = subjson->getInt("ms");
        }
    }
    if(json_val->has("hslMove")) {
        YJSONObject* subjson = json_val->getYJSONObject("hslMove");
        if (subjson->has("moving")) {
            _hslMove.moving = subjson->getInt("moving");
        }
        if (subjson->has("target")) {
            _hslMove.target = subjson->getInt("target");
        }
        if (subjson->has("ms")) {
            _hslMove.ms = subjson->getInt("ms");
        }
    }
    if(json_val->has("rgbColorAtPowerOn")) {
        _rgbColorAtPowerOn =  json_val->getInt("rgbColorAtPowerOn");
    }
    if(json_val->has("blinkSeqSize")) {
        _blinkSeqSize =  json_val->getInt("blinkSeqSize");
    }
    if(json_val->has("blinkSeqMaxSize")) {
        _blinkSeqMaxSize =  json_val->getInt("blinkSeqMaxSize");
    }
    if(json_val->has("blinkSeqSignature")) {
        _blinkSeqSignature =  json_val->getInt("blinkSeqSignature");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the current RGB color of the LED.
 *
 * @return an integer corresponding to the current RGB color of the LED
 *
 * On failure, throws an exception or returns Y_RGBCOLOR_INVALID.
 */
int YColorLed::get_rgbColor(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::RGBCOLOR_INVALID;
                }
            }
        }
        res = _rgbColor;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the current color of the LED, using an RGB color. Encoding is done as follows: 0xRRGGBB.
 *
 * @param newval : an integer corresponding to the current color of the LED, using an RGB color
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLed::set_rgbColor(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"0x%06x",newval); rest_val = string(buf);
        res = _setAttr("rgbColor", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current HSL color of the LED.
 *
 * @return an integer corresponding to the current HSL color of the LED
 *
 * On failure, throws an exception or returns Y_HSLCOLOR_INVALID.
 */
int YColorLed::get_hslColor(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::HSLCOLOR_INVALID;
                }
            }
        }
        res = _hslColor;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the current color of the LED, using a color HSL. Encoding is done as follows: 0xHHSSLL.
 *
 * @param newval : an integer corresponding to the current color of the LED, using a color HSL
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLed::set_hslColor(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"0x%06x",newval); rest_val = string(buf);
        res = _setAttr("hslColor", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

YMove YColorLed::get_rgbMove(void)
{
    YMove res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::RGBMOVE_INVALID;
                }
            }
        }
        res = _rgbMove;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YColorLed::set_rgbMove(YMove newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buff[64]; sprintf(buff,"%d:%d",newval.target,newval.ms); rest_val = string(buff);
        res = _setAttr("rgbMove", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Performs a smooth transition in the RGB color space between the current color and a target color.
 *
 * @param rgb_target  : desired RGB color at the end of the transition
 * @param ms_duration : duration of the transition, in millisecond
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLed::rgbMove(int rgb_target,int ms_duration)
{
    string rest_val;
    char buff[64]; sprintf(buff,"%d:%d",rgb_target,ms_duration); rest_val = string(buff);
    return _setAttr("rgbMove", rest_val);
}

YMove YColorLed::get_hslMove(void)
{
    YMove res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::HSLMOVE_INVALID;
                }
            }
        }
        res = _hslMove;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YColorLed::set_hslMove(YMove newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buff[64]; sprintf(buff,"%d:%d",newval.target,newval.ms); rest_val = string(buff);
        res = _setAttr("hslMove", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Performs a smooth transition in the HSL color space between the current color and a target color.
 *
 * @param hsl_target  : desired HSL color at the end of the transition
 * @param ms_duration : duration of the transition, in millisecond
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLed::hslMove(int hsl_target,int ms_duration)
{
    string rest_val;
    char buff[64]; sprintf(buff,"%d:%d",hsl_target,ms_duration); rest_val = string(buff);
    return _setAttr("hslMove", rest_val);
}

/**
 * Returns the configured color to be displayed when the module is turned on.
 *
 * @return an integer corresponding to the configured color to be displayed when the module is turned on
 *
 * On failure, throws an exception or returns Y_RGBCOLORATPOWERON_INVALID.
 */
int YColorLed::get_rgbColorAtPowerOn(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::RGBCOLORATPOWERON_INVALID;
                }
            }
        }
        res = _rgbColorAtPowerOn;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the color that the LED will display by default when the module is turned on.
 *
 * @param newval : an integer corresponding to the color that the LED will display by default when the
 * module is turned on
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLed::set_rgbColorAtPowerOn(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"0x%06x",newval); rest_val = string(buf);
        res = _setAttr("rgbColorAtPowerOn", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current length of the blinking sequence.
 *
 * @return an integer corresponding to the current length of the blinking sequence
 *
 * On failure, throws an exception or returns Y_BLINKSEQSIZE_INVALID.
 */
int YColorLed::get_blinkSeqSize(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::BLINKSEQSIZE_INVALID;
                }
            }
        }
        res = _blinkSeqSize;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the maximum length of the blinking sequence.
 *
 * @return an integer corresponding to the maximum length of the blinking sequence
 *
 * On failure, throws an exception or returns Y_BLINKSEQMAXSIZE_INVALID.
 */
int YColorLed::get_blinkSeqMaxSize(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration == 0) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::BLINKSEQMAXSIZE_INVALID;
                }
            }
        }
        res = _blinkSeqMaxSize;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Return the blinking sequence signature. Since blinking
 * sequences cannot be read from the device, this can be used
 * to detect if a specific blinking sequence is already
 * programmed.
 *
 * @return an integer
 *
 * On failure, throws an exception or returns Y_BLINKSEQSIGNATURE_INVALID.
 */
int YColorLed::get_blinkSeqSignature(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::BLINKSEQSIGNATURE_INVALID;
                }
            }
        }
        res = _blinkSeqSignature;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YColorLed::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLed::COMMAND_INVALID;
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

int YColorLed::set_command(const string& newval)
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
 * Retrieves an RGB LED for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the RGB LED is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YColorLed.isOnline() to test if the RGB LED is
 * indeed online at a given time. In case of ambiguity when looking for
 * an RGB LED by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the RGB LED
 *
 * @return a YColorLed object allowing you to drive the RGB LED.
 */
YColorLed* YColorLed::FindColorLed(string func)
{
    YColorLed* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YColorLed*) YFunction::_FindFromCache("ColorLed", func);
        if (obj == NULL) {
            obj = new YColorLed(func);
            YFunction::_AddToCache("ColorLed", func, obj);
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
int YColorLed::registerValueCallback(YColorLedValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackColorLed = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YColorLed::_invokeValueCallback(string value)
{
    if (_valueCallbackColorLed != NULL) {
        _valueCallbackColorLed(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YColorLed::sendCommand(string command)
{
    return this->set_command(command);
}

/**
 * Add a new transition to the blinking sequence, the move will
 * be performed in the HSL space.
 *
 * @param HSLcolor : desired HSL color when the traisntion is completed
 * @param msDelay : duration of the color transition, in milliseconds.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YColorLed::addHslMoveToBlinkSeq(int HSLcolor,int msDelay)
{
    return this->sendCommand(YapiWrapper::ysprintf("H%d,%d",HSLcolor,msDelay));
}

/**
 * Adds a new transition to the blinking sequence, the move is
 * performed in the RGB space.
 *
 * @param RGBcolor : desired RGB color when the transition is completed
 * @param msDelay : duration of the color transition, in milliseconds.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YColorLed::addRgbMoveToBlinkSeq(int RGBcolor,int msDelay)
{
    return this->sendCommand(YapiWrapper::ysprintf("R%d,%d",RGBcolor,msDelay));
}

/**
 * Starts the preprogrammed blinking sequence. The sequence is
 * run in a loop until it is stopped by stopBlinkSeq or an explicit
 * change.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YColorLed::startBlinkSeq(void)
{
    return this->sendCommand("S");
}

/**
 * Stops the preprogrammed blinking sequence.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YColorLed::stopBlinkSeq(void)
{
    return this->sendCommand("X");
}

/**
 * Resets the preprogrammed blinking sequence.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YColorLed::resetBlinkSeq(void)
{
    return this->sendCommand("Z");
}

YColorLed *YColorLed::nextColorLed(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YColorLed::FindColorLed(hwid);
}

YColorLed* YColorLed::FirstColorLed(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("ColorLed", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YColorLed::FindColorLed(serial+"."+funcId);
}

//--- (end of YColorLed implementation)

//--- (YColorLed functions)
//--- (end of YColorLed functions)
