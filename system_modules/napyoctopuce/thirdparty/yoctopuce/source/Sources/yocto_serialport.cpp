/*********************************************************************
 *
 * $Id: yocto_serialport.cpp 28753 2017-10-03 11:23:38Z seb $
 *
 * Implements yFindSerialPort(), the high-level API for SerialPort functions
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
#include "yocto_serialport.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "serialport"


YSnoopingRecord::YSnoopingRecord(const string& json):
//--- (generated code: YSnoopingRecord initialization)
    _tim(0)
    ,_dir(0)
//--- (end of generated code: YSnoopingRecord initialization)
{
    yJsonStateMachine j;
    // Parse JSON data
    j.src = json.c_str();
    j.end = j.src + strlen(j.src);
    j.st = YJSON_START;
    if(yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        return ;
    }
    while(yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
        if (!strcmp(j.token, "m")) {
            string tmp;
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return ;
            }
            tmp = (string)j.token;
            while(j.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j) == YJSON_PARSE_AVAIL) {
                tmp +=(string)j.token;
            }
            _dir = (tmp[0] == '<' ? 1 : 0);
            _msg = tmp.substr(1);
        } else if(!strcmp(j.token, "t")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return;
            }
            _tim = atoi(j.token);;
        } else {
            yJsonSkip(&j, 1);
        }
    }
}

//--- (generated code: YSnoopingRecord implementation)
// static attributes


int YSnoopingRecord::get_time(void)
{
    return _tim;
}

int YSnoopingRecord::get_direction(void)
{
    return _dir;
}

string YSnoopingRecord::get_message(void)
{
    return _msg;
}
//--- (end of generated code: YSnoopingRecord implementation)


YSerialPort::YSerialPort(const string& func): YFunction(func)
//--- (generated code: YSerialPort initialization)
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
    ,_serialMode(SERIALMODE_INVALID)
    ,_valueCallbackSerialPort(NULL)
    ,_rxptr(0)
    ,_rxbuffptr(0)
//--- (end of generated code: YSerialPort initialization)
{
    _className="SerialPort";
}

YSerialPort::~YSerialPort()
{
//--- (generated code: YSerialPort cleanup)
//--- (end of generated code: YSerialPort cleanup)
}
//--- (generated code: YSerialPort implementation)
// static attributes
const string YSerialPort::LASTMSG_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::CURRENTJOB_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::STARTUPJOB_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::COMMAND_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::PROTOCOL_INVALID = YAPI_INVALID_STRING;
const string YSerialPort::SERIALMODE_INVALID = YAPI_INVALID_STRING;

int YSerialPort::_parseAttr(YJSONObject* json_val)
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
    if(json_val->has("serialMode")) {
        _serialMode =  json_val->getString("serialMode");
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
int YSerialPort::get_rxCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::RXCOUNT_INVALID;
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
int YSerialPort::get_txCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::TXCOUNT_INVALID;
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
int YSerialPort::get_errCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::ERRCOUNT_INVALID;
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
int YSerialPort::get_rxMsgCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::RXMSGCOUNT_INVALID;
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
int YSerialPort::get_txMsgCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::TXMSGCOUNT_INVALID;
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
 * Returns the latest message fully received (for Line, Frame and Modbus protocols).
 *
 * @return a string corresponding to the latest message fully received (for Line, Frame and Modbus protocols)
 *
 * On failure, throws an exception or returns Y_LASTMSG_INVALID.
 */
