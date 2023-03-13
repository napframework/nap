/*********************************************************************
 *
 * $Id: yocto_cellular.cpp 28753 2017-10-03 11:23:38Z seb $
 *
 * Implements yFindCellular(), the high-level API for Cellular functions
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
#include "yocto_cellular.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "cellular"



YCellRecord::YCellRecord(int mcc,int mnc,int lac,int cellId,int dbm,int tad,const string &oper):
//--- (generated code: YCellRecord initialization)
    _mcc(0)
    ,_mnc(0)
    ,_lac(0)
    ,_cid(0)
    ,_dbm(0)
    ,_tad(0)
//--- (end of generated code: YCellRecord initialization)
{
    _oper = oper;
    _mcc = mcc;
    _mnc = mnc;
    _lac = lac;
    _cid = cellId;
    _dbm = dbm;
    _tad = tad;

}

//--- (generated code: YCellRecord implementation)
// static attributes


string YCellRecord::get_cellOperator(void)
{
    return _oper;
}

int YCellRecord::get_mobileCountryCode(void)
{
    return _mcc;
}

int YCellRecord::get_mobileNetworkCode(void)
{
    return _mnc;
}

int YCellRecord::get_locationAreaCode(void)
{
    return _lac;
}

int YCellRecord::get_cellId(void)
{
    return _cid;
}

int YCellRecord::get_signalStrength(void)
{
    return _dbm;
}

int YCellRecord::get_timingAdvance(void)
{
    return _tad;
}
//--- (end of generated code: YCellRecord implementation)


YCellular::YCellular(const string& func): YFunction(func)
//--- (generated code: YCellular initialization)
    ,_linkQuality(LINKQUALITY_INVALID)
    ,_cellOperator(CELLOPERATOR_INVALID)
    ,_cellIdentifier(CELLIDENTIFIER_INVALID)
    ,_cellType(CELLTYPE_INVALID)
    ,_imsi(IMSI_INVALID)
    ,_message(MESSAGE_INVALID)
    ,_pin(PIN_INVALID)
    ,_lockedOperator(LOCKEDOPERATOR_INVALID)
    ,_airplaneMode(AIRPLANEMODE_INVALID)
    ,_enableData(ENABLEDATA_INVALID)
    ,_apn(APN_INVALID)
    ,_apnSecret(APNSECRET_INVALID)
    ,_pingInterval(PINGINTERVAL_INVALID)
    ,_dataSent(DATASENT_INVALID)
    ,_dataReceived(DATARECEIVED_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackCellular(NULL)
//--- (end of generated code: YCellular initialization)
{
    _className="Cellular";
}

YCellular::~YCellular()
{
//--- (generated code: YCellular cleanup)
//--- (end of generated code: YCellular cleanup)
}
//--- (generated code: YCellular implementation)
// static attributes
const string YCellular::CELLOPERATOR_INVALID = YAPI_INVALID_STRING;
const string YCellular::CELLIDENTIFIER_INVALID = YAPI_INVALID_STRING;
const string YCellular::IMSI_INVALID = YAPI_INVALID_STRING;
const string YCellular::MESSAGE_INVALID = YAPI_INVALID_STRING;
const string YCellular::PIN_INVALID = YAPI_INVALID_STRING;
const string YCellular::LOCKEDOPERATOR_INVALID = YAPI_INVALID_STRING;
const string YCellular::APN_INVALID = YAPI_INVALID_STRING;
const string YCellular::APNSECRET_INVALID = YAPI_INVALID_STRING;
const string YCellular::COMMAND_INVALID = YAPI_INVALID_STRING;

int YCellular::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("linkQuality")) {
        _linkQuality =  json_val->getInt("linkQuality");
    }
    if(json_val->has("cellOperator")) {
        _cellOperator =  json_val->getString("cellOperator");
    }
    if(json_val->has("cellIdentifier")) {
        _cellIdentifier =  json_val->getString("cellIdentifier");
    }
    if(json_val->has("cellType")) {
        _cellType =  (Y_CELLTYPE_enum)json_val->getInt("cellType");
    }
    if(json_val->has("imsi")) {
        _imsi =  json_val->getString("imsi");
    }
    if(json_val->has("message")) {
        _message =  json_val->getString("message");
    }
    if(json_val->has("pin")) {
        _pin =  json_val->getString("pin");
    }
    if(json_val->has("lockedOperator")) {
        _lockedOperator =  json_val->getString("lockedOperator");
    }
    if(json_val->has("airplaneMode")) {
        _airplaneMode =  (Y_AIRPLANEMODE_enum)json_val->getInt("airplaneMode");
    }
    if(json_val->has("enableData")) {
        _enableData =  (Y_ENABLEDATA_enum)json_val->getInt("enableData");
    }
    if(json_val->has("apn")) {
        _apn =  json_val->getString("apn");
    }
    if(json_val->has("apnSecret")) {
        _apnSecret =  json_val->getString("apnSecret");
    }
    if(json_val->has("pingInterval")) {
        _pingInterval =  json_val->getInt("pingInterval");
    }
    if(json_val->has("dataSent")) {
        _dataSent =  json_val->getInt("dataSent");
    }
    if(json_val->has("dataReceived")) {
        _dataReceived =  json_val->getInt("dataReceived");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the link quality, expressed in percent.
 *
 * @return an integer corresponding to the link quality, expressed in percent
 *
 * On failure, throws an exception or returns Y_LINKQUALITY_INVALID.
 */
