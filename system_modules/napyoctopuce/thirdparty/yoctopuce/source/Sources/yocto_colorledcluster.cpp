/*********************************************************************
 *
 * $Id: yocto_colorledcluster.cpp 29186 2017-11-16 10:04:13Z seb $
 *
 * Implements yFindColorLedCluster(), the high-level API for ColorLedCluster functions
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
#include "yocto_colorledcluster.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "colorledcluster"

YColorLedCluster::YColorLedCluster(const string& func): YFunction(func)
//--- (YColorLedCluster initialization)
    ,_activeLedCount(ACTIVELEDCOUNT_INVALID)
    ,_maxLedCount(MAXLEDCOUNT_INVALID)
    ,_blinkSeqMaxCount(BLINKSEQMAXCOUNT_INVALID)
    ,_blinkSeqMaxSize(BLINKSEQMAXSIZE_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackColorLedCluster(NULL)
//--- (end of YColorLedCluster initialization)
{
    _className="ColorLedCluster";
}

YColorLedCluster::~YColorLedCluster()
{
//--- (YColorLedCluster cleanup)
//--- (end of YColorLedCluster cleanup)
}
//--- (YColorLedCluster implementation)
// static attributes
const string YColorLedCluster::COMMAND_INVALID = YAPI_INVALID_STRING;

int YColorLedCluster::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("activeLedCount")) {
        _activeLedCount =  json_val->getInt("activeLedCount");
    }
    if(json_val->has("maxLedCount")) {
        _maxLedCount =  json_val->getInt("maxLedCount");
    }
    if(json_val->has("blinkSeqMaxCount")) {
        _blinkSeqMaxCount =  json_val->getInt("blinkSeqMaxCount");
    }
    if(json_val->has("blinkSeqMaxSize")) {
        _blinkSeqMaxSize =  json_val->getInt("blinkSeqMaxSize");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the number of LEDs currently handled by the device.
 *
 * @return an integer corresponding to the number of LEDs currently handled by the device
 *
 * On failure, throws an exception or returns Y_ACTIVELEDCOUNT_INVALID.
 */
int YColorLedCluster::get_activeLedCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLedCluster::ACTIVELEDCOUNT_INVALID;
                }
            }
        }
        res = _activeLedCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the number of LEDs currently handled by the device.
 *
 * @param newval : an integer corresponding to the number of LEDs currently handled by the device
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_activeLedCount(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("activeLedCount", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the maximum number of LEDs that the device can handle.
 *
 * @return an integer corresponding to the maximum number of LEDs that the device can handle
 *
 * On failure, throws an exception or returns Y_MAXLEDCOUNT_INVALID.
 */
int YColorLedCluster::get_maxLedCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration == 0) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLedCluster::MAXLEDCOUNT_INVALID;
                }
            }
        }
        res = _maxLedCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the maximum number of sequences that the device can handle.
 *
 * @return an integer corresponding to the maximum number of sequences that the device can handle
 *
 * On failure, throws an exception or returns Y_BLINKSEQMAXCOUNT_INVALID.
 */
int YColorLedCluster::get_blinkSeqMaxCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration == 0) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLedCluster::BLINKSEQMAXCOUNT_INVALID;
                }
            }
        }
        res = _blinkSeqMaxCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the maximum length of sequences.
 *
 * @return an integer corresponding to the maximum length of sequences
 *
 * On failure, throws an exception or returns Y_BLINKSEQMAXSIZE_INVALID.
 */
int YColorLedCluster::get_blinkSeqMaxSize(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration == 0) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLedCluster::BLINKSEQMAXSIZE_INVALID;
                }
            }
        }
        res = _blinkSeqMaxSize;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YColorLedCluster::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YColorLedCluster::COMMAND_INVALID;
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

int YColorLedCluster::set_command(const string& newval)
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
 * Retrieves a RGB LED cluster for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the RGB LED cluster is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YColorLedCluster.isOnline() to test if the RGB LED cluster is
 * indeed online at a given time. In case of ambiguity when looking for
 * a RGB LED cluster by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the RGB LED cluster
 *
 * @return a YColorLedCluster object allowing you to drive the RGB LED cluster.
 */
