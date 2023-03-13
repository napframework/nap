/*********************************************************************
 *
 * $Id: yocto_pwmoutput.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindPwmOutput(), the high-level API for PwmOutput functions
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


#ifndef YOCTO_PWMOUTPUT_H
#define YOCTO_PWMOUTPUT_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YPwmOutput return codes)
//--- (end of YPwmOutput return codes)
//--- (YPwmOutput definitions)
class YPwmOutput; // forward declaration

typedef void (*YPwmOutputValueCallback)(YPwmOutput *func, const string& functionValue);
#ifndef _Y_ENABLED_ENUM
#define _Y_ENABLED_ENUM
typedef enum {
    Y_ENABLED_FALSE = 0,
    Y_ENABLED_TRUE = 1,
    Y_ENABLED_INVALID = -1,
} Y_ENABLED_enum;
#endif
#ifndef _Y_ENABLEDATPOWERON_ENUM
#define _Y_ENABLEDATPOWERON_ENUM
typedef enum {
    Y_ENABLEDATPOWERON_FALSE = 0,
    Y_ENABLEDATPOWERON_TRUE = 1,
    Y_ENABLEDATPOWERON_INVALID = -1,
} Y_ENABLEDATPOWERON_enum;
#endif
#define Y_FREQUENCY_INVALID             (YAPI_INVALID_DOUBLE)
#define Y_PERIOD_INVALID                (YAPI_INVALID_DOUBLE)
#define Y_DUTYCYCLE_INVALID             (YAPI_INVALID_DOUBLE)
#define Y_PULSEDURATION_INVALID         (YAPI_INVALID_DOUBLE)
#define Y_PWMTRANSITION_INVALID         (YAPI_INVALID_STRING)
#define Y_DUTYCYCLEATPOWERON_INVALID    (YAPI_INVALID_DOUBLE)
//--- (end of YPwmOutput definitions)

//--- (YPwmOutput declaration)
/**
 * YPwmOutput Class: PwmOutput function interface
 *
 * The Yoctopuce application programming interface allows you to configure, start, and stop the PWM.
 */
