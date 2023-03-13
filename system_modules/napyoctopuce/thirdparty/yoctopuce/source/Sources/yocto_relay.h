/*********************************************************************
 *
 * $Id: yocto_relay.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindRelay(), the high-level API for Relay functions
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


#ifndef YOCTO_RELAY_H
#define YOCTO_RELAY_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YRelay return codes)
//--- (end of YRelay return codes)
//--- (YRelay definitions)
class YRelay; // forward declaration

typedef void (*YRelayValueCallback)(YRelay *func, const string& functionValue);
#ifndef _Y_STATE_ENUM
#define _Y_STATE_ENUM
typedef enum {
    Y_STATE_A = 0,
    Y_STATE_B = 1,
    Y_STATE_INVALID = -1,
} Y_STATE_enum;
#endif
#ifndef _Y_STATEATPOWERON_ENUM
#define _Y_STATEATPOWERON_ENUM
typedef enum {
    Y_STATEATPOWERON_UNCHANGED = 0,
    Y_STATEATPOWERON_A = 1,
    Y_STATEATPOWERON_B = 2,
    Y_STATEATPOWERON_INVALID = -1,
} Y_STATEATPOWERON_enum;
#endif
#ifndef _Y_OUTPUT_ENUM
#define _Y_OUTPUT_ENUM
typedef enum {
    Y_OUTPUT_OFF = 0,
    Y_OUTPUT_ON = 1,
    Y_OUTPUT_INVALID = -1,
} Y_OUTPUT_enum;
#endif
#ifndef _CLASS_YDELAYEDPULSE
#define _CLASS_YDELAYEDPULSE
class YOCTO_CLASS_EXPORT YDelayedPulse {
public:
    int             target;
    int             ms;
    int             moving;

    YDelayedPulse()
        :target(YAPI_INVALID_INT), ms(YAPI_INVALID_INT), moving(YAPI_INVALID_UINT)
    {}

    bool operator==(const YDelayedPulse& o) const {
         return (target == o.target) && (ms == o.ms) && (moving == o.moving);
    }
};
#endif
#define Y_MAXTIMEONSTATEA_INVALID       (YAPI_INVALID_LONG)
#define Y_MAXTIMEONSTATEB_INVALID       (YAPI_INVALID_LONG)
#define Y_PULSETIMER_INVALID            (YAPI_INVALID_LONG)
#define Y_COUNTDOWN_INVALID             (YAPI_INVALID_LONG)
//--- (end of YRelay definitions)

//--- (YRelay declaration)
/**
 * YRelay Class: Relay function interface
 *
 * The Yoctopuce application programming interface allows you to switch the relay state.
 * This change is not persistent: the relay will automatically return to its idle position
 * whenever power is lost or if the module is restarted.
 * The library can also generate automatically short pulses of determined duration.
 * On devices with two output for each relay (double throw), the two outputs are named A and B,
 * with output A corresponding to the idle position (at power off) and the output B corresponding to the
 * active state. If you prefer the alternate default state, simply switch your cables on the board.
 */
