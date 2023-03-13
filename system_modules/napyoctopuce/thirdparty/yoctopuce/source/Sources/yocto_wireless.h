/*********************************************************************
 *
 * $Id: yocto_wireless.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindWireless(), the high-level API for Wireless functions
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

#ifndef YOCTO_WIRELESS_H
#define YOCTO_WIRELESS_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (generated code: YWireless definitions)
class YWireless; // forward declaration

typedef void (*YWirelessValueCallback)(YWireless *func, const string& functionValue);
#ifndef _Y_SECURITY_ENUM
#define _Y_SECURITY_ENUM
typedef enum {
    Y_SECURITY_UNKNOWN = 0,
    Y_SECURITY_OPEN = 1,
    Y_SECURITY_WEP = 2,
    Y_SECURITY_WPA = 3,
    Y_SECURITY_WPA2 = 4,
    Y_SECURITY_INVALID = -1,
} Y_SECURITY_enum;
#endif
#ifndef _Y_WLANSTATE_ENUM
#define _Y_WLANSTATE_ENUM
typedef enum {
    Y_WLANSTATE_DOWN = 0,
    Y_WLANSTATE_SCANNING = 1,
    Y_WLANSTATE_CONNECTED = 2,
    Y_WLANSTATE_REJECTED = 3,
    Y_WLANSTATE_INVALID = -1,
} Y_WLANSTATE_enum;
#endif
#define Y_LINKQUALITY_INVALID           (YAPI_INVALID_UINT)
#define Y_SSID_INVALID                  (YAPI_INVALID_STRING)
#define Y_CHANNEL_INVALID               (YAPI_INVALID_UINT)
#define Y_MESSAGE_INVALID               (YAPI_INVALID_STRING)
#define Y_WLANCONFIG_INVALID            (YAPI_INVALID_STRING)
//--- (end of generated code: YWireless definitions)


//--- (generated code: YWlanRecord definitions)
//--- (end of generated code: YWlanRecord definitions)

//--- (generated code: YWlanRecord declaration)
/**
 * YWlanRecord Class: Description of a wireless network
 *
 *
 */
class YOCTO_CLASS_EXPORT YWlanRecord {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YWlanRecord declaration)
    //--- (generated code: YWlanRecord attributes)
    // Attributes (function value cache)
    string          _ssid;
    int             _channel;
    string          _sec;
    int             _rssi;
    //--- (end of generated code: YWlanRecord attributes)
    //--- (generated code: YWlanRecord constructor)

    //--- (end of generated code: YWlanRecord constructor)
    //--- (generated code: YWlanRecord initialization)
    //--- (end of generated code: YWlanRecord initialization)

public:
    YWlanRecord(const string& json);
    //--- (generated code: YWlanRecord accessors declaration)


    virtual string      get_ssid(void);

    virtual int         get_channel(void);

    virtual string      get_security(void);

    virtual int         get_linkQuality(void);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YWlanRecord accessors declaration)
};




//--- (generated code: YWireless declaration)
/**
 * YWireless Class: Wireless function interface
 *
 * YWireless functions provides control over wireless network parameters
 * and status for devices that are wireless-enabled.
 */
