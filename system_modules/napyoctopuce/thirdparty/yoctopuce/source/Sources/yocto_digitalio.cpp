/*********************************************************************
 *
 * $Id: yocto_digitalio.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindDigitalIO(), the high-level API for DigitalIO functions
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
#include "yocto_digitalio.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "digitalio"

YDigitalIO::YDigitalIO(const string& func): YFunction(func)
//--- (YDigitalIO initialization)
    ,_portState(PORTSTATE_INVALID)
    ,_portDirection(PORTDIRECTION_INVALID)
    ,_portOpenDrain(PORTOPENDRAIN_INVALID)
    ,_portPolarity(PORTPOLARITY_INVALID)
    ,_portDiags(PORTDIAGS_INVALID)
    ,_portSize(PORTSIZE_INVALID)
    ,_outputVoltage(OUTPUTVOLTAGE_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackDigitalIO(NULL)
//--- (end of YDigitalIO initialization)
{
    _className="DigitalIO";
}

YDigitalIO::~YDigitalIO()
{
//--- (YDigitalIO cleanup)
//--- (end of YDigitalIO cleanup)
}
//--- (YDigitalIO implementation)
// static attributes
const string YDigitalIO::COMMAND_INVALID = YAPI_INVALID_STRING;

int YDigitalIO::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("portState")) {
        _portState =  json_val->getInt("portState");
    }
    if(json_val->has("portDirection")) {
        _portDirection =  json_val->getInt("portDirection");
    }
    if(json_val->has("portOpenDrain")) {
        _portOpenDrain =  json_val->getInt("portOpenDrain");
    }
    if(json_val->has("portPolarity")) {
        _portPolarity =  json_val->getInt("portPolarity");
    }
    if(json_val->has("portDiags")) {
        _portDiags =  json_val->getInt("portDiags");
    }
    if(json_val->has("portSize")) {
        _portSize =  json_val->getInt("portSize");
    }
    if(json_val->has("outputVoltage")) {
        _outputVoltage =  (Y_OUTPUTVOLTAGE_enum)json_val->getInt("outputVoltage");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the digital IO port state: bit 0 represents input 0, and so on.
 *
 * @return an integer corresponding to the digital IO port state: bit 0 represents input 0, and so on
 *
 * On failure, throws an exception or returns Y_PORTSTATE_INVALID.
 */
int YDigitalIO::get_portState(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::PORTSTATE_INVALID;
                }
            }
        }
        res = _portState;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the digital IO port state: bit 0 represents input 0, and so on. This function has no effect
 * on bits configured as input in portDirection.
 *
 * @param newval : an integer corresponding to the digital IO port state: bit 0 represents input 0, and so on
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_portState(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("portState", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the IO direction of all bits of the port: 0 makes a bit an input, 1 makes it an output.
 *
 * @return an integer corresponding to the IO direction of all bits of the port: 0 makes a bit an
 * input, 1 makes it an output
 *
 * On failure, throws an exception or returns Y_PORTDIRECTION_INVALID.
 */
int YDigitalIO::get_portDirection(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::PORTDIRECTION_INVALID;
                }
            }
        }
        res = _portDirection;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the IO direction of all bits of the port: 0 makes a bit an input, 1 makes it an output.
 * Remember to call the saveToFlash() method  to make sure the setting is kept after a reboot.
 *
 * @param newval : an integer corresponding to the IO direction of all bits of the port: 0 makes a bit
 * an input, 1 makes it an output
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_portDirection(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("portDirection", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the electrical interface for each bit of the port. For each bit set to 0  the matching I/O
 * works in the regular,
 * intuitive way, for each bit set to 1, the I/O works in reverse mode.
 *
 * @return an integer corresponding to the electrical interface for each bit of the port
 *
 * On failure, throws an exception or returns Y_PORTOPENDRAIN_INVALID.
 */
int YDigitalIO::get_portOpenDrain(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::PORTOPENDRAIN_INVALID;
                }
            }
        }
        res = _portOpenDrain;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the electrical interface for each bit of the port. 0 makes a bit a regular input/output, 1 makes
 * it an open-drain (open-collector) input/output. Remember to call the
 * saveToFlash() method  to make sure the setting is kept after a reboot.
 *
 * @param newval : an integer corresponding to the electrical interface for each bit of the port
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_portOpenDrain(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("portOpenDrain", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the polarity of all the bits of the port.  For each bit set to 0, the matching I/O works the regular,
 * intuitive way; for each bit set to 1, the I/O works in reverse mode.
 *
 * @return an integer corresponding to the polarity of all the bits of the port
 *
 * On failure, throws an exception or returns Y_PORTPOLARITY_INVALID.
 */
int YDigitalIO::get_portPolarity(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::PORTPOLARITY_INVALID;
                }
            }
        }
        res = _portPolarity;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the polarity of all the bits of the port: For each bit set to 0, the matching I/O works the regular,
 * intuitive way; for each bit set to 1, the I/O works in reverse mode.
 * Remember to call the saveToFlash() method  to make sure the setting will be kept after a reboot.
 *
 * @param newval : an integer corresponding to the polarity of all the bits of the port: For each bit
 * set to 0, the matching I/O works the regular,
 *         intuitive way; for each bit set to 1, the I/O works in reverse mode
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_portPolarity(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("portPolarity", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the port state diagnostics (Yocto-IO and Yocto-MaxiIO-V2 only). Bit 0 indicates a shortcut on
 * output 0, etc. Bit 8 indicates a power failure, and bit 9 signals overheating (overcurrent).
 * During normal use, all diagnostic bits should stay clear.
 *
 * @return an integer corresponding to the port state diagnostics (Yocto-IO and Yocto-MaxiIO-V2 only)
 *
 * On failure, throws an exception or returns Y_PORTDIAGS_INVALID.
 */
int YDigitalIO::get_portDiags(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::PORTDIAGS_INVALID;
                }
            }
        }
        res = _portDiags;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of bits implemented in the I/O port.
 *
 * @return an integer corresponding to the number of bits implemented in the I/O port
 *
 * On failure, throws an exception or returns Y_PORTSIZE_INVALID.
 */
