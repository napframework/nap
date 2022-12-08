/*********************************************************************
 *
 * $Id: yocto_longitude.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindLongitude(), the high-level API for Longitude functions
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


#ifndef YOCTO_LONGITUDE_H
#define YOCTO_LONGITUDE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YLongitude return codes)
//--- (end of YLongitude return codes)
//--- (YLongitude definitions)
class YLongitude; // forward declaration

typedef void (*YLongitudeValueCallback)(YLongitude *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YLongitudeTimedReportCallback)(YLongitude *func, YMeasure measure);
//--- (end of YLongitude definitions)

//--- (YLongitude declaration)
/**
 * YLongitude Class: Longitude function interface
 *
 * The Yoctopuce class YLongitude allows you to read the longitude from Yoctopuce
 * geolocalization sensors. It inherits from the YSensor class the core functions to
 * read measurements, register callback functions, access the autonomous
 * datalogger.
 */
class YOCTO_CLASS_EXPORT YLongitude: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YLongitude declaration)
protected:
    //--- (YLongitude attributes)
    // Attributes (function value cache)
    YLongitudeValueCallback _valueCallbackLongitude;
    YLongitudeTimedReportCallback _timedReportCallbackLongitude;

    friend YLongitude *yFindLongitude(const string& func);
    friend YLongitude *yFirstLongitude(void);

    // Constructor is protected, use yFindLongitude factory function to instantiate
    YLongitude(const string& func);
    //--- (end of YLongitude attributes)

public:
    ~YLongitude();
    //--- (YLongitude accessors declaration)


    /**
     * Retrieves a longitude sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the longitude sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YLongitude.isOnline() to test if the longitude sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a longitude sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the longitude sensor
     *
     * @return a YLongitude object allowing you to drive the longitude sensor.
     */
    static YLongitude*  FindLongitude(string func);

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
    virtual int         registerValueCallback(YLongitudeValueCallback callback);
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
    virtual int         registerTimedReportCallback(YLongitudeTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YLongitude* Find(string func)
    { return YLongitude::FindLongitude(func); }

    /**
     * Continues the enumeration of longitude sensors started using yFirstLongitude().
     *
     * @return a pointer to a YLongitude object, corresponding to
     *         a longitude sensor currently online, or a NULL pointer
     *         if there are no more longitude sensors to enumerate.
     */
           YLongitude      *nextLongitude(void);
    inline YLongitude      *next(void)
    { return this->nextLongitude();}

    /**
     * Starts the enumeration of longitude sensors currently accessible.
     * Use the method YLongitude.nextLongitude() to iterate on
     * next longitude sensors.
     *
     * @return a pointer to a YLongitude object, corresponding to
     *         the first longitude sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YLongitude* FirstLongitude(void);
    inline static YLongitude* First(void)
    { return YLongitude::FirstLongitude();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YLongitude accessors declaration)
};

//--- (YLongitude functions declaration)

/**
 * Retrieves a longitude sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the longitude sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YLongitude.isOnline() to test if the longitude sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a longitude sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the longitude sensor
 *
 * @return a YLongitude object allowing you to drive the longitude sensor.
 */
inline YLongitude* yFindLongitude(const string& func)
{ return YLongitude::FindLongitude(func);}
/**
 * Starts the enumeration of longitude sensors currently accessible.
 * Use the method YLongitude.nextLongitude() to iterate on
 * next longitude sensors.
 *
 * @return a pointer to a YLongitude object, corresponding to
 *         the first longitude sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YLongitude* yFirstLongitude(void)
{ return YLongitude::FirstLongitude();}

//--- (end of YLongitude functions declaration)

#endif
