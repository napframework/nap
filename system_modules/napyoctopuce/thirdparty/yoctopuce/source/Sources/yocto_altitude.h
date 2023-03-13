/*********************************************************************
 *
 * $Id: yocto_altitude.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindAltitude(), the high-level API for Altitude functions
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


#ifndef YOCTO_ALTITUDE_H
#define YOCTO_ALTITUDE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YAltitude return codes)
//--- (end of YAltitude return codes)
//--- (YAltitude definitions)
class YAltitude; // forward declaration

typedef void (*YAltitudeValueCallback)(YAltitude *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YAltitudeTimedReportCallback)(YAltitude *func, YMeasure measure);
#define Y_QNH_INVALID                   (YAPI_INVALID_DOUBLE)
#define Y_TECHNOLOGY_INVALID            (YAPI_INVALID_STRING)
//--- (end of YAltitude definitions)

//--- (YAltitude declaration)
/**
 * YAltitude Class: Altitude function interface
 *
 * The Yoctopuce class YAltitude allows you to read and configure Yoctopuce altitude
 * sensors. It inherits from the YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 * This class adds the ability to configure the barometric pressure adjusted to
 * sea level (QNH) for barometric sensors.
 */
class YOCTO_CLASS_EXPORT YAltitude: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YAltitude declaration)
protected:
    //--- (YAltitude attributes)
    // Attributes (function value cache)
    double          _qnh;
    string          _technology;
    YAltitudeValueCallback _valueCallbackAltitude;
    YAltitudeTimedReportCallback _timedReportCallbackAltitude;

    friend YAltitude *yFindAltitude(const string& func);
    friend YAltitude *yFirstAltitude(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindAltitude factory function to instantiate
    YAltitude(const string& func);
    //--- (end of YAltitude attributes)

public:
    ~YAltitude();
    //--- (YAltitude accessors declaration)

    static const double QNH_INVALID;
    static const string TECHNOLOGY_INVALID;

    /**
     * Changes the current estimated altitude. This allows to compensate for
     * ambient pressure variations and to work in relative mode.
     *
     * @param newval : a floating point number corresponding to the current estimated altitude
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_currentValue(double newval);
    inline int      setCurrentValue(double newval)
    { return this->set_currentValue(newval); }

    /**
     * Changes the barometric pressure adjusted to sea level used to compute
     * the altitude (QNH). This enables you to compensate for atmospheric pressure
     * changes due to weather conditions.
     *
     * @param newval : a floating point number corresponding to the barometric pressure adjusted to sea
     * level used to compute
     *         the altitude (QNH)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_qnh(double newval);
    inline int      setQnh(double newval)
    { return this->set_qnh(newval); }

    /**
     * Returns the barometric pressure adjusted to sea level used to compute
     * the altitude (QNH).
     *
     * @return a floating point number corresponding to the barometric pressure adjusted to sea level used to compute
     *         the altitude (QNH)
     *
     * On failure, throws an exception or returns Y_QNH_INVALID.
     */
    double              get_qnh(void);

    inline double       qnh(void)
    { return this->get_qnh(); }

    /**
     * Returns the technology used by the sesnor to compute
     * altitude. Possibles values are  "barometric" and "gps"
     *
     * @return a string corresponding to the technology used by the sesnor to compute
     *         altitude
     *
     * On failure, throws an exception or returns Y_TECHNOLOGY_INVALID.
     */
    string              get_technology(void);

    inline string       technology(void)
    { return this->get_technology(); }

    /**
     * Retrieves an altimeter for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the altimeter is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YAltitude.isOnline() to test if the altimeter is
     * indeed online at a given time. In case of ambiguity when looking for
     * an altimeter by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the altimeter
     *
     * @return a YAltitude object allowing you to drive the altimeter.
     */
    static YAltitude*   FindAltitude(string func);

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
    virtual int         registerValueCallback(YAltitudeValueCallback callback);
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
    virtual int         registerTimedReportCallback(YAltitudeTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YAltitude* Find(string func)
    { return YAltitude::FindAltitude(func); }

    /**
     * Continues the enumeration of altimeters started using yFirstAltitude().
     *
     * @return a pointer to a YAltitude object, corresponding to
     *         an altimeter currently online, or a NULL pointer
     *         if there are no more altimeters to enumerate.
     */
           YAltitude       *nextAltitude(void);
    inline YAltitude       *next(void)
    { return this->nextAltitude();}

    /**
     * Starts the enumeration of altimeters currently accessible.
     * Use the method YAltitude.nextAltitude() to iterate on
     * next altimeters.
     *
     * @return a pointer to a YAltitude object, corresponding to
     *         the first altimeter currently online, or a NULL pointer
     *         if there are none.
     */
           static YAltitude* FirstAltitude(void);
    inline static YAltitude* First(void)
    { return YAltitude::FirstAltitude();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YAltitude accessors declaration)
};

//--- (YAltitude functions declaration)

/**
 * Retrieves an altimeter for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the altimeter is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YAltitude.isOnline() to test if the altimeter is
 * indeed online at a given time. In case of ambiguity when looking for
 * an altimeter by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the altimeter
 *
 * @return a YAltitude object allowing you to drive the altimeter.
 */
inline YAltitude* yFindAltitude(const string& func)
{ return YAltitude::FindAltitude(func);}
/**
 * Starts the enumeration of altimeters currently accessible.
 * Use the method YAltitude.nextAltitude() to iterate on
 * next altimeters.
 *
 * @return a pointer to a YAltitude object, corresponding to
 *         the first altimeter currently online, or a NULL pointer
 *         if there are none.
 */
inline YAltitude* yFirstAltitude(void)
{ return YAltitude::FirstAltitude();}

//--- (end of YAltitude functions declaration)

#endif
