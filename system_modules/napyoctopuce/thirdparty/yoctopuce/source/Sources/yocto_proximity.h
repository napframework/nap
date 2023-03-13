/*********************************************************************
 *
 * $Id: yocto_proximity.h 29767 2018-01-26 08:53:27Z seb $
 *
 * Declares yFindProximity(), the high-level API for Proximity functions
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


#ifndef YOCTO_PROXIMITY_H
#define YOCTO_PROXIMITY_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YProximity return codes)
//--- (end of YProximity return codes)
//--- (YProximity definitions)
class YProximity; // forward declaration

typedef void (*YProximityValueCallback)(YProximity *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YProximityTimedReportCallback)(YProximity *func, YMeasure measure);
#ifndef _Y_ISPRESENT_ENUM
#define _Y_ISPRESENT_ENUM
typedef enum {
    Y_ISPRESENT_FALSE = 0,
    Y_ISPRESENT_TRUE = 1,
    Y_ISPRESENT_INVALID = -1,
} Y_ISPRESENT_enum;
#endif
#ifndef _Y_PROXIMITYREPORTMODE_ENUM
#define _Y_PROXIMITYREPORTMODE_ENUM
typedef enum {
    Y_PROXIMITYREPORTMODE_NUMERIC = 0,
    Y_PROXIMITYREPORTMODE_PRESENCE = 1,
    Y_PROXIMITYREPORTMODE_PULSECOUNT = 2,
    Y_PROXIMITYREPORTMODE_INVALID = -1,
} Y_PROXIMITYREPORTMODE_enum;
#endif
#define Y_SIGNALVALUE_INVALID           (YAPI_INVALID_DOUBLE)
#define Y_DETECTIONTHRESHOLD_INVALID    (YAPI_INVALID_UINT)
#define Y_DETECTIONHYSTERESIS_INVALID   (YAPI_INVALID_UINT)
#define Y_PRESENCEMINTIME_INVALID       (YAPI_INVALID_UINT)
#define Y_REMOVALMINTIME_INVALID        (YAPI_INVALID_UINT)
#define Y_LASTTIMEAPPROACHED_INVALID    (YAPI_INVALID_LONG)
#define Y_LASTTIMEREMOVED_INVALID       (YAPI_INVALID_LONG)
#define Y_PULSECOUNTER_INVALID          (YAPI_INVALID_LONG)
#define Y_PULSETIMER_INVALID            (YAPI_INVALID_LONG)
//--- (end of YProximity definitions)

//--- (YProximity declaration)
/**
 * YProximity Class: Proximity function interface
 *
 * The Yoctopuce class YProximity allows you to use and configure Yoctopuce proximity
 * sensors. It inherits from the YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 * This class adds the ability to easily perform a one-point linear calibration
 * to compensate the effect of a glass or filter placed in front of the sensor.
 */
