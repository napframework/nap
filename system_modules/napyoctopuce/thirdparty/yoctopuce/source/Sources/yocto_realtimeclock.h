/*********************************************************************
 *
 * $Id: yocto_realtimeclock.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindRealTimeClock(), the high-level API for RealTimeClock functions
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


#ifndef YOCTO_REALTIMECLOCK_H
#define YOCTO_REALTIMECLOCK_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YRealTimeClock return codes)
//--- (end of YRealTimeClock return codes)
//--- (YRealTimeClock definitions)
class YRealTimeClock; // forward declaration

typedef void (*YRealTimeClockValueCallback)(YRealTimeClock *func, const string& functionValue);
#ifndef _Y_TIMESET_ENUM
#define _Y_TIMESET_ENUM
typedef enum {
    Y_TIMESET_FALSE = 0,
    Y_TIMESET_TRUE = 1,
    Y_TIMESET_INVALID = -1,
} Y_TIMESET_enum;
#endif
#define Y_UNIXTIME_INVALID              (YAPI_INVALID_LONG)
#define Y_DATETIME_INVALID              (YAPI_INVALID_STRING)
#define Y_UTCOFFSET_INVALID             (YAPI_INVALID_INT)
//--- (end of YRealTimeClock definitions)

//--- (YRealTimeClock declaration)
/**
 * YRealTimeClock Class: Real Time Clock function interface
 *
 * The RealTimeClock function maintains and provides current date and time, even accross power cut
 * lasting several days. It is the base for automated wake-up functions provided by the WakeUpScheduler.
 * The current time may represent a local time as well as an UTC time, but no automatic time change
 * will occur to account for daylight saving time.
 */
class YOCTO_CLASS_EXPORT YRealTimeClock: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YRealTimeClock declaration)
protected:
    //--- (YRealTimeClock attributes)
    // Attributes (function value cache)
    s64             _unixTime;
    string          _dateTime;
    int             _utcOffset;
    Y_TIMESET_enum  _timeSet;
    YRealTimeClockValueCallback _valueCallbackRealTimeClock;

    friend YRealTimeClock *yFindRealTimeClock(const string& func);
    friend YRealTimeClock *yFirstRealTimeClock(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindRealTimeClock factory function to instantiate
    YRealTimeClock(const string& func);
    //--- (end of YRealTimeClock attributes)

public:
    ~YRealTimeClock();
    //--- (YRealTimeClock accessors declaration)

    static const s64 UNIXTIME_INVALID = YAPI_INVALID_LONG;
    static const string DATETIME_INVALID;
    static const int UTCOFFSET_INVALID = YAPI_INVALID_INT;
    static const Y_TIMESET_enum TIMESET_FALSE = Y_TIMESET_FALSE;
    static const Y_TIMESET_enum TIMESET_TRUE = Y_TIMESET_TRUE;
    static const Y_TIMESET_enum TIMESET_INVALID = Y_TIMESET_INVALID;

    /**
     * Returns the current time in Unix format (number of elapsed seconds since Jan 1st, 1970).
     *
     * @return an integer corresponding to the current time in Unix format (number of elapsed seconds
     * since Jan 1st, 1970)
     *
     * On failure, throws an exception or returns Y_UNIXTIME_INVALID.
     */
    s64                 get_unixTime(void);

    inline s64          unixTime(void)
    { return this->get_unixTime(); }

    /**
     * Changes the current time. Time is specifid in Unix format (number of elapsed seconds since Jan 1st, 1970).
     *
     * @param newval : an integer corresponding to the current time
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_unixTime(s64 newval);
    inline int      setUnixTime(s64 newval)
    { return this->set_unixTime(newval); }

    /**
     * Returns the current time in the form "YYYY/MM/DD hh:mm:ss".
     *
     * @return a string corresponding to the current time in the form "YYYY/MM/DD hh:mm:ss"
     *
     * On failure, throws an exception or returns Y_DATETIME_INVALID.
     */
    string              get_dateTime(void);

    inline string       dateTime(void)
    { return this->get_dateTime(); }

    /**
     * Returns the number of seconds between current time and UTC time (time zone).
     *
     * @return an integer corresponding to the number of seconds between current time and UTC time (time zone)
     *
     * On failure, throws an exception or returns Y_UTCOFFSET_INVALID.
     */
    int                 get_utcOffset(void);

    inline int          utcOffset(void)
    { return this->get_utcOffset(); }

    /**
     * Changes the number of seconds between current time and UTC time (time zone).
     * The timezone is automatically rounded to the nearest multiple of 15 minutes.
     *
     * @param newval : an integer corresponding to the number of seconds between current time and UTC time (time zone)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_utcOffset(int newval);
    inline int      setUtcOffset(int newval)
    { return this->set_utcOffset(newval); }

    /**
     * Returns true if the clock has been set, and false otherwise.
     *
     * @return either Y_TIMESET_FALSE or Y_TIMESET_TRUE, according to true if the clock has been set, and
     * false otherwise
     *
     * On failure, throws an exception or returns Y_TIMESET_INVALID.
     */
    Y_TIMESET_enum      get_timeSet(void);

    inline Y_TIMESET_enum timeSet(void)
    { return this->get_timeSet(); }

    /**
     * Retrieves a clock for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the clock is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YRealTimeClock.isOnline() to test if the clock is
     * indeed online at a given time. In case of ambiguity when looking for
     * a clock by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the clock
     *
     * @return a YRealTimeClock object allowing you to drive the clock.
     */
    static YRealTimeClock* FindRealTimeClock(string func);

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
    virtual int         registerValueCallback(YRealTimeClockValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YRealTimeClock* Find(string func)
    { return YRealTimeClock::FindRealTimeClock(func); }

    /**
     * Continues the enumeration of clocks started using yFirstRealTimeClock().
     *
     * @return a pointer to a YRealTimeClock object, corresponding to
     *         a clock currently online, or a NULL pointer
     *         if there are no more clocks to enumerate.
     */
           YRealTimeClock  *nextRealTimeClock(void);
    inline YRealTimeClock  *next(void)
    { return this->nextRealTimeClock();}

    /**
     * Starts the enumeration of clocks currently accessible.
     * Use the method YRealTimeClock.nextRealTimeClock() to iterate on
     * next clocks.
     *
     * @return a pointer to a YRealTimeClock object, corresponding to
     *         the first clock currently online, or a NULL pointer
     *         if there are none.
     */
           static YRealTimeClock* FirstRealTimeClock(void);
    inline static YRealTimeClock* First(void)
    { return YRealTimeClock::FirstRealTimeClock();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YRealTimeClock accessors declaration)
};

//--- (YRealTimeClock functions declaration)

/**
 * Retrieves a clock for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the clock is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YRealTimeClock.isOnline() to test if the clock is
 * indeed online at a given time. In case of ambiguity when looking for
 * a clock by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the clock
 *
 * @return a YRealTimeClock object allowing you to drive the clock.
 */
inline YRealTimeClock* yFindRealTimeClock(const string& func)
{ return YRealTimeClock::FindRealTimeClock(func);}
/**
 * Starts the enumeration of clocks currently accessible.
 * Use the method YRealTimeClock.nextRealTimeClock() to iterate on
 * next clocks.
 *
 * @return a pointer to a YRealTimeClock object, corresponding to
 *         the first clock currently online, or a NULL pointer
 *         if there are none.
 */
inline YRealTimeClock* yFirstRealTimeClock(void)
{ return YRealTimeClock::FirstRealTimeClock();}

//--- (end of YRealTimeClock functions declaration)

#endif
