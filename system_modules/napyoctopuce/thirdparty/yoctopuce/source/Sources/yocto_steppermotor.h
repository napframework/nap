/*********************************************************************
 *
 * $Id: yocto_steppermotor.h 29495 2017-12-22 16:45:20Z mvuilleu $
 *
 * Declares yFindStepperMotor(), the high-level API for StepperMotor functions
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


#ifndef YOCTO_STEPPERMOTOR_H
#define YOCTO_STEPPERMOTOR_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YStepperMotor return codes)
//--- (end of YStepperMotor return codes)
//--- (YStepperMotor definitions)
class YStepperMotor; // forward declaration

typedef void (*YStepperMotorValueCallback)(YStepperMotor *func, const string& functionValue);
#ifndef _Y_MOTORSTATE_ENUM
#define _Y_MOTORSTATE_ENUM
typedef enum {
    Y_MOTORSTATE_ABSENT = 0,
    Y_MOTORSTATE_ALERT = 1,
    Y_MOTORSTATE_HI_Z = 2,
    Y_MOTORSTATE_STOP = 3,
    Y_MOTORSTATE_RUN = 4,
    Y_MOTORSTATE_BATCH = 5,
    Y_MOTORSTATE_INVALID = -1,
} Y_MOTORSTATE_enum;
#endif
#ifndef _Y_STEPPING_ENUM
#define _Y_STEPPING_ENUM
typedef enum {
    Y_STEPPING_MICROSTEP16 = 0,
    Y_STEPPING_MICROSTEP8 = 1,
    Y_STEPPING_MICROSTEP4 = 2,
    Y_STEPPING_HALFSTEP = 3,
    Y_STEPPING_FULLSTEP = 4,
    Y_STEPPING_INVALID = -1,
} Y_STEPPING_enum;
#endif
#define Y_DIAGS_INVALID                 (YAPI_INVALID_UINT)
#define Y_STEPPOS_INVALID               (YAPI_INVALID_DOUBLE)
#define Y_SPEED_INVALID                 (YAPI_INVALID_DOUBLE)
#define Y_PULLINSPEED_INVALID           (YAPI_INVALID_DOUBLE)
#define Y_MAXACCEL_INVALID              (YAPI_INVALID_DOUBLE)
#define Y_MAXSPEED_INVALID              (YAPI_INVALID_DOUBLE)
#define Y_OVERCURRENT_INVALID           (YAPI_INVALID_UINT)
#define Y_TCURRSTOP_INVALID             (YAPI_INVALID_UINT)
#define Y_TCURRRUN_INVALID              (YAPI_INVALID_UINT)
#define Y_ALERTMODE_INVALID             (YAPI_INVALID_STRING)
#define Y_AUXMODE_INVALID               (YAPI_INVALID_STRING)
#define Y_AUXSIGNAL_INVALID             (YAPI_INVALID_INT)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YStepperMotor definitions)

//--- (YStepperMotor declaration)
/**
 * YStepperMotor Class: StepperMotor function interface
 *
 * The Yoctopuce application programming interface allows you to drive a stepper motor.
 */
