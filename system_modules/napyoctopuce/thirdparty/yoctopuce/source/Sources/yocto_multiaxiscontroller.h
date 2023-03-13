/*********************************************************************
 *
 * $Id: yocto_multiaxiscontroller.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindMultiAxisController(), the high-level API for MultiAxisController functions
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


#ifndef YOCTO_MULTIAXISCONTROLLER_H
#define YOCTO_MULTIAXISCONTROLLER_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YMultiAxisController return codes)
//--- (end of YMultiAxisController return codes)
//--- (YMultiAxisController definitions)
class YMultiAxisController; // forward declaration

typedef void (*YMultiAxisControllerValueCallback)(YMultiAxisController *func, const string& functionValue);
#ifndef _Y_GLOBALSTATE_ENUM
#define _Y_GLOBALSTATE_ENUM
typedef enum {
    Y_GLOBALSTATE_ABSENT = 0,
    Y_GLOBALSTATE_ALERT = 1,
    Y_GLOBALSTATE_HI_Z = 2,
    Y_GLOBALSTATE_STOP = 3,
    Y_GLOBALSTATE_RUN = 4,
    Y_GLOBALSTATE_BATCH = 5,
    Y_GLOBALSTATE_INVALID = -1,
} Y_GLOBALSTATE_enum;
#endif
#define Y_NAXIS_INVALID                 (YAPI_INVALID_UINT)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YMultiAxisController definitions)

//--- (YMultiAxisController declaration)
/**
 * YMultiAxisController Class: MultiAxisController function interface
 *
 * The Yoctopuce application programming interface allows you to drive a stepper motor.
 */
