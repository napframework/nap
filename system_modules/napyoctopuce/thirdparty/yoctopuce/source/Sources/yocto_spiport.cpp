/*********************************************************************
 *
 * $Id: yocto_spiport.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindSpiPort(), the high-level API for SpiPort functions
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
#include "yocto_spiport.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "spiport"

YSpiPort::YSpiPort(const string& func): YFunction(func)
//--- (YSpiPort initialization)
    ,_rxCount(RXCOUNT_INVALID)
    ,_txCount(TXCOUNT_INVALID)
    ,_errCount(ERRCOUNT_INVALID)
    ,_rxMsgCount(RXMSGCOUNT_INVALID)
    ,_txMsgCount(TXMSGCOUNT_INVALID)
    ,_lastMsg(LASTMSG_INVALID)
    ,_currentJob(CURRENTJOB_INVALID)
    ,_startupJob(STARTUPJOB_INVALID)
    ,_command(COMMAND_INVALID)
    ,_voltageLevel(VOLTAGELEVEL_INVALID)
    ,_protocol(PROTOCOL_INVALID)
    ,_spiMode(SPIMODE_INVALID)
    ,_ssPolarity(SSPOLARITY_INVALID)
    ,_shitftSampling(SHITFTSAMPLING_INVALID)
    ,_valueCallbackSpiPort(NULL)
    ,_rxptr(0)
    ,_rxbuffptr(0)
//--- (end of YSpiPort initialization)
{
    _className="SpiPort";
}

YSpiPort::~YSpiPort()
{
//--- (YSpiPort cleanup)
//--- (end of YSpiPort cleanup)
}
//--- (YSpiPort implementation)
// static attributes
const string YSpiPort::LASTMSG_INVALID = YAPI_INVALID_STRING;
const string YSpiPort::CURRENTJOB_INVALID = YAPI_INVALID_STRING;
const string YSpiPort::STARTUPJOB_INVALID = YAPI_INVALID_STRING;
const string YSpiPort::COMMAND_INVALID = YAPI_INVALID_STRING;
const string YSpiPort::PROTOCOL_INVALID = YAPI_INVALID_STRING;
const string YSpiPort::SPIMODE_INVALID = YAPI_INVALID_STRING;

int YSpiPort::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("rxCount")) {
        _rxCount =  json_val->getInt("rxCount");
    }
    if(json_val->has("txCount")) {
        _txCount =  json_val->getInt("txCount");
    }
    if(json_val->has("errCount")) {
        _errCount =  json_val->getInt("errCount");
    }
    if(json_val->has("rxMsgCount")) {
        _rxMsgCount =  json_val->getInt("rxMsgCount");
    }
    if(json_val->has("txMsgCount")) {
        _txMsgCount =  json_val->getInt("txMsgCount");
    }
    if(json_val->has("lastMsg")) {
        _lastMsg =  json_val->getString("lastMsg");
    }
    if(json_val->has("currentJob")) {
        _currentJob =  json_val->getString("currentJob");
    }
    if(json_val->has("startupJob")) {
        _startupJob =  json_val->getString("startupJob");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    if(json_val->has("voltageLevel")) {
        _voltageLevel =  (Y_VOLTAGELEVEL_enum)json_val->getInt("voltageLevel");
    }
    if(json_val->has("protocol")) {
        _protocol =  json_val->getString("protocol");
    }
    if(json_val->has("spiMode")) {
        _spiMode =  json_val->getString("spiMode");
    }
    if(json_val->has("ssPolarity")) {
        _ssPolarity =  (Y_SSPOLARITY_enum)json_val->getInt("ssPolarity");
    }
    if(json_val->has("shitftSampling")) {
        _shitftSampling =  (Y_SHITFTSAMPLING_enum)json_val->getInt("shitftSampling");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the total number of bytes received since last reset.
 *
 * @return an integer corresponding to the total number of bytes received since last reset
 *
 * On failure, throws an exception or returns Y_RXCOUNT_INVALID.
 */
int YSpiPort::get_rxCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::RXCOUNT_INVALID;
                }
            }
        }
        res = _rxCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the total number of bytes transmitted since last reset.
 *
 * @return an integer corresponding to the total number of bytes transmitted since last reset
 *
 * On failure, throws an exception or returns Y_TXCOUNT_INVALID.
 */
