/*********************************************************************
 *
 * $Id: yocto_multiaxiscontroller.cpp 29507 2017-12-28 14:14:56Z mvuilleu $
 *
 * Implements yFindMultiAxisController(), the high-level API for MultiAxisController functions
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
#include "yocto_multiaxiscontroller.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "multiaxiscontroller"

YMultiAxisController::YMultiAxisController(const string& func): YFunction(func)
//--- (YMultiAxisController initialization)
    ,_nAxis(NAXIS_INVALID)
    ,_globalState(GLOBALSTATE_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackMultiAxisController(NULL)
//--- (end of YMultiAxisController initialization)
{
    _className="MultiAxisController";
}

YMultiAxisController::~YMultiAxisController()
{
//--- (YMultiAxisController cleanup)
//--- (end of YMultiAxisController cleanup)
}
//--- (YMultiAxisController implementation)
// static attributes
const string YMultiAxisController::COMMAND_INVALID = YAPI_INVALID_STRING;

int YMultiAxisController::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("nAxis")) {
        _nAxis =  json_val->getInt("nAxis");
    }
    if(json_val->has("globalState")) {
        _globalState =  (Y_GLOBALSTATE_enum)json_val->getInt("globalState");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the number of synchronized controllers.
 *
 * @return an integer corresponding to the number of synchronized controllers
 *
 * On failure, throws an exception or returns Y_NAXIS_INVALID.
 */
int YMultiAxisController::get_nAxis(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMultiAxisController::NAXIS_INVALID;
                }
            }
        }
        res = _nAxis;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the number of synchronized controllers.
 *
 * @param newval : an integer corresponding to the number of synchronized controllers
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::set_nAxis(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("nAxis", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the stepper motor set overall state.
 *
 * @return a value among Y_GLOBALSTATE_ABSENT, Y_GLOBALSTATE_ALERT, Y_GLOBALSTATE_HI_Z,
 * Y_GLOBALSTATE_STOP, Y_GLOBALSTATE_RUN and Y_GLOBALSTATE_BATCH corresponding to the stepper motor
 * set overall state
 *
 * On failure, throws an exception or returns Y_GLOBALSTATE_INVALID.
 */
Y_GLOBALSTATE_enum YMultiAxisController::get_globalState(void)
{
    Y_GLOBALSTATE_enum res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMultiAxisController::GLOBALSTATE_INVALID;
                }
            }
        }
        res = _globalState;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YMultiAxisController::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMultiAxisController::COMMAND_INVALID;
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

int YMultiAxisController::set_command(const string& newval)
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
 * Retrieves a multi-axis controller for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the multi-axis controller is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YMultiAxisController.isOnline() to test if the multi-axis controller is
 * indeed online at a given time. In case of ambiguity when looking for
 * a multi-axis controller by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the multi-axis controller
 *
 * @return a YMultiAxisController object allowing you to drive the multi-axis controller.
 */
