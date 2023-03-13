/*********************************************************************
 *
 * $Id: yocto_wakeupschedule.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindWakeUpSchedule(), the high-level API for WakeUpSchedule functions
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


#ifndef YOCTO_WAKEUPSCHEDULE_H
#define YOCTO_WAKEUPSCHEDULE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YWakeUpSchedule return codes)
//--- (end of YWakeUpSchedule return codes)
//--- (YWakeUpSchedule definitions)
class YWakeUpSchedule; // forward declaration

typedef void (*YWakeUpScheduleValueCallback)(YWakeUpSchedule *func, const string& functionValue);
#define Y_MINUTESA_INVALID              (YAPI_INVALID_UINT)
#define Y_MINUTESB_INVALID              (YAPI_INVALID_UINT)
#define Y_HOURS_INVALID                 (YAPI_INVALID_UINT)
#define Y_WEEKDAYS_INVALID              (YAPI_INVALID_UINT)
#define Y_MONTHDAYS_INVALID             (YAPI_INVALID_UINT)
#define Y_MONTHS_INVALID                (YAPI_INVALID_UINT)
#define Y_NEXTOCCURENCE_INVALID         (YAPI_INVALID_LONG)
//--- (end of YWakeUpSchedule definitions)

//--- (YWakeUpSchedule declaration)
/**
 * YWakeUpSchedule Class: WakeUpSchedule function interface
 *
 * The WakeUpSchedule function implements a wake up condition. The wake up time is
 * specified as a set of months and/or days and/or hours and/or minutes when the
 * wake up should happen.
 */