class YOCTO_CLASS_EXPORT YMultiAxisController: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YMultiAxisController declaration)
protected:
    //--- (YMultiAxisController attributes)
    // Attributes (function value cache)
    int             _nAxis;
    Y_GLOBALSTATE_enum _globalState;
    string          _command;
    YMultiAxisControllerValueCallback _valueCallbackMultiAxisController;

    friend YMultiAxisController *yFindMultiAxisController(const string& func);
    friend YMultiAxisController *yFirstMultiAxisController(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindMultiAxisController factory function to instantiate
    YMultiAxisController(const string& func);
    //--- (end of YMultiAxisController attributes)

public:
    ~YMultiAxisController();
    //--- (YMultiAxisController accessors declaration)

    static const int NAXIS_INVALID = YAPI_INVALID_UINT;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_ABSENT = Y_GLOBALSTATE_ABSENT;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_ALERT = Y_GLOBALSTATE_ALERT;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_HI_Z = Y_GLOBALSTATE_HI_Z;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_STOP = Y_GLOBALSTATE_STOP;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_RUN = Y_GLOBALSTATE_RUN;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_BATCH = Y_GLOBALSTATE_BATCH;
    static const Y_GLOBALSTATE_enum GLOBALSTATE_INVALID = Y_GLOBALSTATE_INVALID;
    static const string COMMAND_INVALID;

    /**
     * Returns the number of synchronized controllers.
     *
     * @return an integer corresponding to the number of synchronized controllers
     *
     * On failure, throws an exception or returns Y_NAXIS_INVALID.
     */
    int                 get_nAxis(void);

    inline int          nAxis(void)
    { return this->get_nAxis(); }

    /**
     * Changes the number of synchronized controllers.
     *
     * @param newval : an integer corresponding to the number of synchronized controllers
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_nAxis(int newval);
    inline int      setNAxis(int newval)
    { return this->set_nAxis(newval); }

    /**
     * Returns the stepper motor set overall state.
     *
     * @return a value among Y_GLOBALSTATE_ABSENT, Y_GLOBALSTATE_ALERT, Y_GLOBALSTATE_HI_Z,
     * Y_GLOBALSTATE_STOP, Y_GLOBALSTATE_RUN and Y_GLOBALSTATE_BATCH corresponding to the stepper motor
     * set overall state
     *
     * On failure, throws an exception or returns Y_GLOBALSTATE_INVALID.
     */
    Y_GLOBALSTATE_enum  get_globalState(void);

    inline Y_GLOBALSTATE_enum globalState(void)
    { return this->get_globalState(); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a multi-axis controller for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the multi-axis controller is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YMultiAxisController.isOnline() to test if the multi-axis controller is
     * indeed online at a given time. In case of ambiguity when looking for
     * a multi-axis controller by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the multi-axis controller
     *
     * @return a YMultiAxisController object allowing you to drive the multi-axis controller.
     */
    static YMultiAxisController* FindMultiAxisController(string func);

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
    virtual int         registerValueCallback(YMultiAxisControllerValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    virtual int         sendCommand(string command);

    /**
     * Reinitialize all controllers and clear all alert flags.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         reset(void);

    /**
     * Starts all motors backward at the specified speeds, to search for the motor home position.
     *
     * @param speed : desired speed for all axis, in steps per second.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         findHomePosition(vector<double> speed);

    /**
     * Starts all motors synchronously to reach a given absolute position.
     * The time needed to reach the requested position will depend on the lowest
     * acceleration and max speed parameters configured for all motors.
     * The final position will be reached on all axis at the same time.
     *
     * @param absPos : absolute position, measured in steps from each origin.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         moveTo(vector<double> absPos);

    /**
     * Starts all motors synchronously to reach a given relative position.
     * The time needed to reach the requested position will depend on the lowest
     * acceleration and max speed parameters configured for all motors.
     * The final position will be reached on all axis at the same time.
     *
     * @param relPos : relative position, measured in steps from the current position.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         moveRel(vector<double> relPos);

    /**
     * Keep the motor in the same state for the specified amount of time, before processing next command.
     *
     * @param waitMs : wait time, specified in milliseconds.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         pause(int waitMs);

    /**
     * Stops the motor with an emergency alert, without taking any additional precaution.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         emergencyStop(void);

    /**
     * Stops the motor smoothly as soon as possible, without waiting for ongoing move completion.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         abortAndBrake(void);

    /**
     * Turn the controller into Hi-Z mode immediately, without waiting for ongoing move completion.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         abortAndHiZ(void);


    inline static YMultiAxisController* Find(string func)
    { return YMultiAxisController::FindMultiAxisController(func); }

    /**
     * Continues the enumeration of multi-axis controllers started using yFirstMultiAxisController().
     *
     * @return a pointer to a YMultiAxisController object, corresponding to
     *         a multi-axis controller currently online, or a NULL pointer
     *         if there are no more multi-axis controllers to enumerate.
     */
           YMultiAxisController *nextMultiAxisController(void);
    inline YMultiAxisController *next(void)
    { return this->nextMultiAxisController();}

    /**
     * Starts the enumeration of multi-axis controllers currently accessible.
     * Use the method YMultiAxisController.nextMultiAxisController() to iterate on
     * next multi-axis controllers.
     *
     * @return a pointer to a YMultiAxisController object, corresponding to
     *         the first multi-axis controller currently online, or a NULL pointer
     *         if there are none.
     */
           static YMultiAxisController* FirstMultiAxisController(void);
    inline static YMultiAxisController* First(void)
    { return YMultiAxisController::FirstMultiAxisController();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YMultiAxisController accessors declaration)
};

//--- (YMultiAxisController functions declaration)

/**
 * Retrieves a multi-axis controller for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the multi-axis controller is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YMultiAxisController.isOnline() to test if the multi-axis controller is
 * indeed online at a given time. In case of ambiguity when looking for
 * a multi-axis controller by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the multi-axis controller
 *
 * @return a YMultiAxisController object allowing you to drive the multi-axis controller.
 */
inline YMultiAxisController* yFindMultiAxisController(const string& func)
{ return YMultiAxisController::FindMultiAxisController(func);}
/**
 * Starts the enumeration of multi-axis controllers currently accessible.
 * Use the method YMultiAxisController.nextMultiAxisController() to iterate on
 * next multi-axis controllers.
 *
 * @return a pointer to a YMultiAxisController object, corresponding to
 *         the first multi-axis controller currently online, or a NULL pointer
 *         if there are none.
 */
inline YMultiAxisController* yFirstMultiAxisController(void)
{ return YMultiAxisController::FirstMultiAxisController();}

//--- (end of YMultiAxisController functions declaration)

#endif
