/*********************************************************************
 *
 * $Id: yocto_wireless.cpp 28753 2017-10-03 11:23:38Z seb $
 *
 * Implements yFindWireless(), the high-level API for Wireless functions
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
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
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
#include "yocto_wireless.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define  __FILE_ID__  "wireless"



YWlanRecord::YWlanRecord(const string& json):
//--- (generated code: YWlanRecord initialization)
    _channel(0)
    ,_rssi(0)
//--- (end of generated code: YWlanRecord initialization)
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
        if (!strcmp(j.token, "ssid")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return ;
            }
            _ssid = (string)j.token;
            while(j.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j) == YJSON_PARSE_AVAIL) {
                _ssid +=(string)j.token;
            }
        }else if (!strcmp(j.token, "sec")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return ;
            }
            _sec = (string)j.token;
            while(j.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j) == YJSON_PARSE_AVAIL) {
                _sec +=(string)j.token;
            }
        } else if(!strcmp(j.token, "channel")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return;
            }
            _channel = atoi(j.token);;
        } else if(!strcmp(j.token, "rssi")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return;
            }
            _rssi = atoi(j.token);;
        } else {
            yJsonSkip(&j, 1);
        }
    }
}

//--- (generated code: YWlanRecord implementation)
// static attributes


string YWlanRecord::get_ssid(void)
{
    return _ssid;
}

int YWlanRecord::get_channel(void)
{
    return _channel;
}

string YWlanRecord::get_security(void)
{
    return _sec;
}

int YWlanRecord::get_linkQuality(void)
{
    return _rssi;
}
//--- (end of generated code: YWlanRecord implementation)


YWireless::YWireless(const string& func): YFunction(func)
//--- (generated code: YWireless initialization)
    ,_linkQuality(LINKQUALITY_INVALID)
    ,_ssid(SSID_INVALID)
    ,_channel(CHANNEL_INVALID)
    ,_security(SECURITY_INVALID)
    ,_message(MESSAGE_INVALID)
    ,_wlanConfig(WLANCONFIG_INVALID)
    ,_wlanState(WLANSTATE_INVALID)
    ,_valueCallbackWireless(NULL)
//--- (end of generated code: YWireless initialization)
{
    _className = "Wireless";
}

YWireless::~YWireless()
{
//--- (generated code: YWireless cleanup)
//--- (end of generated code: YWireless cleanup)
}
//--- (generated code: YWireless implementation)
// static attributes
const string YWireless::SSID_INVALID = YAPI_INVALID_STRING;
const string YWireless::MESSAGE_INVALID = YAPI_INVALID_STRING;
const string YWireless::WLANCONFIG_INVALID = YAPI_INVALID_STRING;

int YWireless::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("linkQuality")) {
        _linkQuality =  json_val->getInt("linkQuality");
    }
    if(json_val->has("ssid")) {
        _ssid =  json_val->getString("ssid");
    }
    if(json_val->has("channel")) {
        _channel =  json_val->getInt("channel");
    }
    if(json_val->has("security")) {
        _security =  (Y_SECURITY_enum)json_val->getInt("security");
    }
    if(json_val->has("message")) {
        _message =  json_val->getString("message");
    }
    if(json_val->has("wlanConfig")) {
        _wlanConfig =  json_val->getString("wlanConfig");
    }
    if(json_val->has("wlanState")) {
        _wlanState =  (Y_WLANSTATE_enum)json_val->getInt("wlanState");
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
int YWireless::get_linkQuality(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::LINKQUALITY_INVALID;
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
 * Returns the wireless network name (SSID).
 *
 * @return a string corresponding to the wireless network name (SSID)
 *
 * On failure, throws an exception or returns Y_SSID_INVALID.
 */
string YWireless::get_ssid(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::SSID_INVALID;
                }
            }
        }
        res = _ssid;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the 802.11 channel currently used, or 0 when the selected network has not been found.
 *
 * @return an integer corresponding to the 802.11 channel currently used, or 0 when the selected
 * network has not been found
 *
 * On failure, throws an exception or returns Y_CHANNEL_INVALID.
 */
int YWireless::get_channel(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::CHANNEL_INVALID;
                }
            }
        }
        res = _channel;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the security algorithm used by the selected wireless network.
 *
 * @return a value among Y_SECURITY_UNKNOWN, Y_SECURITY_OPEN, Y_SECURITY_WEP, Y_SECURITY_WPA and
 * Y_SECURITY_WPA2 corresponding to the security algorithm used by the selected wireless network
 *
 * On failure, throws an exception or returns Y_SECURITY_INVALID.
 */
Y_SECURITY_enum YWireless::get_security(void)
{
    Y_SECURITY_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::SECURITY_INVALID;
                }
            }
        }
        res = _security;
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
string YWireless::get_message(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::MESSAGE_INVALID;
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

string YWireless::get_wlanConfig(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::WLANCONFIG_INVALID;
                }
            }
        }
        res = _wlanConfig;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YWireless::set_wlanConfig(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("wlanConfig", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current state of the wireless interface. The state Y_WLANSTATE_DOWN means that the
 * network interface is
 * not connected to a network. The state Y_WLANSTATE_SCANNING means that the network interface is
 * scanning available
 * frequencies. During this stage, the device is not reachable, and the network settings are not yet
 * applied. The state
 * Y_WLANSTATE_CONNECTED means that the network settings have been successfully applied ant that the
 * device is reachable
 * from the wireless network. If the device is configured to use ad-hoc or Soft AP mode, it means that
 * the wireless network
 * is up and that other devices can join the network. The state Y_WLANSTATE_REJECTED means that the
 * network interface has
 * not been able to join the requested network. The description of the error can be obtain with the
 * get_message() method.
 *
 * @return a value among Y_WLANSTATE_DOWN, Y_WLANSTATE_SCANNING, Y_WLANSTATE_CONNECTED and
 * Y_WLANSTATE_REJECTED corresponding to the current state of the wireless interface
 *
 * On failure, throws an exception or returns Y_WLANSTATE_INVALID.
 */
Y_WLANSTATE_enum YWireless::get_wlanState(void)
{
    Y_WLANSTATE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWireless::WLANSTATE_INVALID;
                }
            }
        }
        res = _wlanState;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a wireless lan interface for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the wireless lan interface is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YWireless.isOnline() to test if the wireless lan interface is
 * indeed online at a given time. In case of ambiguity when looking for
 * a wireless lan interface by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the wireless lan interface
 *
 * @return a YWireless object allowing you to drive the wireless lan interface.
 */
