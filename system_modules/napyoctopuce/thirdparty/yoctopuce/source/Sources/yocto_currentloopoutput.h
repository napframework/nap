/*********************************************************************
 *
 * $Id: yocto_currentloopoutput.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindCurrentLoopOutput(), the high-level API for CurrentLoopOutput functions
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


#ifndef YOCTO_CURRENTLOOPOUTPUT_H
#define YOCTO_CURRENTLOOPOUTPUT_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YCurrentLoopOutput return codes)
//--- (end of YCurrentLoopOutput return codes)
//--- (YCurrentLoopOutput definitions)
class YCurrentLoopOutput; // forward declaration

typedef void (*YCurrentLoopOutputValueCallback)(YCurrentLoopOutput *func, const string& functionValue);
#ifndef _Y_LOOPPOWER_ENUM
#define _Y_LOOPPOWER_ENUM
typedef enum {
    Y_LOOPPOWER_NOPWR = 0,
    Y_LOOPPOWER_LOWPWR = 1,
    Y_LOOPPOWER_POWEROK = 2,
    Y_LOOPPOWER_INVALID = -1,
} Y_LOOPPOWER_enum;
#endif
#define Y_CURRENT_INVALID               (YAPI_INVALID_DOUBLE)
#define Y_CURRENTTRANSITION_INVALID     (YAPI_INVALID_STRING)
#define Y_CURRENTATSTARTUP_INVALID      (YAPI_INVALID_DOUBLE)
//--- (end of YCurrentLoopOutput definitions)

//--- (YCurrentLoopOutput declaration)
/**
 * YCurrentLoopOutput Class: CurrentLoopOutput function interface
 *
 * The Yoctopuce application programming interface allows you to change the value of the 4-20mA
 * output as well as to know the current loop state.
 */
