/*********************************************************************
 *
 * $Id: yocto_temperature.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindTemperature(), the high-level API for Temperature functions
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


#ifndef YOCTO_TEMPERATURE_H
#define YOCTO_TEMPERATURE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YTemperature return codes)
//--- (end of YTemperature return codes)
//--- (YTemperature definitions)
class YTemperature; // forward declaration

typedef void (*YTemperatureValueCallback)(YTemperature *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YTemperatureTimedReportCallback)(YTemperature *func, YMeasure measure);
#ifndef _Y_SENSORTYPE_ENUM
#define _Y_SENSORTYPE_ENUM
typedef enum {
    Y_SENSORTYPE_DIGITAL = 0,
    Y_SENSORTYPE_TYPE_K = 1,
    Y_SENSORTYPE_TYPE_E = 2,
    Y_SENSORTYPE_TYPE_J = 3,
    Y_SENSORTYPE_TYPE_N = 4,
    Y_SENSORTYPE_TYPE_R = 5,
    Y_SENSORTYPE_TYPE_S = 6,
    Y_SENSORTYPE_TYPE_T = 7,
    Y_SENSORTYPE_PT100_4WIRES = 8,
    Y_SENSORTYPE_PT100_3WIRES = 9,
    Y_SENSORTYPE_PT100_2WIRES = 10,
    Y_SENSORTYPE_RES_OHM = 11,
    Y_SENSORTYPE_RES_NTC = 12,
    Y_SENSORTYPE_RES_LINEAR = 13,
    Y_SENSORTYPE_RES_INTERNAL = 14,
    Y_SENSORTYPE_INVALID = -1,
} Y_SENSORTYPE_enum;
#endif
#define Y_SIGNALVALUE_INVALID           (YAPI_INVALID_DOUBLE)
#define Y_SIGNALUNIT_INVALID            (YAPI_INVALID_STRING)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YTemperature definitions)

//--- (YTemperature declaration)
/**
 * YTemperature Class: Temperature function interface
 *
 * The Yoctopuce class YTemperature allows you to read and configure Yoctopuce temperature
 * sensors. It inherits from YSensor class the core functions to read measurements, to
 * register callback functions, to access the autonomous datalogger.
 * This class adds the ability to configure some specific parameters for some
 * sensors (connection type, temperature mapping table).
 */