string YSerialPort::get_lastMsg(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::LASTMSG_INVALID;
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
string YSerialPort::get_currentJob(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::CURRENTJOB_INVALID;
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
int YSerialPort::set_currentJob(const string& newval)
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
string YSerialPort::get_startupJob(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::STARTUPJOB_INVALID;
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
int YSerialPort::set_startupJob(const string& newval)
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

string YSerialPort::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::COMMAND_INVALID;
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

int YSerialPort::set_command(const string& newval)
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
Y_VOLTAGELEVEL_enum YSerialPort::get_voltageLevel(void)
{
    Y_VOLTAGELEVEL_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::VOLTAGELEVEL_INVALID;
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
int YSerialPort::set_voltageLevel(Y_VOLTAGELEVEL_enum newval)
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
 * "Modbus-ASCII" for MODBUS messages in ASCII mode,
 * "Modbus-RTU" for MODBUS messages in RTU mode,
 * "Wiegand-ASCII" for Wiegand messages in ASCII mode,
 * "Wiegand-26","Wiegand-34", etc for Wiegand messages in byte mode,
 * "Char" for a continuous ASCII stream or
 * "Byte" for a continuous binary stream.
 *
 * @return a string corresponding to the type of protocol used over the serial line, as a string
 *
 * On failure, throws an exception or returns Y_PROTOCOL_INVALID.
 */
string YSerialPort::get_protocol(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::PROTOCOL_INVALID;
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
 * "Modbus-ASCII" for MODBUS messages in ASCII mode,
 * "Modbus-RTU" for MODBUS messages in RTU mode,
 * "Wiegand-ASCII" for Wiegand messages in ASCII mode,
 * "Wiegand-26","Wiegand-34", etc for Wiegand messages in byte mode,
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
int YSerialPort::set_protocol(const string& newval)
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
 * Returns the serial port communication parameters, as a string such as
 * "9600,8N1". The string includes the baud rate, the number of data bits,
 * the parity, and the number of stop bits. An optional suffix is included
 * if flow control is active: "CtsRts" for hardware handshake, "XOnXOff"
 * for logical flow control and "Simplex" for acquiring a shared bus using
 * the RTS line (as used by some RS485 adapters for instance).
 *
 * @return a string corresponding to the serial port communication parameters, as a string such as
 *         "9600,8N1"
 *
 * On failure, throws an exception or returns Y_SERIALMODE_INVALID.
 */
string YSerialPort::get_serialMode(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YSerialPort::SERIALMODE_INVALID;
                }
            }
        }
        res = _serialMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the serial port communication parameters, with a string such as
 * "9600,8N1". The string includes the baud rate, the number of data bits,
 * the parity, and the number of stop bits. An optional suffix can be added
 * to enable flow control: "CtsRts" for hardware handshake, "XOnXOff"
 * for logical flow control and "Simplex" for acquiring a shared bus using
 * the RTS line (as used by some RS485 adapters for instance).
 *
 * @param newval : a string corresponding to the serial port communication parameters, with a string such as
 *         "9600,8N1"
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::set_serialMode(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("serialMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a serial port for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the serial port is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YSerialPort.isOnline() to test if the serial port is
 * indeed online at a given time. In case of ambiguity when looking for
 * a serial port by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the serial port
 *
 * @return a YSerialPort object allowing you to drive the serial port.
 */
YSerialPort* YSerialPort::FindSerialPort(string func)
{
    YSerialPort* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YSerialPort*) YFunction::_FindFromCache("SerialPort", func);
        if (obj == NULL) {
            obj = new YSerialPort(func);
            YFunction::_AddToCache("SerialPort", func, obj);
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
int YSerialPort::registerValueCallback(YSerialPortValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackSerialPort = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YSerialPort::_invokeValueCallback(string value)
{
    if (_valueCallbackSerialPort != NULL) {
        _valueCallbackSerialPort(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YSerialPort::sendCommand(string text)
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
int YSerialPort::reset(void)
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
int YSerialPort::writeByte(int code)
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
int YSerialPort::writeStr(string text)
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
int YSerialPort::writeBin(string buff)
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
int YSerialPort::writeArray(vector<int> byteList)
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
int YSerialPort::writeHex(string hexString)
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
int YSerialPort::writeLine(string text)
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
int YSerialPort::readByte(void)
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
string YSerialPort::readStr(int nChars)
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
string YSerialPort::readBin(int nChars)
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
vector<int> YSerialPort::readArray(int nChars)
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
string YSerialPort::readHex(int nBytes)
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
string YSerialPort::readLine(void)
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
vector<string> YSerialPort::readMessages(string pattern,int maxWait)
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
int YSerialPort::read_seek(int absPos)
{
    _rxptr = absPos;
    return YAPI_SUCCESS;
}

/**
 * Returns the current absolute stream position pointer of the API object.
 *
 * @return the absolute position index for next read operations.
 */
int YSerialPort::read_tell(void)
{
    return _rxptr;
}

/**
 * Returns the number of bytes available to read in the input buffer starting from the
 * current absolute stream position pointer of the API object.
 *
 * @return the number of bytes available to read
 */
int YSerialPort::read_avail(void)
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
string YSerialPort::queryLine(string query,int maxWait)
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
int YSerialPort::uploadJob(string jobfile,string jsonDef)
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
int YSerialPort::selectJob(string jobfile)
{
    return this->set_currentJob(jobfile);
}

/**
 * Manually sets the state of the RTS line. This function has no effect when
 * hardware handshake is enabled, as the RTS line is driven automatically.
 *
 * @param val : 1 to turn RTS on, 0 to turn RTS off
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::set_RTS(int val)
{
    return this->sendCommand(YapiWrapper::ysprintf("R%d",val));
}

/**
 * Reads the level of the CTS line. The CTS line is usually driven by
 * the RTS signal of the connected serial device.
 *
 * @return 1 if the CTS line is high, 0 if the CTS line is low.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::get_CTS(void)
{
    string buff;
    int res = 0;

    buff = this->_download("cts.txt");
    if (!((int)(buff).size() == 1)) {
        _throw(YAPI_IO_ERROR,"invalid CTS reply");
        return YAPI_IO_ERROR;
    }
    res = ((u8)buff[0]) - 48;
    return res;
}

/**
 * Retrieves messages (both direction) in the serial port buffer, starting at current position.
 * This function will only compare and return printable characters in the message strings.
 * Binary protocols are handled as hexadecimal strings.
 *
 * If no message is found, the search waits for one up to the specified maximum timeout
 * (in milliseconds).
 *
 * @param maxWait : the maximum number of milliseconds to wait for a message if none is found
 *         in the receive buffer.
 *
 * @return an array of YSnoopingRecord objects containing the messages found, if any.
 *         Binary messages are converted to hexadecimal representation.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<YSnoopingRecord> YSerialPort::snoopMessages(int maxWait)
{
    string url;
    string msgbin;
    vector<string> msgarr;
    int msglen = 0;
    vector<YSnoopingRecord> res;
    int idx = 0;

    url = YapiWrapper::ysprintf("rxmsg.json?pos=%d&maxw=%d&t=0", _rxptr,maxWait);
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
        res.push_back(YSnoopingRecord(msgarr[idx]));
        idx = idx + 1;
    }
    return res;
}

/**
 * Sends a MODBUS message (provided as a hexadecimal string) to the serial port.
 * The message must start with the slave address. The MODBUS CRC/LRC is
 * automatically added by the function. This function does not wait for a reply.
 *
 * @param hexString : a hexadecimal message string, including device address but no CRC/LRC
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YSerialPort::writeMODBUS(string hexString)
{
    return this->sendCommand(YapiWrapper::ysprintf(":%s",hexString.c_str()));
}

/**
 * Sends a message to a specified MODBUS slave connected to the serial port, and reads the
 * reply, if any. The message is the PDU, provided as a vector of bytes.
 *
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduBytes : the message to send (PDU), as a vector of bytes. The first byte of the
 *         PDU is the MODBUS function code.
 *
 * @return the received reply, as a vector of bytes.
 *
 * On failure, throws an exception or returns an empty array (or a MODBUS error reply).
 */
vector<int> YSerialPort::queryMODBUS(int slaveNo,vector<int> pduBytes)
{
    int funCode = 0;
    int nib = 0;
    int i = 0;
    string cmd;
    string url;
    string pat;
    string msgs;
    vector<string> reps;
    string rep;
    vector<int> res;
    int replen = 0;
    int hexb = 0;
    funCode = pduBytes[0];
    nib = ((funCode) >> (4));
    pat = YapiWrapper::ysprintf("%02x[%x%x]%x.*", slaveNo, nib, (nib+8),((funCode) & (15)));
    cmd = YapiWrapper::ysprintf("%02x%02x", slaveNo,funCode);
    i = 1;
    while (i < (int)pduBytes.size()) {
        cmd = YapiWrapper::ysprintf("%s%02x", cmd.c_str(),((pduBytes[i]) & (0xff)));
        i = i + 1;
    }

    url = YapiWrapper::ysprintf("rxmsg.json?cmd=:%s&pat=:%s", cmd.c_str(),pat.c_str());
    msgs = this->_download(url);
    reps = this->_json_get_array(msgs);
    if (!((int)reps.size() > 1)) {
        _throw(YAPI_IO_ERROR,"no reply from slave");
        return res;
    }
    if ((int)reps.size() > 1) {
        rep = this->_json_get_string(reps[0]);
        replen = (((int)(rep).length() - 3) >> (1));
        i = 0;
        while (i < replen) {
            hexb = (int)strtoul((rep).substr(2 * i + 3, 2).c_str(), NULL, 16);
            res.push_back(hexb);
            i = i + 1;
        }
        if (res[0] != funCode) {
            i = res[1];
            if (!(i > 1)) {
                _throw(YAPI_NOT_SUPPORTED,"MODBUS error: unsupported function code");
                return res;
            }
            if (!(i > 2)) {
                _throw(YAPI_INVALID_ARGUMENT,"MODBUS error: illegal data address");
                return res;
            }
            if (!(i > 3)) {
                _throw(YAPI_INVALID_ARGUMENT,"MODBUS error: illegal data value");
                return res;
            }
            if (!(i > 4)) {
                _throw(YAPI_INVALID_ARGUMENT,"MODBUS error: failed to execute function");
                return res;
            }
        }
    }
    return res;
}

/**
 * Reads one or more contiguous internal bits (or coil status) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x01 (Read Coils).
 *
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first bit/coil to read (zero-based)
 * @param nBits : the number of bits/coils to read
 *
 * @return a vector of integers, each corresponding to one bit.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadBits(int slaveNo,int pduAddr,int nBits)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int bitpos = 0;
    int idx = 0;
    int val = 0;
    int mask = 0;
    pdu.push_back(0x01);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nBits) >> (8)));
    pdu.push_back(((nBits) & (0xff)));

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    bitpos = 0;
    idx = 2;
    val = reply[idx];
    mask = 1;
    while (bitpos < nBits) {
        if (((val) & (mask)) == 0) {
            res.push_back(0);
        } else {
            res.push_back(1);
        }
        bitpos = bitpos + 1;
        if (mask == 0x80) {
            idx = idx + 1;
            val = reply[idx];
            mask = 1;
        } else {
            mask = ((mask) << (1));
        }
    }
    return res;
}

/**
 * Reads one or more contiguous input bits (or discrete inputs) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x02 (Read Discrete Inputs).
 *
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first bit/input to read (zero-based)
 * @param nBits : the number of bits/inputs to read
 *
 * @return a vector of integers, each corresponding to one bit.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadInputBits(int slaveNo,int pduAddr,int nBits)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int bitpos = 0;
    int idx = 0;
    int val = 0;
    int mask = 0;
    pdu.push_back(0x02);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nBits) >> (8)));
    pdu.push_back(((nBits) & (0xff)));

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    bitpos = 0;
    idx = 2;
    val = reply[idx];
    mask = 1;
    while (bitpos < nBits) {
        if (((val) & (mask)) == 0) {
            res.push_back(0);
        } else {
            res.push_back(1);
        }
        bitpos = bitpos + 1;
        if (mask == 0x80) {
            idx = idx + 1;
            val = reply[idx];
            mask = 1;
        } else {
            mask = ((mask) << (1));
        }
    }
    return res;
}

/**
 * Reads one or more contiguous internal registers (holding registers) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x03 (Read Holding Registers).
 *
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first holding register to read (zero-based)
 * @param nWords : the number of holding registers to read
 *
 * @return a vector of integers, each corresponding to one 16-bit register value.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadRegisters(int slaveNo,int pduAddr,int nWords)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int regpos = 0;
    int idx = 0;
    int val = 0;
    pdu.push_back(0x03);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nWords) >> (8)));
    pdu.push_back(((nWords) & (0xff)));

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    regpos = 0;
    idx = 2;
    while (regpos < nWords) {
        val = ((reply[idx]) << (8));
        idx = idx + 1;
        val = val + reply[idx];
        idx = idx + 1;
        res.push_back(val);
        regpos = regpos + 1;
    }
    return res;
}

/**
 * Reads one or more contiguous input registers (read-only registers) from a MODBUS serial device.
 * This method uses the MODBUS function code 0x04 (Read Input Registers).
 *
 * @param slaveNo : the address of the slave MODBUS device to query
 * @param pduAddr : the relative address of the first input register to read (zero-based)
 * @param nWords : the number of input registers to read
 *
 * @return a vector of integers, each corresponding to one 16-bit input value.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusReadInputRegisters(int slaveNo,int pduAddr,int nWords)
{
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    int regpos = 0;
    int idx = 0;
    int val = 0;
    pdu.push_back(0x04);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nWords) >> (8)));
    pdu.push_back(((nWords) & (0xff)));

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    regpos = 0;
    idx = 2;
    while (regpos < nWords) {
        val = ((reply[idx]) << (8));
        idx = idx + 1;
        val = val + reply[idx];
        idx = idx + 1;
        res.push_back(val);
        regpos = regpos + 1;
    }
    return res;
}

/**
 * Sets a single internal bit (or coil) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x05 (Write Single Coil).
 *
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the bit/coil to set (zero-based)
 * @param value : the value to set (0 for OFF state, non-zero for ON state)
 *
 * @return the number of bits/coils affected on the device (1)
 *
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteBit(int slaveNo,int pduAddr,int value)
{
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    if (value != 0) {
        value = 0xff;
    }
    pdu.push_back(0x05);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(value);
    pdu.push_back(0x00);

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = 1;
    return res;
}

/**
 * Sets several contiguous internal bits (or coils) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x0f (Write Multiple Coils).
 *
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the first bit/coil to set (zero-based)
 * @param bits : the vector of bits to be set (one integer per bit)
 *
 * @return the number of bits/coils affected on the device
 *
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteBits(int slaveNo,int pduAddr,vector<int> bits)
{
    int nBits = 0;
    int nBytes = 0;
    int bitpos = 0;
    int val = 0;
    int mask = 0;
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    nBits = (int)bits.size();
    nBytes = (((nBits + 7)) >> (3));
    pdu.push_back(0x0f);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nBits) >> (8)));
    pdu.push_back(((nBits) & (0xff)));
    pdu.push_back(nBytes);
    bitpos = 0;
    val = 0;
    mask = 1;
    while (bitpos < nBits) {
        if (bits[bitpos] != 0) {
            val = ((val) | (mask));
        }
        bitpos = bitpos + 1;
        if (mask == 0x80) {
            pdu.push_back(val);
            val = 0;
            mask = 1;
        } else {
            mask = ((mask) << (1));
        }
    }
    if (mask != 1) {
        pdu.push_back(val);
    }

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = ((reply[3]) << (8));
    res = res + reply[4];
    return res;
}

/**
 * Sets a single internal register (or holding register) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x06 (Write Single Register).
 *
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the register to set (zero-based)
 * @param value : the 16 bit value to set
 *
 * @return the number of registers affected on the device (1)
 *
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteRegister(int slaveNo,int pduAddr,int value)
{
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    pdu.push_back(0x06);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((value) >> (8)));
    pdu.push_back(((value) & (0xff)));

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = 1;
    return res;
}

/**
 * Sets several contiguous internal registers (or holding registers) on a MODBUS serial device.
 * This method uses the MODBUS function code 0x10 (Write Multiple Registers).
 *
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduAddr : the relative address of the first internal register to set (zero-based)
 * @param values : the vector of 16 bit values to set
 *
 * @return the number of registers affected on the device
 *
 * On failure, throws an exception or returns zero.
 */
int YSerialPort::modbusWriteRegisters(int slaveNo,int pduAddr,vector<int> values)
{
    int nWords = 0;
    int nBytes = 0;
    int regpos = 0;
    int val = 0;
    vector<int> pdu;
    vector<int> reply;
    int res = 0;
    res = 0;
    nWords = (int)values.size();
    nBytes = 2 * nWords;
    pdu.push_back(0x10);
    pdu.push_back(((pduAddr) >> (8)));
    pdu.push_back(((pduAddr) & (0xff)));
    pdu.push_back(((nWords) >> (8)));
    pdu.push_back(((nWords) & (0xff)));
    pdu.push_back(nBytes);
    regpos = 0;
    while (regpos < nWords) {
        val = values[regpos];
        pdu.push_back(((val) >> (8)));
        pdu.push_back(((val) & (0xff)));
        regpos = regpos + 1;
    }

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    res = ((reply[3]) << (8));
    res = res + reply[4];
    return res;
}

/**
 * Sets several contiguous internal registers (holding registers) on a MODBUS serial device,
 * then performs a contiguous read of a set of (possibly different) internal registers.
 * This method uses the MODBUS function code 0x17 (Read/Write Multiple Registers).
 *
 * @param slaveNo : the address of the slave MODBUS device to drive
 * @param pduWriteAddr : the relative address of the first internal register to set (zero-based)
 * @param values : the vector of 16 bit values to set
 * @param pduReadAddr : the relative address of the first internal register to read (zero-based)
 * @param nReadWords : the number of 16 bit values to read
 *
 * @return a vector of integers, each corresponding to one 16-bit register value read.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YSerialPort::modbusWriteAndReadRegisters(int slaveNo,int pduWriteAddr,vector<int> values,int pduReadAddr,int nReadWords)
{
    int nWriteWords = 0;
    int nBytes = 0;
    int regpos = 0;
    int val = 0;
    int idx = 0;
    vector<int> pdu;
    vector<int> reply;
    vector<int> res;
    nWriteWords = (int)values.size();
    nBytes = 2 * nWriteWords;
    pdu.push_back(0x17);
    pdu.push_back(((pduReadAddr) >> (8)));
    pdu.push_back(((pduReadAddr) & (0xff)));
    pdu.push_back(((nReadWords) >> (8)));
    pdu.push_back(((nReadWords) & (0xff)));
    pdu.push_back(((pduWriteAddr) >> (8)));
    pdu.push_back(((pduWriteAddr) & (0xff)));
    pdu.push_back(((nWriteWords) >> (8)));
    pdu.push_back(((nWriteWords) & (0xff)));
    pdu.push_back(nBytes);
    regpos = 0;
    while (regpos < nWriteWords) {
        val = values[regpos];
        pdu.push_back(((val) >> (8)));
        pdu.push_back(((val) & (0xff)));
        regpos = regpos + 1;
    }

    reply = this->queryMODBUS(slaveNo, pdu);
    if ((int)reply.size() == 0) {
        return res;
    }
    if (reply[0] != pdu[0]) {
        return res;
    }
    regpos = 0;
    idx = 2;
    while (regpos < nReadWords) {
        val = ((reply[idx]) << (8));
        idx = idx + 1;
        val = val + reply[idx];
        idx = idx + 1;
        res.push_back(val);
        regpos = regpos + 1;
    }
    return res;
}

YSerialPort *YSerialPort::nextSerialPort(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YSerialPort::FindSerialPort(hwid);
}

YSerialPort* YSerialPort::FirstSerialPort(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("SerialPort", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YSerialPort::FindSerialPort(serial+"."+funcId);
}

//--- (end of generated code: YSerialPort implementation)

//--- (generated code: YSerialPort functions)
//--- (end of generated code: YSerialPort functions)
