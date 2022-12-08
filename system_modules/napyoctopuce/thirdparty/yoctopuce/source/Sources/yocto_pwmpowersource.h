/*********************************************************************
 *
 * $Id: yocto_pwmpowersource.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindPwmPowerSource(), the high-level API for PwmPowerSource functions
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


#ifndef YOCTO_PWMPOWERSOURCE_H
#define YOCTO_PWMPOWERSOURCE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YPwmPowerSource return codes)
//--- (end of YPwmPowerSource return codes)
//--- (YPwmPowerSource definitions)
class YPwmPowerSource; // forward declaration

typedef void (*YPwmPowerSourceValueCallback)(YPwmPowerSource *func, const string& functionValue);
#ifndef _Y_POWERMODE_ENUM
#define _Y_POWERMODE_ENUM
typedef enum {
    Y_POWERMODE_USB_5V = 0,
    Y_POWERMODE_USB_3V = 1,
    Y_POWERMODE_EXT_V = 2,
    Y_POWERMODE_OPNDRN = 3,
    Y_POWERMODE_INVALID = -1,
} Y_POWERMODE_enum;
#endif
//--- (end of YPwmPowerSource definitions)

//--- (YPwmPowerSource declaration)
/**
 * YPwmPowerSource Class: PwmPowerSource function interface
 *
 * The Yoctopuce application programming interface allows you to configure
 * the voltage source used by all PWM on the same device.
 */
class YOCTO_CLASS_EXPORT YPwmPowerSource: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YPwmPowerSource declaration)
protected:
    //--- (YPwmPowerSource attributes)
    // Attributes (function value cache)
    Y_POWERMODE_enum _powerMode;
    YPwmPowerSourceValueCallback _valueCallbackPwmPowerSource;

    friend YPwmPowerSource *yFindPwmPowerSource(const string& func);
    friend YPwmPowerSource *yFirstPwmPowerSource(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindPwmPowerSource factory function to instantiate
    YPwmPowerSource(const string& func);
    //--- (end of YPwmPowerSource attributes)

public:
    ~YPwmPowerSource();
    //--- (YPwmPowerSource accessors declaration)

    static const Y_POWERMODE_enum POWERMODE_USB_5V = Y_POWERMODE_USB_5V;
    static const Y_POWERMODE_enum POWERMODE_USB_3V = Y_POWERMODE_USB_3V;
    static const Y_POWERMODE_enum POWERMODE_EXT_V = Y_POWERMODE_EXT_V;
    static const Y_POWERMODE_enum POWERMODE_OPNDRN = Y_POWERMODE_OPNDRN;
    static const Y_POWERMODE_enum POWERMODE_INVALID = Y_POWERMODE_INVALID;

    /**
     * Returns the selected power source for the PWM on the same device.
     *
     * @return a value among Y_POWERMODE_USB_5V, Y_POWERMODE_USB_3V, Y_POWERMODE_EXT_V and
     * Y_POWERMODE_OPNDRN corresponding to the selected power source for the PWM on the same device
     *
     * On failure, throws an exception or returns Y_POWERMODE_INVALID.
     */
    Y_POWERMODE_enum    get_powerMode(void);

    inline Y_POWERMODE_enum powerMode(void)
    { return this->get_powerMode(); }

    /**
     * Changes  the PWM power source. PWM can use isolated 5V from USB, isolated 3V from USB or
     * voltage from an external power source. The PWM can also work in open drain  mode. In that
     * mode, the PWM actively pulls the line down.
     * Warning: this setting is common to all PWM on the same device. If you change that parameter,
     * all PWM located on the same device are  affected.
     * If you want the change to be kept after a device reboot, make sure  to call the matching
     * module saveToFlash().
     *
     * @param newval : a value among Y_POWERMODE_USB_5V, Y_POWERMODE_USB_3V, Y_POWERMODE_EXT_V and
     * Y_POWERMODE_OPNDRN corresponding to  the PWM power source
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_powerMode(Y_POWERMODE_enum newval);
    inline int      setPowerMode(Y_POWERMODE_enum newval)
    { return this->set_powerMode(newval); }

    /**
     * Retrieves a voltage source for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the voltage source is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YPwmPowerSource.isOnline() to test if the voltage source is
     * indeed online at a given time. In case of ambiguity when looking for
     * a voltage source by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the voltage source
     *
     * @return a YPwmPowerSource object allowing you to drive the voltage source.
     */
    static YPwmPowerSource* FindPwmPowerSource(string func);

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
    virtual int         registerValueCallback(YPwmPowerSourceValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YPwmPowerSource* Find(string func)
    { return YPwmPowerSource::FindPwmPowerSource(func); }

    /**
     * Continues the enumeration of Voltage sources started using yFirstPwmPowerSource().
     *
     * @return a pointer to a YPwmPowerSource object, corresponding to
     *         a voltage source currently online, or a NULL pointer
     *         if there are no more Voltage sources to enumerate.
     */
           YPwmPowerSource *nextPwmPowerSource(void);
    inline YPwmPowerSource *next(void)
    { return this->nextPwmPowerSource();}

    /**
     * Starts the enumeration of Voltage sources currently accessible.
     * Use the method YPwmPowerSource.nextPwmPowerSource() to iterate on
     * next Voltage sources.
     *
     * @return a pointer to a YPwmPowerSource object, corresponding to
     *         the first source currently online, or a NULL pointer
     *         if there are none.
     */
           static YPwmPowerSource* FirstPwmPowerSource(void);
    inline static YPwmPowerSource* First(void)
    { return YPwmPowerSource::FirstPwmPowerSource();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YPwmPowerSource accessors declaration)
};

//--- (YPwmPowerSource functions declaration)

/**
 * Retrieves a voltage source for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the voltage source is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YPwmPowerSource.isOnline() to test if the voltage source is
 * indeed online at a given time. In case of ambiguity when looking for
 * a voltage source by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the voltage source
 *
 * @return a YPwmPowerSource object allowing you to drive the voltage source.
 */
inline YPwmPowerSource* yFindPwmPowerSource(const string& func)
{ return YPwmPowerSource::FindPwmPowerSource(func);}
/**
 * Starts the enumeration of Voltage sources currently accessible.
 * Use the method YPwmPowerSource.nextPwmPowerSource() to iterate on
 * next Voltage sources.
 *
 * @return a pointer to a YPwmPowerSource object, corresponding to
 *         the first source currently online, or a NULL pointer
 *         if there are none.
 */
inline YPwmPowerSource* yFirstPwmPowerSource(void)
{ return YPwmPowerSource::FirstPwmPowerSource();}

//--- (end of YPwmPowerSource functions declaration)

#endif
