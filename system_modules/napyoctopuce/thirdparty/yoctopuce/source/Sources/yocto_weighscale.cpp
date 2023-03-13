/*********************************************************************
 *
 * $Id: yocto_weighscale.cpp 29804 2018-01-30 18:05:21Z mvuilleu $
 *
 * Implements yFindWeighScale(), the high-level API for WeighScale functions
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
#include "yocto_weighscale.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "weighscale"

YWeighScale::YWeighScale(const string& func): YSensor(func)
//--- (YWeighScale initialization)
    ,_excitation(EXCITATION_INVALID)
    ,_compTempAdaptRatio(COMPTEMPADAPTRATIO_INVALID)
    ,_compTempAvg(COMPTEMPAVG_INVALID)
    ,_compTempChg(COMPTEMPCHG_INVALID)
    ,_compensation(COMPENSATION_INVALID)
    ,_zeroTracking(ZEROTRACKING_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackWeighScale(NULL)
    ,_timedReportCallbackWeighScale(NULL)
//--- (end of YWeighScale initialization)
{
    _className="WeighScale";
}

YWeighScale::~YWeighScale()
{
//--- (YWeighScale cleanup)
//--- (end of YWeighScale cleanup)
}
//--- (YWeighScale implementation)
// static attributes
const double YWeighScale::COMPTEMPADAPTRATIO_INVALID = YAPI_INVALID_DOUBLE;
const double YWeighScale::COMPTEMPAVG_INVALID = YAPI_INVALID_DOUBLE;
const double YWeighScale::COMPTEMPCHG_INVALID = YAPI_INVALID_DOUBLE;
const double YWeighScale::COMPENSATION_INVALID = YAPI_INVALID_DOUBLE;
const double YWeighScale::ZEROTRACKING_INVALID = YAPI_INVALID_DOUBLE;
const string YWeighScale::COMMAND_INVALID = YAPI_INVALID_STRING;

int YWeighScale::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("excitation")) {
        _excitation =  (Y_EXCITATION_enum)json_val->getInt("excitation");
    }
    if(json_val->has("compTempAdaptRatio")) {
        _compTempAdaptRatio =  floor(json_val->getDouble("compTempAdaptRatio") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("compTempAvg")) {
        _compTempAvg =  floor(json_val->getDouble("compTempAvg") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("compTempChg")) {
        _compTempChg =  floor(json_val->getDouble("compTempChg") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("compensation")) {
        _compensation =  floor(json_val->getDouble("compensation") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("zeroTracking")) {
        _zeroTracking =  floor(json_val->getDouble("zeroTracking") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Changes the measuring unit for the weight.
 * Remember to call the saveToFlash() method of the module if the
 * modification must be kept.
 *
 * @param newval : a string corresponding to the measuring unit for the weight
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_unit(const string& newval)
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
 * Returns the current load cell bridge excitation method.
 *
 * @return a value among Y_EXCITATION_OFF, Y_EXCITATION_DC and Y_EXCITATION_AC corresponding to the
 * current load cell bridge excitation method
 *
 * On failure, throws an exception or returns Y_EXCITATION_INVALID.
 */