class YOCTO_CLASS_EXPORT YWireless: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YWireless declaration)
    //--- (generated code: YWireless attributes)
    // Attributes (function value cache)
    int             _linkQuality;
    string          _ssid;
    int             _channel;
    Y_SECURITY_enum _security;
    string          _message;
    string          _wlanConfig;
    Y_WLANSTATE_enum _wlanState;
    YWirelessValueCallback _valueCallbackWireless;

    friend YWireless *yFindWireless(const string& func);
    friend YWireless *yFirstWireless(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindWireless factory function to instantiate
    YWireless(const string& func);
    //--- (end of generated code: YWireless attributes)
    //--- (generated code: YWireless initialization)
    //--- (end of generated code: YWireless initialization)

public:
    ~YWireless();
    //--- (generated code: YWireless accessors declaration)

    static const int LINKQUALITY_INVALID = YAPI_INVALID_UINT;
    static const string SSID_INVALID;
    static const int CHANNEL_INVALID = YAPI_INVALID_UINT;
    static const Y_SECURITY_enum SECURITY_UNKNOWN = Y_SECURITY_UNKNOWN;
    static const Y_SECURITY_enum SECURITY_OPEN = Y_SECURITY_OPEN;
    static const Y_SECURITY_enum SECURITY_WEP = Y_SECURITY_WEP;
    static const Y_SECURITY_enum SECURITY_WPA = Y_SECURITY_WPA;
    static const Y_SECURITY_enum SECURITY_WPA2 = Y_SECURITY_WPA2;
    static const Y_SECURITY_enum SECURITY_INVALID = Y_SECURITY_INVALID;
    static const string MESSAGE_INVALID;
    static const string WLANCONFIG_INVALID;
    static const Y_WLANSTATE_enum WLANSTATE_DOWN = Y_WLANSTATE_DOWN;
    static const Y_WLANSTATE_enum WLANSTATE_SCANNING = Y_WLANSTATE_SCANNING;
    static const Y_WLANSTATE_enum WLANSTATE_CONNECTED = Y_WLANSTATE_CONNECTED;
    static const Y_WLANSTATE_enum WLANSTATE_REJECTED = Y_WLANSTATE_REJECTED;
    static const Y_WLANSTATE_enum WLANSTATE_INVALID = Y_WLANSTATE_INVALID;

    /**
     * Returns the link quality, expressed in percent.
     *
     * @return an integer corresponding to the link quality, expressed in percent
     *
     * On failure, throws an exception or returns Y_LINKQUALITY_INVALID.
     */
    int                 get_linkQuality(void);

    inline int          linkQuality(void)
    { return this->get_linkQuality(); }

    /**
     * Returns the wireless network name (SSID).
     *
     * @return a string corresponding to the wireless network name (SSID)
     *
     * On failure, throws an exception or returns Y_SSID_INVALID.
     */
    string              get_ssid(void);

    inline string       ssid(void)
    { return this->get_ssid(); }

    /**
     * Returns the 802.11 channel currently used, or 0 when the selected network has not been found.
     *
     * @return an integer corresponding to the 802.11 channel currently used, or 0 when the selected
     * network has not been found
     *
     * On failure, throws an exception or returns Y_CHANNEL_INVALID.
     */
    int                 get_channel(void);

    inline int          channel(void)
    { return this->get_channel(); }

    /**
     * Returns the security algorithm used by the selected wireless network.
     *
     * @return a value among Y_SECURITY_UNKNOWN, Y_SECURITY_OPEN, Y_SECURITY_WEP, Y_SECURITY_WPA and
     * Y_SECURITY_WPA2 corresponding to the security algorithm used by the selected wireless network
     *
     * On failure, throws an exception or returns Y_SECURITY_INVALID.
     */
    Y_SECURITY_enum     get_security(void);

    inline Y_SECURITY_enum security(void)
    { return this->get_security(); }

    /**
     * Returns the latest status message from the wireless interface.
     *
     * @return a string corresponding to the latest status message from the wireless interface
     *
     * On failure, throws an exception or returns Y_MESSAGE_INVALID.
     */
    string              get_message(void);

    inline string       message(void)
    { return this->get_message(); }

    string              get_wlanConfig(void);

    inline string       wlanConfig(void)
    { return this->get_wlanConfig(); }

    int             set_wlanConfig(const string& newval);
    inline int      setWlanConfig(const string& newval)
    { return this->set_wlanConfig(newval); }

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
    Y_WLANSTATE_enum    get_wlanState(void);

    inline Y_WLANSTATE_enum wlanState(void)
    { return this->get_wlanState(); }

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
    static YWireless*   FindWireless(string func);

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
    virtual int         registerValueCallback(YWirelessValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

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
    virtual int         startWlanScan(void);

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
    virtual int         joinNetwork(string ssid,string securityKey);

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
    virtual int         adhocNetwork(string ssid,string securityKey);

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
    virtual int         softAPNetwork(string ssid,string securityKey);

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
    virtual vector<YWlanRecord> get_detectedWlans(void);


    inline static YWireless* Find(string func)
    { return YWireless::FindWireless(func); }

    /**
     * Continues the enumeration of wireless lan interfaces started using yFirstWireless().
     *
     * @return a pointer to a YWireless object, corresponding to
     *         a wireless lan interface currently online, or a NULL pointer
     *         if there are no more wireless lan interfaces to enumerate.
     */
           YWireless       *nextWireless(void);
    inline YWireless       *next(void)
    { return this->nextWireless();}

    /**
     * Starts the enumeration of wireless lan interfaces currently accessible.
     * Use the method YWireless.nextWireless() to iterate on
     * next wireless lan interfaces.
     *
     * @return a pointer to a YWireless object, corresponding to
     *         the first wireless lan interface currently online, or a NULL pointer
     *         if there are none.
     */
           static YWireless* FirstWireless(void);
    inline static YWireless* First(void)
    { return YWireless::FirstWireless();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YWireless accessors declaration)
};

//--- (generated code: YWireless functions declaration)

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
inline YWireless* yFindWireless(const string& func)
{ return YWireless::FindWireless(func);}
/**
 * Starts the enumeration of wireless lan interfaces currently accessible.
 * Use the method YWireless.nextWireless() to iterate on
 * next wireless lan interfaces.
 *
 * @return a pointer to a YWireless object, corresponding to
 *         the first wireless lan interface currently online, or a NULL pointer
 *         if there are none.
 */
inline YWireless* yFirstWireless(void)
{ return YWireless::FirstWireless();}

//--- (end of generated code: YWireless functions declaration)

#endif
