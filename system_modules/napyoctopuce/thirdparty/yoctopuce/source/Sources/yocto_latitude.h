/*********************************************************************
 *
 * $Id: yocto_latitude.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindLatitude(), the high-level API for Latitude functions
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


#ifndef YOCTO_LATITUDE_H
#define YOCTO_LATITUDE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YLatitude return codes)
//--- (end of YLatitude return codes)
//--- (YLatitude definitions)
class YLatitude; // forward declaration

typedef void (*YLatitudeValueCallback)(YLatitude *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YLatitudeTimedReportCallback)(YLatitude *func, YMeasure measure);
//--- (end of YLatitude definitions)

//--- (YLatitude declaration)
/**
 * YLatitude Class: Latitude function interface
 *
 * The Yoctopuce class YLatitude allows you to read the latitude from Yoctopuce
 * geolocalization sensors. It inherits from the YSensor class the core functions to
 * read measurements, to register callback functions, to access the autonomous
 * datalogger.
 */
class YOCTO_CLASS_EXPORT YLatitude: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YLatitude declaration)
protected:
    //--- (YLatitude attributes)
    // Attributes (function value cache)
    YLatitudeValueCallback _valueCallbackLatitude;
    YLatitudeTimedReportCallback _timedReportCallbackLatitude;

    friend YLatitude *yFindLatitude(const string& func);
    friend YLatitude *yFirstLatitude(void);

    // Constructor is protected, use yFindLatitude factory function to instantiate
    YLatitude(const string& func);
    //--- (end of YLatitude attributes)

public:
    ~YLatitude();
    //--- (YLatitude accessors declaration)


    /**
     * Retrieves a latitude sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the latitude sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YLatitude.isOnline() to test if the latitude sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a latitude sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the latitude sensor
     *
     * @return a YLatitude object allowing you to drive the latitude sensor.
     */
    static YLatitude*   FindLatitude(string func);

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
    virtual int         registerValueCallback(YLatitudeValueCallback callback);
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
    virtual int         registerTimedReportCallback(YLatitudeTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YLatitude* Find(string func)
    { return YLatitude::FindLatitude(func); }

    /**
     * Continues the enumeration of latitude sensors started using yFirstLatitude().
     *
     * @return a pointer to a YLatitude object, corresponding to
     *         a latitude sensor currently online, or a NULL pointer
     *         if there are no more latitude sensors to enumerate.
     */
           YLatitude       *nextLatitude(void);
    inline YLatitude       *next(void)
    { return this->nextLatitude();}

    /**
     * Starts the enumeration of latitude sensors currently accessible.
     * Use the method YLatitude.nextLatitude() to iterate on
     * next latitude sensors.
     *
     * @return a pointer to a YLatitude object, corresponding to
     *         the first latitude sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YLatitude* FirstLatitude(void);
    inline static YLatitude* First(void)
    { return YLatitude::FirstLatitude();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YLatitude accessors declaration)
};

//--- (YLatitude functions declaration)

/**
 * Retrieves a latitude sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the latitude sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YLatitude.isOnline() to test if the latitude sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a latitude sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the latitude sensor
 *
 * @return a YLatitude object allowing you to drive the latitude sensor.
 */
inline YLatitude* yFindLatitude(const string& func)
{ return YLatitude::FindLatitude(func);}
/**
 * Starts the enumeration of latitude sensors currently accessible.
 * Use the method YLatitude.nextLatitude() to iterate on
 * next latitude sensors.
 *
 * @return a pointer to a YLatitude object, corresponding to
 *         the first latitude sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YLatitude* yFirstLatitude(void)
{ return YLatitude::FirstLatitude();}

//--- (end of YLatitude functions declaration)

#endif