Y_EXCITATION_enum YWeighScale::get_excitation(void)
{
    Y_EXCITATION_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::EXCITATION_INVALID;
                }
            }
        }
        res = _excitation;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the current load cell bridge excitation method.
 *
 * @param newval : a value among Y_EXCITATION_OFF, Y_EXCITATION_DC and Y_EXCITATION_AC corresponding
 * to the current load cell bridge excitation method
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_excitation(Y_EXCITATION_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("excitation", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the averaged temperature update rate, in percents.
 * The averaged temperature is updated every 10 seconds, by applying this adaptation rate
 * to the difference between the measures ambiant temperature and the current compensation
 * temperature. The standard rate is 0.04 percents, and the maximal rate is 65 percents.
 *
 * @param newval : a floating point number corresponding to the averaged temperature update rate, in percents
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_compTempAdaptRatio(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("compTempAdaptRatio", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the averaged temperature update rate, in percents.
 * The averaged temperature is updated every 10 seconds, by applying this adaptation rate
 * to the difference between the measures ambiant temperature and the current compensation
 * temperature. The standard rate is 0.04 percents, and the maximal rate is 65 percents.
 *
 * @return a floating point number corresponding to the averaged temperature update rate, in percents
 *
 * On failure, throws an exception or returns Y_COMPTEMPADAPTRATIO_INVALID.
 */
double YWeighScale::get_compTempAdaptRatio(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::COMPTEMPADAPTRATIO_INVALID;
                }
            }
        }
        res = _compTempAdaptRatio;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current averaged temperature, used for thermal compensation.
 *
 * @return a floating point number corresponding to the current averaged temperature, used for thermal compensation
 *
 * On failure, throws an exception or returns Y_COMPTEMPAVG_INVALID.
 */
double YWeighScale::get_compTempAvg(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::COMPTEMPAVG_INVALID;
                }
            }
        }
        res = _compTempAvg;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current temperature variation, used for thermal compensation.
 *
 * @return a floating point number corresponding to the current temperature variation, used for
 * thermal compensation
 *
 * On failure, throws an exception or returns Y_COMPTEMPCHG_INVALID.
 */
double YWeighScale::get_compTempChg(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::COMPTEMPCHG_INVALID;
                }
            }
        }
        res = _compTempChg;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current current thermal compensation value.
 *
 * @return a floating point number corresponding to the current current thermal compensation value
 *
 * On failure, throws an exception or returns Y_COMPENSATION_INVALID.
 */
double YWeighScale::get_compensation(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::COMPENSATION_INVALID;
                }
            }
        }
        res = _compensation;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the zero tracking threshold value. When this threshold is larger than
 * zero, any measure under the threshold will automatically be ignored and the
 * zero compensation will be updated.
 *
 * @param newval : a floating point number corresponding to the zero tracking threshold value
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_zeroTracking(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("zeroTracking", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the zero tracking threshold value. When this threshold is larger than
 * zero, any measure under the threshold will automatically be ignored and the
 * zero compensation will be updated.
 *
 * @return a floating point number corresponding to the zero tracking threshold value
 *
 * On failure, throws an exception or returns Y_ZEROTRACKING_INVALID.
 */
double YWeighScale::get_zeroTracking(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::ZEROTRACKING_INVALID;
                }
            }
        }
        res = _zeroTracking;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YWeighScale::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YWeighScale::COMMAND_INVALID;
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

int YWeighScale::set_command(const string& newval)
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
 * Retrieves a weighing scale sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the weighing scale sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YWeighScale.isOnline() to test if the weighing scale sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a weighing scale sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the weighing scale sensor
 *
 * @return a YWeighScale object allowing you to drive the weighing scale sensor.
 */