class YOCTO_CLASS_EXPORT YTemperature: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YTemperature declaration)
protected:
    //--- (YTemperature attributes)
    // Attributes (function value cache)
    Y_SENSORTYPE_enum _sensorType;
    double          _signalValue;
    string          _signalUnit;
    string          _command;
    YTemperatureValueCallback _valueCallbackTemperature;
    YTemperatureTimedReportCallback _timedReportCallbackTemperature;

    friend YTemperature *yFindTemperature(const string& func);
    friend YTemperature *yFirstTemperature(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindTemperature factory function to instantiate
    YTemperature(const string& func);
    //--- (end of YTemperature attributes)

public:
    ~YTemperature();
    //--- (YTemperature accessors declaration)

    static const Y_SENSORTYPE_enum SENSORTYPE_DIGITAL = Y_SENSORTYPE_DIGITAL;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_K = Y_SENSORTYPE_TYPE_K;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_E = Y_SENSORTYPE_TYPE_E;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_J = Y_SENSORTYPE_TYPE_J;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_N = Y_SENSORTYPE_TYPE_N;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_R = Y_SENSORTYPE_TYPE_R;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_S = Y_SENSORTYPE_TYPE_S;
    static const Y_SENSORTYPE_enum SENSORTYPE_TYPE_T = Y_SENSORTYPE_TYPE_T;
    static const Y_SENSORTYPE_enum SENSORTYPE_PT100_4WIRES = Y_SENSORTYPE_PT100_4WIRES;
    static const Y_SENSORTYPE_enum SENSORTYPE_PT100_3WIRES = Y_SENSORTYPE_PT100_3WIRES;
    static const Y_SENSORTYPE_enum SENSORTYPE_PT100_2WIRES = Y_SENSORTYPE_PT100_2WIRES;
    static const Y_SENSORTYPE_enum SENSORTYPE_RES_OHM = Y_SENSORTYPE_RES_OHM;
    static const Y_SENSORTYPE_enum SENSORTYPE_RES_NTC = Y_SENSORTYPE_RES_NTC;
    static const Y_SENSORTYPE_enum SENSORTYPE_RES_LINEAR = Y_SENSORTYPE_RES_LINEAR;
    static const Y_SENSORTYPE_enum SENSORTYPE_RES_INTERNAL = Y_SENSORTYPE_RES_INTERNAL;
    static const Y_SENSORTYPE_enum SENSORTYPE_INVALID = Y_SENSORTYPE_INVALID;
    static const double SIGNALVALUE_INVALID;
    static const string SIGNALUNIT_INVALID;
    static const string COMMAND_INVALID;

    /**
     * Changes the measuring unit for the measured temperature. That unit is a string.
     * If that strings end with the letter F all temperatures values will returned in
     * Fahrenheit degrees. If that String ends with the letter K all values will be
     * returned in Kelvin degrees. If that string ends with the letter C all values will be
     * returned in Celsius degrees.  If the string ends with any other character the
     * change will be ignored. Remember to call the
     * saveToFlash() method of the module if the modification must be kept.
     * WARNING: if a specific calibration is defined for the temperature function, a
     * unit system change will probably break it.
     *
     * @param newval : a string corresponding to the measuring unit for the measured temperature
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_unit(const string& newval);
    inline int      setUnit(const string& newval)
    { return this->set_unit(newval); }

    /**
     * Returns the temperature sensor type.
     *
     * @return a value among Y_SENSORTYPE_DIGITAL, Y_SENSORTYPE_TYPE_K, Y_SENSORTYPE_TYPE_E,
     * Y_SENSORTYPE_TYPE_J, Y_SENSORTYPE_TYPE_N, Y_SENSORTYPE_TYPE_R, Y_SENSORTYPE_TYPE_S,
     * Y_SENSORTYPE_TYPE_T, Y_SENSORTYPE_PT100_4WIRES, Y_SENSORTYPE_PT100_3WIRES,
     * Y_SENSORTYPE_PT100_2WIRES, Y_SENSORTYPE_RES_OHM, Y_SENSORTYPE_RES_NTC, Y_SENSORTYPE_RES_LINEAR and
     * Y_SENSORTYPE_RES_INTERNAL corresponding to the temperature sensor type
     *
     * On failure, throws an exception or returns Y_SENSORTYPE_INVALID.
     */
    Y_SENSORTYPE_enum   get_sensorType(void);

    inline Y_SENSORTYPE_enum sensorType(void)
    { return this->get_sensorType(); }

    /**
     * Changes the temperature sensor type.  This function is used
     * to define the type of thermocouple (K,E...) used with the device.
     * It has no effect if module is using a digital sensor or a thermistor.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : a value among Y_SENSORTYPE_DIGITAL, Y_SENSORTYPE_TYPE_K, Y_SENSORTYPE_TYPE_E,
     * Y_SENSORTYPE_TYPE_J, Y_SENSORTYPE_TYPE_N, Y_SENSORTYPE_TYPE_R, Y_SENSORTYPE_TYPE_S,
     * Y_SENSORTYPE_TYPE_T, Y_SENSORTYPE_PT100_4WIRES, Y_SENSORTYPE_PT100_3WIRES,
     * Y_SENSORTYPE_PT100_2WIRES, Y_SENSORTYPE_RES_OHM, Y_SENSORTYPE_RES_NTC, Y_SENSORTYPE_RES_LINEAR and
     * Y_SENSORTYPE_RES_INTERNAL corresponding to the temperature sensor type
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_sensorType(Y_SENSORTYPE_enum newval);
    inline int      setSensorType(Y_SENSORTYPE_enum newval)
    { return this->set_sensorType(newval); }

    /**
     * Returns the current value of the electrical signal measured by the sensor.
     *
     * @return a floating point number corresponding to the current value of the electrical signal
     * measured by the sensor
     *
     * On failure, throws an exception or returns Y_SIGNALVALUE_INVALID.
     */
    double              get_signalValue(void);

    inline double       signalValue(void)
    { return this->get_signalValue(); }

    /**
     * Returns the measuring unit of the electrical signal used by the sensor.
     *
     * @return a string corresponding to the measuring unit of the electrical signal used by the sensor
     *
     * On failure, throws an exception or returns Y_SIGNALUNIT_INVALID.
     */
    string              get_signalUnit(void);

    inline string       signalUnit(void)
    { return this->get_signalUnit(); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a temperature sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the temperature sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YTemperature.isOnline() to test if the temperature sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a temperature sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the temperature sensor
     *
     * @return a YTemperature object allowing you to drive the temperature sensor.
     */
    static YTemperature* FindTemperature(string func);

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
    virtual int         registerValueCallback(YTemperatureValueCallback callback);
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
    virtual int         registerTimedReportCallback(YTemperatureTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);

    /**
     * Configures NTC thermistor parameters in order to properly compute the temperature from
     * the measured resistance. For increased precision, you can enter a complete mapping
     * table using set_thermistorResponseTable. This function can only be used with a
     * temperature sensor based on thermistors.
     *
     * @param res25 : thermistor resistance at 25 degrees Celsius
     * @param beta : Beta value
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_ntcParameters(double res25,double beta);

    /**
     * Records a thermistor response table, in order to interpolate the temperature from
     * the measured resistance. This function can only be used with a temperature
     * sensor based on thermistors.
     *
     * @param tempValues : array of floating point numbers, corresponding to all
     *         temperatures (in degrees Celcius) for which the resistance of the
     *         thermistor is specified.
     * @param resValues : array of floating point numbers, corresponding to the resistance
     *         values (in Ohms) for each of the temperature included in the first
     *         argument, index by index.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_thermistorResponseTable(vector<double> tempValues,vector<double> resValues);

    /**
     * Retrieves the thermistor response table previously configured using the
     * set_thermistorResponseTable function. This function can only be used with a
     * temperature sensor based on thermistors.
     *
     * @param tempValues : array of floating point numbers, that is filled by the function
     *         with all temperatures (in degrees Celcius) for which the resistance
     *         of the thermistor is specified.
     * @param resValues : array of floating point numbers, that is filled by the function
     *         with the value (in Ohms) for each of the temperature included in the
     *         first argument, index by index.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         loadThermistorResponseTable(vector<double>& tempValues,vector<double>& resValues);


    inline static YTemperature* Find(string func)
    { return YTemperature::FindTemperature(func); }

    /**
     * Continues the enumeration of temperature sensors started using yFirstTemperature().
     *
     * @return a pointer to a YTemperature object, corresponding to
     *         a temperature sensor currently online, or a NULL pointer
     *         if there are no more temperature sensors to enumerate.
     */
           YTemperature    *nextTemperature(void);
    inline YTemperature    *next(void)
    { return this->nextTemperature();}

    /**
     * Starts the enumeration of temperature sensors currently accessible.
     * Use the method YTemperature.nextTemperature() to iterate on
     * next temperature sensors.
     *
     * @return a pointer to a YTemperature object, corresponding to
     *         the first temperature sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YTemperature* FirstTemperature(void);
    inline static YTemperature* First(void)
    { return YTemperature::FirstTemperature();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YTemperature accessors declaration)
};

//--- (YTemperature functions declaration)

/**
 * Retrieves a temperature sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the temperature sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YTemperature.isOnline() to test if the temperature sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a temperature sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the temperature sensor
 *
 * @return a YTemperature object allowing you to drive the temperature sensor.
 */
inline YTemperature* yFindTemperature(const string& func)
{ return YTemperature::FindTemperature(func);}
/**
 * Starts the enumeration of temperature sensors currently accessible.
 * Use the method YTemperature.nextTemperature() to iterate on
 * next temperature sensors.
 *
 * @return a pointer to a YTemperature object, corresponding to
 *         the first temperature sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YTemperature* yFirstTemperature(void)
{ return YTemperature::FirstTemperature();}

//--- (end of YTemperature functions declaration)

#endif