YWireless* YWireless::FindWireless(string func)
{
    YWireless* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YWireless*) YFunction::_FindFromCache("Wireless", func);
        if (obj == NULL) {
            obj = new YWireless(func);
            YFunction::_AddToCache("Wireless", func, obj);
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
int YWireless::registerValueCallback(YWirelessValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackWireless = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YWireless::_invokeValueCallback(string value)
{
    if (_valueCallbackWireless != NULL) {
        _valueCallbackWireless(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Triggers a scan of the wireless frequency and builds the list of available networks.
 * The scan forces a disconnection from the current network. At then end of the process, the
 * the network interface attempts to reconnect to the previous network. During the scan, the wlanState
 * switches to Y_WLANSTATE_DOWN, then to Y_WLANSTATE_SCANNING. When the scan is completed,
 * get_wlanState() returns either Y_WLANSTATE_DOWN or Y_WLANSTATE_SCANNING. At this
 * point, the list of detected network can be retrieved with the get_detectedWlans() method.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWireless::startWlanScan(void)
{
    string config;
    config = this->get_wlanConfig();
    // a full scan is triggered when a config is applied
    return this->set_wlanConfig(config);
}

/**
 * Changes the configuration of the wireless lan interface to connect to an existing
 * access point (infrastructure mode).
 * Remember to call the saveToFlash() method and then to reboot the module to apply this setting.
 *
 * @param ssid : the name of the network to connect to
 * @param securityKey : the network key, as a character string
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWireless::joinNetwork(string ssid,string securityKey)
{
    return this->set_wlanConfig(YapiWrapper::ysprintf("INFRA:%s\\%s", ssid.c_str(),securityKey.c_str()));
}

/**
 * Changes the configuration of the wireless lan interface to create an ad-hoc
 * wireless network, without using an access point. On the YoctoHub-Wireless-g,
 * it is best to use softAPNetworkInstead(), which emulates an access point
 * (Soft AP) which is more efficient and more widely supported than ad-hoc networks.
 *
 * When a security key is specified for an ad-hoc network, the network is protected
 * by a WEP40 key (5 characters or 10 hexadecimal digits) or WEP128 key (13 characters
 * or 26 hexadecimal digits). It is recommended to use a well-randomized WEP128 key
 * using 26 hexadecimal digits to maximize security.
 * Remember to call the saveToFlash() method and then to reboot the module
 * to apply this setting.
 *
 * @param ssid : the name of the network to connect to
 * @param securityKey : the network key, as a character string
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWireless::adhocNetwork(string ssid,string securityKey)
{
    return this->set_wlanConfig(YapiWrapper::ysprintf("ADHOC:%s\\%s", ssid.c_str(),securityKey.c_str()));
}

/**
 * Changes the configuration of the wireless lan interface to create a new wireless
 * network by emulating a WiFi access point (Soft AP). This function can only be
 * used with the YoctoHub-Wireless-g.
 *
 * When a security key is specified for a SoftAP network, the network is protected
 * by a WEP40 key (5 characters or 10 hexadecimal digits) or WEP128 key (13 characters
 * or 26 hexadecimal digits). It is recommended to use a well-randomized WEP128 key
 * using 26 hexadecimal digits to maximize security.
 * Remember to call the saveToFlash() method and then to reboot the module to apply this setting.
 *
 * @param ssid : the name of the network to connect to
 * @param securityKey : the network key, as a character string
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWireless::softAPNetwork(string ssid,string securityKey)
{
    return this->set_wlanConfig(YapiWrapper::ysprintf("SOFTAP:%s\\%s", ssid.c_str(),securityKey.c_str()));
}

/**
 * Returns a list of YWlanRecord objects that describe detected Wireless networks.
 * This list is not updated when the module is already connected to an acces point (infrastructure mode).
 * To force an update of this list, startWlanScan() must be called.
 * Note that an languages without garbage collections, the returned list must be freed by the caller.
 *
 * @return a list of YWlanRecord objects, containing the SSID, channel,
 *         link quality and the type of security of the wireless network.
 *
 * On failure, throws an exception or returns an empty list.
 */
vector<YWlanRecord> YWireless::get_detectedWlans(void)
{
    string json;
    vector<string> wlanlist;
    vector<YWlanRecord> res;

    json = this->_download("wlan.json?by=name");
    wlanlist = this->_json_get_array(json);
    res.clear();
    for (unsigned ii = 0; ii < wlanlist.size(); ii++) {
        res.push_back(YWlanRecord(wlanlist[ii]));
    }
    return res;
}

YWireless *YWireless::nextWireless(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YWireless::FindWireless(hwid);
}

YWireless* YWireless::FirstWireless(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Wireless", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YWireless::FindWireless(serial+"."+funcId);
}

//--- (end of generated code: YWireless implementation)

//--- (generated code: YWireless functions)
//--- (end of generated code: YWireless functions)
