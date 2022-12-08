/*********************************************************************
 *
 * $Id: yocto_magnetometer.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindMagnetometer(), the high-level API for Magnetometer functions
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


#ifndef YOCTO_MAGNETOMETER_H
#define YOCTO_MAGNETOMETER_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YMagnetometer return codes)
//--- (end of YMagnetometer return codes)
//--- (YMagnetometer definitions)
class YMagnetometer; // forward declaration

typedef void (*YMagnetometerValueCallback)(YMagnetometer *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YMagnetometerTimedReportCallback)(YMagnetometer *func, YMeasure measure);
#define Y_BANDWIDTH_INVALID             (YAPI_INVALID_INT)
#define Y_XVALUE_INVALID                (YAPI_INVALID_DOUBLE)
#define Y_YVALUE_INVALID                (YAPI_INVALID_DOUBLE)
#define Y_ZVALUE_INVALID                (YAPI_INVALID_DOUBLE)
//--- (end of YMagnetometer definitions)

//--- (YMagnetometer declaration)
/**
 * YMagnetometer Class: Magnetometer function interface
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
class YOCTO_CLASS_EXPORT YMagnetometer: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YMagnetometer declaration)
protected:
    //--- (YMagnetometer attributes)
    // Attributes (function value cache)
    int             _bandwidth;
    double          _xValue;
    double          _yValue;
    double          _zValue;
    YMagnetometerValueCallback _valueCallbackMagnetometer;
    YMagnetometerTimedReportCallback _timedReportCallbackMagnetometer;

    friend YMagnetometer *yFindMagnetometer(const string& func);
    friend YMagnetometer *yFirstMagnetometer(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindMagnetometer factory function to instantiate
    YMagnetometer(const string& func);
    //--- (end of YMagnetometer attributes)

public:
    ~YMagnetometer();
    //--- (YMagnetometer accessors declaration)

    static const int BANDWIDTH_INVALID = YAPI_INVALID_INT;
    static const double XVALUE_INVALID;
    static const double YVALUE_INVALID;
    static const double ZVALUE_INVALID;

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

    /**
     * Returns the X component of the magnetic field, as a floating point number.
     *
     * @return a floating point number corresponding to the X component of the magnetic field, as a
     * floating point number
     *
     * On failure, throws an exception or returns Y_XVALUE_INVALID.
     */
    double              get_xValue(void);

    inline double       xValue(void)
    { return this->get_xValue(); }

    /**
     * Returns the Y component of the magnetic field, as a floating point number.
     *
     * @return a floating point number corresponding to the Y component of the magnetic field, as a
     * floating point number
     *
     * On failure, throws an exception or returns Y_YVALUE_INVALID.
     */
    double              get_yValue(void);

    inline double       yValue(void)
    { return this->get_yValue(); }

    /**
     * Returns the Z component of the magnetic field, as a floating point number.
     *
     * @return a floating point number corresponding to the Z component of the magnetic field, as a
     * floating point number
     *
     * On failure, throws an exception or returns Y_ZVALUE_INVALID.
     */
    double              get_zValue(void);

    inline double       zValue(void)
    { return this->get_zValue(); }

    /**
     * Retrieves a magnetometer for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the magnetometer is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YMagnetometer.isOnline() to test if the magnetometer is
     * indeed online at a given time. In case of ambiguity when looking for
     * a magnetometer by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the magnetometer
     *
     * @return a YMagnetometer object allowing you to drive the magnetometer.
     */
    static YMagnetometer* FindMagnetometer(string func);

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
    virtual int         registerValueCallback(YMagnetometerValueCallback callback);
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
    virtual int         registerTimedReportCallback(YMagnetometerTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YMagnetometer* Find(string func)
    { return YMagnetometer::FindMagnetometer(func); }

    /**
     * Continues the enumeration of magnetometers started using yFirstMagnetometer().
     *
     * @return a pointer to a YMagnetometer object, corresponding to
     *         a magnetometer currently online, or a NULL pointer
     *         if there are no more magnetometers to enumerate.
     */
           YMagnetometer   *nextMagnetometer(void);
    inline YMagnetometer   *next(void)
    { return this->nextMagnetometer();}

    /**
     * Starts the enumeration of magnetometers currently accessible.
     * Use the method YMagnetometer.nextMagnetometer() to iterate on
     * next magnetometers.
     *
     * @return a pointer to a YMagnetometer object, corresponding to
     *         the first magnetometer currently online, or a NULL pointer
     *         if there are none.
     */
           static YMagnetometer* FirstMagnetometer(void);
    inline static YMagnetometer* First(void)
    { return YMagnetometer::FirstMagnetometer();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YMagnetometer accessors declaration)
};

//--- (YMagnetometer functions declaration)

/**
 * Retrieves a magnetometer for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the magnetometer is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YMagnetometer.isOnline() to test if the magnetometer is
 * indeed online at a given time. In case of ambiguity when looking for
 * a magnetometer by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the magnetometer
 *
 * @return a YMagnetometer object allowing you to drive the magnetometer.
 */
inline YMagnetometer* yFindMagnetometer(const string& func)
{ return YMagnetometer::FindMagnetometer(func);}
/**
 * Starts the enumeration of magnetometers currently accessible.
 * Use the method YMagnetometer.nextMagnetometer() to iterate on
 * next magnetometers.
 *
 * @return a pointer to a YMagnetometer object, corresponding to
 *         the first magnetometer currently online, or a NULL pointer
 *         if there are none.
 */
inline YMagnetometer* yFirstMagnetometer(void)
{ return YMagnetometer::FirstMagnetometer();}

//--- (end of YMagnetometer functions declaration)

#endif
