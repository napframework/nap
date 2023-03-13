/*********************************************************************
 *
 * $Id: yocto_dualpower.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindDualPower(), the high-level API for DualPower functions
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


#ifndef YOCTO_DUALPOWER_H
#define YOCTO_DUALPOWER_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YDualPower return codes)
//--- (end of YDualPower return codes)
//--- (YDualPower definitions)
class YDualPower; // forward declaration

typedef void (*YDualPowerValueCallback)(YDualPower *func, const string& functionValue);
#ifndef _Y_POWERSTATE_ENUM
#define _Y_POWERSTATE_ENUM
typedef enum {
    Y_POWERSTATE_OFF = 0,
    Y_POWERSTATE_FROM_USB = 1,
    Y_POWERSTATE_FROM_EXT = 2,
    Y_POWERSTATE_INVALID = -1,
} Y_POWERSTATE_enum;
#endif
#ifndef _Y_POWERCONTROL_ENUM
#define _Y_POWERCONTROL_ENUM
typedef enum {
    Y_POWERCONTROL_AUTO = 0,
    Y_POWERCONTROL_FROM_USB = 1,
    Y_POWERCONTROL_FROM_EXT = 2,
    Y_POWERCONTROL_OFF = 3,
    Y_POWERCONTROL_INVALID = -1,
} Y_POWERCONTROL_enum;
#endif
#define Y_EXTVOLTAGE_INVALID            (YAPI_INVALID_UINT)
//--- (end of YDualPower definitions)

//--- (YDualPower declaration)
/**
 * YDualPower Class: External power supply control interface
 *
 * Yoctopuce application programming interface allows you to control
 * the power source to use for module functions that require high current.
 * The module can also automatically disconnect the external power
 * when a voltage drop is observed on the external power source
 * (external battery running out of power).
 */