class YOCTO_CLASS_EXPORT YCurrentLoopOutput: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YCurrentLoopOutput declaration)
protected:
    //--- (YCurrentLoopOutput attributes)
    // Attributes (function value cache)
    double          _current;
    string          _currentTransition;
    double          _currentAtStartUp;
    Y_LOOPPOWER_enum _loopPower;
    YCurrentLoopOutputValueCallback _valueCallbackCurrentLoopOutput;

    friend YCurrentLoopOutput *yFindCurrentLoopOutput(const string& func);
    friend YCurrentLoopOutput *yFirstCurrentLoopOutput(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindCurrentLoopOutput factory function to instantiate
    YCurrentLoopOutput(const string& func);
    //--- (end of YCurrentLoopOutput attributes)

public:
    ~YCurrentLoopOutput();
    //--- (YCurrentLoopOutput accessors declaration)

    static const double CURRENT_INVALID;
    static const string CURRENTTRANSITION_INVALID;
    static const double CURRENTATSTARTUP_INVALID;
    static const Y_LOOPPOWER_enum LOOPPOWER_NOPWR = Y_LOOPPOWER_NOPWR;
    static const Y_LOOPPOWER_enum LOOPPOWER_LOWPWR = Y_LOOPPOWER_LOWPWR;
    static const Y_LOOPPOWER_enum LOOPPOWER_POWEROK = Y_LOOPPOWER_POWEROK;
    static const Y_LOOPPOWER_enum LOOPPOWER_INVALID = Y_LOOPPOWER_INVALID;

    /**
     * Changes the current loop, the valid range is from 3 to 21mA. If the loop is
     * not propely powered, the  target current is not reached and
     * loopPower is set to LOWPWR.
     *
     * @param newval : a floating point number corresponding to the current loop, the valid range is from 3 to 21mA
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_current(double newval);
    inline int      setCurrent(double newval)
    { return this->set_current(newval); }

    /**
     * Returns the loop current set point in mA.
     *
     * @return a floating point number corresponding to the loop current set point in mA
     *
     * On failure, throws an exception or returns Y_CURRENT_INVALID.
     */
    double              get_current(void);

    inline double       current(void)
    { return this->get_current(); }

    string              get_currentTransition(void);

    inline string       currentTransition(void)
    { return this->get_currentTransition(); }

    int             set_currentTransition(const string& newval);
    inline int      setCurrentTransition(const string& newval)
    { return this->set_currentTransition(newval); }

    /**
     * Changes the loop current at device start up. Remember to call the matching
     * module saveToFlash() method, otherwise this call has no effect.
     *
     * @param newval : a floating point number corresponding to the loop current at device start up
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_currentAtStartUp(double newval);
    inline int      setCurrentAtStartUp(double newval)
    { return this->set_currentAtStartUp(newval); }

    /**
     * Returns the current in the loop at device startup, in mA.
     *
     * @return a floating point number corresponding to the current in the loop at device startup, in mA
     *
     * On failure, throws an exception or returns Y_CURRENTATSTARTUP_INVALID.
     */
    double              get_currentAtStartUp(void);

    inline double       currentAtStartUp(void)
    { return this->get_currentAtStartUp(); }

    /**
     * Returns the loop powerstate.  POWEROK: the loop
     * is powered. NOPWR: the loop in not powered. LOWPWR: the loop is not
     * powered enough to maintain the current required (insufficient voltage).
     *
     * @return a value among Y_LOOPPOWER_NOPWR, Y_LOOPPOWER_LOWPWR and Y_LOOPPOWER_POWEROK corresponding
     * to the loop powerstate
     *
     * On failure, throws an exception or returns Y_LOOPPOWER_INVALID.
     */
    Y_LOOPPOWER_enum    get_loopPower(void);

    inline Y_LOOPPOWER_enum loopPower(void)
    { return this->get_loopPower(); }

    /**
     * Retrieves a 4-20mA output for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the 4-20mA output is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YCurrentLoopOutput.isOnline() to test if the 4-20mA output is
     * indeed online at a given time. In case of ambiguity when looking for
     * a 4-20mA output by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the 4-20mA output
     *
     * @return a YCurrentLoopOutput object allowing you to drive the 4-20mA output.
     */
    static YCurrentLoopOutput* FindCurrentLoopOutput(string func);

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
    virtual int         registerValueCallback(YCurrentLoopOutputValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Performs a smooth transistion of current flowing in the loop. Any current explicit
     * change cancels any ongoing transition process.
     *
     * @param mA_target   : new current value at the end of the transition
     *         (floating-point number, representing the end current in mA)
     * @param ms_duration : total duration of the transition, in milliseconds
     *
     * @return YAPI_SUCCESS when the call succeeds.
     */
    virtual int         currentMove(double mA_target,int ms_duration);


    inline static YCurrentLoopOutput* Find(string func)
    { return YCurrentLoopOutput::FindCurrentLoopOutput(func); }

    /**
     * Continues the enumeration of 4-20mA outputs started using yFirstCurrentLoopOutput().
     *
     * @return a pointer to a YCurrentLoopOutput object, corresponding to
     *         a 4-20mA output currently online, or a NULL pointer
     *         if there are no more 4-20mA outputs to enumerate.
     */
           YCurrentLoopOutput *nextCurrentLoopOutput(void);
    inline YCurrentLoopOutput *next(void)
    { return this->nextCurrentLoopOutput();}

    /**
     * Starts the enumeration of 4-20mA outputs currently accessible.
     * Use the method YCurrentLoopOutput.nextCurrentLoopOutput() to iterate on
     * next 4-20mA outputs.
     *
     * @return a pointer to a YCurrentLoopOutput object, corresponding to
     *         the first 4-20mA output currently online, or a NULL pointer
     *         if there are none.
     */
           static YCurrentLoopOutput* FirstCurrentLoopOutput(void);
    inline static YCurrentLoopOutput* First(void)
    { return YCurrentLoopOutput::FirstCurrentLoopOutput();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YCurrentLoopOutput accessors declaration)
};

//--- (YCurrentLoopOutput functions declaration)

/**
 * Retrieves a 4-20mA output for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the 4-20mA output is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCurrentLoopOutput.isOnline() to test if the 4-20mA output is
 * indeed online at a given time. In case of ambiguity when looking for
 * a 4-20mA output by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the 4-20mA output
 *
 * @return a YCurrentLoopOutput object allowing you to drive the 4-20mA output.
 */
inline YCurrentLoopOutput* yFindCurrentLoopOutput(const string& func)
{ return YCurrentLoopOutput::FindCurrentLoopOutput(func);}
/**
 * Starts the enumeration of 4-20mA outputs currently accessible.
 * Use the method YCurrentLoopOutput.nextCurrentLoopOutput() to iterate on
 * next 4-20mA outputs.
 *
 * @return a pointer to a YCurrentLoopOutput object, corresponding to
 *         the first 4-20mA output currently online, or a NULL pointer
 *         if there are none.
 */
inline YCurrentLoopOutput* yFirstCurrentLoopOutput(void)
{ return YCurrentLoopOutput::FirstCurrentLoopOutput();}

//--- (end of YCurrentLoopOutput functions declaration)

#endif
