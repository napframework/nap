/*********************************************************************
 *
 * $Id: yocto_temperature.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindTemperature(), the high-level API for Temperature functions
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


#define _CRT_SECURE_NO_DEPRECATE //do not use windows secure crt
#include "yocto_temperature.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "temperature"

YTemperature::YTemperature(const string& func): YSensor(func)
//--- (YTemperature initialization)
    ,_sensorType(SENSORTYPE_INVALID)
    ,_signalValue(SIGNALVALUE_INVALID)
    ,_signalUnit(SIGNALUNIT_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackTemperature(NULL)
    ,_timedReportCallbackTemperature(NULL)
//--- (end of YTemperature initialization)
{
    _className="Temperature";
}

YTemperature::~YTemperature()
{
//--- (YTemperature cleanup)
//--- (end of YTemperature cleanup)
}
//--- (YTemperature implementation)
// static attributes
const double YTemperature::SIGNALVALUE_INVALID = YAPI_INVALID_DOUBLE;
const string YTemperature::SIGNALUNIT_INVALID = YAPI_INVALID_STRING;
const string YTemperature::COMMAND_INVALID = YAPI_INVALID_STRING;

int YTemperature::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("sensorType")) {
        _sensorType =  (Y_SENSORTYPE_enum)json_val->getInt("sensorType");
    }
    if(json_val->has("signalValue")) {
        _signalValue =  floor(json_val->getDouble("signalValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("signalUnit")) {
        _signalUnit =  json_val->getString("signalUnit");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YSensor::_parseAttr(json_val);
}


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
int YTemperature::set_unit(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("unit", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
Y_SENSORTYPE_enum YTemperature::get_sensorType(void)
{
    Y_SENSORTYPE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YTemperature::SENSORTYPE_INVALID;
                }
            }
        }
        res = _sensorType;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
int YTemperature::set_sensorType(Y_SENSORTYPE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("sensorType", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current value of the electrical signal measured by the sensor.
 *
 * @return a floating point number corresponding to the current value of the electrical signal
 * measured by the sensor
 *
 * On failure, throws an exception or returns Y_SIGNALVALUE_INVALID.
 */
double YTemperature::get_signalValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YTemperature::SIGNALVALUE_INVALID;
                }
            }
        }
        res = floor(_signalValue * 1000+0.5) / 1000;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the measuring unit of the electrical signal used by the sensor.
 *
 * @return a string corresponding to the measuring unit of the electrical signal used by the sensor
 *
 * On failure, throws an exception or returns Y_SIGNALUNIT_INVALID.
 */