int YSpiPort::get_txCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::TXCOUNT_INVALID;
                }
            }
        }
        res = _txCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the total number of communication errors detected since last reset.
 *
 * @return an integer corresponding to the total number of communication errors detected since last reset
 *
 * On failure, throws an exception or returns Y_ERRCOUNT_INVALID.
 */
int YSpiPort::get_errCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::ERRCOUNT_INVALID;
                }
            }
        }
        res = _errCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the total number of messages received since last reset.
 *
 * @return an integer corresponding to the total number of messages received since last reset
 *
 * On failure, throws an exception or returns Y_RXMSGCOUNT_INVALID.
 */
int YSpiPort::get_rxMsgCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::RXMSGCOUNT_INVALID;
                }
            }
        }
        res = _rxMsgCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the total number of messages send since last reset.
 *
 * @return an integer corresponding to the total number of messages send since last reset
 *
 * On failure, throws an exception or returns Y_TXMSGCOUNT_INVALID.
 */
int YSpiPort::get_txMsgCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::TXMSGCOUNT_INVALID;
                }
            }
        }
        res = _txMsgCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the latest message fully received (for Line and Frame protocols).
 *
 * @return a string corresponding to the latest message fully received (for Line and Frame protocols)
 *
 * On failure, throws an exception or returns Y_LASTMSG_INVALID.
 */
string YSpiPort::get_lastMsg(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::LASTMSG_INVALID;
                }
            }
        }
        res = _lastMsg;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the name of the job file currently in use.
 *
 * @return a string corresponding to the name of the job file currently in use
 *
 * On failure, throws an exception or returns Y_CURRENTJOB_INVALID.
 */