class YOCTO_CLASS_EXPORT YRelay: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YRelay declaration)
protected:
    //--- (YRelay attributes)
    // Attributes (function value cache)
    Y_STATE_enum    _state;
    Y_STATEATPOWERON_enum _stateAtPowerOn;
    s64             _maxTimeOnStateA;
    s64             _maxTimeOnStateB;
    Y_OUTPUT_enum   _output;
    s64             _pulseTimer;
    YDelayedPulse   _delayedPulseTimer;
    s64             _countdown;
    YRelayValueCallback _valueCallbackRelay;

    friend YRelay *yFindRelay(const string& func);
    friend YRelay *yFirstRelay(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindRelay factory function to instantiate
    YRelay(const string& func);
    //--- (end of YRelay attributes)

public:
    ~YRelay();
    //--- (YRelay accessors declaration)

    static const Y_STATE_enum STATE_A = Y_STATE_A;
    static const Y_STATE_enum STATE_B = Y_STATE_B;
    static const Y_STATE_enum STATE_INVALID = Y_STATE_INVALID;
    static const Y_STATEATPOWERON_enum STATEATPOWERON_UNCHANGED = Y_STATEATPOWERON_UNCHANGED;
    static const Y_STATEATPOWERON_enum STATEATPOWERON_A = Y_STATEATPOWERON_A;
    static const Y_STATEATPOWERON_enum STATEATPOWERON_B = Y_STATEATPOWERON_B;
    static const Y_STATEATPOWERON_enum STATEATPOWERON_INVALID = Y_STATEATPOWERON_INVALID;
    static const s64 MAXTIMEONSTATEA_INVALID = YAPI_INVALID_LONG;
    static const s64 MAXTIMEONSTATEB_INVALID = YAPI_INVALID_LONG;
    static const Y_OUTPUT_enum OUTPUT_OFF = Y_OUTPUT_OFF;
    static const Y_OUTPUT_enum OUTPUT_ON = Y_OUTPUT_ON;
    static const Y_OUTPUT_enum OUTPUT_INVALID = Y_OUTPUT_INVALID;
    static const s64 PULSETIMER_INVALID = YAPI_INVALID_LONG;
    static const YDelayedPulse DELAYEDPULSETIMER_INVALID;
    static const s64 COUNTDOWN_INVALID = YAPI_INVALID_LONG;

    /**
     * Returns the state of the relays (A for the idle position, B for the active position).
     *
     * @return either Y_STATE_A or Y_STATE_B, according to the state of the relays (A for the idle
     * position, B for the active position)
     *
     * On failure, throws an exception or returns Y_STATE_INVALID.
     */
    Y_STATE_enum        get_state(void);

    inline Y_STATE_enum state(void)
    { return this->get_state(); }

    /**
     * Changes the state of the relays (A for the idle position, B for the active position).
     *
     * @param newval : either Y_STATE_A or Y_STATE_B, according to the state of the relays (A for the idle
     * position, B for the active position)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_state(Y_STATE_enum newval);
    inline int      setState(Y_STATE_enum newval)
    { return this->set_state(newval); }

    /**
     * Returns the state of the relays at device startup (A for the idle position, B for the active
     * position, UNCHANGED for no change).
     *
     * @return a value among Y_STATEATPOWERON_UNCHANGED, Y_STATEATPOWERON_A and Y_STATEATPOWERON_B
     * corresponding to the state of the relays at device startup (A for the idle position, B for the
     * active position, UNCHANGED for no change)
     *
     * On failure, throws an exception or returns Y_STATEATPOWERON_INVALID.
     */
    Y_STATEATPOWERON_enum get_stateAtPowerOn(void);

    inline Y_STATEATPOWERON_enum stateAtPowerOn(void)
    { return this->get_stateAtPowerOn(); }

    /**
     * Preset the state of the relays at device startup (A for the idle position,
     * B for the active position, UNCHANGED for no modification). Remember to call the matching module saveToFlash()
     * method, otherwise this call will have no effect.
     *
     * @param newval : a value among Y_STATEATPOWERON_UNCHANGED, Y_STATEATPOWERON_A and Y_STATEATPOWERON_B
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_stateAtPowerOn(Y_STATEATPOWERON_enum newval);
    inline int      setStateAtPowerOn(Y_STATEATPOWERON_enum newval)
    { return this->set_stateAtPowerOn(newval); }

    /**
     * Retourne the maximum time (ms) allowed for $THEFUNCTIONS$ to stay in state A before automatically
     * switching back in to B state. Zero means no maximum time.
     *
     * @return an integer
     *
     * On failure, throws an exception or returns Y_MAXTIMEONSTATEA_INVALID.
     */
    s64                 get_maxTimeOnStateA(void);

    inline s64          maxTimeOnStateA(void)
    { return this->get_maxTimeOnStateA(); }

    /**
     * Sets the maximum time (ms) allowed for $THEFUNCTIONS$ to stay in state A before automatically
     * switching back in to B state. Use zero for no maximum time.
     *
     * @param newval : an integer
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_maxTimeOnStateA(s64 newval);
    inline int      setMaxTimeOnStateA(s64 newval)
    { return this->set_maxTimeOnStateA(newval); }

    /**
     * Retourne the maximum time (ms) allowed for $THEFUNCTIONS$ to stay in state B before automatically
     * switching back in to A state. Zero means no maximum time.
     *
     * @return an integer
     *
     * On failure, throws an exception or returns Y_MAXTIMEONSTATEB_INVALID.
     */
    s64                 get_maxTimeOnStateB(void);

    inline s64          maxTimeOnStateB(void)
    { return this->get_maxTimeOnStateB(); }

    /**
     * Sets the maximum time (ms) allowed for $THEFUNCTIONS$ to stay in state B before automatically
     * switching back in to A state. Use zero for no maximum time.
     *
     * @param newval : an integer
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_maxTimeOnStateB(s64 newval);
    inline int      setMaxTimeOnStateB(s64 newval)
    { return this->set_maxTimeOnStateB(newval); }

    /**
     * Returns the output state of the relays, when used as a simple switch (single throw).
     *
     * @return either Y_OUTPUT_OFF or Y_OUTPUT_ON, according to the output state of the relays, when used
     * as a simple switch (single throw)
     *
     * On failure, throws an exception or returns Y_OUTPUT_INVALID.
     */
    Y_OUTPUT_enum       get_output(void);

    inline Y_OUTPUT_enum output(void)
    { return this->get_output(); }

    /**
     * Changes the output state of the relays, when used as a simple switch (single throw).
     *
     * @param newval : either Y_OUTPUT_OFF or Y_OUTPUT_ON, according to the output state of the relays,
     * when used as a simple switch (single throw)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_output(Y_OUTPUT_enum newval);
    inline int      setOutput(Y_OUTPUT_enum newval)
    { return this->set_output(newval); }

    /**
     * Returns the number of milliseconds remaining before the relays is returned to idle position
     * (state A), during a measured pulse generation. When there is no ongoing pulse, returns zero.
     *
     * @return an integer corresponding to the number of milliseconds remaining before the relays is
     * returned to idle position
     *         (state A), during a measured pulse generation
     *
     * On failure, throws an exception or returns Y_PULSETIMER_INVALID.
     */
    s64                 get_pulseTimer(void);

    inline s64          pulseTimer(void)
    { return this->get_pulseTimer(); }

    int             set_pulseTimer(s64 newval);
    inline int      setPulseTimer(s64 newval)
    { return this->set_pulseTimer(newval); }

    /**
     * Sets the relay to output B (active) for a specified duration, then brings it
     * automatically back to output A (idle state).
     *
     * @param ms_duration : pulse duration, in millisecondes
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             pulse(int ms_duration);

    YDelayedPulse       get_delayedPulseTimer(void);

    inline YDelayedPulse delayedPulseTimer(void)
    { return this->get_delayedPulseTimer(); }

    int             set_delayedPulseTimer(YDelayedPulse newval);
    inline int      setDelayedPulseTimer(YDelayedPulse newval)
    { return this->set_delayedPulseTimer(newval); }

    /**
     * Schedules a pulse.
     *
     * @param ms_delay : waiting time before the pulse, in millisecondes
     * @param ms_duration : pulse duration, in millisecondes
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             delayedPulse(int ms_delay,int ms_duration);

    /**
     * Returns the number of milliseconds remaining before a pulse (delayedPulse() call)
     * When there is no scheduled pulse, returns zero.
     *
     * @return an integer corresponding to the number of milliseconds remaining before a pulse (delayedPulse() call)
     *         When there is no scheduled pulse, returns zero
     *
     * On failure, throws an exception or returns Y_COUNTDOWN_INVALID.
     */
    s64                 get_countdown(void);

    inline s64          countdown(void)
    { return this->get_countdown(); }

    /**
     * Retrieves a relay for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the relay is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YRelay.isOnline() to test if the relay is
     * indeed online at a given time. In case of ambiguity when looking for
     * a relay by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the relay
     *
     * @return a YRelay object allowing you to drive the relay.
     */
    static YRelay*      FindRelay(string func);

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
    virtual int         registerValueCallback(YRelayValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YRelay* Find(string func)
    { return YRelay::FindRelay(func); }

    /**
     * Continues the enumeration of relays started using yFirstRelay().
     *
     * @return a pointer to a YRelay object, corresponding to
     *         a relay currently online, or a NULL pointer
     *         if there are no more relays to enumerate.
     */
           YRelay          *nextRelay(void);
    inline YRelay          *next(void)
    { return this->nextRelay();}

    /**
     * Starts the enumeration of relays currently accessible.
     * Use the method YRelay.nextRelay() to iterate on
     * next relays.
     *
     * @return a pointer to a YRelay object, corresponding to
     *         the first relay currently online, or a NULL pointer
     *         if there are none.
     */
           static YRelay* FirstRelay(void);
    inline static YRelay* First(void)
    { return YRelay::FirstRelay();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YRelay accessors declaration)
};

//--- (YRelay functions declaration)

/**
 * Retrieves a relay for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the relay is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YRelay.isOnline() to test if the relay is
 * indeed online at a given time. In case of ambiguity when looking for
 * a relay by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the relay
 *
 * @return a YRelay object allowing you to drive the relay.
 */
inline YRelay* yFindRelay(const string& func)
{ return YRelay::FindRelay(func);}
/**
 * Starts the enumeration of relays currently accessible.
 * Use the method YRelay.nextRelay() to iterate on
 * next relays.
 *
 * @return a pointer to a YRelay object, corresponding to
 *         the first relay currently online, or a NULL pointer
 *         if there are none.
 */
inline YRelay* yFirstRelay(void)
{ return YRelay::FirstRelay();}

//--- (end of YRelay functions declaration)

#endif
