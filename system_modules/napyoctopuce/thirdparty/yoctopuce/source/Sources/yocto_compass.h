/*********************************************************************
 *
 * $Id: yocto_compass.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindCompass(), the high-level API for Compass functions
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


#ifndef YOCTO_COMPASS_H
#define YOCTO_COMPASS_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YCompass return codes)
//--- (end of YCompass return codes)
//--- (YCompass definitions)
class YCompass; // forward declaration

typedef void (*YCompassValueCallback)(YCompass *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YCompassTimedReportCallback)(YCompass *func, YMeasure measure);
#ifndef _Y_AXIS_ENUM
#define _Y_AXIS_ENUM
typedef enum {
    Y_AXIS_X = 0,
    Y_AXIS_Y = 1,
    Y_AXIS_Z = 2,
    Y_AXIS_INVALID = -1,
} Y_AXIS_enum;
#endif
#define Y_BANDWIDTH_INVALID             (YAPI_INVALID_INT)
#define Y_MAGNETICHEADING_INVALID       (YAPI_INVALID_DOUBLE)
//--- (end of YCompass definitions)

//--- (YCompass declaration)
/**
 * YCompass Class: Compass function interface
 *
 * The YSensor class is the parent class for all Yoctopuce sensors. It can be
 * used to read the current value and unit of any sensor, read the min/max
 * value, configure autonomous recording frequency and access recorded data.
 * It also provide a function to register a callback invoked each time the
 * observed value changes, or at a predefined interval. Using this class rather
 * than a specific subclass makes it possible to create generic applications
 * that work with any Yoctopuce sensor, even those that do not yet exist.
 * Note: The YAnButton class is the only analog input which does not inherit
 * from YSensor.
 */
class YOCTO_CLASS_EXPORT YCompass: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YCompass declaration)
protected:
    //--- (YCompass attributes)
    // Attributes (function value cache)
    int             _bandwidth;
    Y_AXIS_enum     _axis;
    double          _magneticHeading;
    YCompassValueCallback _valueCallbackCompass;
    YCompassTimedReportCallback _timedReportCallbackCompass;

    friend YCompass *yFindCompass(const string& func);
    friend YCompass *yFirstCompass(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindCompass factory function to instantiate
    YCompass(const string& func);
    //--- (end of YCompass attributes)

public:
    ~YCompass();
    //--- (YCompass accessors declaration)

    static const int BANDWIDTH_INVALID = YAPI_INVALID_INT;
    static const Y_AXIS_enum AXIS_X = Y_AXIS_X;
    static const Y_AXIS_enum AXIS_Y = Y_AXIS_Y;
    static const Y_AXIS_enum AXIS_Z = Y_AXIS_Z;
    static const Y_AXIS_enum AXIS_INVALID = Y_AXIS_INVALID;
    static const double MAGNETICHEADING_INVALID;

    /**
     * Returns the measure update frequency, measured in Hz (Yocto-3D-V2 only).
     *
     * @return an integer corresponding to the measure update frequency, measured in Hz (Yocto-3D-V2 only)
     *
     * On failure, throws an exception or returns Y_BANDWIDTH_INVALID.
     */
    int                 get_bandwidth(void);

    inline int          bandwidth(void)
    { return this->get_bandwidth(); }

    /**
     * Changes the measure update frequency, measured in Hz (Yocto-3D-V2 only). When the
     * frequency is lower, the device performs averaging.
     *
     * @param newval : an integer corresponding to the measure update frequency, measured in Hz (Yocto-3D-V2 only)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_bandwidth(int newval);
    inline int      setBandwidth(int newval)
    { return this->set_bandwidth(newval); }

    Y_AXIS_enum         get_axis(void);

    inline Y_AXIS_enum  axis(void)
    { return this->get_axis(); }

    /**
     * Returns the magnetic heading, regardless of the configured bearing.
     *
     * @return a floating point number corresponding to the magnetic heading, regardless of the configured bearing
     *
     * On failure, throws an exception or returns Y_MAGNETICHEADING_INVALID.
     */
    double              get_magneticHeading(void);

    inline double       magneticHeading(void)
    { return this->get_magneticHeading(); }

    /**
     * Retrieves a compass for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the compass is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YCompass.isOnline() to test if the compass is
     * indeed online at a given time. In case of ambiguity when looking for
     * a compass by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the compass
     *
     * @return a YCompass object allowing you to drive the compass.
     */
    static YCompass*    FindCompass(string func);

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
    virtual int         registerValueCallback(YCompassValueCallback callback);
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
    virtual int         registerTimedReportCallback(YCompassTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YCompass* Find(string func)
    { return YCompass::FindCompass(func); }

    /**
     * Continues the enumeration of compasses started using yFirstCompass().
     *
     * @return a pointer to a YCompass object, corresponding to
     *         a compass currently online, or a NULL pointer
     *         if there are no more compasses to enumerate.
     */
           YCompass        *nextCompass(void);
    inline YCompass        *next(void)
    { return this->nextCompass();}

    /**
     * Starts the enumeration of compasses currently accessible.
     * Use the method YCompass.nextCompass() to iterate on
     * next compasses.
     *
     * @return a pointer to a YCompass object, corresponding to
     *         the first compass currently online, or a NULL pointer
     *         if there are none.
     */
           static YCompass* FirstCompass(void);
    inline static YCompass* First(void)
    { return YCompass::FirstCompass();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YCompass accessors declaration)
};

//--- (YCompass functions declaration)

/**
 * Retrieves a compass for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the compass is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCompass.isOnline() to test if the compass is
 * indeed online at a given time. In case of ambiguity when looking for
 * a compass by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the compass
 *
 * @return a YCompass object allowing you to drive the compass.
 */
inline YCompass* yFindCompass(const string& func)
{ return YCompass::FindCompass(func);}
/**
 * Starts the enumeration of compasses currently accessible.
 * Use the method YCompass.nextCompass() to iterate on
 * next compasses.
 *
 * @return a pointer to a YCompass object, corresponding to
 *         the first compass currently online, or a NULL pointer
 *         if there are none.
 */
inline YCompass* yFirstCompass(void)
{ return YCompass::FirstCompass();}

//--- (end of YCompass functions declaration)

#endif