string YTemperature::get_signalUnit(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration == 0) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YTemperature::SIGNALUNIT_INVALID;
                }
            }
        }
        res = _signalUnit;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YTemperature::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YTemperature::COMMAND_INVALID;
                }
            }
        }
        res = _command;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YTemperature::set_command(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("command", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
YTemperature* YTemperature::FindTemperature(string func)
{
    YTemperature* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YTemperature*) YFunction::_FindFromCache("Temperature", func);
        if (obj == NULL) {
            obj = new YTemperature(func);
            YFunction::_AddToCache("Temperature", func, obj);
        }
    } catch (std::exception) {
        if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
        throw;
    }
    if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
    return obj;
}

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
int YTemperature::registerValueCallback(YTemperatureValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackTemperature = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YTemperature::_invokeValueCallback(string value)
{
    if (_valueCallbackTemperature != NULL) {
        _valueCallbackTemperature(this, value);
    } else {
        YSensor::_invokeValueCallback(value);
    }
    return 0;
}

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
int YTemperature::registerTimedReportCallback(YTemperatureTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackTemperature = callback;
    return 0;
}

int YTemperature::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackTemperature != NULL) {
        _timedReportCallbackTemperature(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

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
int YTemperature::set_ntcParameters(double res25,double beta)
{
    double t0 = 0.0;
    double t1 = 0.0;
    double res100 = 0.0;
    vector<double> tempValues;
    vector<double> resValues;
    t0 = 25.0+275.15;
    t1 = 100.0+275.15;
    res100 = res25 * exp(beta*(1.0/t1 - 1.0/t0));
    tempValues.clear();
    resValues.clear();
    tempValues.push_back(25.0);
    resValues.push_back(res25);
    tempValues.push_back(100.0);
    resValues.push_back(res100);
    return this->set_thermistorResponseTable(tempValues, resValues);
}

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
int YTemperature::set_thermistorResponseTable(vector<double> tempValues,vector<double> resValues)
{
    int siz = 0;
    int res = 0;
    int idx = 0;
    int found = 0;
    double prev = 0.0;
    double curr = 0.0;
    double currTemp = 0.0;
    double idxres = 0.0;
    siz = (int)tempValues.size();
    if (!(siz >= 2)) {
        _throw(YAPI_INVALID_ARGUMENT,"thermistor response table must have at least two points");
        return YAPI_INVALID_ARGUMENT;
    }
    if (!(siz == (int)resValues.size())) {
        _throw(YAPI_INVALID_ARGUMENT,"table sizes mismatch");
        return YAPI_INVALID_ARGUMENT;
    }

    res = this->set_command("Z");
    if (!(res==YAPI_SUCCESS)) {
        _throw(YAPI_IO_ERROR,"unable to reset thermistor parameters");
        return YAPI_IO_ERROR;
    }
    // add records in growing resistance value
    found = 1;
    prev = 0.0;
    while (found > 0) {
        found = 0;
        curr = 99999999.0;
        currTemp = -999999.0;
        idx = 0;
        while (idx < siz) {
            idxres = resValues[idx];
            if ((idxres > prev) && (idxres < curr)) {
                curr = idxres;
                currTemp = tempValues[idx];
                found = 1;
            }
            idx = idx + 1;
        }
        if (found > 0) {
            res = this->set_command(YapiWrapper::ysprintf("m%d:%d", (int) floor(1000*curr+0.5),(int) floor(1000*currTemp+0.5)));
            if (!(res==YAPI_SUCCESS)) {
                _throw(YAPI_IO_ERROR,"unable to reset thermistor parameters");
                return YAPI_IO_ERROR;
            }
            prev = curr;
        }
    }
    return YAPI_SUCCESS;
}

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
int YTemperature::loadThermistorResponseTable(vector<double>& tempValues,vector<double>& resValues)
{
    string id;
    string bin_json;
    vector<string> paramlist;
    vector<double> templist;
    int siz = 0;
    int idx = 0;
    double temp = 0.0;
    int found = 0;
    double prev = 0.0;
    double curr = 0.0;
    double currRes = 0.0;
    tempValues.clear();
    resValues.clear();

    id = this->get_functionId();
    id = (id).substr( 11, (int)(id).length() - 11);
    bin_json = this->_download(YapiWrapper::ysprintf("extra.json?page=%s",id.c_str()));
    paramlist = this->_json_get_array(bin_json);
    // first convert all temperatures to float
    siz = (((int)paramlist.size()) >> (1));
    templist.clear();
    idx = 0;
    while (idx < siz) {
        temp = atof((paramlist[2*idx+1]).c_str())/1000.0;
        templist.push_back(temp);
        idx = idx + 1;
    }
    // then add records in growing temperature value
    tempValues.clear();
    resValues.clear();
    found = 1;
    prev = -999999.0;
    while (found > 0) {
        found = 0;
        curr = 999999.0;
        currRes = -999999.0;
        idx = 0;
        while (idx < siz) {
            temp = templist[idx];
            if ((temp > prev) && (temp < curr)) {
                curr = temp;
                currRes = atof((paramlist[2*idx]).c_str())/1000.0;
                found = 1;
            }
            idx = idx + 1;
        }
        if (found > 0) {
            tempValues.push_back(curr);
            resValues.push_back(currRes);
            prev = curr;
        }
    }
    return YAPI_SUCCESS;
}

YTemperature *YTemperature::nextTemperature(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YTemperature::FindTemperature(hwid);
}

YTemperature* YTemperature::FirstTemperature(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Temperature", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YTemperature::FindTemperature(serial+"."+funcId);
}

//--- (end of YTemperature implementation)

//--- (YTemperature functions)
//--- (end of YTemperature functions)
