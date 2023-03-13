/*********************************************************************
 *
 * $Id: yocto_refframe.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindRefFrame(), the high-level API for RefFrame functions
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
#include "yocto_refframe.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "refframe"

YRefFrame::YRefFrame(const string& func): YFunction(func)
//--- (YRefFrame initialization)
    ,_mountPos(MOUNTPOS_INVALID)
    ,_bearing(BEARING_INVALID)
    ,_calibrationParam(CALIBRATIONPARAM_INVALID)
    ,_fusionMode(FUSIONMODE_INVALID)
    ,_valueCallbackRefFrame(NULL)
    ,_calibV2(0)
    ,_calibStage(0)
    ,_calibStageProgress(0)
    ,_calibProgress(0)
    ,_calibCount(0)
    ,_calibInternalPos(0)
    ,_calibPrevTick(0)
    ,_calibAccXOfs(0.0)
    ,_calibAccYOfs(0.0)
    ,_calibAccZOfs(0.0)
    ,_calibAccXScale(0.0)
    ,_calibAccYScale(0.0)
    ,_calibAccZScale(0.0)
//--- (end of YRefFrame initialization)
{
    _className="RefFrame";
}

YRefFrame::~YRefFrame()
{
//--- (YRefFrame cleanup)
//--- (end of YRefFrame cleanup)
}
//--- (YRefFrame implementation)
// static attributes
const double YRefFrame::BEARING_INVALID = YAPI_INVALID_DOUBLE;
const string YRefFrame::CALIBRATIONPARAM_INVALID = YAPI_INVALID_STRING;

int YRefFrame::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("mountPos")) {
        _mountPos =  json_val->getInt("mountPos");
    }
    if(json_val->has("bearing")) {
        _bearing =  floor(json_val->getDouble("bearing") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("calibrationParam")) {
        _calibrationParam =  json_val->getString("calibrationParam");
    }
    if(json_val->has("fusionMode")) {
        _fusionMode =  (Y_FUSIONMODE_enum)json_val->getInt("fusionMode");
    }
    return YFunction::_parseAttr(json_val);
}


int YRefFrame::get_mountPos(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRefFrame::MOUNTPOS_INVALID;
                }
            }
        }
        res = _mountPos;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YRefFrame::set_mountPos(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("mountPos", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the reference bearing used by the compass. The relative bearing
 * indicated by the compass is the difference between the measured magnetic
 * heading and the reference bearing indicated here.
 *
 * For instance, if you setup as reference bearing the value of the earth
 * magnetic declination, the compass will provide the orientation relative
 * to the geographic North.
 *
 * Similarly, when the sensor is not mounted along the standard directions
 * because it has an additional yaw angle, you can set this angle in the reference
 * bearing so that the compass provides the expected natural direction.
 *
 * Remember to call the saveToFlash()
 * method of the module if the modification must be kept.
 *
 * @param newval : a floating point number corresponding to the reference bearing used by the compass
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRefFrame::set_bearing(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("bearing", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the reference bearing used by the compass. The relative bearing
 * indicated by the compass is the difference between the measured magnetic
 * heading and the reference bearing indicated here.
 *
 * @return a floating point number corresponding to the reference bearing used by the compass
 *
 * On failure, throws an exception or returns Y_BEARING_INVALID.
 */