int YCellular::get_linkQuality(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::LINKQUALITY_INVALID;
                }
            }
        }
        res = _linkQuality;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the name of the cell operator currently in use.
 *
 * @return a string corresponding to the name of the cell operator currently in use
 *
 * On failure, throws an exception or returns Y_CELLOPERATOR_INVALID.
 */
string YCellular::get_cellOperator(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::CELLOPERATOR_INVALID;
                }
            }
        }
        res = _cellOperator;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the unique identifier of the cellular antenna in use: MCC, MNC, LAC and Cell ID.
 *
 * @return a string corresponding to the unique identifier of the cellular antenna in use: MCC, MNC,
 * LAC and Cell ID
 *
 * On failure, throws an exception or returns Y_CELLIDENTIFIER_INVALID.
 */
string YCellular::get_cellIdentifier(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::CELLIDENTIFIER_INVALID;
                }
            }
        }
        res = _cellIdentifier;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Active cellular connection type.
 *
 * @return a value among Y_CELLTYPE_GPRS, Y_CELLTYPE_EGPRS, Y_CELLTYPE_WCDMA, Y_CELLTYPE_HSDPA,
 * Y_CELLTYPE_NONE and Y_CELLTYPE_CDMA
 *
 * On failure, throws an exception or returns Y_CELLTYPE_INVALID.
 */
Y_CELLTYPE_enum YCellular::get_cellType(void)
{
    Y_CELLTYPE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::CELLTYPE_INVALID;
                }
            }
        }
        res = _cellType;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns an opaque string if a PIN code has been configured in the device to access
 * the SIM card, or an empty string if none has been configured or if the code provided
 * was rejected by the SIM card.
 *
 * @return a string corresponding to an opaque string if a PIN code has been configured in the device to access
 *         the SIM card, or an empty string if none has been configured or if the code provided
 *         was rejected by the SIM card
 *
 * On failure, throws an exception or returns Y_IMSI_INVALID.
 */
string YCellular::get_imsi(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::IMSI_INVALID;
                }
            }
        }
        res = _imsi;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the latest status message from the wireless interface.
 *
 * @return a string corresponding to the latest status message from the wireless interface
 *
 * On failure, throws an exception or returns Y_MESSAGE_INVALID.
 */
string YCellular::get_message(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::MESSAGE_INVALID;
                }
            }
        }
        res = _message;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns an opaque string if a PIN code has been configured in the device to access
 * the SIM card, or an empty string if none has been configured or if the code provided
 * was rejected by the SIM card.
 *
 * @return a string corresponding to an opaque string if a PIN code has been configured in the device to access
 *         the SIM card, or an empty string if none has been configured or if the code provided
 *         was rejected by the SIM card
 *
 * On failure, throws an exception or returns Y_PIN_INVALID.
 */
