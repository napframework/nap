/*********************************************************************
 *
 * $Id: yocto_gyro.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindGyro(), the high-level API for Gyro functions
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
#include "yocto_gyro.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "gyro"



YQt::YQt(const string& func): YSensor(func)
//--- (generated code: YQt initialization)
    ,_valueCallbackQt(NULL)
    ,_timedReportCallbackQt(NULL)
//--- (end of generated code: YQt initialization)
{
    _className="Qt";
}

YQt::~YQt()
{
//--- (generated code: YQt cleanup)
//--- (end of generated code: YQt cleanup)
}
//--- (generated code: YQt implementation)
// static attributes


/**
 * Retrieves a quaternion component for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the quaternion component is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YQt.isOnline() to test if the quaternion component is
 * indeed online at a given time. In case of ambiguity when looking for
 * a quaternion component by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the quaternion component
 *
 * @return a YQt object allowing you to drive the quaternion component.
 */
YQt* YQt::FindQt(string func)
{
    YQt* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YQt*) YFunction::_FindFromCache("Qt", func);
        if (obj == NULL) {
            obj = new YQt(func);
            YFunction::_AddToCache("Qt", func, obj);
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
int YQt::registerValueCallback(YQtValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackQt = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YQt::_invokeValueCallback(string value)
{
    if (_valueCallbackQt != NULL) {
        _valueCallbackQt(this, value);
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
int YQt::registerTimedReportCallback(YQtTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackQt = callback;
    return 0;
}

int YQt::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackQt != NULL) {
        _timedReportCallbackQt(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

YQt *YQt::nextQt(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YQt::FindQt(hwid);
}

YQt* YQt::FirstQt(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Qt", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YQt::FindQt(serial+"."+funcId);
}

//--- (end of generated code: YQt implementation)

//--- (generated code: YQt functions)
//--- (end of generated code: YQt functions)


static void yInternalGyroCallback(YQt *obj, const string& value)
{
    YGyro *gyro = (YGyro*) obj->get_userData();
    if (gyro == NULL) {
        return;
    }
    string tmp = obj->get_functionId().substr(2);
    int idx = atoi(tmp.c_str());
    double dbl_value = atof(value.c_str());
    gyro->_invokeGyroCallbacks(idx, dbl_value);
}




YGyro::YGyro(const string& func): YSensor(func)
//--- (generated code: YGyro initialization)
    ,_bandwidth(BANDWIDTH_INVALID)
    ,_xValue(XVALUE_INVALID)
    ,_yValue(YVALUE_INVALID)
    ,_zValue(ZVALUE_INVALID)
    ,_valueCallbackGyro(NULL)
    ,_timedReportCallbackGyro(NULL)
    ,_qt_stamp(0)
    ,_qt_w(NULL)
    ,_qt_x(NULL)
    ,_qt_y(NULL)
    ,_qt_z(NULL)
    ,_w(0.0)
    ,_x(0.0)
    ,_y(0.0)
    ,_z(0.0)
    ,_angles_stamp(0)
    ,_head(0.0)
    ,_pitch(0.0)
    ,_roll(0.0)
    ,_quatCallback(NULL)
    ,_anglesCallback(NULL)
//--- (end of generated code: YGyro initialization)
{
    _className="Gyro";
}

YGyro::~YGyro()
{
//--- (generated code: YGyro cleanup)
//--- (end of generated code: YGyro cleanup)
}
//--- (generated code: YGyro implementation)
// static attributes
const double YGyro::XVALUE_INVALID = YAPI_INVALID_DOUBLE;
const double YGyro::YVALUE_INVALID = YAPI_INVALID_DOUBLE;
const double YGyro::ZVALUE_INVALID = YAPI_INVALID_DOUBLE;

int YGyro::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("bandwidth")) {
        _bandwidth =  json_val->getInt("bandwidth");
    }
    if(json_val->has("xValue")) {
        _xValue =  floor(json_val->getDouble("xValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("yValue")) {
        _yValue =  floor(json_val->getDouble("yValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("zValue")) {
        _zValue =  floor(json_val->getDouble("zValue") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    return YSensor::_parseAttr(json_val);
}


/**
 * Returns the measure update frequency, measured in Hz (Yocto-3D-V2 only).
 *
 * @return an integer corresponding to the measure update frequency, measured in Hz (Yocto-3D-V2 only)
 *
 * On failure, throws an exception or returns Y_BANDWIDTH_INVALID.
 */
int YGyro::get_bandwidth(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YGyro::BANDWIDTH_INVALID;
                }
            }
        }
        res = _bandwidth;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
int YGyro::set_bandwidth(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("bandwidth", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the angular velocity around the X axis of the device, as a floating point number.
 *
 * @return a floating point number corresponding to the angular velocity around the X axis of the
 * device, as a floating point number
 *
 * On failure, throws an exception or returns Y_XVALUE_INVALID.
 */
double YGyro::get_xValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YGyro::XVALUE_INVALID;
                }
            }
        }
        res = _xValue;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the angular velocity around the Y axis of the device, as a floating point number.
 *
 * @return a floating point number corresponding to the angular velocity around the Y axis of the
 * device, as a floating point number
 *
 * On failure, throws an exception or returns Y_YVALUE_INVALID.
 */
double YGyro::get_yValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YGyro::YVALUE_INVALID;
                }
            }
        }
        res = _yValue;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the angular velocity around the Z axis of the device, as a floating point number.
 *
 * @return a floating point number corresponding to the angular velocity around the Z axis of the
 * device, as a floating point number
 *
 * On failure, throws an exception or returns Y_ZVALUE_INVALID.
 */
double YGyro::get_zValue(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YGyro::ZVALUE_INVALID;
                }
            }
        }
        res = _zValue;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a gyroscope for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the gyroscope is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YGyro.isOnline() to test if the gyroscope is
 * indeed online at a given time. In case of ambiguity when looking for
 * a gyroscope by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the gyroscope
 *
 * @return a YGyro object allowing you to drive the gyroscope.
 */
YGyro* YGyro::FindGyro(string func)
{
    YGyro* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YGyro*) YFunction::_FindFromCache("Gyro", func);
        if (obj == NULL) {
            obj = new YGyro(func);
            YFunction::_AddToCache("Gyro", func, obj);
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
int YGyro::registerValueCallback(YGyroValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackGyro = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YGyro::_invokeValueCallback(string value)
{
    if (_valueCallbackGyro != NULL) {
        _valueCallbackGyro(this, value);
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
int YGyro::registerTimedReportCallback(YGyroTimedReportCallback callback)
{
    YSensor* sensor = NULL;
    sensor = this;
    if (callback != NULL) {
        YFunction::_UpdateTimedReportCallbackList(sensor, true);
    } else {
        YFunction::_UpdateTimedReportCallbackList(sensor, false);
    }
    _timedReportCallbackGyro = callback;
    return 0;
}

int YGyro::_invokeTimedReportCallback(YMeasure value)
{
    if (_timedReportCallbackGyro != NULL) {
        _timedReportCallbackGyro(this, value);
    } else {
        YSensor::_invokeTimedReportCallback(value);
    }
    return 0;
}

int YGyro::_loadQuaternion(void)
{
    int now_stamp = 0;
    int age_ms = 0;
    now_stamp = (int) ((YAPI::GetTickCount()) & (0x7FFFFFFF));
    age_ms = (((now_stamp - _qt_stamp)) & (0x7FFFFFFF));
    if ((age_ms >= 10) || (_qt_stamp == 0)) {
        if (this->_load_unsafe(10) != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        if (_qt_stamp == 0) {
            _qt_w = YQt::FindQt(YapiWrapper::ysprintf("%s.qt1",_serial.c_str()));
            _qt_x = YQt::FindQt(YapiWrapper::ysprintf("%s.qt2",_serial.c_str()));
            _qt_y = YQt::FindQt(YapiWrapper::ysprintf("%s.qt3",_serial.c_str()));
            _qt_z = YQt::FindQt(YapiWrapper::ysprintf("%s.qt4",_serial.c_str()));
        }
        if (_qt_w->load(9) != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        if (_qt_x->load(9) != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        if (_qt_y->load(9) != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        if (_qt_z->load(9) != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        _w = _qt_w->get_currentValue();
        _x = _qt_x->get_currentValue();
        _y = _qt_y->get_currentValue();
        _z = _qt_z->get_currentValue();
        _qt_stamp = now_stamp;
    }
    return YAPI_SUCCESS;
}

int YGyro::_loadAngles(void)
{
    double sqw = 0.0;
    double sqx = 0.0;
    double sqy = 0.0;
    double sqz = 0.0;
    double norm = 0.0;
    double delta = 0.0;

    if (this->_loadQuaternion() != YAPI_SUCCESS) {
        return YAPI_DEVICE_NOT_FOUND;
    }
    if (_angles_stamp != _qt_stamp) {
        sqw = _w * _w;
        sqx = _x * _x;
        sqy = _y * _y;
        sqz = _z * _z;
        norm = sqx + sqy + sqz + sqw;
        delta = _y * _w - _x * _z;
        if (delta > 0.499 * norm) {
            // singularity at north pole
            _pitch = 90.0;
            _head  = floor(2.0 * 1800.0/3.141592653589793238463 * atan2(_x,-_w)+0.5) / 10.0;
        } else {
            if (delta < -0.499 * norm) {
                // singularity at south pole
                _pitch = -90.0;
                _head  = floor(-2.0 * 1800.0/3.141592653589793238463 * atan2(_x,-_w)+0.5) / 10.0;
            } else {
                _roll  = floor(1800.0/3.141592653589793238463 * atan2(2.0 * (_w * _x + _y * _z),sqw - sqx - sqy + sqz)+0.5) / 10.0;
                _pitch = floor(1800.0/3.141592653589793238463 * asin(2.0 * delta / norm)+0.5) / 10.0;
                _head  = floor(1800.0/3.141592653589793238463 * atan2(2.0 * (_x * _y + _z * _w),sqw + sqx - sqy - sqz)+0.5) / 10.0;
            }
        }
        _angles_stamp = _qt_stamp;
    }
    return YAPI_SUCCESS;
}

/**
 * Returns the estimated roll angle, based on the integration of
 * gyroscopic measures combined with acceleration and
 * magnetic field measurements.
 * The axis corresponding to the roll angle can be mapped to any
 * of the device X, Y or Z physical directions using methods of
 * the class YRefFrame.
 *
 * @return a floating-point number corresponding to roll angle
 *         in degrees, between -180 and +180.
 */
double YGyro::get_roll(void)
{
    this->_loadAngles();
    return _roll;
}

/**
 * Returns the estimated pitch angle, based on the integration of
 * gyroscopic measures combined with acceleration and
 * magnetic field measurements.
 * The axis corresponding to the pitch angle can be mapped to any
 * of the device X, Y or Z physical directions using methods of
 * the class YRefFrame.
 *
 * @return a floating-point number corresponding to pitch angle
 *         in degrees, between -90 and +90.
 */
double YGyro::get_pitch(void)
{
    this->_loadAngles();
    return _pitch;
}

/**
 * Returns the estimated heading angle, based on the integration of
 * gyroscopic measures combined with acceleration and
 * magnetic field measurements.
 * The axis corresponding to the heading can be mapped to any
 * of the device X, Y or Z physical directions using methods of
 * the class YRefFrame.
 *
 * @return a floating-point number corresponding to heading
 *         in degrees, between 0 and 360.
 */
double YGyro::get_heading(void)
{
    this->_loadAngles();
    return _head;
}

/**
 * Returns the w component (real part) of the quaternion
 * describing the device estimated orientation, based on the
 * integration of gyroscopic measures combined with acceleration and
 * magnetic field measurements.
 *
 * @return a floating-point number corresponding to the w
 *         component of the quaternion.
 */
double YGyro::get_quaternionW(void)
{
    this->_loadQuaternion();
    return _w;
}

/**
 * Returns the x component of the quaternion
 * describing the device estimated orientation, based on the
 * integration of gyroscopic measures combined with acceleration and
 * magnetic field measurements. The x component is
 * mostly correlated with rotations on the roll axis.
 *
 * @return a floating-point number corresponding to the x
 *         component of the quaternion.
 */
double YGyro::get_quaternionX(void)
{
    this->_loadQuaternion();
    return _x;
}

/**
 * Returns the y component of the quaternion
 * describing the device estimated orientation, based on the
 * integration of gyroscopic measures combined with acceleration and
 * magnetic field measurements. The y component is
 * mostly correlated with rotations on the pitch axis.
 *
 * @return a floating-point number corresponding to the y
 *         component of the quaternion.
 */
double YGyro::get_quaternionY(void)
{
    this->_loadQuaternion();
    return _y;
}

/**
 * Returns the x component of the quaternion
 * describing the device estimated orientation, based on the
 * integration of gyroscopic measures combined with acceleration and
 * magnetic field measurements. The x component is
 * mostly correlated with changes of heading.
 *
 * @return a floating-point number corresponding to the z
 *         component of the quaternion.
 */
double YGyro::get_quaternionZ(void)
{
    this->_loadQuaternion();
    return _z;
}

/**
 * Registers a callback function that will be invoked each time that the estimated
 * device orientation has changed. The call frequency is typically around 95Hz during a move.
 * The callback is invoked only during the execution of ySleep or yHandleEvents.
 * This provides control over the time when the callback is triggered.
 * For good responsiveness, remember to call one of these two functions periodically.
 * To unregister a callback, pass a NULL pointer as argument.
 *
 * @param callback : the callback function to invoke, or a NULL pointer.
 *         The callback function should take five arguments:
 *         the YGyro object of the turning device, and the floating
 *         point values of the four components w, x, y and z
 *         (as floating-point numbers).
 * @noreturn
 */
int YGyro::registerQuaternionCallback(YQuatCallback callback)
{
    _quatCallback = callback;
    if (callback != NULL) {
        if (this->_loadQuaternion() != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        _qt_w->set_userData(this);
        _qt_x->set_userData(this);
        _qt_y->set_userData(this);
        _qt_z->set_userData(this);
        _qt_w->registerValueCallback(yInternalGyroCallback);
        _qt_x->registerValueCallback(yInternalGyroCallback);
        _qt_y->registerValueCallback(yInternalGyroCallback);
        _qt_z->registerValueCallback(yInternalGyroCallback);
    } else {
        if (!(_anglesCallback != NULL)) {
            _qt_w->registerValueCallback((YQtValueCallback) NULL);
            _qt_x->registerValueCallback((YQtValueCallback) NULL);
            _qt_y->registerValueCallback((YQtValueCallback) NULL);
            _qt_z->registerValueCallback((YQtValueCallback) NULL);
        }
    }
    return 0;
}

/**
 * Registers a callback function that will be invoked each time that the estimated
 * device orientation has changed. The call frequency is typically around 95Hz during a move.
 * The callback is invoked only during the execution of ySleep or yHandleEvents.
 * This provides control over the time when the callback is triggered.
 * For good responsiveness, remember to call one of these two functions periodically.
 * To unregister a callback, pass a NULL pointer as argument.
 *
 * @param callback : the callback function to invoke, or a NULL pointer.
 *         The callback function should take four arguments:
 *         the YGyro object of the turning device, and the floating
 *         point values of the three angles roll, pitch and heading
 *         in degrees (as floating-point numbers).
 * @noreturn
 */
int YGyro::registerAnglesCallback(YAnglesCallback callback)
{
    _anglesCallback = callback;
    if (callback != NULL) {
        if (this->_loadQuaternion() != YAPI_SUCCESS) {
            return YAPI_DEVICE_NOT_FOUND;
        }
        _qt_w->set_userData(this);
        _qt_x->set_userData(this);
        _qt_y->set_userData(this);
        _qt_z->set_userData(this);
        _qt_w->registerValueCallback(yInternalGyroCallback);
        _qt_x->registerValueCallback(yInternalGyroCallback);
        _qt_y->registerValueCallback(yInternalGyroCallback);
        _qt_z->registerValueCallback(yInternalGyroCallback);
    } else {
        if (!(_quatCallback != NULL)) {
            _qt_w->registerValueCallback((YQtValueCallback) NULL);
            _qt_x->registerValueCallback((YQtValueCallback) NULL);
            _qt_y->registerValueCallback((YQtValueCallback) NULL);
            _qt_z->registerValueCallback((YQtValueCallback) NULL);
        }
    }
    return 0;
}

int YGyro::_invokeGyroCallbacks(int qtIndex,double qtValue)
{
    switch(qtIndex - 1) {
    case 0:
        _w = qtValue;
        break;
    case 1:
        _x = qtValue;
        break;
    case 2:
        _y = qtValue;
        break;
    case 3:
        _z = qtValue;
        break;
    }
    if (qtIndex < 4) {
        return 0;
    }
    _qt_stamp = (int) ((YAPI::GetTickCount()) & (0x7FFFFFFF));
    if (_quatCallback != NULL) {
        _quatCallback(this, _w, _x, _y, _z);
    }
    if (_anglesCallback != NULL) {
        this->_loadAngles();
        _anglesCallback(this, _roll, _pitch, _head);
    }
    return 0;
}

YGyro *YGyro::nextGyro(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YGyro::FindGyro(hwid);
}

YGyro* YGyro::FirstGyro(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Gyro", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YGyro::FindGyro(serial+"."+funcId);
}

//--- (end of generated code: YGyro implementation)

//--- (generated code: YGyro functions)
//--- (end of generated code: YGyro functions)