class YOCTO_CLASS_EXPORT YDualPower: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YDualPower declaration)
protected:
    //--- (YDualPower attributes)
    // Attributes (function value cache)
    Y_POWERSTATE_enum _powerState;
    Y_POWERCONTROL_enum _powerControl;
    int             _extVoltage;
    YDualPowerValueCallback _valueCallbackDualPower;

    friend YDualPower *yFindDualPower(const string& func);
    friend YDualPower *yFirstDualPower(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindDualPower factory function to instantiate
    YDualPower(const string& func);
    //--- (end of YDualPower attributes)

public:
    ~YDualPower();
    //--- (YDualPower accessors declaration)

    static const Y_POWERSTATE_enum POWERSTATE_OFF = Y_POWERSTATE_OFF;
    static const Y_POWERSTATE_enum POWERSTATE_FROM_USB = Y_POWERSTATE_FROM_USB;
    static const Y_POWERSTATE_enum POWERSTATE_FROM_EXT = Y_POWERSTATE_FROM_EXT;
    static const Y_POWERSTATE_enum POWERSTATE_INVALID = Y_POWERSTATE_INVALID;
    static const Y_POWERCONTROL_enum POWERCONTROL_AUTO = Y_POWERCONTROL_AUTO;
    static const Y_POWERCONTROL_enum POWERCONTROL_FROM_USB = Y_POWERCONTROL_FROM_USB;
    static const Y_POWERCONTROL_enum POWERCONTROL_FROM_EXT = Y_POWERCONTROL_FROM_EXT;
    static const Y_POWERCONTROL_enum POWERCONTROL_OFF = Y_POWERCONTROL_OFF;
    static const Y_POWERCONTROL_enum POWERCONTROL_INVALID = Y_POWERCONTROL_INVALID;
    static const int EXTVOLTAGE_INVALID = YAPI_INVALID_UINT;

    /**
     * Returns the current power source for module functions that require lots of current.
     *
     * @return a value among Y_POWERSTATE_OFF, Y_POWERSTATE_FROM_USB and Y_POWERSTATE_FROM_EXT
     * corresponding to the current power source for module functions that require lots of current
     *
     * On failure, throws an exception or returns Y_POWERSTATE_INVALID.
     */
    Y_POWERSTATE_enum   get_powerState(void);

    inline Y_POWERSTATE_enum powerState(void)
    { return this->get_powerState(); }

    /**
     * Returns the selected power source for module functions that require lots of current.
     *
     * @return a value among Y_POWERCONTROL_AUTO, Y_POWERCONTROL_FROM_USB, Y_POWERCONTROL_FROM_EXT and
     * Y_POWERCONTROL_OFF corresponding to the selected power source for module functions that require lots of current
     *
     * On failure, throws an exception or returns Y_POWERCONTROL_INVALID.
     */
    Y_POWERCONTROL_enum get_powerControl(void);

    inline Y_POWERCONTROL_enum powerControl(void)
    { return this->get_powerControl(); }

    /**
     * Changes the selected power source for module functions that require lots of current.
     *
     * @param newval : a value among Y_POWERCONTROL_AUTO, Y_POWERCONTROL_FROM_USB, Y_POWERCONTROL_FROM_EXT
     * and Y_POWERCONTROL_OFF corresponding to the selected power source for module functions that require
     * lots of current
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_powerControl(Y_POWERCONTROL_enum newval);
    inline int      setPowerControl(Y_POWERCONTROL_enum newval)
    { return this->set_powerControl(newval); }

    /**
     * Returns the measured voltage on the external power source, in millivolts.
     *
     * @return an integer corresponding to the measured voltage on the external power source, in millivolts
     *
     * On failure, throws an exception or returns Y_EXTVOLTAGE_INVALID.
     */
    int                 get_extVoltage(void);

    inline int          extVoltage(void)
    { return this->get_extVoltage(); }

    /**
     * Retrieves a dual power control for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the power control is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YDualPower.isOnline() to test if the power control is
     * indeed online at a given time. In case of ambiguity when looking for
     * a dual power control by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the power control
     *
     * @return a YDualPower object allowing you to drive the power control.
     */
    static YDualPower*  FindDualPower(string func);

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
    virtual int         registerValueCallback(YDualPowerValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YDualPower* Find(string func)
    { return YDualPower::FindDualPower(func); }

    /**
     * Continues the enumeration of dual power controls started using yFirstDualPower().
     *
     * @return a pointer to a YDualPower object, corresponding to
     *         a dual power control currently online, or a NULL pointer
     *         if there are no more dual power controls to enumerate.
     */
           YDualPower      *nextDualPower(void);
    inline YDualPower      *next(void)
    { return this->nextDualPower();}

    /**
     * Starts the enumeration of dual power controls currently accessible.
     * Use the method YDualPower.nextDualPower() to iterate on
     * next dual power controls.
     *
     * @return a pointer to a YDualPower object, corresponding to
     *         the first dual power control currently online, or a NULL pointer
     *         if there are none.
     */
           static YDualPower* FirstDualPower(void);
    inline static YDualPower* First(void)
    { return YDualPower::FirstDualPower();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YDualPower accessors declaration)
};

//--- (YDualPower functions declaration)

/**
 * Retrieves a dual power control for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the power control is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YDualPower.isOnline() to test if the power control is
 * indeed online at a given time. In case of ambiguity when looking for
 * a dual power control by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the power control
 *
 * @return a YDualPower object allowing you to drive the power control.
 */
inline YDualPower* yFindDualPower(const string& func)
{ return YDualPower::FindDualPower(func);}
/**
 * Starts the enumeration of dual power controls currently accessible.
 * Use the method YDualPower.nextDualPower() to iterate on
 * next dual power controls.
 *
 * @return a pointer to a YDualPower object, corresponding to
 *         the first dual power control currently online, or a NULL pointer
 *         if there are none.
 */
inline YDualPower* yFirstDualPower(void)
{ return YDualPower::FirstDualPower();}

//--- (end of YDualPower functions declaration)

#endif