string YSpiPort::get_currentJob(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::CURRENTJOB_INVALID;
                }
            }
        }
        res = _currentJob;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the job to use when the device is powered on.
 * Remember to call the saveToFlash() method of the module if the
 * modification must be kept.
 *
 * @param newval : a string corresponding to the job to use when the device is powered on
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_currentJob(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("currentJob", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the job file to use when the device is powered on.
 *
 * @return a string corresponding to the job file to use when the device is powered on
 *
 * On failure, throws an exception or returns Y_STARTUPJOB_INVALID.
 */
string YSpiPort::get_startupJob(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::STARTUPJOB_INVALID;
                }
            }
        }
        res = _startupJob;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the job to use when the device is powered on.
 * Remember to call the saveToFlash() method of the module if the
 * modification must be kept.
 *
 * @param newval : a string corresponding to the job to use when the device is powered on
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_startupJob(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("startupJob", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YSpiPort::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::COMMAND_INVALID;
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

int YSpiPort::set_command(const string& newval)
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
 * Returns the voltage level used on the serial line.
 *
 * @return a value among Y_VOLTAGELEVEL_OFF, Y_VOLTAGELEVEL_TTL3V, Y_VOLTAGELEVEL_TTL3VR,
 * Y_VOLTAGELEVEL_TTL5V, Y_VOLTAGELEVEL_TTL5VR, Y_VOLTAGELEVEL_RS232 and Y_VOLTAGELEVEL_RS485
 * corresponding to the voltage level used on the serial line
 *
 * On failure, throws an exception or returns Y_VOLTAGELEVEL_INVALID.
 */
Y_VOLTAGELEVEL_enum YSpiPort::get_voltageLevel(void)
{
    Y_VOLTAGELEVEL_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::VOLTAGELEVEL_INVALID;
                }
            }
        }
        res = _voltageLevel;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the voltage type used on the serial line. Valid
 * values  will depend on the Yoctopuce device model featuring
 * the serial port feature.  Check your device documentation
 * to find out which values are valid for that specific model.
 * Trying to set an invalid value will have no effect.
 *
 * @param newval : a value among Y_VOLTAGELEVEL_OFF, Y_VOLTAGELEVEL_TTL3V, Y_VOLTAGELEVEL_TTL3VR,
 * Y_VOLTAGELEVEL_TTL5V, Y_VOLTAGELEVEL_TTL5VR, Y_VOLTAGELEVEL_RS232 and Y_VOLTAGELEVEL_RS485
 * corresponding to the voltage type used on the serial line
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_voltageLevel(Y_VOLTAGELEVEL_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("voltageLevel", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the type of protocol used over the serial line, as a string.
 * Possible values are "Line" for ASCII messages separated by CR and/or LF,
 * "Frame:[timeout]ms" for binary messages separated by a delay time,
 * "Char" for a continuous ASCII stream or
 * "Byte" for a continuous binary stream.
 *
 * @return a string corresponding to the type of protocol used over the serial line, as a string
 *
 * On failure, throws an exception or returns Y_PROTOCOL_INVALID.
 */
string YSpiPort::get_protocol(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::PROTOCOL_INVALID;
                }
            }
        }
        res = _protocol;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the type of protocol used over the serial line.
 * Possible values are "Line" for ASCII messages separated by CR and/or LF,
 * "Frame:[timeout]ms" for binary messages separated by a delay time,
 * "Char" for a continuous ASCII stream or
 * "Byte" for a continuous binary stream.
 * The suffix "/[wait]ms" can be added to reduce the transmit rate so that there
 * is always at lest the specified number of milliseconds between each bytes sent.
 *
 * @param newval : a string corresponding to the type of protocol used over the serial line
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_protocol(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("protocol", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the SPI port communication parameters, as a string such as
 * "125000,0,msb". The string includes the baud rate, the SPI mode (between
 * 0 and 3) and the bit order.
 *
 * @return a string corresponding to the SPI port communication parameters, as a string such as
 *         "125000,0,msb"
 *
 * On failure, throws an exception or returns Y_SPIMODE_INVALID.
 */
string YSpiPort::get_spiMode(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::SPIMODE_INVALID;
                }
            }
        }
        res = _spiMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the SPI port communication parameters, with a string such as
 * "125000,0,msb". The string includes the baud rate, the SPI mode (between
 * 0 and 3) and the bit order.
 *
 * @param newval : a string corresponding to the SPI port communication parameters, with a string such as
 *         "125000,0,msb"
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_spiMode(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("spiMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the SS line polarity.
 *
 * @return either Y_SSPOLARITY_ACTIVE_LOW or Y_SSPOLARITY_ACTIVE_HIGH, according to the SS line polarity
 *
 * On failure, throws an exception or returns Y_SSPOLARITY_INVALID.
 */
Y_SSPOLARITY_enum YSpiPort::get_ssPolarity(void)
{
    Y_SSPOLARITY_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::SSPOLARITY_INVALID;
                }
            }
        }
        res = _ssPolarity;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the SS line polarity.
 *
 * @param newval : either Y_SSPOLARITY_ACTIVE_LOW or Y_SSPOLARITY_ACTIVE_HIGH, according to the SS line polarity
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_ssPolarity(Y_SSPOLARITY_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = (newval>0 ? "1" : "0");
        res = _setAttr("ssPolarity", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns true when the SDI line phase is shifted with regards to the SDO line.
 *
 * @return either Y_SHITFTSAMPLING_OFF or Y_SHITFTSAMPLING_ON, according to true when the SDI line
 * phase is shifted with regards to the SDO line
 *
 * On failure, throws an exception or returns Y_SHITFTSAMPLING_INVALID.
 */
Y_SHITFTSAMPLING_enum YSpiPort::get_shitftSampling(void)
{
    Y_SHITFTSAMPLING_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSpiPort::SHITFTSAMPLING_INVALID;
                }
            }
        }
        res = _shitftSampling;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the SDI line sampling shift. When disabled, SDI line is
 * sampled in the middle of data output time. When enabled, SDI line is
 * samples at the end of data output time.
 *
 * @param newval : either Y_SHITFTSAMPLING_OFF or Y_SHITFTSAMPLING_ON, according to the SDI line sampling shift
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_shitftSampling(Y_SHITFTSAMPLING_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = (newval>0 ? "1" : "0");
        res = _setAttr("shitftSampling", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a SPI port for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the SPI port is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YSpiPort.isOnline() to test if the SPI port is
 * indeed online at a given time. In case of ambiguity when looking for
 * a SPI port by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the SPI port
 *
 * @return a YSpiPort object allowing you to drive the SPI port.
 */
YSpiPort* YSpiPort::FindSpiPort(string func)
{
    YSpiPort* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YSpiPort*) YFunction::_FindFromCache("SpiPort", func);
        if (obj == NULL) {
            obj = new YSpiPort(func);
            YFunction::_AddToCache("SpiPort", func, obj);
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
int YSpiPort::registerValueCallback(YSpiPortValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackSpiPort = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YSpiPort::_invokeValueCallback(string value)
{
    if (_valueCallbackSpiPort != NULL) {
        _valueCallbackSpiPort(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YSpiPort::sendCommand(string text)
{
    return this->set_command(text);
}

/**
 * Clears the serial port buffer and resets counters to zero.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::reset(void)
{
    _rxptr = 0;
    _rxbuffptr = 0;
    _rxbuff = string(0, (char)0);

    return this->sendCommand("Z");
}

/**
 * Sends a single byte to the serial port.
 *
 * @param code : the byte to send
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::writeByte(int code)
{
    return this->sendCommand(YapiWrapper::ysprintf("$%02x",code));
}

/**
 * Sends an ASCII string to the serial port, as is.
 *
 * @param text : the text string to send
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::writeStr(string text)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int ch = 0;
    buff = text;
    bufflen = (int)(buff).size();
    if (bufflen < 100) {
        // if string is pure text, we can send it as a simple command (faster)
        ch = 0x20;
        idx = 0;
        while ((idx < bufflen) && (ch != 0)) {
            ch = ((u8)buff[idx]);
            if ((ch >= 0x20) && (ch < 0x7f)) {
                idx = idx + 1;
            } else {
                ch = 0;
            }
        }
        if (idx >= bufflen) {
            return this->sendCommand(YapiWrapper::ysprintf("+%s",text.c_str()));
        }
    }
    // send string using file upload
    return this->_upload("txdata", buff);
}

/**
 * Sends a binary buffer to the serial port, as is.
 *
 * @param buff : the binary buffer to send
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::writeBin(string buff)
{
    return this->_upload("txdata", buff);
}

/**
 * Sends a byte sequence (provided as a list of bytes) to the serial port.
 *
 * @param byteList : a list of byte codes
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::writeArray(vector<int> byteList)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int hexb = 0;
    int res = 0;
    bufflen = (int)byteList.size();
    buff = string(bufflen, (char)0);
    idx = 0;
    while (idx < bufflen) {
        hexb = byteList[idx];
        buff[idx] = (char)(hexb);
        idx = idx + 1;
    }

    res = this->_upload("txdata", buff);
    return res;
}

/**
 * Sends a byte sequence (provided as a hexadecimal string) to the serial port.
 *
 * @param hexString : a string of hexadecimal byte codes
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::writeHex(string hexString)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int hexb = 0;
    int res = 0;
    bufflen = (int)(hexString).length();
    if (bufflen < 100) {
        return this->sendCommand(YapiWrapper::ysprintf("$%s",hexString.c_str()));
    }
    bufflen = ((bufflen) >> (1));
    buff = string(bufflen, (char)0);
    idx = 0;
    while (idx < bufflen) {
        hexb = (int)strtoul((hexString).substr( 2 * idx, 2).c_str(), NULL, 16);
        buff[idx] = (char)(hexb);
        idx = idx + 1;
    }

    res = this->_upload("txdata", buff);
    return res;
}

/**
 * Sends an ASCII string to the serial port, followed by a line break (CR LF).
 *
 * @param text : the text string to send
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::writeLine(string text)
{
    string buff;
    int bufflen = 0;
    int idx = 0;
    int ch = 0;
    buff = YapiWrapper::ysprintf("%s\r\n",text.c_str());
    bufflen = (int)(buff).size()-2;
    if (bufflen < 100) {
        // if string is pure text, we can send it as a simple command (faster)
        ch = 0x20;
        idx = 0;
        while ((idx < bufflen) && (ch != 0)) {
            ch = ((u8)buff[idx]);
            if ((ch >= 0x20) && (ch < 0x7f)) {
                idx = idx + 1;
            } else {
                ch = 0;
            }
        }
        if (idx >= bufflen) {
            return this->sendCommand(YapiWrapper::ysprintf("!%s",text.c_str()));
        }
    }
    // send string using file upload
    return this->_upload("txdata", buff);
}

/**
 * Reads one byte from the receive buffer, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer,
 * or if there is no data available yet, the function returns YAPI_NO_MORE_DATA.
 *
 * @return the next byte
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::readByte(void)
{
    int currpos = 0;
    int reqlen = 0;
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    int res = 0;
    // first check if we have the requested character in the look-ahead buffer
    bufflen = (int)(_rxbuff).size();
    if ((_rxptr >= _rxbuffptr) && (_rxptr < _rxbuffptr+bufflen)) {
        res = ((u8)_rxbuff[_rxptr-_rxbuffptr]);
        _rxptr = _rxptr + 1;
        return res;
    }
    // try to preload more than one byte to speed-up byte-per-byte access
    currpos = _rxptr;
    reqlen = 1024;
    buff = this->readBin(reqlen);
    bufflen = (int)(buff).size();
    if (_rxptr == currpos+bufflen) {
        res = ((u8)buff[0]);
        _rxptr = currpos+1;
        _rxbuffptr = currpos;
        _rxbuff = buff;
        return res;
    }
    // mixed bidirectional data, retry with a smaller block
    _rxptr = currpos;
    reqlen = 16;
    buff = this->readBin(reqlen);
    bufflen = (int)(buff).size();
    if (_rxptr == currpos+bufflen) {
        res = ((u8)buff[0]);
        _rxptr = currpos+1;
        _rxbuffptr = currpos;
        _rxbuff = buff;
        return res;
    }
    // still mixed, need to process character by character
    _rxptr = currpos;

    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=1",_rxptr));
    bufflen = (int)(buff).size() - 1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (((u8)buff[bufflen]) != 64)) {
        endpos = endpos + mult * (((u8)buff[bufflen]) - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    _rxptr = endpos;
    if (bufflen == 0) {
        return YAPI_NO_MORE_DATA;
    }
    res = ((u8)buff[0]);
    return res;
}

/**
 * Reads data from the receive buffer as a string, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer, the
 * function performs a short read.
 *
 * @param nChars : the maximum number of characters to read
 *
 * @return a string with receive buffer contents
 *
 * On failure, throws an exception or returns a negative error code.
 */
string YSpiPort::readStr(int nChars)
{
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    string res;
    if (nChars > 65535) {
        nChars = 65535;
    }

    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=%d", _rxptr,nChars));
    bufflen = (int)(buff).size() - 1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (((u8)buff[bufflen]) != 64)) {
        endpos = endpos + mult * (((u8)buff[bufflen]) - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    _rxptr = endpos;
    res = (buff).substr( 0, bufflen);
    return res;
}

/**
 * Reads data from the receive buffer as a binary buffer, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer, the
 * function performs a short read.
 *
 * @param nChars : the maximum number of bytes to read
 *
 * @return a binary object with receive buffer contents
 *
 * On failure, throws an exception or returns a negative error code.
 */
string YSpiPort::readBin(int nChars)
{
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    int idx = 0;
    string res;
    if (nChars > 65535) {
        nChars = 65535;
    }

    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=%d", _rxptr,nChars));
    bufflen = (int)(buff).size() - 1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (((u8)buff[bufflen]) != 64)) {
        endpos = endpos + mult * (((u8)buff[bufflen]) - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    _rxptr = endpos;
    res = string(bufflen, (char)0);
    idx = 0;
    while (idx < bufflen) {
        res[idx] = (char)(((u8)buff[idx]));
        idx = idx + 1;
    }
    return res;
}

/**
 * Reads data from the receive buffer as a list of bytes, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer, the
 * function performs a short read.
 *
 * @param nChars : the maximum number of bytes to read
 *
 * @return a sequence of bytes with receive buffer contents
 *
 * On failure, throws an exception or returns a negative error code.
 */
vector<int> YSpiPort::readArray(int nChars)
{
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    int idx = 0;
    int b = 0;
    vector<int> res;
    if (nChars > 65535) {
        nChars = 65535;
    }

    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=%d", _rxptr,nChars));
    bufflen = (int)(buff).size() - 1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (((u8)buff[bufflen]) != 64)) {
        endpos = endpos + mult * (((u8)buff[bufflen]) - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    _rxptr = endpos;
    res.clear();
    idx = 0;
    while (idx < bufflen) {
        b = ((u8)buff[idx]);
        res.push_back(b);
        idx = idx + 1;
    }
    return res;
}

/**
 * Reads data from the receive buffer as a hexadecimal string, starting at current stream position.
 * If data at current stream position is not available anymore in the receive buffer, the
 * function performs a short read.
 *
 * @param nBytes : the maximum number of bytes to read
 *
 * @return a string with receive buffer contents, encoded in hexadecimal
 *
 * On failure, throws an exception or returns a negative error code.
 */
string YSpiPort::readHex(int nBytes)
{
    string buff;
    int bufflen = 0;
    int mult = 0;
    int endpos = 0;
    int ofs = 0;
    string res;
    if (nBytes > 65535) {
        nBytes = 65535;
    }

    buff = this->_download(YapiWrapper::ysprintf("rxdata.bin?pos=%d&len=%d", _rxptr,nBytes));
    bufflen = (int)(buff).size() - 1;
    endpos = 0;
    mult = 1;
    while ((bufflen > 0) && (((u8)buff[bufflen]) != 64)) {
        endpos = endpos + mult * (((u8)buff[bufflen]) - 48);
        mult = mult * 10;
        bufflen = bufflen - 1;
    }
    _rxptr = endpos;
    res = "";
    ofs = 0;
    while (ofs + 3 < bufflen) {
        res = YapiWrapper::ysprintf("%s%02x%02x%02x%02x", res.c_str(), ((u8)buff[ofs]), ((u8)buff[ofs + 1]), ((u8)buff[ofs + 2]),((u8)buff[ofs + 3]));
        ofs = ofs + 4;
    }
    while (ofs < bufflen) {
        res = YapiWrapper::ysprintf("%s%02x", res.c_str(),((u8)buff[ofs]));
        ofs = ofs + 1;
    }
    return res;
}

/**
 * Reads a single line (or message) from the receive buffer, starting at current stream position.
 * This function is intended to be used when the serial port is configured for a message protocol,
 * such as 'Line' mode or frame protocols.
 *
 * If data at current stream position is not available anymore in the receive buffer,
 * the function returns the oldest available line and moves the stream position just after.
 * If no new full line is received, the function returns an empty line.
 *
 * @return a string with a single line of text
 *
 * On failure, throws an exception or returns a negative error code.
 */
string YSpiPort::readLine(void)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    string res;

    url = YapiWrapper::ysprintf("rxmsg.json?pos=%d&len=1&maxw=1",_rxptr);
    msgbin = this->_download(url);
    msgarr = this->_json_get_array(msgbin);
    msglen = (int)msgarr.size();
    if (msglen == 0) {
        return "";
    }
    // last element of array is the new position
    msglen = msglen - 1;
    _rxptr = atoi((msgarr[msglen]).c_str());
    if (msglen == 0) {
        return "";
    }
    res = this->_json_get_string(msgarr[0]);
    return res;
}

/**
 * Searches for incoming messages in the serial port receive buffer matching a given pattern,
 * starting at current position. This function will only compare and return printable characters
 * in the message strings. Binary protocols are handled as hexadecimal strings.
 *
 * The search returns all messages matching the expression provided as argument in the buffer.
 * If no matching message is found, the search waits for one up to the specified maximum timeout
 * (in milliseconds).
 *
 * @param pattern : a limited regular expression describing the expected message format,
 *         or an empty string if all messages should be returned (no filtering).
 *         When using binary protocols, the format applies to the hexadecimal
 *         representation of the message.
 * @param maxWait : the maximum number of milliseconds to wait for a message if none is found
 *         in the receive buffer.
 *
 * @return an array of strings containing the messages found, if any.
 *         Binary messages are converted to hexadecimal representation.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<string> YSpiPort::readMessages(string pattern,int maxWait)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    vector<string> res;
    int idx = 0;

    url = YapiWrapper::ysprintf("rxmsg.json?pos=%d&maxw=%d&pat=%s", _rxptr, maxWait,pattern.c_str());
    msgbin = this->_download(url);
    msgarr = this->_json_get_array(msgbin);
    msglen = (int)msgarr.size();
    if (msglen == 0) {
        return res;
    }
    // last element of array is the new position
    msglen = msglen - 1;
    _rxptr = atoi((msgarr[msglen]).c_str());
    idx = 0;
    while (idx < msglen) {
        res.push_back(this->_json_get_string(msgarr[idx]));
        idx = idx + 1;
    }
    return res;
}

/**
 * Changes the current internal stream position to the specified value. This function
 * does not affect the device, it only changes the value stored in the API object
 * for the next read operations.
 *
 * @param absPos : the absolute position index for next read operations.
 *
 * @return nothing.
 */
int YSpiPort::read_seek(int absPos)
{
    _rxptr = absPos;
    return YAPI_SUCCESS;
}

/**
 * Returns the current absolute stream position pointer of the API object.
 *
 * @return the absolute position index for next read operations.
 */
int YSpiPort::read_tell(void)
{
    return _rxptr;
}

/**
 * Returns the number of bytes available to read in the input buffer starting from the
 * current absolute stream position pointer of the API object.
 *
 * @return the number of bytes available to read
 */
int YSpiPort::read_avail(void)
{
    string buff;
    int bufflen = 0;
    int res = 0;

    buff = this->_download(YapiWrapper::ysprintf("rxcnt.bin?pos=%d",_rxptr));
    bufflen = (int)(buff).size() - 1;
    while ((bufflen > 0) && (((u8)buff[bufflen]) != 64)) {
        bufflen = bufflen - 1;
    }
    res = atoi(((buff).substr( 0, bufflen)).c_str());
    return res;
}

/**
 * Sends a text line query to the serial port, and reads the reply, if any.
 * This function is intended to be used when the serial port is configured for 'Line' protocol.
 *
 * @param query : the line query to send (without CR/LF)
 * @param maxWait : the maximum number of milliseconds to wait for a reply.
 *
 * @return the next text line received after sending the text query, as a string.
 *         Additional lines can be obtained by calling readLine or readMessages.
 *
 * On failure, throws an exception or returns an empty array.
 */
string YSpiPort::queryLine(string query,int maxWait)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    string res;

    url = YapiWrapper::ysprintf("rxmsg.json?len=1&maxw=%d&cmd=!%s", maxWait,query.c_str());
    msgbin = this->_download(url);
    msgarr = this->_json_get_array(msgbin);
    msglen = (int)msgarr.size();
    if (msglen == 0) {
        return "";
    }
    // last element of array is the new position
    msglen = msglen - 1;
    _rxptr = atoi((msgarr[msglen]).c_str());
    if (msglen == 0) {
        return "";
    }
    res = this->_json_get_string(msgarr[0]);
    return res;
}

/**
 * Saves the job definition string (JSON data) into a job file.
 * The job file can be later enabled using selectJob().
 *
 * @param jobfile : name of the job file to save on the device filesystem
 * @param jsonDef : a string containing a JSON definition of the job
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::uploadJob(string jobfile,string jsonDef)
{
    this->_upload(jobfile, jsonDef);
    return YAPI_SUCCESS;
}

/**
 * Load and start processing the specified job file. The file must have
 * been previously created using the user interface or uploaded on the
 * device filesystem using the uploadJob() function.
 *
 * @param jobfile : name of the job file (on the device filesystem)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::selectJob(string jobfile)
{
    return this->set_currentJob(jobfile);
}

/**
 * Manually sets the state of the SS line. This function has no effect when
 * the SS line is handled automatically.
 *
 * @param val : 1 to turn SS active, 0 to release SS.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSpiPort::set_SS(int val)
{
    return this->sendCommand(YapiWrapper::ysprintf("S%d",val));
}

YSpiPort *YSpiPort::nextSpiPort(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YSpiPort::FindSpiPort(hwid);
}

YSpiPort* YSpiPort::FirstSpiPort(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("SpiPort", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YSpiPort::FindSpiPort(serial+"."+funcId);
}

//--- (end of YSpiPort implementation)

//--- (YSpiPort functions)
//--- (end of YSpiPort functions)