YColorLedCluster* YColorLedCluster::FindColorLedCluster(string func)
{
    YColorLedCluster* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YColorLedCluster*) YFunction::_FindFromCache("ColorLedCluster", func);
        if (obj == NULL) {
            obj = new YColorLedCluster(func);
            YFunction::_AddToCache("ColorLedCluster", func, obj);
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
int YColorLedCluster::registerValueCallback(YColorLedClusterValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackColorLedCluster = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YColorLedCluster::_invokeValueCallback(string value)
{
    if (_valueCallbackColorLedCluster != NULL) {
        _valueCallbackColorLedCluster(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YColorLedCluster::sendCommand(string command)
{
    return this->set_command(command);
}

/**
 * Changes the current color of consecutve LEDs in the cluster, using a RGB color. Encoding is done as
 * follows: 0xRRGGBB.
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param rgbValue :  new color.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_rgbColor(int ledIndex,int count,int rgbValue)
{
    return this->sendCommand(YapiWrapper::ysprintf("SR%d,%d,%x",ledIndex,count,rgbValue));
}

/**
 * Changes the  color at device startup of consecutve LEDs in the cluster, using a RGB color. Encoding
 * is done as follows: 0xRRGGBB.
 * Don't forget to call saveLedsConfigAtPowerOn() to make sure the modification is saved in the device
 * flash memory.
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param rgbValue :  new color.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_rgbColorAtPowerOn(int ledIndex,int count,int rgbValue)
{
    return this->sendCommand(YapiWrapper::ysprintf("SC%d,%d,%x",ledIndex,count,rgbValue));
}

/**
 * Changes the current color of consecutive LEDs in the cluster, using a HSL color. Encoding is done
 * as follows: 0xHHSSLL.
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param hslValue :  new color.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_hslColor(int ledIndex,int count,int hslValue)
{
    return this->sendCommand(YapiWrapper::ysprintf("SH%d,%d,%x",ledIndex,count,hslValue));
}

/**
 * Allows you to modify the current color of a group of adjacent LEDs to another color, in a seamless and
 * autonomous manner. The transition is performed in the RGB space.
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param rgbValue :  new color (0xRRGGBB).
 * @param delay    :  transition duration in ms
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::rgb_move(int ledIndex,int count,int rgbValue,int delay)
{
    return this->sendCommand(YapiWrapper::ysprintf("MR%d,%d,%x,%d",ledIndex,count,rgbValue,delay));
}

/**
 * Allows you to modify the current color of a group of adjacent LEDs  to another color, in a seamless and
 * autonomous manner. The transition is performed in the HSL space. In HSL, hue is a circular
 * value (0..360°). There are always two paths to perform the transition: by increasing
 * or by decreasing the hue. The module selects the shortest transition.
 * If the difference is exactly 180°, the module selects the transition which increases
 * the hue.
 *
 * @param ledIndex :  index of the fisrt affected LED.
 * @param count    :  affected LED count.
 * @param hslValue :  new color (0xHHSSLL).
 * @param delay    :  transition duration in ms
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::hsl_move(int ledIndex,int count,int hslValue,int delay)
{
    return this->sendCommand(YapiWrapper::ysprintf("MH%d,%d,%x,%d",ledIndex,count,hslValue,delay));
}

/**
 * Adds an RGB transition to a sequence. A sequence is a transition list, which can
 * be executed in loop by a group of LEDs.  Sequences are persistent and are saved
 * in the device flash memory as soon as the saveBlinkSeq() method is called.
 *
 * @param seqIndex :  sequence index.
 * @param rgbValue :  target color (0xRRGGBB)
 * @param delay    :  transition duration in ms
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::addRgbMoveToBlinkSeq(int seqIndex,int rgbValue,int delay)
{
    return this->sendCommand(YapiWrapper::ysprintf("AR%d,%x,%d",seqIndex,rgbValue,delay));
}

/**
 * Adds an HSL transition to a sequence. A sequence is a transition list, which can
 * be executed in loop by an group of LEDs.  Sequences are persistant and are saved
 * in the device flash memory as soon as the saveBlinkSeq() method is called.
 *
 * @param seqIndex : sequence index.
 * @param hslValue : target color (0xHHSSLL)
 * @param delay    : transition duration in ms
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::addHslMoveToBlinkSeq(int seqIndex,int hslValue,int delay)
{
    return this->sendCommand(YapiWrapper::ysprintf("AH%d,%x,%d",seqIndex,hslValue,delay));
}

/**
 * Adds a mirror ending to a sequence. When the sequence will reach the end of the last
 * transition, its running speed will automatically be reversed so that the sequence plays
 * in the reverse direction, like in a mirror. After the first transition of the sequence
 * is played at the end of the reverse execution, the sequence starts again in
 * the initial direction.
 *
 * @param seqIndex : sequence index.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::addMirrorToBlinkSeq(int seqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("AC%d,0,0",seqIndex));
}

/**
 * Adds to a sequence a jump to another sequence. When a pixel will reach this jump,
 * it will be automatically relinked to the new sequence, and will run it starting
 * from the beginning.
 *
 * @param seqIndex : sequence index.
 * @param linkSeqIndex : index of the sequence to chain.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::addJumpToBlinkSeq(int seqIndex,int linkSeqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("AC%d,100,%d,1000",seqIndex,linkSeqIndex));
}

/**
 * Adds a to a sequence a hard stop code. When a pixel will reach this stop code,
 * instead of restarting the sequence in a loop it will automatically be unlinked
 * from the sequence.
 *
 * @param seqIndex : sequence index.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::addUnlinkToBlinkSeq(int seqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("AC%d,100,-1,1000",seqIndex));
}

/**
 * Links adjacent LEDs to a specific sequence. These LEDs start to execute
 * the sequence as soon as  startBlinkSeq is called. It is possible to add an offset
 * in the execution: that way we  can have several groups of LED executing the same
 * sequence, with a  temporal offset. A LED cannot be linked to more than one sequence.
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param seqIndex :  sequence index.
 * @param offset   :  execution offset in ms.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::linkLedToBlinkSeq(int ledIndex,int count,int seqIndex,int offset)
{
    return this->sendCommand(YapiWrapper::ysprintf("LS%d,%d,%d,%d",ledIndex,count,seqIndex,offset));
}

/**
 * Links adjacent LEDs to a specific sequence at device poweron. Don't forget to configure
 * the sequence auto start flag as well and call saveLedsConfigAtPowerOn(). It is possible to add an offset
 * in the execution: that way we  can have several groups of LEDs executing the same
 * sequence, with a  temporal offset. A LED cannot be linked to more than one sequence.
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param seqIndex :  sequence index.
 * @param offset   :  execution offset in ms.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::linkLedToBlinkSeqAtPowerOn(int ledIndex,int count,int seqIndex,int offset)
{
    return this->sendCommand(YapiWrapper::ysprintf("LO%d,%d,%d,%d",ledIndex,count,seqIndex,offset));
}

/**
 * Links adjacent LEDs to a specific sequence. These LED start to execute
 * the sequence as soon as  startBlinkSeq is called. This function automatically
 * introduces a shift between LEDs so that the specified number of sequence periods
 * appears on the group of LEDs (wave effect).
 *
 * @param ledIndex :  index of the first affected LED.
 * @param count    :  affected LED count.
 * @param seqIndex :  sequence index.
 * @param periods  :  number of periods to show on LEDs.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::linkLedToPeriodicBlinkSeq(int ledIndex,int count,int seqIndex,int periods)
{
    return this->sendCommand(YapiWrapper::ysprintf("LP%d,%d,%d,%d",ledIndex,count,seqIndex,periods));
}

/**
 * Unlinks adjacent LEDs from a  sequence.
 *
 * @param ledIndex  :  index of the first affected LED.
 * @param count     :  affected LED count.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::unlinkLedFromBlinkSeq(int ledIndex,int count)
{
    return this->sendCommand(YapiWrapper::ysprintf("US%d,%d",ledIndex,count));
}

/**
 * Starts a sequence execution: every LED linked to that sequence starts to
 * run it in a loop. Note that a sequence with a zero duration can't be started.
 *
 * @param seqIndex :  index of the sequence to start.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::startBlinkSeq(int seqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("SS%d",seqIndex));
}

/**
 * Stops a sequence execution. If started again, the execution
 * restarts from the beginning.
 *
 * @param seqIndex :  index of the sequence to stop.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::stopBlinkSeq(int seqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("XS%d",seqIndex));
}

/**
 * Stops a sequence execution and resets its contents. Leds linked to this
 * sequence are not automatically updated anymore.
 *
 * @param seqIndex :  index of the sequence to reset
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::resetBlinkSeq(int seqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("ZS%d",seqIndex));
}

/**
 * Configures a sequence to make it start automatically at device
 * startup. Note that a sequence with a zero duration can't be started.
 * Don't forget to call saveBlinkSeq() to make sure the
 * modification is saved in the device flash memory.
 *
 * @param seqIndex :  index of the sequence to reset.
 * @param autostart : 0 to keep the sequence turned off and 1 to start it automatically.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_blinkSeqStateAtPowerOn(int seqIndex,int autostart)
{
    return this->sendCommand(YapiWrapper::ysprintf("AS%d,%d",seqIndex,autostart));
}

/**
 * Changes the execution speed of a sequence. The natural execution speed is 1000 per
 * thousand. If you configure a slower speed, you can play the sequence in slow-motion.
 * If you set a negative speed, you can play the sequence in reverse direction.
 *
 * @param seqIndex :  index of the sequence to start.
 * @param speed :     sequence running speed (-1000...1000).
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_blinkSeqSpeed(int seqIndex,int speed)
{
    return this->sendCommand(YapiWrapper::ysprintf("CS%d,%d",seqIndex,speed));
}

/**
 * Saves the LEDs power-on configuration. This includes the start-up color or
 * sequence binding for all LEDs. Warning: if some LEDs are linked to a sequence, the
 * method saveBlinkSeq() must also be called to save the sequence definition.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::saveLedsConfigAtPowerOn(void)
{
    return this->sendCommand("WL");
}

int YColorLedCluster::saveLedsState(void)
{
    return this->sendCommand("WL");
}

/**
 * Saves the definition of a sequence. Warning: only sequence steps and flags are saved.
 * to save the LEDs startup bindings, the method saveLedsConfigAtPowerOn()
 * must be called.
 *
 * @param seqIndex :  index of the sequence to start.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::saveBlinkSeq(int seqIndex)
{
    return this->sendCommand(YapiWrapper::ysprintf("WS%d",seqIndex));
}

/**
 * Sends a binary buffer to the LED RGB buffer, as is.
 * First three bytes are RGB components for LED specified as parameter, the
 * next three bytes for the next LED, etc.
 *
 * @param ledIndex : index of the first LED which should be updated
 * @param buff : the binary buffer to send
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_rgbColorBuffer(int ledIndex,string buff)
{
    return this->_upload(YapiWrapper::ysprintf("rgb:0:%d",ledIndex), buff);
}

/**
 * Sends 24bit RGB colors (provided as a list of integers) to the LED RGB buffer, as is.
 * The first number represents the RGB value of the LED specified as parameter, the second
 * number represents the RGB value of the next LED, etc.
 *
 * @param ledIndex : index of the first LED which should be updated
 * @param rgbList : a list of 24bit RGB codes, in the form 0xRRGGBB
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_rgbColorArray(int ledIndex,vector<int> rgbList)
{
    int listlen = 0;
    string buff;
    int idx = 0;
    int rgb = 0;
    int res = 0;
    listlen = (int)rgbList.size();
    buff = string(3*listlen, (char)0);
    idx = 0;
    while (idx < listlen) {
        rgb = rgbList[idx];
        buff[3*idx] = (char)(((((rgb) >> (16))) & (255)));
        buff[3*idx+1] = (char)(((((rgb) >> (8))) & (255)));
        buff[3*idx+2] = (char)(((rgb) & (255)));
        idx = idx + 1;
    }

    res = this->_upload(YapiWrapper::ysprintf("rgb:0:%d",ledIndex), buff);
    return res;
}

/**
 * Sets up a smooth RGB color transition to the specified pixel-by-pixel list of RGB
 * color codes. The first color code represents the target RGB value of the first LED,
 * the next color code represents the target value of the next LED, etc.
 *
 * @param ledIndex : index of the first LED which should be updated
 * @param rgbList : a list of target 24bit RGB codes, in the form 0xRRGGBB
 * @param delay   : transition duration in ms
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::rgbArrayOfs_move(int ledIndex,vector<int> rgbList,int delay)
{
    int listlen = 0;
    string buff;
    int idx = 0;
    int rgb = 0;
    int res = 0;
    listlen = (int)rgbList.size();
    buff = string(3*listlen, (char)0);
    idx = 0;
    while (idx < listlen) {
        rgb = rgbList[idx];
        buff[3*idx] = (char)(((((rgb) >> (16))) & (255)));
        buff[3*idx+1] = (char)(((((rgb) >> (8))) & (255)));
        buff[3*idx+2] = (char)(((rgb) & (255)));
        idx = idx + 1;
    }

    res = this->_upload(YapiWrapper::ysprintf("rgb:%d:%d",delay,ledIndex), buff);
    return res;
}

/**
 * Sets up a smooth RGB color transition to the specified pixel-by-pixel list of RGB
 * color codes. The first color code represents the target RGB value of the first LED,
 * the next color code represents the target value of the next LED, etc.
 *
 * @param rgbList : a list of target 24bit RGB codes, in the form 0xRRGGBB
 * @param delay   : transition duration in ms
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::rgbArray_move(vector<int> rgbList,int delay)
{
    int res = 0;

    res = this->rgbArrayOfs_move(0,rgbList,delay);
    return res;
}

/**
 * Sends a binary buffer to the LED HSL buffer, as is.
 * First three bytes are HSL components for the LED specified as parameter, the
 * next three bytes for the second LED, etc.
 *
 * @param ledIndex : index of the first LED which should be updated
 * @param buff : the binary buffer to send
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_hslColorBuffer(int ledIndex,string buff)
{
    return this->_upload(YapiWrapper::ysprintf("hsl:0:%d",ledIndex), buff);
}

/**
 * Sends 24bit HSL colors (provided as a list of integers) to the LED HSL buffer, as is.
 * The first number represents the HSL value of the LED specified as parameter, the second number represents
 * the HSL value of the second LED, etc.
 *
 * @param ledIndex : index of the first LED which should be updated
 * @param hslList : a list of 24bit HSL codes, in the form 0xHHSSLL
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::set_hslColorArray(int ledIndex,vector<int> hslList)
{
    int listlen = 0;
    string buff;
    int idx = 0;
    int hsl = 0;
    int res = 0;
    listlen = (int)hslList.size();
    buff = string(3*listlen, (char)0);
    idx = 0;
    while (idx < listlen) {
        hsl = hslList[idx];
        buff[3*idx] = (char)(((((hsl) >> (16))) & (255)));
        buff[3*idx+1] = (char)(((((hsl) >> (8))) & (255)));
        buff[3*idx+2] = (char)(((hsl) & (255)));
        idx = idx + 1;
    }

    res = this->_upload(YapiWrapper::ysprintf("hsl:0:%d",ledIndex), buff);
    return res;
}

/**
 * Sets up a smooth HSL color transition to the specified pixel-by-pixel list of HSL
 * color codes. The first color code represents the target HSL value of the first LED,
 * the second color code represents the target value of the second LED, etc.
 *
 * @param hslList : a list of target 24bit HSL codes, in the form 0xHHSSLL
 * @param delay   : transition duration in ms
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::hslArray_move(vector<int> hslList,int delay)
{
    int res = 0;

    res = this->hslArrayOfs_move(0,hslList, delay);
    return res;
}

/**
 * Sets up a smooth HSL color transition to the specified pixel-by-pixel list of HSL
 * color codes. The first color code represents the target HSL value of the first LED,
 * the second color code represents the target value of the second LED, etc.
 *
 * @param ledIndex : index of the first LED which should be updated
 * @param hslList : a list of target 24bit HSL codes, in the form 0xHHSSLL
 * @param delay   : transition duration in ms
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YColorLedCluster::hslArrayOfs_move(int ledIndex,vector<int> hslList,int delay)
{
    int listlen = 0;
    string buff;
    int idx = 0;
    int hsl = 0;
    int res = 0;
    listlen = (int)hslList.size();
    buff = string(3*listlen, (char)0);
    idx = 0;
    while (idx < listlen) {
        hsl = hslList[idx];
        buff[3*idx] = (char)(((((hsl) >> (16))) & (255)));
        buff[3*idx+1] = (char)(((((hsl) >> (8))) & (255)));
        buff[3*idx+2] = (char)(((hsl) & (255)));
        idx = idx + 1;
    }

    res = this->_upload(YapiWrapper::ysprintf("hsl:%d:%d",delay,ledIndex), buff);
    return res;
}

/**
 * Returns a binary buffer with content from the LED RGB buffer, as is.
 * First three bytes are RGB components for the first LED in the interval,
 * the next three bytes for the second LED in the interval, etc.
 *
 * @param ledIndex : index of the first LED which should be returned
 * @param count    : number of LEDs which should be returned
 *
 * @return a binary buffer with RGB components of selected LEDs.
 *
 * On failure, throws an exception or returns an empty binary buffer.
 */
string YColorLedCluster::get_rgbColorBuffer(int ledIndex,int count)
{
    return this->_download(YapiWrapper::ysprintf("rgb.bin?typ=0&pos=%d&len=%d",3*ledIndex,3*count));
}

/**
 * Returns a list on 24bit RGB color values with the current colors displayed on
 * the RGB leds. The first number represents the RGB value of the first LED,
 * the second number represents the RGB value of the second LED, etc.
 *
 * @param ledIndex : index of the first LED which should be returned
 * @param count    : number of LEDs which should be returned
 *
 * @return a list of 24bit color codes with RGB components of selected LEDs, as 0xRRGGBB.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_rgbColorArray(int ledIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int r = 0;
    int g = 0;
    int b = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=0&pos=%d&len=%d",3*ledIndex,3*count));
    res.clear();
    idx = 0;
    while (idx < count) {
        r = ((u8)buff[3*idx]);
        g = ((u8)buff[3*idx+1]);
        b = ((u8)buff[3*idx+2]);
        res.push_back(r*65536+g*256+b);
        idx = idx + 1;
    }
    return res;
}

/**
 * Returns a list on 24bit RGB color values with the RGB LEDs startup colors.
 * The first number represents the startup RGB value of the first LED,
 * the second number represents the RGB value of the second LED, etc.
 *
 * @param ledIndex : index of the first LED  which should be returned
 * @param count    : number of LEDs which should be returned
 *
 * @return a list of 24bit color codes with RGB components of selected LEDs, as 0xRRGGBB.
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_rgbColorArrayAtPowerOn(int ledIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int r = 0;
    int g = 0;
    int b = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=4&pos=%d&len=%d",3*ledIndex,3*count));
    res.clear();
    idx = 0;
    while (idx < count) {
        r = ((u8)buff[3*idx]);
        g = ((u8)buff[3*idx+1]);
        b = ((u8)buff[3*idx+2]);
        res.push_back(r*65536+g*256+b);
        idx = idx + 1;
    }
    return res;
}

/**
 * Returns a list on sequence index for each RGB LED. The first number represents the
 * sequence index for the the first LED, the second number represents the sequence
 * index for the second LED, etc.
 *
 * @param ledIndex : index of the first LED which should be returned
 * @param count    : number of LEDs which should be returned
 *
 * @return a list of integers with sequence index
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_linkedSeqArray(int ledIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int seq = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=1&pos=%d&len=%d",ledIndex,count));
    res.clear();
    idx = 0;
    while (idx < count) {
        seq = ((u8)buff[idx]);
        res.push_back(seq);
        idx = idx + 1;
    }
    return res;
}

/**
 * Returns a list on 32 bit signatures for specified blinking sequences.
 * Since blinking sequences cannot be read from the device, this can be used
 * to detect if a specific blinking sequence is already programmed.
 *
 * @param seqIndex : index of the first blinking sequence which should be returned
 * @param count    : number of blinking sequences which should be returned
 *
 * @return a list of 32 bit integer signatures
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_blinkSeqSignatures(int seqIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int hh = 0;
    int hl = 0;
    int lh = 0;
    int ll = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=2&pos=%d&len=%d",4*seqIndex,4*count));
    res.clear();
    idx = 0;
    while (idx < count) {
        hh = ((u8)buff[4*idx]);
        hl = ((u8)buff[4*idx+1]);
        lh = ((u8)buff[4*idx+2]);
        ll = ((u8)buff[4*idx+3]);
        res.push_back(((hh) << (24))+((hl) << (16))+((lh) << (8))+ll);
        idx = idx + 1;
    }
    return res;
}

/**
 * Returns a list of integers with the current speed for specified blinking sequences.
 *
 * @param seqIndex : index of the first sequence speed which should be returned
 * @param count    : number of sequence speeds which should be returned
 *
 * @return a list of integers, 0 for sequences turned off and 1 for sequences running
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_blinkSeqStateSpeed(int seqIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int lh = 0;
    int ll = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=6&pos=%d&len=%d",seqIndex,count));
    res.clear();
    idx = 0;
    while (idx < count) {
        lh = ((u8)buff[2*idx]);
        ll = ((u8)buff[2*idx+1]);
        res.push_back(((lh) << (8))+ll);
        idx = idx + 1;
    }
    return res;
}

/**
 * Returns a list of integers with the "auto-start at power on" flag state for specified blinking sequences.
 *
 * @param seqIndex : index of the first blinking sequence which should be returned
 * @param count    : number of blinking sequences which should be returned
 *
 * @return a list of integers, 0 for sequences turned off and 1 for sequences running
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_blinkSeqStateAtPowerOn(int seqIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int started = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=5&pos=%d&len=%d",seqIndex,count));
    res.clear();
    idx = 0;
    while (idx < count) {
        started = ((u8)buff[idx]);
        res.push_back(started);
        idx = idx + 1;
    }
    return res;
}

/**
 * Returns a list of integers with the started state for specified blinking sequences.
 *
 * @param seqIndex : index of the first blinking sequence which should be returned
 * @param count    : number of blinking sequences which should be returned
 *
 * @return a list of integers, 0 for sequences turned off and 1 for sequences running
 *
 * On failure, throws an exception or returns an empty array.
 */
vector<int> YColorLedCluster::get_blinkSeqState(int seqIndex,int count)
{
    string buff;
    vector<int> res;
    int idx = 0;
    int started = 0;

    buff = this->_download(YapiWrapper::ysprintf("rgb.bin?typ=3&pos=%d&len=%d",seqIndex,count));
    res.clear();
    idx = 0;
    while (idx < count) {
        started = ((u8)buff[idx]);
        res.push_back(started);
        idx = idx + 1;
    }
    return res;
}

YColorLedCluster *YColorLedCluster::nextColorLedCluster(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YColorLedCluster::FindColorLedCluster(hwid);
}

YColorLedCluster* YColorLedCluster::FirstColorLedCluster(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("ColorLedCluster", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YColorLedCluster::FindColorLedCluster(serial+"."+funcId);
}

//--- (end of YColorLedCluster implementation)

//--- (YColorLedCluster functions)
//--- (end of YColorLedCluster functions)