string YCellular::get_pin(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::PIN_INVALID;
                }
            }
        }
        res = _pin;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the PIN code used by the module to access the SIM card.
 * This function does not change the code on the SIM card itself, but only changes
 * the parameter used by the device to try to get access to it. If the SIM code
 * does not work immediately on first try, it will be automatically forgotten
 * and the message will be set to "Enter SIM PIN". The method should then be
 * invoked again with right correct PIN code. After three failed attempts in a row,
 * the message is changed to "Enter SIM PUK" and the SIM card PUK code must be
 * provided using method sendPUK.
 *
 * Remember to call the saveToFlash() method of the module to save the
 * new value in the device flash.
 *
 * @param newval : a string corresponding to the PIN code used by the module to access the SIM card
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_pin(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("pin", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the name of the only cell operator to use if automatic choice is disabled,
 * or an empty string if the SIM card will automatically choose among available
 * cell operators.
 *
 * @return a string corresponding to the name of the only cell operator to use if automatic choice is disabled,
 *         or an empty string if the SIM card will automatically choose among available
 *         cell operators
 *
 * On failure, throws an exception or returns Y_LOCKEDOPERATOR_INVALID.
 */
string YCellular::get_lockedOperator(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::LOCKEDOPERATOR_INVALID;
                }
            }
        }
        res = _lockedOperator;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the name of the cell operator to be used. If the name is an empty
 * string, the choice will be made automatically based on the SIM card. Otherwise,
 * the selected operator is the only one that will be used.
 *
 * @param newval : a string corresponding to the name of the cell operator to be used
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_lockedOperator(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("lockedOperator", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns true if the airplane mode is active (radio turned off).
 *
 * @return either Y_AIRPLANEMODE_OFF or Y_AIRPLANEMODE_ON, according to true if the airplane mode is
 * active (radio turned off)
 *
 * On failure, throws an exception or returns Y_AIRPLANEMODE_INVALID.
 */
Y_AIRPLANEMODE_enum YCellular::get_airplaneMode(void)
{
    Y_AIRPLANEMODE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::AIRPLANEMODE_INVALID;
                }
            }
        }
        res = _airplaneMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the activation state of airplane mode (radio turned off).
 *
 * @param newval : either Y_AIRPLANEMODE_OFF or Y_AIRPLANEMODE_ON, according to the activation state
 * of airplane mode (radio turned off)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_airplaneMode(Y_AIRPLANEMODE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = (newval>0 ? "1" : "0");
        res = _setAttr("airplaneMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the condition for enabling IP data services (GPRS).
 * When data services are disabled, SMS are the only mean of communication.
 *
 * @return a value among Y_ENABLEDATA_HOMENETWORK, Y_ENABLEDATA_ROAMING, Y_ENABLEDATA_NEVER and
 * Y_ENABLEDATA_NEUTRALITY corresponding to the condition for enabling IP data services (GPRS)
 *
 * On failure, throws an exception or returns Y_ENABLEDATA_INVALID.
 */
Y_ENABLEDATA_enum YCellular::get_enableData(void)
{
    Y_ENABLEDATA_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::ENABLEDATA_INVALID;
                }
            }
        }
        res = _enableData;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the condition for enabling IP data services (GPRS).
 * The service can be either fully deactivated, or limited to the SIM home network,
 * or enabled for all partner networks (roaming). Caution: enabling data services
 * on roaming networks may cause prohibitive communication costs !
 *
 * When data services are disabled, SMS are the only mean of communication.
 *
 * @param newval : a value among Y_ENABLEDATA_HOMENETWORK, Y_ENABLEDATA_ROAMING, Y_ENABLEDATA_NEVER
 * and Y_ENABLEDATA_NEUTRALITY corresponding to the condition for enabling IP data services (GPRS)
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_enableData(Y_ENABLEDATA_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("enableData", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the Access Point Name (APN) to be used, if needed.
 * When left blank, the APN suggested by the cell operator will be used.
 *
 * @return a string corresponding to the Access Point Name (APN) to be used, if needed
 *
 * On failure, throws an exception or returns Y_APN_INVALID.
 */
string YCellular::get_apn(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::APN_INVALID;
                }
            }
        }
        res = _apn;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the Access Point Name (APN) to be used, if needed.
 * When left blank, the APN suggested by the cell operator will be used.
 *
 * @param newval : a string
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_apn(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("apn", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns an opaque string if APN authentication parameters have been configured
 * in the device, or an empty string otherwise.
 * To configure these parameters, use set_apnAuth().
 *
 * @return a string corresponding to an opaque string if APN authentication parameters have been configured
 *         in the device, or an empty string otherwise
 *
 * On failure, throws an exception or returns Y_APNSECRET_INVALID.
 */
string YCellular::get_apnSecret(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::APNSECRET_INVALID;
                }
            }
        }
        res = _apnSecret;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YCellular::set_apnSecret(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("apnSecret", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the automated connectivity check interval, in seconds.
 *
 * @return an integer corresponding to the automated connectivity check interval, in seconds
 *
 * On failure, throws an exception or returns Y_PINGINTERVAL_INVALID.
 */
int YCellular::get_pingInterval(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::PINGINTERVAL_INVALID;
                }
            }
        }
        res = _pingInterval;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the automated connectivity check interval, in seconds.
 *
 * @param newval : an integer corresponding to the automated connectivity check interval, in seconds
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_pingInterval(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("pingInterval", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of bytes sent so far.
 *
 * @return an integer corresponding to the number of bytes sent so far
 *
 * On failure, throws an exception or returns Y_DATASENT_INVALID.
 */
int YCellular::get_dataSent(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::DATASENT_INVALID;
                }
            }
        }
        res = _dataSent;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the value of the outgoing data counter.
 *
 * @param newval : an integer corresponding to the value of the outgoing data counter
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_dataSent(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("dataSent", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of bytes received so far.
 *
 * @return an integer corresponding to the number of bytes received so far
 *
 * On failure, throws an exception or returns Y_DATARECEIVED_INVALID.
 */
int YCellular::get_dataReceived(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::DATARECEIVED_INVALID;
                }
            }
        }
        res = _dataReceived;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the value of the incoming data counter.
 *
 * @param newval : an integer corresponding to the value of the incoming data counter
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_dataReceived(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("dataReceived", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YCellular::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YCellular::COMMAND_INVALID;
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

int YCellular::set_command(const string& newval)
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
 * Retrieves a cellular interface for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the cellular interface is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCellular.isOnline() to test if the cellular interface is
 * indeed online at a given time. In case of ambiguity when looking for
 * a cellular interface by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the cellular interface
 *
 * @return a YCellular object allowing you to drive the cellular interface.
 */
YCellular* YCellular::FindCellular(string func)
{
    YCellular* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YCellular*) YFunction::_FindFromCache("Cellular", func);
        if (obj == NULL) {
            obj = new YCellular(func);
            YFunction::_AddToCache("Cellular", func, obj);
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
int YCellular::registerValueCallback(YCellularValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackCellular = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YCellular::_invokeValueCallback(string value)
{
    if (_valueCallbackCellular != NULL) {
        _valueCallbackCellular(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Sends a PUK code to unlock the SIM card after three failed PIN code attempts, and
 * setup a new PIN into the SIM card. Only ten consecutives tentatives are permitted:
 * after that, the SIM card will be blocked permanently without any mean of recovery
 * to use it again. Note that after calling this method, you have usually to invoke
 * method set_pin() to tell the YoctoHub which PIN to use in the future.
 *
 * @param puk : the SIM PUK code
 * @param newPin : new PIN code to configure into the SIM card
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::sendPUK(string puk,string newPin)
{
    string gsmMsg;
    gsmMsg = this->get_message();
    if (!((gsmMsg).substr(0, 13) == "Enter SIM PUK")) {
        _throw(YAPI_INVALID_ARGUMENT,"PUK not expected at this time");
        return YAPI_INVALID_ARGUMENT;
    }
    if (newPin == "") {
        return this->set_command(YapiWrapper::ysprintf("AT+CPIN=%s,0000;+CLCK=SC,0,0000",puk.c_str()));
    }
    return this->set_command(YapiWrapper::ysprintf("AT+CPIN=%s,%s",puk.c_str(),newPin.c_str()));
}

/**
 * Configure authentication parameters to connect to the APN. Both
 * PAP and CHAP authentication are supported.
 *
 * @param username : APN username
 * @param password : APN password
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::set_apnAuth(string username,string password)
{
    return this->set_apnSecret(YapiWrapper::ysprintf("%s,%s",username.c_str(),password.c_str()));
}

/**
 * Clear the transmitted data counters.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YCellular::clearDataCounters(void)
{
    int retcode = 0;

    retcode = this->set_dataReceived(0);
    if (retcode != YAPI_SUCCESS) {
        return retcode;
    }
    retcode = this->set_dataSent(0);
    return retcode;
}

/**
 * Sends an AT command to the GSM module and returns the command output.
 * The command will only execute when the GSM module is in standard
 * command state, and should leave it in the exact same state.
 * Use this function with great care !
 *
 * @param cmd : the AT command to execute, like for instance: "+CCLK?".
 *
 * @return a string with the result of the commands. Empty lines are
 *         automatically removed from the output.
 */
string YCellular::_AT(string cmd)
{
    int chrPos = 0;
    int cmdLen = 0;
    int waitMore = 0;
    string res;
    string buff;
    int bufflen = 0;
    string buffstr;
    int buffstrlen = 0;
    int idx = 0;
    int suffixlen = 0;
    // quote dangerous characters used in AT commands
    cmdLen = (int)(cmd).length();
    chrPos = _ystrpos(cmd, "#");
    while (chrPos >= 0) {
        cmd = YapiWrapper::ysprintf("%s%c23%s", (cmd).substr( 0, chrPos).c_str(), 37,(cmd).substr( chrPos+1, cmdLen-chrPos-1).c_str());
        cmdLen = cmdLen + 2;
        chrPos = _ystrpos(cmd, "#");
    }
    chrPos = _ystrpos(cmd, "+");
    while (chrPos >= 0) {
        cmd = YapiWrapper::ysprintf("%s%c2B%s", (cmd).substr( 0, chrPos).c_str(), 37,(cmd).substr( chrPos+1, cmdLen-chrPos-1).c_str());
        cmdLen = cmdLen + 2;
        chrPos = _ystrpos(cmd, "+");
    }
    chrPos = _ystrpos(cmd, "=");
    while (chrPos >= 0) {
        cmd = YapiWrapper::ysprintf("%s%c3D%s", (cmd).substr( 0, chrPos).c_str(), 37,(cmd).substr( chrPos+1, cmdLen-chrPos-1).c_str());
        cmdLen = cmdLen + 2;
        chrPos = _ystrpos(cmd, "=");
    }
    cmd = YapiWrapper::ysprintf("at.txt?cmd=%s",cmd.c_str());
    res = YapiWrapper::ysprintf("");
    // max 2 minutes (each iteration may take up to 5 seconds if waiting)
    waitMore = 24;
    while (waitMore > 0) {
        buff = this->_download(cmd);
        bufflen = (int)(buff).size();
        buffstr = buff;
        buffstrlen = (int)(buffstr).length();
        idx = bufflen - 1;
        while ((idx > 0) && (((u8)buff[idx]) != 64) && (((u8)buff[idx]) != 10) && (((u8)buff[idx]) != 13)) {
            idx = idx - 1;
        }
        if (((u8)buff[idx]) == 64) {
            // continuation detected
            suffixlen = bufflen - idx;
            cmd = YapiWrapper::ysprintf("at.txt?cmd=%s",(buffstr).substr( buffstrlen - suffixlen, suffixlen).c_str());
            buffstr = (buffstr).substr( 0, buffstrlen - suffixlen);
            waitMore = waitMore - 1;
        } else {
            // request complete
            waitMore = 0;
        }
        res = YapiWrapper::ysprintf("%s%s", res.c_str(),buffstr.c_str());
    }
    return res;
}

/**
 * Returns the list detected cell operators in the neighborhood.
 * This function will typically take between 30 seconds to 1 minute to
 * return. Note that any SIM card can usually only connect to specific
 * operators. All networks returned by this function might therefore
 * not be available for connection.
 *
 * @return a list of string (cell operator names).
 */
vector<string> YCellular::get_availableOperators(void)
{
    string cops;
    int idx = 0;
    int slen = 0;
    vector<string> res;

    cops = this->_AT("+COPS=?");
    slen = (int)(cops).length();
    res.clear();
    idx = _ystrpos(cops, "(");
    while (idx >= 0) {
        slen = slen - (idx+1);
        cops = (cops).substr( idx+1, slen);
        idx = _ystrpos(cops, "\"");
        if (idx > 0) {
            slen = slen - (idx+1);
            cops = (cops).substr( idx+1, slen);
            idx = _ystrpos(cops, "\"");
            if (idx > 0) {
                res.push_back((cops).substr( 0, idx));
            }
        }
        idx = _ystrpos(cops, "(");
    }
    return res;
}

/**
 * Returns a list of nearby cellular antennas, as required for quick
 * geolocation of the device. The first cell listed is the serving
 * cell, and the next ones are the neighboor cells reported by the
 * serving cell.
 *
 * @return a list of YCellRecords.
 */
vector<YCellRecord> YCellular::quickCellSurvey(void)
{
    string moni;
    vector<string> recs;
    int llen = 0;
    string mccs;
    int mcc = 0;
    string mncs;
    int mnc = 0;
    int lac = 0;
    int cellId = 0;
    string dbms;
    int dbm = 0;
    string tads;
    int tad = 0;
    string oper;
    vector<YCellRecord> res;

    moni = this->_AT("+CCED=0;#MONI=7;#MONI");
    mccs = (moni).substr(7, 3);
    if ((mccs).substr(0, 1) == "0") {
        mccs = (mccs).substr(1, 2);
    }
    if ((mccs).substr(0, 1) == "0") {
        mccs = (mccs).substr(1, 1);
    }
    mcc = atoi((mccs).c_str());
    mncs = (moni).substr(11, 3);
    if ((mncs).substr(2, 1) == ",") {
        mncs = (mncs).substr(0, 2);
    }
    if ((mncs).substr(0, 1) == "0") {
        mncs = (mncs).substr(1, (int)(mncs).length()-1);
    }
    mnc = atoi((mncs).c_str());
    recs = _strsplit(moni,'#');
    // process each line in turn
    res.clear();
    for (unsigned ii = 0; ii < recs.size(); ii++) {
        llen = (int)(recs[ii]).length() - 2;
        if (llen >= 44) {
            if ((recs[ii]).substr(41, 3) == "dbm") {
                lac = (int)strtoul((recs[ii]).substr(16, 4).c_str(), NULL, 16);
                cellId = (int)strtoul((recs[ii]).substr(23, 4).c_str(), NULL, 16);
                dbms = (recs[ii]).substr(37, 4);
                if ((dbms).substr(0, 1) == " ") {
                    dbms = (dbms).substr(1, 3);
                }
                dbm = atoi((dbms).c_str());
                if (llen > 66) {
                    tads = (recs[ii]).substr(54, 2);
                    if ((tads).substr(0, 1) == " ") {
                        tads = (tads).substr(1, 3);
                    }
                    tad = atoi((tads).c_str());
                    oper = (recs[ii]).substr(66, llen-66);
                } else {
                    tad = -1;
                    oper = "";
                }
                if (lac < 65535) {
                    res.push_back(YCellRecord(mcc,mnc,lac,cellId,dbm,tad,oper));
                }
            }
        }
    }
    return res;
}

YCellular *YCellular::nextCellular(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YCellular::FindCellular(hwid);
}

YCellular* YCellular::FirstCellular(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Cellular", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YCellular::FindCellular(serial+"."+funcId);
}

//--- (end of generated code: YCellular implementation)

//--- (generated code: YCellular functions)
//--- (end of generated code: YCellular functions)