class YOCTO_CLASS_EXPORT YProximity: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YProximity declaration)
protected:
    //--- (YProximity attributes)
    // Attributes (function value cache)
    double          _signalValue;
    int             _detectionThreshold;
    int             _detectionHysteresis;
    int             _presenceMinTime;
    int             _removalMinTime;
    Y_ISPRESENT_enum _isPresent;
    s64             _lastTimeApproached;
    s64             _lastTimeRemoved;
    s64             _pulseCounter;
    s64             _pulseTimer;
    Y_PROXIMITYREPORTMODE_enum _proximityReportMode;
    YProximityValueCallback _valueCallbackProximity;
    YProximityTimedReportCallback _timedReportCallbackProximity;

    friend YProximity *yFindProximity(const string& func);
    friend YProximity *yFirstProximity(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindProximity factory function to instantiate
    YProximity(const string& func);
    //--- (end of YProximity attributes)

public:
    ~YProximity();
    //--- (YProximity accessors declaration)

    static const double SIGNALVALUE_INVALID;
    static const int DETECTIONTHRESHOLD_INVALID = YAPI_INVALID_UINT;
    static const int DETECTIONHYSTERESIS_INVALID = YAPI_INVALID_UINT;
    static const int PRESENCEMINTIME_INVALID = YAPI_INVALID_UINT;
    static const int REMOVALMINTIME_INVALID = YAPI_INVALID_UINT;
    static const Y_ISPRESENT_enum ISPRESENT_FALSE = Y_ISPRESENT_FALSE;
    static const Y_ISPRESENT_enum ISPRESENT_TRUE = Y_ISPRESENT_TRUE;
    static const Y_ISPRESENT_enum ISPRESENT_INVALID = Y_ISPRESENT_INVALID;
    static const s64 LASTTIMEAPPROACHED_INVALID = YAPI_INVALID_LONG;
    static const s64 LASTTIMEREMOVED_INVALID = YAPI_INVALID_LONG;
    static const s64 PULSECOUNTER_INVALID = YAPI_INVALID_LONG;
    static const s64 PULSETIMER_INVALID = YAPI_INVALID_LONG;
    static const Y_PROXIMITYREPORTMODE_enum PROXIMITYREPORTMODE_NUMERIC = Y_PROXIMITYREPORTMODE_NUMERIC;
    static const Y_PROXIMITYREPORTMODE_enum PROXIMITYREPORTMODE_PRESENCE = Y_PROXIMITYREPORTMODE_PRESENCE;
    static const Y_PROXIMITYREPORTMODE_enum PROXIMITYREPORTMODE_PULSECOUNT = Y_PROXIMITYREPORTMODE_PULSECOUNT;
    static const Y_PROXIMITYREPORTMODE_enum PROXIMITYREPORTMODE_INVALID = Y_PROXIMITYREPORTMODE_INVALID;

    /**
     * Returns the current value of signal measured by the proximity sensor.
     *
     * @return a floating point number corresponding to the current value of signal measured by the proximity sensor
     *
     * On failure, throws an exception or returns Y_SIGNALVALUE_INVALID.
     */
    double              get_signalValue(void);

    inline double       signalValue(void)
    { return this->get_signalValue(); }

    /**
     * Returns the threshold used to determine the logical state of the proximity sensor, when considered
     * as a binary input (on/off).
     *
     * @return an integer corresponding to the threshold used to determine the logical state of the
     * proximity sensor, when considered
     *         as a binary input (on/off)
     *
     * On failure, throws an exception or returns Y_DETECTIONTHRESHOLD_INVALID.
     */
    int                 get_detectionThreshold(void);

    inline int          detectionThreshold(void)
    { return this->get_detectionThreshold(); }

    /**
     * Changes the threshold used to determine the logical state of the proximity sensor, when considered
     * as a binary input (on/off).
     *
     * @param newval : an integer corresponding to the threshold used to determine the logical state of
     * the proximity sensor, when considered
     *         as a binary input (on/off)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_detectionThreshold(int newval);
    inline int      setDetectionThreshold(int newval)
    { return this->set_detectionThreshold(newval); }

    /**
     * Returns the hysteresis used to determine the logical state of the proximity sensor, when considered
     * as a binary input (on/off).
     *
     * @return an integer corresponding to the hysteresis used to determine the logical state of the
     * proximity sensor, when considered
     *         as a binary input (on/off)
     *
     * On failure, throws an exception or returns Y_DETECTIONHYSTERESIS_INVALID.
     */
    int                 get_detectionHysteresis(void);

    inline int          detectionHysteresis(void)
    { return this->get_detectionHysteresis(); }

    /**
     * Changes the hysteresis used to determine the logical state of the proximity sensor, when considered
     * as a binary input (on/off).
     *
     * @param newval : an integer corresponding to the hysteresis used to determine the logical state of
     * the proximity sensor, when considered
     *         as a binary input (on/off)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_detectionHysteresis(int newval);
    inline int      setDetectionHysteresis(int newval)
    { return this->set_detectionHysteresis(newval); }

    /**
     * Returns the minimal detection duration before signaling a presence event. Any shorter detection is
     * considered as noise or bounce (false positive) and filtered out.
     *
     * @return an integer corresponding to the minimal detection duration before signaling a presence event
     *
     * On failure, throws an exception or returns Y_PRESENCEMINTIME_INVALID.
     */
    int                 get_presenceMinTime(void);

    inline int          presenceMinTime(void)
    { return this->get_presenceMinTime(); }

    /**
     * Changes the minimal detection duration before signaling a presence event. Any shorter detection is
     * considered as noise or bounce (false positive) and filtered out.
     *
     * @param newval : an integer corresponding to the minimal detection duration before signaling a presence event
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_presenceMinTime(int newval);
    inline int      setPresenceMinTime(int newval)
    { return this->set_presenceMinTime(newval); }

    /**
     * Returns the minimal detection duration before signaling a removal event. Any shorter detection is
     * considered as noise or bounce (false positive) and filtered out.
     *
     * @return an integer corresponding to the minimal detection duration before signaling a removal event
     *
     * On failure, throws an exception or returns Y_REMOVALMINTIME_INVALID.
     */
    int                 get_removalMinTime(void);

    inline int          removalMinTime(void)
    { return this->get_removalMinTime(); }

    /**
     * Changes the minimal detection duration before signaling a removal event. Any shorter detection is
     * considered as noise or bounce (false positive) and filtered out.
     *
     * @param newval : an integer corresponding to the minimal detection duration before signaling a removal event
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_removalMinTime(int newval);
    inline int      setRemovalMinTime(int newval)
    { return this->set_removalMinTime(newval); }

    /**
     * Returns true if the input (considered as binary) is active (detection value is smaller than the
     * specified threshold), and false otherwise.
     *
     * @return either Y_ISPRESENT_FALSE or Y_ISPRESENT_TRUE, according to true if the input (considered as
     * binary) is active (detection value is smaller than the specified threshold), and false otherwise
     *
     * On failure, throws an exception or returns Y_ISPRESENT_INVALID.
     */
    Y_ISPRESENT_enum    get_isPresent(void);

    inline Y_ISPRESENT_enum isPresent(void)
    { return this->get_isPresent(); }

    /**
     * Returns the number of elapsed milliseconds between the module power on and the last observed
     * detection (the input contact transitioned from absent to present).
     *
     * @return an integer corresponding to the number of elapsed milliseconds between the module power on
     * and the last observed
     *         detection (the input contact transitioned from absent to present)
     *
     * On failure, throws an exception or returns Y_LASTTIMEAPPROACHED_INVALID.
     */
    s64                 get_lastTimeApproached(void);

    inline s64          lastTimeApproached(void)
    { return this->get_lastTimeApproached(); }

    /**
     * Returns the number of elapsed milliseconds between the module power on and the last observed
     * detection (the input contact transitioned from present to absent).
     *
     * @return an integer corresponding to the number of elapsed milliseconds between the module power on
     * and the last observed
     *         detection (the input contact transitioned from present to absent)
     *
     * On failure, throws an exception or returns Y_LASTTIMEREMOVED_INVALID.
     */
    s64                 get_lastTimeRemoved(void);

    inline s64          lastTimeRemoved(void)
    { return this->get_lastTimeRemoved(); }

    /**
     * Returns the pulse counter value. The value is a 32 bit integer. In case
     * of overflow (>=2^32), the counter will wrap. To reset the counter, just
     * call the resetCounter() method.
     *
     * @return an integer corresponding to the pulse counter value
     *
     * On failure, throws an exception or returns Y_PULSECOUNTER_INVALID.
     */
    s64                 get_pulseCounter(void);

    inline s64          pulseCounter(void)
    { return this->get_pulseCounter(); }

    int             set_pulseCounter(s64 newval);
    inline int      setPulseCounter(s64 newval)
    { return this->set_pulseCounter(newval); }

    /**
     * Returns the timer of the pulse counter (ms).
     *
     * @return an integer corresponding to the timer of the pulse counter (ms)
     *
     * On failure, throws an exception or returns Y_PULSETIMER_INVALID.
     */
    s64                 get_pulseTimer(void);

    inline s64          pulseTimer(void)
    { return this->get_pulseTimer(); }

    /**
     * Returns the parameter (sensor value, presence or pulse count) returned by the get_currentValue
     * function and callbacks.
     *
     * @return a value among Y_PROXIMITYREPORTMODE_NUMERIC, Y_PROXIMITYREPORTMODE_PRESENCE and
     * Y_PROXIMITYREPORTMODE_PULSECOUNT corresponding to the parameter (sensor value, presence or pulse
     * count) returned by the get_currentValue function and callbacks
     *
     * On failure, throws an exception or returns Y_PROXIMITYREPORTMODE_INVALID.
     */
    Y_PROXIMITYREPORTMODE_enum get_proximityReportMode(void);

    inline Y_PROXIMITYREPORTMODE_enum proximityReportMode(void)
    { return this->get_proximityReportMode(); }

    /**
     * Changes the  parameter  type (sensor value, presence or pulse count) returned by the
     * get_currentValue function and callbacks.
     * The edge count value is limited to the 6 lowest digits. For values greater than one million, use
     * get_pulseCounter().
     *
     * @param newval : a value among Y_PROXIMITYREPORTMODE_NUMERIC, Y_PROXIMITYREPORTMODE_PRESENCE and
     * Y_PROXIMITYREPORTMODE_PULSECOUNT corresponding to the  parameter  type (sensor value, presence or
     * pulse count) returned by the get_currentValue function and callbacks
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_proximityReportMode(Y_PROXIMITYREPORTMODE_enum newval);
    inline int      setProximityReportMode(Y_PROXIMITYREPORTMODE_enum newval)
    { return this->set_proximityReportMode(newval); }

    /**
     * Retrieves a proximity sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the proximity sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YProximity.isOnline() to test if the proximity sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a proximity sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the proximity sensor
     *
     * @return a YProximity object allowing you to drive the proximity sensor.
     */
    static YProximity*  FindProximity(string func);

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
    virtual int         registerValueCallback(YProximityValueCallback callback);
    using YSensor::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Registers the callback function that is invoked on every periodic timed notification.
     * The callback is invoked only during the execution of ySleep or yHandleEvents.
     * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
     * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
     *
     * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
     *         arguments: the function object of which the value has changed, and an YMeasure object describing
     *         the new advertised value.
     * @noreturn
     */
    virtual int         registerTimedReportCallback(YProximityTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);

    /**
     * Resets the pulse counter value as well as its timer.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         resetCounter(void);


    inline static YProximity* Find(string func)
    { return YProximity::FindProximity(func); }

    /**
     * Continues the enumeration of proximity sensors started using yFirstProximity().
     *
     * @return a pointer to a YProximity object, corresponding to
     *         a proximity sensor currently online, or a NULL pointer
     *         if there are no more proximity sensors to enumerate.
     */
           YProximity      *nextProximity(void);
    inline YProximity      *next(void)
    { return this->nextProximity();}

    /**
     * Starts the enumeration of proximity sensors currently accessible.
     * Use the method YProximity.nextProximity() to iterate on
     * next proximity sensors.
     *
     * @return a pointer to a YProximity object, corresponding to
     *         the first proximity sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YProximity* FirstProximity(void);
    inline static YProximity* First(void)
    { return YProximity::FirstProximity();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YProximity accessors declaration)
};

//--- (YProximity functions declaration)

/**
 * Retrieves a proximity sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the proximity sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YProximity.isOnline() to test if the proximity sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a proximity sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the proximity sensor
 *
 * @return a YProximity object allowing you to drive the proximity sensor.
 */
inline YProximity* yFindProximity(const string& func)
{ return YProximity::FindProximity(func);}
/**
 * Starts the enumeration of proximity sensors currently accessible.
 * Use the method YProximity.nextProximity() to iterate on
 * next proximity sensors.
 *
 * @return a pointer to a YProximity object, corresponding to
 *         the first proximity sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YProximity* yFirstProximity(void)
{ return YProximity::FirstProximity();}

//--- (end of YProximity functions declaration)

#endif
