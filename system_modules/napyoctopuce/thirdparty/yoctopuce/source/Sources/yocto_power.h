/*********************************************************************
 *
 * $Id: yocto_power.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindPower(), the high-level API for Power functions
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


#ifndef YOCTO_POWER_H
#define YOCTO_POWER_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YPower return codes)
//--- (end of YPower return codes)
//--- (YPower definitions)
class YPower; // forward declaration

typedef void (*YPowerValueCallback)(YPower *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YPowerTimedReportCallback)(YPower *func, YMeasure measure);
#define Y_COSPHI_INVALID                (YAPI_INVALID_DOUBLE)
#define Y_METER_INVALID                 (YAPI_INVALID_DOUBLE)
#define Y_METERTIMER_INVALID            (YAPI_INVALID_UINT)
//--- (end of YPower definitions)

//--- (YPower declaration)
/**
 * YPower Class: Power function interface
 *
 * The Yoctopuce class YPower allows you to read and configure Yoctopuce power
 * sensors. It inherits from YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 * This class adds the ability to access the energy counter and the power factor.
 */
class YOCTO_CLASS_EXPORT YPower: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YPower declaration)
protected:
    //--- (YPower attributes)
    // Attributes (function value cache)
    double          _cosPhi;
    double          _meter;
    int             _meterTimer;
    YPowerValueCallback _valueCallbackPower;
    YPowerTimedReportCallback _timedReportCallbackPower;

    friend YPower *yFindPower(const string& func);
    friend YPower *yFirstPower(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindPower factory function to instantiate
    YPower(const string& func);
    //--- (end of YPower attributes)

public:
    ~YPower();
    //--- (YPower accessors declaration)

    static const double COSPHI_INVALID;
    static const double METER_INVALID;
    static const int METERTIMER_INVALID = YAPI_INVALID_UINT;

    /**
     * Returns the power factor (the ratio between the real power consumed,
     * measured in W, and the apparent power provided, measured in VA).
     *
     * @return a floating point number corresponding to the power factor (the ratio between the real power consumed,
     *         measured in W, and the apparent power provided, measured in VA)
     *
     * On failure, throws an exception or returns Y_COSPHI_INVALID.
     */
    double              get_cosPhi(void);

    inline double       cosPhi(void)
    { return this->get_cosPhi(); }

    int             set_meter(double newval);
    inline int      setMeter(double newval)
    { return this->set_meter(newval); }

    /**
     * Returns the energy counter, maintained by the wattmeter by integrating the power consumption over time.
     * Note that this counter is reset at each start of the device.
     *
     * @return a floating point number corresponding to the energy counter, maintained by the wattmeter by
     * integrating the power consumption over time
     *
     * On failure, throws an exception or returns Y_METER_INVALID.
     */
    double              get_meter(void);

    inline double       meter(void)
    { return this->get_meter(); }

    /**
     * Returns the elapsed time since last energy counter reset, in seconds.
     *
     * @return an integer corresponding to the elapsed time since last energy counter reset, in seconds
     *
     * On failure, throws an exception or returns Y_METERTIMER_INVALID.
     */
    int                 get_meterTimer(void);

    inline int          meterTimer(void)
    { return this->get_meterTimer(); }

    /**
     * Retrieves a electrical power sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the electrical power sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YPower.isOnline() to test if the electrical power sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a electrical power sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the electrical power sensor
     *
     * @return a YPower object allowing you to drive the electrical power sensor.
     */
    static YPower*      FindPower(string func);

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
    virtual int         registerValueCallback(YPowerValueCallback callback);
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
    virtual int         registerTimedReportCallback(YPowerTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);

    /**
     * Resets the energy counter.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         reset(void);


    inline static YPower* Find(string func)
    { return YPower::FindPower(func); }

    /**
     * Continues the enumeration of electrical power sensors started using yFirstPower().
     *
     * @return a pointer to a YPower object, corresponding to
     *         a electrical power sensor currently online, or a NULL pointer
     *         if there are no more electrical power sensors to enumerate.
     */
           YPower          *nextPower(void);
    inline YPower          *next(void)
    { return this->nextPower();}

    /**
     * Starts the enumeration of electrical power sensors currently accessible.
     * Use the method YPower.nextPower() to iterate on
     * next electrical power sensors.
     *
     * @return a pointer to a YPower object, corresponding to
     *         the first electrical power sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YPower* FirstPower(void);
    inline static YPower* First(void)
    { return YPower::FirstPower();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YPower accessors declaration)
};

//--- (YPower functions declaration)

/**
 * Retrieves a electrical power sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the electrical power sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YPower.isOnline() to test if the electrical power sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a electrical power sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the electrical power sensor
 *
 * @return a YPower object allowing you to drive the electrical power sensor.
 */
inline YPower* yFindPower(const string& func)
{ return YPower::FindPower(func);}
/**
 * Starts the enumeration of electrical power sensors currently accessible.
 * Use the method YPower.nextPower() to iterate on
 * next electrical power sensors.
 *
 * @return a pointer to a YPower object, corresponding to
 *         the first electrical power sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YPower* yFirstPower(void)
{ return YPower::FirstPower();}

//--- (end of YPower functions declaration)

#endif