YWeighScale* YWeighScale::FindWeighScale(string func)
{
    YWeighScale* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YWeighScale*) YFunction::_FindFromCache("WeighScale", func);
        if (obj == NULL) {
            obj = new YWeighScale(func);
            YFunction::_AddToCache("WeighScale", func, obj);
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
int YWeighScale::registerValueCallback(YWeighScaleValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackWeighScale = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YWeighScale::_invokeValueCallback(string value)
{
    if (_valueCallbackWeighScale != NULL) {
        _valueCallbackWeighScale(this, value);
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
int YWeighScale::registerTimedReportCallback(YWeighScaleTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackWeighScale = callback;
    return 0;
}

int YWeighScale::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackWeighScale != NULL) {
        _timedReportCallbackWeighScale(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

/**
 * Adapts the load cell signal bias (stored in the corresponding genericSensor)
 * so that the current signal corresponds to a zero weight.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::tare(void)
{
    return this->set_command("T");
}

/**
 * Configures the load cell span parameters (stored in the corresponding genericSensor)
 * so that the current signal corresponds to the specified reference weight.
 *
 * @param currWeight : reference weight presently on the load cell.
 * @param maxWeight : maximum weight to be expectect on the load cell.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::setupSpan(double currWeight,double maxWeight)
{
    return this->set_command(YapiWrapper::ysprintf("S%d:%d", (int) floor(1000*currWeight+0.5),(int) floor(1000*maxWeight+0.5)));
}

int YWeighScale::setCompensationTable(int tableIndex,vector<double> tempValues,vector<double> compValues)
{
    int siz = 0;
    int res = 0;
    int idx = 0;
    int found = 0;
    double prev = 0.0;
    double curr = 0.0;
    double currComp = 0.0;
    double idxTemp = 0.0;
    siz = (int)tempValues.size();
    if (!(siz != 1)) {
        _throw(YAPI_INVALID_ARGUMENT,"thermal compensation table must have at least two points");
        return YAPI_INVALID_ARGUMENT;
    }
    if (!(siz == (int)compValues.size())) {
        _throw(YAPI_INVALID_ARGUMENT,"table sizes mismatch");
        return YAPI_INVALID_ARGUMENT;
    }

    res = this->set_command(YapiWrapper::ysprintf("%dZ",tableIndex));
    if (!(res==YAPI_SUCCESS)) {
        _throw(YAPI_IO_ERROR,"unable to reset thermal compensation table");
        return YAPI_IO_ERROR;
    }
    // add records in growing temperature value
    found = 1;
    prev = -999999.0;
    while (found > 0) {
        found = 0;
        curr = 99999999.0;
        currComp = -999999.0;
        idx = 0;
        while (idx < siz) {
            idxTemp = tempValues[idx];
            if ((idxTemp > prev) && (idxTemp < curr)) {
                curr = idxTemp;
                currComp = compValues[idx];
                found = 1;
            }
            idx = idx + 1;
        }
        if (found > 0) {
            res = this->set_command(YapiWrapper::ysprintf("%dm%d:%d", tableIndex, (int) floor(1000*curr+0.5),(int) floor(1000*currComp+0.5)));
            if (!(res==YAPI_SUCCESS)) {
                _throw(YAPI_IO_ERROR,"unable to set thermal compensation table");
                return YAPI_IO_ERROR;
            }
            prev = curr;
        }
    }
    return YAPI_SUCCESS;
}

int YWeighScale::loadCompensationTable(int tableIndex,vector<double>& tempValues,vector<double>& compValues)
{
    string id;
    string bin_json;
    vector<string> paramlist;
    int siz = 0;
    int idx = 0;
    double temp = 0.0;
    double comp = 0.0;

    id = this->get_functionId();
    id = (id).substr( 10, (int)(id).length() - 10);
    bin_json = this->_download(YapiWrapper::ysprintf("extra.json?page=%d",(4*atoi((id).c_str()))+tableIndex));
    paramlist = this->_json_get_array(bin_json);
    // convert all values to float and append records
    siz = (((int)paramlist.size()) >> (1));
    tempValues.clear();
    compValues.clear();
    idx = 0;
    while (idx < siz) {
        temp = atof((paramlist[2*idx]).c_str())/1000.0;
        comp = atof((paramlist[2*idx+1]).c_str())/1000.0;
        tempValues.push_back(temp);
        compValues.push_back(comp);
        idx = idx + 1;
    }
    return YAPI_SUCCESS;
}

/**
 * Records a weight offset thermal compensation table, in order to automatically correct the
 * measured weight based on the averaged compensation temperature.
 * The weight correction will be applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, corresponding to all averaged
 *         temperatures for which an offset correction is specified.
 * @param compValues : array of floating point numbers, corresponding to the offset correction
 *         to apply for each of the temperature included in the first
 *         argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_offsetAvgCompensationTable(vector<double> tempValues,vector<double> compValues)
{
    return this->setCompensationTable(0, tempValues, compValues);
}

/**
 * Retrieves the weight offset thermal compensation table previously configured using the
 * set_offsetAvgCompensationTable function.
 * The weight correction is applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, that is filled by the function
 *         with all averaged temperatures for which an offset correction is specified.
 * @param compValues : array of floating point numbers, that is filled by the function
 *         with the offset correction applied for each of the temperature
 *         included in the first argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::loadOffsetAvgCompensationTable(vector<double>& tempValues,vector<double>& compValues)
{
    return this->loadCompensationTable(0, tempValues, compValues);
}

/**
 * Records a weight offset thermal compensation table, in order to automatically correct the
 * measured weight based on the variation of temperature.
 * The weight correction will be applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, corresponding to temperature
 *         variations for which an offset correction is specified.
 * @param compValues : array of floating point numbers, corresponding to the offset correction
 *         to apply for each of the temperature variation included in the first
 *         argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_offsetChgCompensationTable(vector<double> tempValues,vector<double> compValues)
{
    return this->setCompensationTable(1, tempValues, compValues);
}

/**
 * Retrieves the weight offset thermal compensation table previously configured using the
 * set_offsetChgCompensationTable function.
 * The weight correction is applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, that is filled by the function
 *         with all temperature variations for which an offset correction is specified.
 * @param compValues : array of floating point numbers, that is filled by the function
 *         with the offset correction applied for each of the temperature
 *         variation included in the first argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::loadOffsetChgCompensationTable(vector<double>& tempValues,vector<double>& compValues)
{
    return this->loadCompensationTable(1, tempValues, compValues);
}

/**
 * Records a weight span thermal compensation table, in order to automatically correct the
 * measured weight based on the compensation temperature.
 * The weight correction will be applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, corresponding to all averaged
 *         temperatures for which a span correction is specified.
 * @param compValues : array of floating point numbers, corresponding to the span correction
 *         (in percents) to apply for each of the temperature included in the first
 *         argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_spanAvgCompensationTable(vector<double> tempValues,vector<double> compValues)
{
    return this->setCompensationTable(2, tempValues, compValues);
}

/**
 * Retrieves the weight span thermal compensation table previously configured using the
 * set_spanAvgCompensationTable function.
 * The weight correction is applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, that is filled by the function
 *         with all averaged temperatures for which an span correction is specified.
 * @param compValues : array of floating point numbers, that is filled by the function
 *         with the span correction applied for each of the temperature
 *         included in the first argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::loadSpanAvgCompensationTable(vector<double>& tempValues,vector<double>& compValues)
{
    return this->loadCompensationTable(2, tempValues, compValues);
}

/**
 * Records a weight span thermal compensation table, in order to automatically correct the
 * measured weight based on the variation of temperature.
 * The weight correction will be applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, corresponding to all variations of
 *         temperatures for which a span correction is specified.
 * @param compValues : array of floating point numbers, corresponding to the span correction
 *         (in percents) to apply for each of the temperature variation included
 *         in the first argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::set_spanChgCompensationTable(vector<double> tempValues,vector<double> compValues)
{
    return this->setCompensationTable(3, tempValues, compValues);
}

/**
 * Retrieves the weight span thermal compensation table previously configured using the
 * set_spanChgCompensationTable function.
 * The weight correction is applied by linear interpolation between specified points.
 *
 * @param tempValues : array of floating point numbers, that is filled by the function
 *         with all variation of temperature for which an span correction is specified.
 * @param compValues : array of floating point numbers, that is filled by the function
 *         with the span correction applied for each of variation of temperature
 *         included in the first argument, index by index.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YWeighScale::loadSpanChgCompensationTable(vector<double>& tempValues,vector<double>& compValues)
{
    return this->loadCompensationTable(3, tempValues, compValues);
}

YWeighScale *YWeighScale::nextWeighScale(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YWeighScale::FindWeighScale(hwid);
}

YWeighScale* YWeighScale::FirstWeighScale(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("WeighScale", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YWeighScale::FindWeighScale(serial+"."+funcId);
}

//--- (end of YWeighScale implementation)

//--- (YWeighScale functions)
//--- (end of YWeighScale functions)