int YDigitalIO::get_portSize(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::PORTSIZE_INVALID;
                }
            }
        }
        res = _portSize;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the voltage source used to drive output bits.
 *
 * @return a value among Y_OUTPUTVOLTAGE_USB_5V, Y_OUTPUTVOLTAGE_USB_3V and Y_OUTPUTVOLTAGE_EXT_V
 * corresponding to the voltage source used to drive output bits
 *
 * On failure, throws an exception or returns Y_OUTPUTVOLTAGE_INVALID.
 */
Y_OUTPUTVOLTAGE_enum YDigitalIO::get_outputVoltage(void)
{
    Y_OUTPUTVOLTAGE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::OUTPUTVOLTAGE_INVALID;
                }
            }
        }
        res = _outputVoltage;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the voltage source used to drive output bits.
 * Remember to call the saveToFlash() method  to make sure the setting is kept after a reboot.
 *
 * @param newval : a value among Y_OUTPUTVOLTAGE_USB_5V, Y_OUTPUTVOLTAGE_USB_3V and
 * Y_OUTPUTVOLTAGE_EXT_V corresponding to the voltage source used to drive output bits
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_outputVoltage(Y_OUTPUTVOLTAGE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("outputVoltage", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YDigitalIO::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YDigitalIO::COMMAND_INVALID;
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

int YDigitalIO::set_command(const string& newval)
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
 * Retrieves a digital IO port for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the digital IO port is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YDigitalIO.isOnline() to test if the digital IO port is
 * indeed online at a given time. In case of ambiguity when looking for
 * a digital IO port by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the digital IO port
 *
 * @return a YDigitalIO object allowing you to drive the digital IO port.
 */
YDigitalIO* YDigitalIO::FindDigitalIO(string func)
{
    YDigitalIO* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YDigitalIO*) YFunction::_FindFromCache("DigitalIO", func);
        if (obj == NULL) {
            obj = new YDigitalIO(func);
            YFunction::_AddToCache("DigitalIO", func, obj);
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
int YDigitalIO::registerValueCallback(YDigitalIOValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackDigitalIO = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YDigitalIO::_invokeValueCallback(string value)
{
    if (_valueCallbackDigitalIO != NULL) {
        _valueCallbackDigitalIO(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Sets a single bit of the I/O port.
 *
 * @param bitno : the bit number; lowest bit has index 0
 * @param bitstate : the state of the bit (1 or 0)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_bitState(int bitno,int bitstate)
{
    if (!(bitstate >= 0)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid bitstate");
        return YAPI_INVALID_ARGUMENT;
    }
    if (!(bitstate <= 1)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid bitstate");
        return YAPI_INVALID_ARGUMENT;
    }
    return this->set_command(YapiWrapper::ysprintf("%c%d",82+bitstate,bitno));
}

/**
 * Returns the state of a single bit of the I/O port.
 *
 * @param bitno : the bit number; lowest bit has index 0
 *
 * @return the bit state (0 or 1)
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::get_bitState(int bitno)
{
    int portVal = 0;
    portVal = this->get_portState();
    return ((((portVal) >> (bitno))) & (1));
}

/**
 * Reverts a single bit of the I/O port.
 *
 * @param bitno : the bit number; lowest bit has index 0
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::toggle_bitState(int bitno)
{
    return this->set_command(YapiWrapper::ysprintf("T%d",bitno));
}

/**
 * Changes  the direction of a single bit from the I/O port.
 *
 * @param bitno : the bit number; lowest bit has index 0
 * @param bitdirection : direction to set, 0 makes the bit an input, 1 makes it an output.
 *         Remember to call the   saveToFlash() method to make sure the setting is kept after a reboot.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_bitDirection(int bitno,int bitdirection)
{
    if (!(bitdirection >= 0)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid direction");
        return YAPI_INVALID_ARGUMENT;
    }
    if (!(bitdirection <= 1)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid direction");
        return YAPI_INVALID_ARGUMENT;
    }
    return this->set_command(YapiWrapper::ysprintf("%c%d",73+6*bitdirection,bitno));
}

/**
 * Returns the direction of a single bit from the I/O port (0 means the bit is an input, 1  an output).
 *
 * @param bitno : the bit number; lowest bit has index 0
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::get_bitDirection(int bitno)
{
    int portDir = 0;
    portDir = this->get_portDirection();
    return ((((portDir) >> (bitno))) & (1));
}

/**
 * Changes the polarity of a single bit from the I/O port.
 *
 * @param bitno : the bit number; lowest bit has index 0.
 * @param bitpolarity : polarity to set, 0 makes the I/O work in regular mode, 1 makes the I/O  works
 * in reverse mode.
 *         Remember to call the   saveToFlash() method to make sure the setting is kept after a reboot.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_bitPolarity(int bitno,int bitpolarity)
{
    if (!(bitpolarity >= 0)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid bitpolarity");
        return YAPI_INVALID_ARGUMENT;
    }
    if (!(bitpolarity <= 1)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid bitpolarity");
        return YAPI_INVALID_ARGUMENT;
    }
    return this->set_command(YapiWrapper::ysprintf("%c%d",110+4*bitpolarity,bitno));
}

/**
 * Returns the polarity of a single bit from the I/O port (0 means the I/O works in regular mode, 1
 * means the I/O  works in reverse mode).
 *
 * @param bitno : the bit number; lowest bit has index 0
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::get_bitPolarity(int bitno)
{
    int portPol = 0;
    portPol = this->get_portPolarity();
    return ((((portPol) >> (bitno))) & (1));
}

/**
 * Changes  the electrical interface of a single bit from the I/O port.
 *
 * @param bitno : the bit number; lowest bit has index 0
 * @param opendrain : 0 makes a bit a regular input/output, 1 makes
 *         it an open-drain (open-collector) input/output. Remember to call the
 *         saveToFlash() method to make sure the setting is kept after a reboot.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::set_bitOpenDrain(int bitno,int opendrain)
{
    if (!(opendrain >= 0)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid state");
        return YAPI_INVALID_ARGUMENT;
    }
    if (!(opendrain <= 1)) {
        _throw(YAPI_INVALID_ARGUMENT,"invalid state");
        return YAPI_INVALID_ARGUMENT;
    }
    return this->set_command(YapiWrapper::ysprintf("%c%d",100-32*opendrain,bitno));
}

/**
 * Returns the type of electrical interface of a single bit from the I/O port. (0 means the bit is an
 * input, 1  an output).
 *
 * @param bitno : the bit number; lowest bit has index 0
 *
 * @return   0 means the a bit is a regular input/output, 1 means the bit is an open-drain
 *         (open-collector) input/output.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::get_bitOpenDrain(int bitno)
{
    int portOpenDrain = 0;
    portOpenDrain = this->get_portOpenDrain();
    return ((((portOpenDrain) >> (bitno))) & (1));
}

/**
 * Triggers a pulse on a single bit for a specified duration. The specified bit
 * will be turned to 1, and then back to 0 after the given duration.
 *
 * @param bitno : the bit number; lowest bit has index 0
 * @param ms_duration : desired pulse duration in milliseconds. Be aware that the device time
 *         resolution is not guaranteed up to the millisecond.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::pulse(int bitno,int ms_duration)
{
    return this->set_command(YapiWrapper::ysprintf("Z%d,0,%d", bitno,ms_duration));
}

/**
 * Schedules a pulse on a single bit for a specified duration. The specified bit
 * will be turned to 1, and then back to 0 after the given duration.
 *
 * @param bitno : the bit number; lowest bit has index 0
 * @param ms_delay : waiting time before the pulse, in milliseconds
 * @param ms_duration : desired pulse duration in milliseconds. Be aware that the device time
 *         resolution is not guaranteed up to the millisecond.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YDigitalIO::delayedPulse(int bitno,int ms_delay,int ms_duration)
{
    return this->set_command(YapiWrapper::ysprintf("Z%d,%d,%d",bitno,ms_delay,ms_duration));
}

YDigitalIO *YDigitalIO::nextDigitalIO(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YDigitalIO::FindDigitalIO(hwid);
}

YDigitalIO* YDigitalIO::FirstDigitalIO(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("DigitalIO", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YDigitalIO::FindDigitalIO(serial+"."+funcId);
}

//--- (end of YDigitalIO implementation)

//--- (YDigitalIO functions)
//--- (end of YDigitalIO functions)