class YOCTO_CLASS_EXPORT YPwmOutput: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YPwmOutput declaration)
protected:
    //--- (YPwmOutput attributes)
    // Attributes (function value cache)
    Y_ENABLED_enum  _enabled;
    double          _frequency;
    double          _period;
    double          _dutyCycle;
    double          _pulseDuration;
    string          _pwmTransition;
    Y_ENABLEDATPOWERON_enum _enabledAtPowerOn;
    double          _dutyCycleAtPowerOn;
    YPwmOutputValueCallback _valueCallbackPwmOutput;

    friend YPwmOutput *yFindPwmOutput(const string& func);
    friend YPwmOutput *yFirstPwmOutput(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindPwmOutput factory function to instantiate
    YPwmOutput(const string& func);
    //--- (end of YPwmOutput attributes)

public:
    ~YPwmOutput();
    //--- (YPwmOutput accessors declaration)

    static const Y_ENABLED_enum ENABLED_FALSE = Y_ENABLED_FALSE;
    static const Y_ENABLED_enum ENABLED_TRUE = Y_ENABLED_TRUE;
    static const Y_ENABLED_enum ENABLED_INVALID = Y_ENABLED_INVALID;
    static const double FREQUENCY_INVALID;
    static const double PERIOD_INVALID;
    static const double DUTYCYCLE_INVALID;
    static const double PULSEDURATION_INVALID;
    static const string PWMTRANSITION_INVALID;
    static const Y_ENABLEDATPOWERON_enum ENABLEDATPOWERON_FALSE = Y_ENABLEDATPOWERON_FALSE;
    static const Y_ENABLEDATPOWERON_enum ENABLEDATPOWERON_TRUE = Y_ENABLEDATPOWERON_TRUE;
    static const Y_ENABLEDATPOWERON_enum ENABLEDATPOWERON_INVALID = Y_ENABLEDATPOWERON_INVALID;
    static const double DUTYCYCLEATPOWERON_INVALID;

    /**
     * Returns the state of the PWMs.
     *
     * @return either Y_ENABLED_FALSE or Y_ENABLED_TRUE, according to the state of the PWMs
     *
     * On failure, throws an exception or returns Y_ENABLED_INVALID.
     */
    Y_ENABLED_enum      get_enabled(void);

    inline Y_ENABLED_enum enabled(void)
    { return this->get_enabled(); }

    /**
     * Stops or starts the PWM.
     *
     * @param newval : either Y_ENABLED_FALSE or Y_ENABLED_TRUE
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_enabled(Y_ENABLED_enum newval);
    inline int      setEnabled(Y_ENABLED_enum newval)
    { return this->set_enabled(newval); }

    /**
     * Changes the PWM frequency. The duty cycle is kept unchanged thanks to an
     * automatic pulse width change.
     *
     * @param newval : a floating point number corresponding to the PWM frequency
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_frequency(double newval);
    inline int      setFrequency(double newval)
    { return this->set_frequency(newval); }

    /**
     * Returns the PWM frequency in Hz.
     *
     * @return a floating point number corresponding to the PWM frequency in Hz
     *
     * On failure, throws an exception or returns Y_FREQUENCY_INVALID.
     */
    double              get_frequency(void);

    inline double       frequency(void)
    { return this->get_frequency(); }

    /**
     * Changes the PWM period in milliseconds.
     *
     * @param newval : a floating point number corresponding to the PWM period in milliseconds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_period(double newval);
    inline int      setPeriod(double newval)
    { return this->set_period(newval); }

    /**
     * Returns the PWM period in milliseconds.
     *
     * @return a floating point number corresponding to the PWM period in milliseconds
     *
     * On failure, throws an exception or returns Y_PERIOD_INVALID.
     */
    double              get_period(void);

    inline double       period(void)
    { return this->get_period(); }

    /**
     * Changes the PWM duty cycle, in per cents.
     *
     * @param newval : a floating point number corresponding to the PWM duty cycle, in per cents
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_dutyCycle(double newval);
    inline int      setDutyCycle(double newval)
    { return this->set_dutyCycle(newval); }

    /**
     * Returns the PWM duty cycle, in per cents.
     *
     * @return a floating point number corresponding to the PWM duty cycle, in per cents
     *
     * On failure, throws an exception or returns Y_DUTYCYCLE_INVALID.
     */
    double              get_dutyCycle(void);

    inline double       dutyCycle(void)
    { return this->get_dutyCycle(); }

    /**
     * Changes the PWM pulse length, in milliseconds. A pulse length cannot be longer than period,
     * otherwise it is truncated.
     *
     * @param newval : a floating point number corresponding to the PWM pulse length, in milliseconds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_pulseDuration(double newval);
    inline int      setPulseDuration(double newval)
    { return this->set_pulseDuration(newval); }

    /**
     * Returns the PWM pulse length in milliseconds, as a floating point number.
     *
     * @return a floating point number corresponding to the PWM pulse length in milliseconds, as a
     * floating point number
     *
     * On failure, throws an exception or returns Y_PULSEDURATION_INVALID.
     */
    double              get_pulseDuration(void);

    inline double       pulseDuration(void)
    { return this->get_pulseDuration(); }

    string              get_pwmTransition(void);

    inline string       pwmTransition(void)
    { return this->get_pwmTransition(); }

    int             set_pwmTransition(const string& newval);
    inline int      setPwmTransition(const string& newval)
    { return this->set_pwmTransition(newval); }

    /**
     * Returns the state of the PWM at device power on.
     *
     * @return either Y_ENABLEDATPOWERON_FALSE or Y_ENABLEDATPOWERON_TRUE, according to the state of the
     * PWM at device power on
     *
     * On failure, throws an exception or returns Y_ENABLEDATPOWERON_INVALID.
     */
    Y_ENABLEDATPOWERON_enum get_enabledAtPowerOn(void);

    inline Y_ENABLEDATPOWERON_enum enabledAtPowerOn(void)
    { return this->get_enabledAtPowerOn(); }

    /**
     * Changes the state of the PWM at device power on. Remember to call the matching module saveToFlash()
     * method, otherwise this call will have no effect.
     *
     * @param newval : either Y_ENABLEDATPOWERON_FALSE or Y_ENABLEDATPOWERON_TRUE, according to the state
     * of the PWM at device power on
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_enabledAtPowerOn(Y_ENABLEDATPOWERON_enum newval);
    inline int      setEnabledAtPowerOn(Y_ENABLEDATPOWERON_enum newval)
    { return this->set_enabledAtPowerOn(newval); }

    /**
     * Changes the PWM duty cycle at device power on. Remember to call the matching
     * module saveToFlash() method, otherwise this call will have no effect.
     *
     * @param newval : a floating point number corresponding to the PWM duty cycle at device power on
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_dutyCycleAtPowerOn(double newval);
    inline int      setDutyCycleAtPowerOn(double newval)
    { return this->set_dutyCycleAtPowerOn(newval); }

    /**
     * Returns the PWMs duty cycle at device power on as a floating point number between 0 and 100.
     *
     * @return a floating point number corresponding to the PWMs duty cycle at device power on as a
     * floating point number between 0 and 100
     *
     * On failure, throws an exception or returns Y_DUTYCYCLEATPOWERON_INVALID.
     */
    double              get_dutyCycleAtPowerOn(void);

    inline double       dutyCycleAtPowerOn(void)
    { return this->get_dutyCycleAtPowerOn(); }

    /**
     * Retrieves a PWM for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the PWM is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YPwmOutput.isOnline() to test if the PWM is
     * indeed online at a given time. In case of ambiguity when looking for
     * a PWM by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the PWM
     *
     * @return a YPwmOutput object allowing you to drive the PWM.
     */
    static YPwmOutput*  FindPwmOutput(string func);

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
    virtual int         registerValueCallback(YPwmOutputValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Performs a smooth transistion of the pulse duration toward a given value. Any period,
     * frequency, duty cycle or pulse width change will cancel any ongoing transition process.
     *
     * @param ms_target   : new pulse duration at the end of the transition
     *         (floating-point number, representing the pulse duration in milliseconds)
     * @param ms_duration : total duration of the transition, in milliseconds
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         pulseDurationMove(double ms_target,int ms_duration);

    /**
     * Performs a smooth change of the pulse duration toward a given value.
     *
     * @param target      : new duty cycle at the end of the transition
     *         (floating-point number, between 0 and 1)
     * @param ms_duration : total duration of the transition, in milliseconds
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         dutyCycleMove(double target,int ms_duration);


    inline static YPwmOutput* Find(string func)
    { return YPwmOutput::FindPwmOutput(func); }

    /**
     * Continues the enumeration of PWMs started using yFirstPwmOutput().
     *
     * @return a pointer to a YPwmOutput object, corresponding to
     *         a PWM currently online, or a NULL pointer
     *         if there are no more PWMs to enumerate.
     */
           YPwmOutput      *nextPwmOutput(void);
    inline YPwmOutput      *next(void)
    { return this->nextPwmOutput();}

    /**
     * Starts the enumeration of PWMs currently accessible.
     * Use the method YPwmOutput.nextPwmOutput() to iterate on
     * next PWMs.
     *
     * @return a pointer to a YPwmOutput object, corresponding to
     *         the first PWM currently online, or a NULL pointer
     *         if there are none.
     */
           static YPwmOutput* FirstPwmOutput(void);
    inline static YPwmOutput* First(void)
    { return YPwmOutput::FirstPwmOutput();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YPwmOutput accessors declaration)
};

//--- (YPwmOutput functions declaration)

/**
 * Retrieves a PWM for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the PWM is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YPwmOutput.isOnline() to test if the PWM is
 * indeed online at a given time. In case of ambiguity when looking for
 * a PWM by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the PWM
 *
 * @return a YPwmOutput object allowing you to drive the PWM.
 */
inline YPwmOutput* yFindPwmOutput(const string& func)
{ return YPwmOutput::FindPwmOutput(func);}
/**
 * Starts the enumeration of PWMs currently accessible.
 * Use the method YPwmOutput.nextPwmOutput() to iterate on
 * next PWMs.
 *
 * @return a pointer to a YPwmOutput object, corresponding to
 *         the first PWM currently online, or a NULL pointer
 *         if there are none.
 */
inline YPwmOutput* yFirstPwmOutput(void)
{ return YPwmOutput::FirstPwmOutput();}

//--- (end of YPwmOutput functions declaration)

#endif