YMultiAxisController* YMultiAxisController::FindMultiAxisController(string func)
{
    YMultiAxisController* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YMultiAxisController*) YFunction::_FindFromCache("MultiAxisController", func);
        if (obj == NULL) {
            obj = new YMultiAxisController(func);
            YFunction::_AddToCache("MultiAxisController", func, obj);
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
int YMultiAxisController::registerValueCallback(YMultiAxisControllerValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackMultiAxisController = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YMultiAxisController::_invokeValueCallback(string value)
{
    if (_valueCallbackMultiAxisController != NULL) {
        _valueCallbackMultiAxisController(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YMultiAxisController::sendCommand(string command)
{
    string url;
    string retBin;
    int res = 0;
    url = YapiWrapper::ysprintf("cmd.txt?X=%s",command.c_str());
    //may throw an exception
    retBin = this->_download(url);
    res = ((u8)retBin[0]);
    if (res == 49) {
        if (!(res == 48)) {
            _throw(YAPI_DEVICE_BUSY,"Motor command pipeline is full, try again later");
            return YAPI_DEVICE_BUSY;
        }
    } else {
        if (!(res == 48)) {
            _throw(YAPI_IO_ERROR,"Motor command failed permanently");
            return YAPI_IO_ERROR;
        }
    }
    return YAPI_SUCCESS;
}

/**
 * Reinitialize all controllers and clear all alert flags.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::reset(void)
{
    return this->sendCommand("Z");
}

/**
 * Starts all motors backward at the specified speeds, to search for the motor home position.
 *
 * @param speed : desired speed for all axis, in steps per second.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::findHomePosition(vector<double> speed)
{
    string cmd;
    int i = 0;
    int ndim = 0;
    ndim = (int)speed.size();
    cmd = YapiWrapper::ysprintf("H%d",(int) floor(1000*speed[0]+0.5));
    i = 1;
    while (i < ndim) {
        cmd = YapiWrapper::ysprintf("%s,%d", cmd.c_str(),(int) floor(1000*speed[i]+0.5));
        i = i + 1;
    }
    return this->sendCommand(cmd);
}

/**
 * Starts all motors synchronously to reach a given absolute position.
 * The time needed to reach the requested position will depend on the lowest
 * acceleration and max speed parameters configured for all motors.
 * The final position will be reached on all axis at the same time.
 *
 * @param absPos : absolute position, measured in steps from each origin.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::moveTo(vector<double> absPos)
{
    string cmd;
    int i = 0;
    int ndim = 0;
    ndim = (int)absPos.size();
    cmd = YapiWrapper::ysprintf("M%d",(int) floor(16*absPos[0]+0.5));
    i = 1;
    while (i < ndim) {
        cmd = YapiWrapper::ysprintf("%s,%d", cmd.c_str(),(int) floor(16*absPos[i]+0.5));
        i = i + 1;
    }
    return this->sendCommand(cmd);
}

/**
 * Starts all motors synchronously to reach a given relative position.
 * The time needed to reach the requested position will depend on the lowest
 * acceleration and max speed parameters configured for all motors.
 * The final position will be reached on all axis at the same time.
 *
 * @param relPos : relative position, measured in steps from the current position.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::moveRel(vector<double> relPos)
{
    string cmd;
    int i = 0;
    int ndim = 0;
    ndim = (int)relPos.size();
    cmd = YapiWrapper::ysprintf("m%d",(int) floor(16*relPos[0]+0.5));
    i = 1;
    while (i < ndim) {
        cmd = YapiWrapper::ysprintf("%s,%d", cmd.c_str(),(int) floor(16*relPos[i]+0.5));
        i = i + 1;
    }
    return this->sendCommand(cmd);
}

/**
 * Keep the motor in the same state for the specified amount of time, before processing next command.
 *
 * @param waitMs : wait time, specified in milliseconds.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::pause(int waitMs)
{
    return this->sendCommand(YapiWrapper::ysprintf("_%d",waitMs));
}

/**
 * Stops the motor with an emergency alert, without taking any additional precaution.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::emergencyStop(void)
{
    return this->sendCommand("!");
}

/**
 * Stops the motor smoothly as soon as possible, without waiting for ongoing move completion.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::abortAndBrake(void)
{
    return this->sendCommand("B");
}

/**
 * Turn the controller into Hi-Z mode immediately, without waiting for ongoing move completion.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YMultiAxisController::abortAndHiZ(void)
{
    return this->sendCommand("z");
}

YMultiAxisController *YMultiAxisController::nextMultiAxisController(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YMultiAxisController::FindMultiAxisController(hwid);
}

YMultiAxisController* YMultiAxisController::FirstMultiAxisController(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("MultiAxisController", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YMultiAxisController::FindMultiAxisController(serial+"."+funcId);
}

//--- (end of YMultiAxisController implementation)

//--- (YMultiAxisController functions)
//--- (end of YMultiAxisController functions)
