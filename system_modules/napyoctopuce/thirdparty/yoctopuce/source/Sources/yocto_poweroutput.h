/*********************************************************************
 *
 * $Id: yocto_poweroutput.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindPowerOutput(), the high-level API for PowerOutput functions
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


#ifndef YOCTO_POWEROUTPUT_H
#define YOCTO_POWEROUTPUT_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YPowerOutput return codes)
//--- (end of YPowerOutput return codes)
//--- (YPowerOutput definitions)
class YPowerOutput; // forward declaration

typedef void (*YPowerOutputValueCallback)(YPowerOutput *func, const string& functionValue);
#ifndef _Y_VOLTAGE_ENUM
#define _Y_VOLTAGE_ENUM
typedef enum {
    Y_VOLTAGE_OFF = 0,
    Y_VOLTAGE_OUT3V3 = 1,
    Y_VOLTAGE_OUT5V = 2,
    Y_VOLTAGE_INVALID = -1,
} Y_VOLTAGE_enum;
#endif
//--- (end of YPowerOutput definitions)

//--- (YPowerOutput declaration)
/**
 * YPowerOutput Class: External power supply control interface
 *
 * Yoctopuce application programming interface allows you to control
 * the power ouput featured on some devices such as the Yocto-Serial.
 */
class YOCTO_CLASS_EXPORT YPowerOutput: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YPowerOutput declaration)
protected:
    //--- (YPowerOutput attributes)
    // Attributes (function value cache)
    Y_VOLTAGE_enum  _voltage;
    YPowerOutputValueCallback _valueCallbackPowerOutput;

    friend YPowerOutput *yFindPowerOutput(const string& func);
    friend YPowerOutput *yFirstPowerOutput(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindPowerOutput factory function to instantiate
    YPowerOutput(const string& func);
    //--- (end of YPowerOutput attributes)

public:
    ~YPowerOutput();
    //--- (YPowerOutput accessors declaration)

    static const Y_VOLTAGE_enum VOLTAGE_OFF = Y_VOLTAGE_OFF;
    static const Y_VOLTAGE_enum VOLTAGE_OUT3V3 = Y_VOLTAGE_OUT3V3;
    static const Y_VOLTAGE_enum VOLTAGE_OUT5V = Y_VOLTAGE_OUT5V;
    static const Y_VOLTAGE_enum VOLTAGE_INVALID = Y_VOLTAGE_INVALID;

    /**
     * Returns the voltage on the power output featured by the module.
     *
     * @return a value among Y_VOLTAGE_OFF, Y_VOLTAGE_OUT3V3 and Y_VOLTAGE_OUT5V corresponding to the
     * voltage on the power output featured by the module
     *
     * On failure, throws an exception or returns Y_VOLTAGE_INVALID.
     */
    Y_VOLTAGE_enum      get_voltage(void);

    inline Y_VOLTAGE_enum voltage(void)
    { return this->get_voltage(); }

    /**
     * Changes the voltage on the power output provided by the
     * module. Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : a value among Y_VOLTAGE_OFF, Y_VOLTAGE_OUT3V3 and Y_VOLTAGE_OUT5V corresponding to
     * the voltage on the power output provided by the
     *         module
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_voltage(Y_VOLTAGE_enum newval);
    inline int      setVoltage(Y_VOLTAGE_enum newval)
    { return this->set_voltage(newval); }

    /**
     * Retrieves a dual power  ouput control for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the power ouput control is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YPowerOutput.isOnline() to test if the power ouput control is
     * indeed online at a given time. In case of ambiguity when looking for
     * a dual power  ouput control by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the power ouput control
     *
     * @return a YPowerOutput object allowing you to drive the power ouput control.
     */
    static YPowerOutput* FindPowerOutput(string func);

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
    virtual int         registerValueCallback(YPowerOutputValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YPowerOutput* Find(string func)
    { return YPowerOutput::FindPowerOutput(func); }

    /**
     * Continues the enumeration of dual power ouput controls started using yFirstPowerOutput().
     *
     * @return a pointer to a YPowerOutput object, corresponding to
     *         a dual power  ouput control currently online, or a NULL pointer
     *         if there are no more dual power ouput controls to enumerate.
     */
           YPowerOutput    *nextPowerOutput(void);
    inline YPowerOutput    *next(void)
    { return this->nextPowerOutput();}

    /**
     * Starts the enumeration of dual power ouput controls currently accessible.
     * Use the method YPowerOutput.nextPowerOutput() to iterate on
     * next dual power ouput controls.
     *
     * @return a pointer to a YPowerOutput object, corresponding to
     *         the first dual power ouput control currently online, or a NULL pointer
     *         if there are none.
     */
           static YPowerOutput* FirstPowerOutput(void);
    inline static YPowerOutput* First(void)
    { return YPowerOutput::FirstPowerOutput();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YPowerOutput accessors declaration)
};

//--- (YPowerOutput functions declaration)

/**
 * Retrieves a dual power  ouput control for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the power ouput control is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YPowerOutput.isOnline() to test if the power ouput control is
 * indeed online at a given time. In case of ambiguity when looking for
 * a dual power  ouput control by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the power ouput control
 *
 * @return a YPowerOutput object allowing you to drive the power ouput control.
 */
inline YPowerOutput* yFindPowerOutput(const string& func)
{ return YPowerOutput::FindPowerOutput(func);}
/**
 * Starts the enumeration of dual power ouput controls currently accessible.
 * Use the method YPowerOutput.nextPowerOutput() to iterate on
 * next dual power ouput controls.
 *
 * @return a pointer to a YPowerOutput object, corresponding to
 *         the first dual power ouput control currently online, or a NULL pointer
 *         if there are none.
 */
inline YPowerOutput* yFirstPowerOutput(void)
{ return YPowerOutput::FirstPowerOutput();}

//--- (end of YPowerOutput functions declaration)

#endif