double YRefFrame::get_bearing(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRefFrame::BEARING_INVALID;
                }
            }
        }
        res = _bearing;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YRefFrame::get_calibrationParam(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRefFrame::CALIBRATIONPARAM_INVALID;
                }
            }
        }
        res = _calibrationParam;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YRefFrame::set_calibrationParam(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("calibrationParam", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

Y_FUSIONMODE_enum YRefFrame::get_fusionMode(void)
{
    Y_FUSIONMODE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YRefFrame::FUSIONMODE_INVALID;
                }
            }
        }
        res = _fusionMode;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YRefFrame::set_fusionMode(Y_FUSIONMODE_enum newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("fusionMode", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a reference frame for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the reference frame is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YRefFrame.isOnline() to test if the reference frame is
 * indeed online at a given time. In case of ambiguity when looking for
 * a reference frame by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the reference frame
 *
 * @return a YRefFrame object allowing you to drive the reference frame.
 */
YRefFrame* YRefFrame::FindRefFrame(string func)
{
    YRefFrame* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YRefFrame*) YFunction::_FindFromCache("RefFrame", func);
        if (obj == NULL) {
            obj = new YRefFrame(func);
            YFunction::_AddToCache("RefFrame", func, obj);
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
int YRefFrame::registerValueCallback(YRefFrameValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackRefFrame = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YRefFrame::_invokeValueCallback(string value)
{
    if (_valueCallbackRefFrame != NULL) {
        _valueCallbackRefFrame(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

/**
 * Returns the installation position of the device, as configured
 * in order to define the reference frame for the compass and the
 * pitch/roll tilt sensors.
 *
 * @return a value among the Y_MOUNTPOSITION enumeration
 *         (Y_MOUNTPOSITION_BOTTOM,   Y_MOUNTPOSITION_TOP,
 *         Y_MOUNTPOSITION_FRONT,    Y_MOUNTPOSITION_RIGHT,
 *         Y_MOUNTPOSITION_REAR,     Y_MOUNTPOSITION_LEFT),
 *         corresponding to the installation in a box, on one of the six faces.
 *
 * On failure, throws an exception or returns Y_MOUNTPOSITION_INVALID.
 */
Y_MOUNTPOSITION YRefFrame::get_mountPosition(void)
{
    int position = 0;
    position = this->get_mountPos();
    if (position < 0) {
        return Y_MOUNTPOSITION_INVALID;
    }
    return (Y_MOUNTPOSITION) ((position) >> (2));
}

/**
 * Returns the installation orientation of the device, as configured
 * in order to define the reference frame for the compass and the
 * pitch/roll tilt sensors.
 *
 * @return a value among the enumeration Y_MOUNTORIENTATION
 *         (Y_MOUNTORIENTATION_TWELVE, Y_MOUNTORIENTATION_THREE,
 *         Y_MOUNTORIENTATION_SIX,     Y_MOUNTORIENTATION_NINE)
 *         corresponding to the orientation of the "X" arrow on the device,
 *         as on a clock dial seen from an observer in the center of the box.
 *         On the bottom face, the 12H orientation points to the front, while
 *         on the top face, the 12H orientation points to the rear.
 *
 * On failure, throws an exception or returns Y_MOUNTORIENTATION_INVALID.
 */
Y_MOUNTORIENTATION YRefFrame::get_mountOrientation(void)
{
    int position = 0;
    position = this->get_mountPos();
    if (position < 0) {
        return Y_MOUNTORIENTATION_INVALID;
    }
    return (Y_MOUNTORIENTATION) ((position) & (3));
}

/**
 * Changes the compass and tilt sensor frame of reference. The magnetic compass
 * and the tilt sensors (pitch and roll) naturally work in the plane
 * parallel to the earth surface. In case the device is not installed upright
 * and horizontally, you must select its reference orientation (parallel to
 * the earth surface) so that the measures are made relative to this position.
 *
 * @param position : a value among the Y_MOUNTPOSITION enumeration
 *         (Y_MOUNTPOSITION_BOTTOM,   Y_MOUNTPOSITION_TOP,
 *         Y_MOUNTPOSITION_FRONT,    Y_MOUNTPOSITION_RIGHT,
 *         Y_MOUNTPOSITION_REAR,     Y_MOUNTPOSITION_LEFT),
 *         corresponding to the installation in a box, on one of the six faces.
 * @param orientation : a value among the enumeration Y_MOUNTORIENTATION
 *         (Y_MOUNTORIENTATION_TWELVE, Y_MOUNTORIENTATION_THREE,
 *         Y_MOUNTORIENTATION_SIX,     Y_MOUNTORIENTATION_NINE)
 *         corresponding to the orientation of the "X" arrow on the device,
 *         as on a clock dial seen from an observer in the center of the box.
 *         On the bottom face, the 12H orientation points to the front, while
 *         on the top face, the 12H orientation points to the rear.
 *
 * Remember to call the saveToFlash()
 * method of the module if the modification must be kept.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRefFrame::set_mountPosition(Y_MOUNTPOSITION position,Y_MOUNTORIENTATION orientation)
{
    int mixedPos = 0;
    mixedPos = ((position) << (2)) + orientation;
    return this->set_mountPos(mixedPos);
}

/**
 * Returns the 3D sensor calibration state (Yocto-3D-V2 only). This function returns
 * an integer representing the calibration state of the 3 inertial sensors of
 * the BNO055 chip, found in the Yocto-3D-V2. Hundredths show the calibration state
 * of the accelerometer, tenths show the calibration state of the magnetometer while
 * units show the calibration state of the gyroscope. For each sensor, the value 0
 * means no calibration and the value 3 means full calibration.
 *
 * @return an integer representing the calibration state of Yocto-3D-V2:
 *         333 when fully calibrated, 0 when not calibrated at all.
 *
 * On failure, throws an exception or returns a negative error code.
 * For the Yocto-3D (V1), this function always return -3 (unsupported function).
 */
int YRefFrame::get_calibrationState(void)
{
    string calibParam;
    vector<int> iCalib;
    int caltyp = 0;
    int res = 0;

    calibParam = this->get_calibrationParam();
    iCalib = YAPI::_decodeFloats(calibParam);
    caltyp = ((iCalib[0]) / (1000));
    if (caltyp != 33) {
        return YAPI_NOT_SUPPORTED;
    }
    res = ((iCalib[1]) / (1000));
    return res;
}

/**
 * Returns estimated quality of the orientation (Yocto-3D-V2 only). This function returns
 * an integer between 0 and 3 representing the degree of confidence of the position
 * estimate. When the value is 3, the estimation is reliable. Below 3, one should
 * expect sudden corrections, in particular for heading (compass function).
 * The most frequent causes for values below 3 are magnetic interferences, and
 * accelerations or rotations beyond the sensor range.
 *
 * @return an integer between 0 and 3 (3 when the measure is reliable)
 *
 * On failure, throws an exception or returns a negative error code.
 * For the Yocto-3D (V1), this function always return -3 (unsupported function).
 */
int YRefFrame::get_measureQuality(void)
{
    string calibParam;
    vector<int> iCalib;
    int caltyp = 0;
    int res = 0;

    calibParam = this->get_calibrationParam();
    iCalib = YAPI::_decodeFloats(calibParam);
    caltyp = ((iCalib[0]) / (1000));
    if (caltyp != 33) {
        return YAPI_NOT_SUPPORTED;
    }
    res = ((iCalib[2]) / (1000));
    return res;
}

int YRefFrame::_calibSort(int start,int stopidx)
{
    int idx = 0;
    int changed = 0;
    double a = 0.0;
    double b = 0.0;
    double xa = 0.0;
    double xb = 0.0;
    // bubble sort is good since we will re-sort again after offset adjustment
    changed = 1;
    while (changed > 0) {
        changed = 0;
        a = _calibDataAcc[start];
        idx = start + 1;
        while (idx < stopidx) {
            b = _calibDataAcc[idx];
            if (a > b) {
                _calibDataAcc[idx-1] = b;
                _calibDataAcc[idx] = a;
                xa = _calibDataAccX[idx-1];
                xb = _calibDataAccX[idx];
                _calibDataAccX[idx-1] = xb;
                _calibDataAccX[idx] = xa;
                xa = _calibDataAccY[idx-1];
                xb = _calibDataAccY[idx];
                _calibDataAccY[idx-1] = xb;
                _calibDataAccY[idx] = xa;
                xa = _calibDataAccZ[idx-1];
                xb = _calibDataAccZ[idx];
                _calibDataAccZ[idx-1] = xb;
                _calibDataAccZ[idx] = xa;
                changed = changed + 1;
            } else {
                a = b;
            }
            idx = idx + 1;
        }
    }
    return 0;
}

/**
 * Initiates the sensors tridimensional calibration process.
 * This calibration is used at low level for inertial position estimation
 * and to enhance the precision of the tilt sensors.
 *
 * After calling this method, the device should be moved according to the
 * instructions provided by method get_3DCalibrationHint,
 * and more3DCalibration should be invoked about 5 times per second.
 * The calibration procedure is completed when the method
 * get_3DCalibrationProgress returns 100. At this point,
 * the computed calibration parameters can be applied using method
 * save3DCalibration. The calibration process can be canceled
 * at any time using method cancel3DCalibration.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRefFrame::start3DCalibration(void)
{
    if (!(this->isOnline())) {
        return YAPI_DEVICE_NOT_FOUND;
    }
    if (_calibStage != 0) {
        this->cancel3DCalibration();
    }
    _calibSavedParams = this->get_calibrationParam();
    _calibV2 = (atoi((_calibSavedParams).c_str()) == 33);
    this->set_calibrationParam("0");
    _calibCount = 50;
    _calibStage = 1;
    _calibStageHint = "Set down the device on a steady horizontal surface";
    _calibStageProgress = 0;
    _calibProgress = 1;
    _calibInternalPos = 0;
    _calibPrevTick = (int) ((YAPI::GetTickCount()) & (0x7FFFFFFF));
    _calibOrient.clear();
    _calibDataAccX.clear();
    _calibDataAccY.clear();
    _calibDataAccZ.clear();
    _calibDataAcc.clear();
    return YAPI_SUCCESS;
}

/**
 * Continues the sensors tridimensional calibration process previously
 * initiated using method start3DCalibration.
 * This method should be called approximately 5 times per second, while
 * positioning the device according to the instructions provided by method
 * get_3DCalibrationHint. Note that the instructions change during
 * the calibration process.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRefFrame::more3DCalibration(void)
{
    if (_calibV2) {
        return this->more3DCalibrationV2();
    }
    return this->more3DCalibrationV1();
}

int YRefFrame::more3DCalibrationV1(void)
{
    int currTick = 0;
    string jsonData;
    double xVal = 0.0;
    double yVal = 0.0;
    double zVal = 0.0;
    double xSq = 0.0;
    double ySq = 0.0;
    double zSq = 0.0;
    double norm = 0.0;
    int orient = 0;
    int idx = 0;
    int intpos = 0;
    int err = 0;
    // make sure calibration has been started
    if (_calibStage == 0) {
        return YAPI_INVALID_ARGUMENT;
    }
    if (_calibProgress == 100) {
        return YAPI_SUCCESS;
    }
    // make sure we leave at least 160ms between samples
    currTick =  (int) ((YAPI::GetTickCount()) & (0x7FFFFFFF));
    if (((currTick - _calibPrevTick) & (0x7FFFFFFF)) < 160) {
        return YAPI_SUCCESS;
    }
    // load current accelerometer values, make sure we are on a straight angle
    // (default timeout to 0,5 sec without reading measure when out of range)
    _calibStageHint = "Set down the device on a steady horizontal surface";
    _calibPrevTick = ((currTick + 500) & (0x7FFFFFFF));
    jsonData = this->_download("api/accelerometer.json");
    xVal = atoi((this->_json_get_key(jsonData, "xValue")).c_str()) / 65536.0;
    yVal = atoi((this->_json_get_key(jsonData, "yValue")).c_str()) / 65536.0;
    zVal = atoi((this->_json_get_key(jsonData, "zValue")).c_str()) / 65536.0;
    xSq = xVal * xVal;
    if (xSq >= 0.04 && xSq < 0.64) {
        return YAPI_SUCCESS;
    }
    if (xSq >= 1.44) {
        return YAPI_SUCCESS;
    }
    ySq = yVal * yVal;
    if (ySq >= 0.04 && ySq < 0.64) {
        return YAPI_SUCCESS;
    }
    if (ySq >= 1.44) {
        return YAPI_SUCCESS;
    }
    zSq = zVal * zVal;
    if (zSq >= 0.04 && zSq < 0.64) {
        return YAPI_SUCCESS;
    }
    if (zSq >= 1.44) {
        return YAPI_SUCCESS;
    }
    norm = sqrt(xSq + ySq + zSq);
    if (norm < 0.8 || norm > 1.2) {
        return YAPI_SUCCESS;
    }
    _calibPrevTick = currTick;
    // Determine the device orientation index
    orient = 0;
    if (zSq > 0.5) {
        if (zVal > 0) {
            orient = 0;
        } else {
            orient = 1;
        }
    }
    if (xSq > 0.5) {
        if (xVal > 0) {
            orient = 2;
        } else {
            orient = 3;
        }
    }
    if (ySq > 0.5) {
        if (yVal > 0) {
            orient = 4;
        } else {
            orient = 5;
        }
    }
    // Discard measures that are not in the proper orientation
    if (_calibStageProgress == 0) {
        // New stage, check that this orientation is not yet done
        idx = 0;
        err = 0;
        while (idx + 1 < _calibStage) {
            if (_calibOrient[idx] == orient) {
                err = 1;
            }
            idx = idx + 1;
        }
        if (err != 0) {
            _calibStageHint = "Turn the device on another face";
            return YAPI_SUCCESS;
        }
        _calibOrient.push_back(orient);
    } else {
        // Make sure device is not turned before stage is completed
        if (orient != _calibOrient[_calibStage-1]) {
            _calibStageHint = "Not yet done, please move back to the previous face";
            return YAPI_SUCCESS;
        }
    }
    // Save measure
    _calibStageHint = "calibrating...";
    _calibDataAccX.push_back(xVal);
    _calibDataAccY.push_back(yVal);
    _calibDataAccZ.push_back(zVal);
    _calibDataAcc.push_back(norm);
    _calibInternalPos = _calibInternalPos + 1;
    _calibProgress = 1 + 16 * (_calibStage - 1) + ((16 * _calibInternalPos) / (_calibCount));
    if (_calibInternalPos < _calibCount) {
        _calibStageProgress = 1 + ((99 * _calibInternalPos) / (_calibCount));
        return YAPI_SUCCESS;
    }
    // Stage done, compute preliminary result
    intpos = (_calibStage - 1) * _calibCount;
    this->_calibSort(intpos, intpos + _calibCount);
    intpos = intpos + ((_calibCount) / (2));
    _calibLogMsg = YapiWrapper::ysprintf("Stage %d: median is %d,%d,%d", _calibStage,
    (int) floor(1000*_calibDataAccX[intpos]+0.5),
    (int) floor(1000*_calibDataAccY[intpos]+0.5),(int) floor(1000*_calibDataAccZ[intpos]+0.5));
    // move to next stage
    _calibStage = _calibStage + 1;
    if (_calibStage < 7) {
        _calibStageHint = "Turn the device on another face";
        _calibPrevTick = ((currTick + 500) & (0x7FFFFFFF));
        _calibStageProgress = 0;
        _calibInternalPos = 0;
        return YAPI_SUCCESS;
    }
    // Data collection completed, compute accelerometer shift
    xVal = 0;
    yVal = 0;
    zVal = 0;
    idx = 0;
    while (idx < 6) {
        intpos = idx * _calibCount + ((_calibCount) / (2));
        orient = _calibOrient[idx];
        if (orient == 0 || orient == 1) {
            zVal = zVal + _calibDataAccZ[intpos];
        }
        if (orient == 2 || orient == 3) {
            xVal = xVal + _calibDataAccX[intpos];
        }
        if (orient == 4 || orient == 5) {
            yVal = yVal + _calibDataAccY[intpos];
        }
        idx = idx + 1;
    }
    _calibAccXOfs = xVal / 2.0;
    _calibAccYOfs = yVal / 2.0;
    _calibAccZOfs = zVal / 2.0;
    // Recompute all norms, taking into account the computed shift, and re-sort
    intpos = 0;
    while (intpos < (int)_calibDataAcc.size()) {
        xVal = _calibDataAccX[intpos] - _calibAccXOfs;
        yVal = _calibDataAccY[intpos] - _calibAccYOfs;
        zVal = _calibDataAccZ[intpos] - _calibAccZOfs;
        norm = sqrt(xVal * xVal + yVal * yVal + zVal * zVal);
        _calibDataAcc[intpos] = norm;
        intpos = intpos + 1;
    }
    idx = 0;
    while (idx < 6) {
        intpos = idx * _calibCount;
        this->_calibSort(intpos, intpos + _calibCount);
        idx = idx + 1;
    }
    // Compute the scaling factor for each axis
    xVal = 0;
    yVal = 0;
    zVal = 0;
    idx = 0;
    while (idx < 6) {
        intpos = idx * _calibCount + ((_calibCount) / (2));
        orient = _calibOrient[idx];
        if (orient == 0 || orient == 1) {
            zVal = zVal + _calibDataAcc[intpos];
        }
        if (orient == 2 || orient == 3) {
            xVal = xVal + _calibDataAcc[intpos];
        }
        if (orient == 4 || orient == 5) {
            yVal = yVal + _calibDataAcc[intpos];
        }
        idx = idx + 1;
    }
    _calibAccXScale = xVal / 2.0;
    _calibAccYScale = yVal / 2.0;
    _calibAccZScale = zVal / 2.0;
    // Report completion
    _calibProgress = 100;
    _calibStageHint = "Calibration data ready for saving";
    return YAPI_SUCCESS;
}

int YRefFrame::more3DCalibrationV2(void)
{
    int currTick = 0;
    string calibParam;
    vector<int> iCalib;
    int cal3 = 0;
    int calAcc = 0;
    int calMag = 0;
    int calGyr = 0;
    // make sure calibration has been started
    if (_calibStage == 0) {
        return YAPI_INVALID_ARGUMENT;
    }
    if (_calibProgress == 100) {
        return YAPI_SUCCESS;
    }
    // make sure we don't start before previous calibration is cleared
    if (_calibStage == 1) {
        currTick = (int) ((YAPI::GetTickCount()) & (0x7FFFFFFF));
        currTick = ((currTick - _calibPrevTick) & (0x7FFFFFFF));
        if (currTick < 1600) {
            _calibStageHint = "Set down the device on a steady horizontal surface";
            _calibStageProgress = ((currTick) / (40));
            _calibProgress = 1;
            return YAPI_SUCCESS;
        }
    }

    calibParam = this->_download("api/refFrame/calibrationParam.txt");
    iCalib = YAPI::_decodeFloats(calibParam);
    cal3 = ((iCalib[1]) / (1000));
    calAcc = ((cal3) / (100));
    calMag = ((cal3) / (10)) - 10*calAcc;
    calGyr = ((cal3) % (10));
    if (calGyr < 3) {
        _calibStageHint = "Set down the device on a steady horizontal surface";
        _calibStageProgress = 40 + calGyr*20;
        _calibProgress = 4 + calGyr*2;
    } else {
        _calibStage = 2;
        if (calMag < 3) {
            _calibStageHint = "Slowly draw '8' shapes along the 3 axis";
            _calibStageProgress = 1 + calMag*33;
            _calibProgress = 10 + calMag*5;
        } else {
            _calibStage = 3;
            if (calAcc < 3) {
                _calibStageHint = "Slowly turn the device, stopping at each 90 degrees";
                _calibStageProgress = 1 + calAcc*33;
                _calibProgress = 25 + calAcc*25;
            } else {
                _calibStageProgress = 99;
                _calibProgress = 100;
            }
        }
    }
    return YAPI_SUCCESS;
}

/**
 * Returns instructions to proceed to the tridimensional calibration initiated with
 * method start3DCalibration.
 *
 * @return a character string.
 */
string YRefFrame::get_3DCalibrationHint(void)
{
    return _calibStageHint;
}

/**
 * Returns the global process indicator for the tridimensional calibration
 * initiated with method start3DCalibration.
 *
 * @return an integer between 0 (not started) and 100 (stage completed).
 */
int YRefFrame::get_3DCalibrationProgress(void)
{
    return _calibProgress;
}

/**
 * Returns index of the current stage of the calibration
 * initiated with method start3DCalibration.
 *
 * @return an integer, growing each time a calibration stage is completed.
 */
int YRefFrame::get_3DCalibrationStage(void)
{
    return _calibStage;
}

/**
 * Returns the process indicator for the current stage of the calibration
 * initiated with method start3DCalibration.
 *
 * @return an integer between 0 (not started) and 100 (stage completed).
 */
int YRefFrame::get_3DCalibrationStageProgress(void)
{
    return _calibStageProgress;
}

/**
 * Returns the latest log message from the calibration process.
 * When no new message is available, returns an empty string.
 *
 * @return a character string.
 */
string YRefFrame::get_3DCalibrationLogMsg(void)
{
    string msg;
    msg = _calibLogMsg;
    _calibLogMsg = "";
    return msg;
}

/**
 * Applies the sensors tridimensional calibration parameters that have just been computed.
 * Remember to call the saveToFlash()  method of the module if the changes
 * must be kept when the device is restarted.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRefFrame::save3DCalibration(void)
{
    if (_calibV2) {
        return this->save3DCalibrationV2();
    }
    return this->save3DCalibrationV1();
}

int YRefFrame::save3DCalibrationV1(void)
{
    int shiftX = 0;
    int shiftY = 0;
    int shiftZ = 0;
    int scaleExp = 0;
    int scaleX = 0;
    int scaleY = 0;
    int scaleZ = 0;
    int scaleLo = 0;
    int scaleHi = 0;
    string newcalib;
    if (_calibProgress != 100) {
        return YAPI_INVALID_ARGUMENT;
    }
    // Compute integer values (correction unit is 732ug/count)
    shiftX = -(int) floor(_calibAccXOfs / 0.000732+0.5);
    if (shiftX < 0) {
        shiftX = shiftX + 65536;
    }
    shiftY = -(int) floor(_calibAccYOfs / 0.000732+0.5);
    if (shiftY < 0) {
        shiftY = shiftY + 65536;
    }
    shiftZ = -(int) floor(_calibAccZOfs / 0.000732+0.5);
    if (shiftZ < 0) {
        shiftZ = shiftZ + 65536;
    }
    scaleX = (int) floor(2048.0 / _calibAccXScale+0.5) - 2048;
    scaleY = (int) floor(2048.0 / _calibAccYScale+0.5) - 2048;
    scaleZ = (int) floor(2048.0 / _calibAccZScale+0.5) - 2048;
    if (scaleX < -2048 || scaleX >= 2048 || scaleY < -2048 || scaleY >= 2048 || scaleZ < -2048 || scaleZ >= 2048) {
        scaleExp = 3;
    } else {
        if (scaleX < -1024 || scaleX >= 1024 || scaleY < -1024 || scaleY >= 1024 || scaleZ < -1024 || scaleZ >= 1024) {
            scaleExp = 2;
        } else {
            if (scaleX < -512 || scaleX >= 512 || scaleY < -512 || scaleY >= 512 || scaleZ < -512 || scaleZ >= 512) {
                scaleExp = 1;
            } else {
                scaleExp = 0;
            }
        }
    }
    if (scaleExp > 0) {
        scaleX = ((scaleX) >> (scaleExp));
        scaleY = ((scaleY) >> (scaleExp));
        scaleZ = ((scaleZ) >> (scaleExp));
    }
    if (scaleX < 0) {
        scaleX = scaleX + 1024;
    }
    if (scaleY < 0) {
        scaleY = scaleY + 1024;
    }
    if (scaleZ < 0) {
        scaleZ = scaleZ + 1024;
    }
    scaleLo = ((((scaleY) & (15))) << (12)) + ((scaleX) << (2)) + scaleExp;
    scaleHi = ((scaleZ) << (6)) + ((scaleY) >> (4));
    // Save calibration parameters
    newcalib = YapiWrapper::ysprintf("5,%d,%d,%d,%d,%d", shiftX, shiftY, shiftZ, scaleLo,scaleHi);
    _calibStage = 0;
    return this->set_calibrationParam(newcalib);
}

int YRefFrame::save3DCalibrationV2(void)
{
    return this->set_calibrationParam("5,5,5,5,5,5");
}

/**
 * Aborts the sensors tridimensional calibration process et restores normal settings.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YRefFrame::cancel3DCalibration(void)
{
    if (_calibStage == 0) {
        return YAPI_SUCCESS;
    }

    _calibStage = 0;
    return this->set_calibrationParam(_calibSavedParams);
}

YRefFrame *YRefFrame::nextRefFrame(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YRefFrame::FindRefFrame(hwid);
}

YRefFrame* YRefFrame::FirstRefFrame(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("RefFrame", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YRefFrame::FindRefFrame(serial+"."+funcId);
}

//--- (end of YRefFrame implementation)

//--- (YRefFrame functions)
//--- (end of YRefFrame functions)