class YOCTO_CLASS_EXPORT YWakeUpSchedule: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YWakeUpSchedule declaration)
protected:
    //--- (YWakeUpSchedule attributes)
    // Attributes (function value cache)
    int             _minutesA;
    int             _minutesB;
    int             _hours;
    int             _weekDays;
    int             _monthDays;
    int             _months;
    s64             _nextOccurence;
    YWakeUpScheduleValueCallback _valueCallbackWakeUpSchedule;

    friend YWakeUpSchedule *yFindWakeUpSchedule(const string& func);
    friend YWakeUpSchedule *yFirstWakeUpSchedule(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindWakeUpSchedule factory function to instantiate
    YWakeUpSchedule(const string& func);
    //--- (end of YWakeUpSchedule attributes)

public:
    ~YWakeUpSchedule();
    //--- (YWakeUpSchedule accessors declaration)

    static const int MINUTESA_INVALID = YAPI_INVALID_UINT;
    static const int MINUTESB_INVALID = YAPI_INVALID_UINT;
    static const int HOURS_INVALID = YAPI_INVALID_UINT;
    static const int WEEKDAYS_INVALID = YAPI_INVALID_UINT;
    static const int MONTHDAYS_INVALID = YAPI_INVALID_UINT;
    static const int MONTHS_INVALID = YAPI_INVALID_UINT;
    static const s64 NEXTOCCURENCE_INVALID = YAPI_INVALID_LONG;

    /**
     * Returns the minutes in the 00-29 interval of each hour scheduled for wake up.
     *
     * @return an integer corresponding to the minutes in the 00-29 interval of each hour scheduled for wake up
     *
     * On failure, throws an exception or returns Y_MINUTESA_INVALID.
     */
    int                 get_minutesA(void);

    inline int          minutesA(void)
    { return this->get_minutesA(); }

    /**
     * Changes the minutes in the 00-29 interval when a wake up must take place.
     *
     * @param newval : an integer corresponding to the minutes in the 00-29 interval when a wake up must take place
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_minutesA(int newval);
    inline int      setMinutesA(int newval)
    { return this->set_minutesA(newval); }

    /**
     * Returns the minutes in the 30-59 intervalof each hour scheduled for wake up.
     *
     * @return an integer corresponding to the minutes in the 30-59 intervalof each hour scheduled for wake up
     *
     * On failure, throws an exception or returns Y_MINUTESB_INVALID.
     */
    int                 get_minutesB(void);

    inline int          minutesB(void)
    { return this->get_minutesB(); }

    /**
     * Changes the minutes in the 30-59 interval when a wake up must take place.
     *
     * @param newval : an integer corresponding to the minutes in the 30-59 interval when a wake up must take place
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_minutesB(int newval);
    inline int      setMinutesB(int newval)
    { return this->set_minutesB(newval); }

    /**
     * Returns the hours scheduled for wake up.
     *
     * @return an integer corresponding to the hours scheduled for wake up
     *
     * On failure, throws an exception or returns Y_HOURS_INVALID.
     */
    int                 get_hours(void);

    inline int          hours(void)
    { return this->get_hours(); }

    /**
     * Changes the hours when a wake up must take place.
     *
     * @param newval : an integer corresponding to the hours when a wake up must take place
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_hours(int newval);
    inline int      setHours(int newval)
    { return this->set_hours(newval); }

    /**
     * Returns the days of the week scheduled for wake up.
     *
     * @return an integer corresponding to the days of the week scheduled for wake up
     *
     * On failure, throws an exception or returns Y_WEEKDAYS_INVALID.
     */
    int                 get_weekDays(void);

    inline int          weekDays(void)
    { return this->get_weekDays(); }

    /**
     * Changes the days of the week when a wake up must take place.
     *
     * @param newval : an integer corresponding to the days of the week when a wake up must take place
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_weekDays(int newval);
    inline int      setWeekDays(int newval)
    { return this->set_weekDays(newval); }

    /**
     * Returns the days of the month scheduled for wake up.
     *
     * @return an integer corresponding to the days of the month scheduled for wake up
     *
     * On failure, throws an exception or returns Y_MONTHDAYS_INVALID.
     */
    int                 get_monthDays(void);

    inline int          monthDays(void)
    { return this->get_monthDays(); }

    /**
     * Changes the days of the month when a wake up must take place.
     *
     * @param newval : an integer corresponding to the days of the month when a wake up must take place
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_monthDays(int newval);
    inline int      setMonthDays(int newval)
    { return this->set_monthDays(newval); }

    /**
     * Returns the months scheduled for wake up.
     *
     * @return an integer corresponding to the months scheduled for wake up
     *
     * On failure, throws an exception or returns Y_MONTHS_INVALID.
     */
    int                 get_months(void);

    inline int          months(void)
    { return this->get_months(); }

    /**
     * Changes the months when a wake up must take place.
     *
     * @param newval : an integer corresponding to the months when a wake up must take place
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_months(int newval);
    inline int      setMonths(int newval)
    { return this->set_months(newval); }

    /**
     * Returns the date/time (seconds) of the next wake up occurence.
     *
     * @return an integer corresponding to the date/time (seconds) of the next wake up occurence
     *
     * On failure, throws an exception or returns Y_NEXTOCCURENCE_INVALID.
     */
    s64                 get_nextOccurence(void);

    inline s64          nextOccurence(void)
    { return this->get_nextOccurence(); }

    /**
     * Retrieves a wake up schedule for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the wake up schedule is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YWakeUpSchedule.isOnline() to test if the wake up schedule is
     * indeed online at a given time. In case of ambiguity when looking for
     * a wake up schedule by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the wake up schedule
     *
     * @return a YWakeUpSchedule object allowing you to drive the wake up schedule.
     */
    static YWakeUpSchedule* FindWakeUpSchedule(string func);

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
    virtual int         registerValueCallback(YWakeUpScheduleValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Returns all the minutes of each hour that are scheduled for wake up.
     */
    virtual s64         get_minutes(void);

    /**
     * Changes all the minutes where a wake up must take place.
     *
     * @param bitmap : Minutes 00-59 of each hour scheduled for wake up.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_minutes(s64 bitmap);


    inline static YWakeUpSchedule* Find(string func)
    { return YWakeUpSchedule::FindWakeUpSchedule(func); }

    /**
     * Continues the enumeration of wake up schedules started using yFirstWakeUpSchedule().
     *
     * @return a pointer to a YWakeUpSchedule object, corresponding to
     *         a wake up schedule currently online, or a NULL pointer
     *         if there are no more wake up schedules to enumerate.
     */
           YWakeUpSchedule *nextWakeUpSchedule(void);
    inline YWakeUpSchedule *next(void)
    { return this->nextWakeUpSchedule();}

    /**
     * Starts the enumeration of wake up schedules currently accessible.
     * Use the method YWakeUpSchedule.nextWakeUpSchedule() to iterate on
     * next wake up schedules.
     *
     * @return a pointer to a YWakeUpSchedule object, corresponding to
     *         the first wake up schedule currently online, or a NULL pointer
     *         if there are none.
     */
           static YWakeUpSchedule* FirstWakeUpSchedule(void);
    inline static YWakeUpSchedule* First(void)
    { return YWakeUpSchedule::FirstWakeUpSchedule();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YWakeUpSchedule accessors declaration)
};

//--- (YWakeUpSchedule functions declaration)

/**
 * Retrieves a wake up schedule for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the wake up schedule is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YWakeUpSchedule.isOnline() to test if the wake up schedule is
 * indeed online at a given time. In case of ambiguity when looking for
 * a wake up schedule by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the wake up schedule
 *
 * @return a YWakeUpSchedule object allowing you to drive the wake up schedule.
 */
inline YWakeUpSchedule* yFindWakeUpSchedule(const string& func)
{ return YWakeUpSchedule::FindWakeUpSchedule(func);}
/**
 * Starts the enumeration of wake up schedules currently accessible.
 * Use the method YWakeUpSchedule.nextWakeUpSchedule() to iterate on
 * next wake up schedules.
 *
 * @return a pointer to a YWakeUpSchedule object, corresponding to
 *         the first wake up schedule currently online, or a NULL pointer
 *         if there are none.
 */
inline YWakeUpSchedule* yFirstWakeUpSchedule(void)
{ return YWakeUpSchedule::FirstWakeUpSchedule();}

//--- (end of YWakeUpSchedule functions declaration)

#endif