class YOCTO_CLASS_EXPORT YStepperMotor: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YStepperMotor declaration)
protected:
    //--- (YStepperMotor attributes)
    // Attributes (function value cache)
    Y_MOTORSTATE_enum _motorState;
    int             _diags;
    double          _stepPos;
    double          _speed;
    double          _pullinSpeed;
    double          _maxAccel;
    double          _maxSpeed;
    Y_STEPPING_enum _stepping;
    int             _overcurrent;
    int             _tCurrStop;
    int             _tCurrRun;
    string          _alertMode;
    string          _auxMode;
    int             _auxSignal;
    string          _command;
    YStepperMotorValueCallback _valueCallbackStepperMotor;

    friend YStepperMotor *yFindStepperMotor(const string& func);
    friend YStepperMotor *yFirstStepperMotor(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindStepperMotor factory function to instantiate
    YStepperMotor(const string& func);
    //--- (end of YStepperMotor attributes)

public:
    ~YStepperMotor();
    //--- (YStepperMotor accessors declaration)

    static const Y_MOTORSTATE_enum MOTORSTATE_ABSENT = Y_MOTORSTATE_ABSENT;
    static const Y_MOTORSTATE_enum MOTORSTATE_ALERT = Y_MOTORSTATE_ALERT;
    static const Y_MOTORSTATE_enum MOTORSTATE_HI_Z = Y_MOTORSTATE_HI_Z;
    static const Y_MOTORSTATE_enum MOTORSTATE_STOP = Y_MOTORSTATE_STOP;
    static const Y_MOTORSTATE_enum MOTORSTATE_RUN = Y_MOTORSTATE_RUN;
    static const Y_MOTORSTATE_enum MOTORSTATE_BATCH = Y_MOTORSTATE_BATCH;
    static const Y_MOTORSTATE_enum MOTORSTATE_INVALID = Y_MOTORSTATE_INVALID;
    static const int DIAGS_INVALID = YAPI_INVALID_UINT;
    static const double STEPPOS_INVALID;
    static const double SPEED_INVALID;
    static const double PULLINSPEED_INVALID;
    static const double MAXACCEL_INVALID;
    static const double MAXSPEED_INVALID;
    static const Y_STEPPING_enum STEPPING_MICROSTEP16 = Y_STEPPING_MICROSTEP16;
    static const Y_STEPPING_enum STEPPING_MICROSTEP8 = Y_STEPPING_MICROSTEP8;
    static const Y_STEPPING_enum STEPPING_MICROSTEP4 = Y_STEPPING_MICROSTEP4;
    static const Y_STEPPING_enum STEPPING_HALFSTEP = Y_STEPPING_HALFSTEP;
    static const Y_STEPPING_enum STEPPING_FULLSTEP = Y_STEPPING_FULLSTEP;
    static const Y_STEPPING_enum STEPPING_INVALID = Y_STEPPING_INVALID;
    static const int OVERCURRENT_INVALID = YAPI_INVALID_UINT;
    static const int TCURRSTOP_INVALID = YAPI_INVALID_UINT;
    static const int TCURRRUN_INVALID = YAPI_INVALID_UINT;
    static const string ALERTMODE_INVALID;
    static const string AUXMODE_INVALID;
    static const int AUXSIGNAL_INVALID = YAPI_INVALID_INT;
    static const string COMMAND_INVALID;

    /**
     * Returns the motor working state.
     *
     * @return a value among Y_MOTORSTATE_ABSENT, Y_MOTORSTATE_ALERT, Y_MOTORSTATE_HI_Z,
     * Y_MOTORSTATE_STOP, Y_MOTORSTATE_RUN and Y_MOTORSTATE_BATCH corresponding to the motor working state
     *
     * On failure, throws an exception or returns Y_MOTORSTATE_INVALID.
     */
    Y_MOTORSTATE_enum   get_motorState(void);

    inline Y_MOTORSTATE_enum motorState(void)
    { return this->get_motorState(); }

    /**
     * Returns the stepper motor controller diagnostics, as a bitmap.
     *
     * @return an integer corresponding to the stepper motor controller diagnostics, as a bitmap
     *
     * On failure, throws an exception or returns Y_DIAGS_INVALID.
     */
    int                 get_diags(void);

    inline int          diags(void)
    { return this->get_diags(); }

    /**
     * Changes the current logical motor position, measured in steps.
     * This command does not cause any motor move, as its purpose is only to setup
     * the origin of the position counter. The fractional part of the position,
     * that corresponds to the physical position of the rotor, is not changed.
     * To trigger a motor move, use methods moveTo() or moveRel()
     * instead.
     *
     * @param newval : a floating point number corresponding to the current logical motor position, measured in steps
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_stepPos(double newval);
    inline int      setStepPos(double newval)
    { return this->set_stepPos(newval); }

    /**
     * Returns the current logical motor position, measured in steps.
     * The value may include a fractional part when micro-stepping is in use.
     *
     * @return a floating point number corresponding to the current logical motor position, measured in steps
     *
     * On failure, throws an exception or returns Y_STEPPOS_INVALID.
     */
    double              get_stepPos(void);

    inline double       stepPos(void)
    { return this->get_stepPos(); }

    /**
     * Returns current motor speed, measured in steps per second.
     * To change speed, use method changeSpeed().
     *
     * @return a floating point number corresponding to current motor speed, measured in steps per second
     *
     * On failure, throws an exception or returns Y_SPEED_INVALID.
     */
    double              get_speed(void);

    inline double       speed(void)
    { return this->get_speed(); }

    /**
     * Changes the motor speed immediately reachable from stop state, measured in steps per second.
     *
     * @param newval : a floating point number corresponding to the motor speed immediately reachable from
     * stop state, measured in steps per second
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_pullinSpeed(double newval);
    inline int      setPullinSpeed(double newval)
    { return this->set_pullinSpeed(newval); }

    /**
     * Returns the motor speed immediately reachable from stop state, measured in steps per second.
     *
     * @return a floating point number corresponding to the motor speed immediately reachable from stop
     * state, measured in steps per second
     *
     * On failure, throws an exception or returns Y_PULLINSPEED_INVALID.
     */
    double              get_pullinSpeed(void);

    inline double       pullinSpeed(void)
    { return this->get_pullinSpeed(); }

    /**
     * Changes the maximal motor acceleration, measured in steps per second^2.
     *
     * @param newval : a floating point number corresponding to the maximal motor acceleration, measured
     * in steps per second^2
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_maxAccel(double newval);
    inline int      setMaxAccel(double newval)
    { return this->set_maxAccel(newval); }

    /**
     * Returns the maximal motor acceleration, measured in steps per second^2.
     *
     * @return a floating point number corresponding to the maximal motor acceleration, measured in steps per second^2
     *
     * On failure, throws an exception or returns Y_MAXACCEL_INVALID.
     */
    double              get_maxAccel(void);

    inline double       maxAccel(void)
    { return this->get_maxAccel(); }

    /**
     * Changes the maximal motor speed, measured in steps per second.
     *
     * @param newval : a floating point number corresponding to the maximal motor speed, measured in steps per second
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_maxSpeed(double newval);
    inline int      setMaxSpeed(double newval)
    { return this->set_maxSpeed(newval); }

    /**
     * Returns the maximal motor speed, measured in steps per second.
     *
     * @return a floating point number corresponding to the maximal motor speed, measured in steps per second
     *
     * On failure, throws an exception or returns Y_MAXSPEED_INVALID.
     */
    double              get_maxSpeed(void);

    inline double       maxSpeed(void)
    { return this->get_maxSpeed(); }

    /**
     * Returns the stepping mode used to drive the motor.
     *
     * @return a value among Y_STEPPING_MICROSTEP16, Y_STEPPING_MICROSTEP8, Y_STEPPING_MICROSTEP4,
     * Y_STEPPING_HALFSTEP and Y_STEPPING_FULLSTEP corresponding to the stepping mode used to drive the motor
     *
     * On failure, throws an exception or returns Y_STEPPING_INVALID.
     */
    Y_STEPPING_enum     get_stepping(void);

    inline Y_STEPPING_enum stepping(void)
    { return this->get_stepping(); }

    /**
     * Changes the stepping mode used to drive the motor.
     *
     * @param newval : a value among Y_STEPPING_MICROSTEP16, Y_STEPPING_MICROSTEP8, Y_STEPPING_MICROSTEP4,
     * Y_STEPPING_HALFSTEP and Y_STEPPING_FULLSTEP corresponding to the stepping mode used to drive the motor
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_stepping(Y_STEPPING_enum newval);
    inline int      setStepping(Y_STEPPING_enum newval)
    { return this->set_stepping(newval); }

    /**
     * Returns the overcurrent alert and emergency stop threshold, measured in mA.
     *
     * @return an integer corresponding to the overcurrent alert and emergency stop threshold, measured in mA
     *
     * On failure, throws an exception or returns Y_OVERCURRENT_INVALID.
     */
    int                 get_overcurrent(void);

    inline int          overcurrent(void)
    { return this->get_overcurrent(); }

    /**
     * Changes the overcurrent alert and emergency stop threshold, measured in mA.
     *
     * @param newval : an integer corresponding to the overcurrent alert and emergency stop threshold, measured in mA
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_overcurrent(int newval);
    inline int      setOvercurrent(int newval)
    { return this->set_overcurrent(newval); }

    /**
     * Returns the torque regulation current when the motor is stopped, measured in mA.
     *
     * @return an integer corresponding to the torque regulation current when the motor is stopped, measured in mA
     *
     * On failure, throws an exception or returns Y_TCURRSTOP_INVALID.
     */
    int                 get_tCurrStop(void);

    inline int          tCurrStop(void)
    { return this->get_tCurrStop(); }

    /**
     * Changes the torque regulation current when the motor is stopped, measured in mA.
     *
     * @param newval : an integer corresponding to the torque regulation current when the motor is
     * stopped, measured in mA
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_tCurrStop(int newval);
    inline int      setTCurrStop(int newval)
    { return this->set_tCurrStop(newval); }

    /**
     * Returns the torque regulation current when the motor is running, measured in mA.
     *
     * @return an integer corresponding to the torque regulation current when the motor is running, measured in mA
     *
     * On failure, throws an exception or returns Y_TCURRRUN_INVALID.
     */
    int                 get_tCurrRun(void);

    inline int          tCurrRun(void)
    { return this->get_tCurrRun(); }

    /**
     * Changes the torque regulation current when the motor is running, measured in mA.
     *
     * @param newval : an integer corresponding to the torque regulation current when the motor is
     * running, measured in mA
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_tCurrRun(int newval);
    inline int      setTCurrRun(int newval)
    { return this->set_tCurrRun(newval); }

    string              get_alertMode(void);

    inline string       alertMode(void)
    { return this->get_alertMode(); }

    int             set_alertMode(const string& newval);
    inline int      setAlertMode(const string& newval)
    { return this->set_alertMode(newval); }

    string              get_auxMode(void);

    inline string       auxMode(void)
    { return this->get_auxMode(); }

    int             set_auxMode(const string& newval);
    inline int      setAuxMode(const string& newval)
    { return this->set_auxMode(newval); }

    /**
     * Returns the current value of the signal generated on the auxiliary output.
     *
     * @return an integer corresponding to the current value of the signal generated on the auxiliary output
     *
     * On failure, throws an exception or returns Y_AUXSIGNAL_INVALID.
     */
    int                 get_auxSignal(void);

    inline int          auxSignal(void)
    { return this->get_auxSignal(); }

    /**
     * Changes the value of the signal generated on the auxiliary output.
     * Acceptable values depend on the auxiliary output signal type configured.
     *
     * @param newval : an integer corresponding to the value of the signal generated on the auxiliary output
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_auxSignal(int newval);
    inline int      setAuxSignal(int newval)
    { return this->set_auxSignal(newval); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a stepper motor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the stepper motor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YStepperMotor.isOnline() to test if the stepper motor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a stepper motor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the stepper motor
     *
     * @return a YStepperMotor object allowing you to drive the stepper motor.
     */
    static YStepperMotor* FindStepperMotor(string func);

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
    virtual int         registerValueCallback(YStepperMotorValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    virtual int         sendCommand(string command);

    /**
     * Reinitialize the controller and clear all alert flags.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         reset(void);

    /**
     * Starts the motor backward at the specified speed, to search for the motor home position.
     *
     * @param speed : desired speed, in steps per second.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         findHomePosition(double speed);

    /**
     * Starts the motor at a given speed. The time needed to reach the requested speed
     * will depend on the acceleration parameters configured for the motor.
     *
     * @param speed : desired speed, in steps per second. The minimal non-zero speed
     *         is 0.001 pulse per second.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         changeSpeed(double speed);

    /**
     * Starts the motor to reach a given absolute position. The time needed to reach the requested
     * position will depend on the acceleration and max speed parameters configured for
     * the motor.
     *
     * @param absPos : absolute position, measured in steps from the origin.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         moveTo(double absPos);

    /**
     * Starts the motor to reach a given relative position. The time needed to reach the requested
     * position will depend on the acceleration and max speed parameters configured for
     * the motor.
     *
     * @param relPos : relative position, measured in steps from the current position.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         moveRel(double relPos);

    /**
     * Starts the motor to reach a given relative position, keeping the speed under the
     * specified limit. The time needed to reach the requested position will depend on
     * the acceleration parameters configured for the motor.
     *
     * @param relPos : relative position, measured in steps from the current position.
     * @param maxSpeed : limit speed, in steps per second.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         moveRelSlow(double relPos,double maxSpeed);

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
     * Move one step in the direction opposite the direction set when the most recent alert was raised.
     * The move occures even if the system is still in alert mode (end switch depressed). Caution.
     * use this function with great care as it may cause mechanical damages !
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         alertStepOut(void);

    /**
     * Move one single step in the selected direction without regards to end switches.
     * The move occures even if the system is still in alert mode (end switch depressed). Caution.
     * use this function with great care as it may cause mechanical damages !
     *
     * @param dir : Value +1 ou -1, according to the desired direction of the move
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         alertStepDir(int dir);

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


    inline static YStepperMotor* Find(string func)
    { return YStepperMotor::FindStepperMotor(func); }

    /**
     * Continues the enumeration of stepper motors started using yFirstStepperMotor().
     *
     * @return a pointer to a YStepperMotor object, corresponding to
     *         a stepper motor currently online, or a NULL pointer
     *         if there are no more stepper motors to enumerate.
     */
           YStepperMotor   *nextStepperMotor(void);
    inline YStepperMotor   *next(void)
    { return this->nextStepperMotor();}

    /**
     * Starts the enumeration of stepper motors currently accessible.
     * Use the method YStepperMotor.nextStepperMotor() to iterate on
     * next stepper motors.
     *
     * @return a pointer to a YStepperMotor object, corresponding to
     *         the first stepper motor currently online, or a NULL pointer
     *         if there are none.
     */
           static YStepperMotor* FirstStepperMotor(void);
    inline static YStepperMotor* First(void)
    { return YStepperMotor::FirstStepperMotor();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YStepperMotor accessors declaration)
};

//--- (YStepperMotor functions declaration)

/**
 * Retrieves a stepper motor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the stepper motor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YStepperMotor.isOnline() to test if the stepper motor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a stepper motor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the stepper motor
 *
 * @return a YStepperMotor object allowing you to drive the stepper motor.
 */
inline YStepperMotor* yFindStepperMotor(const string& func)
{ return YStepperMotor::FindStepperMotor(func);}
/**
 * Starts the enumeration of stepper motors currently accessible.
 * Use the method YStepperMotor.nextStepperMotor() to iterate on
 * next stepper motors.
 *
 * @return a pointer to a YStepperMotor object, corresponding to
 *         the first stepper motor currently online, or a NULL pointer
 *         if there are none.
 */
inline YStepperMotor* yFirstStepperMotor(void)
{ return YStepperMotor::FirstStepperMotor();}

//--- (end of YStepperMotor functions declaration)

#endif
