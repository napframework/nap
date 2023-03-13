/*********************************************************************
 *
 * $Id: yocto_quadraturedecoder.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindQuadratureDecoder(), the high-level API for QuadratureDecoder functions
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


#ifndef YOCTO_QUADRATUREDECODER_H
#define YOCTO_QUADRATUREDECODER_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YQuadratureDecoder return codes)
//--- (end of YQuadratureDecoder return codes)
//--- (YQuadratureDecoder definitions)
class YQuadratureDecoder; // forward declaration

typedef void (*YQuadratureDecoderValueCallback)(YQuadratureDecoder *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YQuadratureDecoderTimedReportCallback)(YQuadratureDecoder *func, YMeasure measure);
#ifndef _Y_DECODING_ENUM
#define _Y_DECODING_ENUM
typedef enum {
    Y_DECODING_OFF = 0,
    Y_DECODING_ON = 1,
    Y_DECODING_INVALID = -1,
} Y_DECODING_enum;
#endif
#define Y_SPEED_INVALID                 (YAPI_INVALID_DOUBLE)
//--- (end of YQuadratureDecoder definitions)

//--- (YQuadratureDecoder declaration)
/**
 * YQuadratureDecoder Class: QuadratureDecoder function interface
 *
 * The class YQuadratureDecoder allows you to decode a two-wire signal produced by a
 * quadrature encoder. It inherits from YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 */
class YOCTO_CLASS_EXPORT YQuadratureDecoder: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YQuadratureDecoder declaration)
protected:
    //--- (YQuadratureDecoder attributes)
    // Attributes (function value cache)
    double          _speed;
    Y_DECODING_enum _decoding;
    YQuadratureDecoderValueCallback _valueCallbackQuadratureDecoder;
    YQuadratureDecoderTimedReportCallback _timedReportCallbackQuadratureDecoder;

    friend YQuadratureDecoder *yFindQuadratureDecoder(const string& func);
    friend YQuadratureDecoder *yFirstQuadratureDecoder(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindQuadratureDecoder factory function to instantiate
    YQuadratureDecoder(const string& func);
    //--- (end of YQuadratureDecoder attributes)

public:
    ~YQuadratureDecoder();
    //--- (YQuadratureDecoder accessors declaration)

    static const double SPEED_INVALID;
    static const Y_DECODING_enum DECODING_OFF = Y_DECODING_OFF;
    static const Y_DECODING_enum DECODING_ON = Y_DECODING_ON;
    static const Y_DECODING_enum DECODING_INVALID = Y_DECODING_INVALID;

    /**
     * Changes the current expected position of the quadrature decoder.
     * Invoking this function implicitely activates the quadrature decoder.
     *
     * @param newval : a floating point number corresponding to the current expected position of the quadrature decoder
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_currentValue(double newval);
    inline int      setCurrentValue(double newval)
    { return this->set_currentValue(newval); }

    /**
     * Returns the increments frequency, in Hz.
     *
     * @return a floating point number corresponding to the increments frequency, in Hz
     *
     * On failure, throws an exception or returns Y_SPEED_INVALID.
     */
    double              get_speed(void);

    inline double       speed(void)
    { return this->get_speed(); }

    /**
     * Returns the current activation state of the quadrature decoder.
     *
     * @return either Y_DECODING_OFF or Y_DECODING_ON, according to the current activation state of the
     * quadrature decoder
     *
     * On failure, throws an exception or returns Y_DECODING_INVALID.
     */
    Y_DECODING_enum     get_decoding(void);

    inline Y_DECODING_enum decoding(void)
    { return this->get_decoding(); }

    /**
     * Changes the activation state of the quadrature decoder.
     *
     * @param newval : either Y_DECODING_OFF or Y_DECODING_ON, according to the activation state of the
     * quadrature decoder
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_decoding(Y_DECODING_enum newval);
    inline int      setDecoding(Y_DECODING_enum newval)
    { return this->set_decoding(newval); }

    /**
     * Retrieves a quadrature decoder for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the quadrature decoder is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YQuadratureDecoder.isOnline() to test if the quadrature decoder is
     * indeed online at a given time. In case of ambiguity when looking for
     * a quadrature decoder by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the quadrature decoder
     *
     * @return a YQuadratureDecoder object allowing you to drive the quadrature decoder.
     */
    static YQuadratureDecoder* FindQuadratureDecoder(string func);

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
    virtual int         registerValueCallback(YQuadratureDecoderValueCallback callback);
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
    virtual int         registerTimedReportCallback(YQuadratureDecoderTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YQuadratureDecoder* Find(string func)
    { return YQuadratureDecoder::FindQuadratureDecoder(func); }

    /**
     * Continues the enumeration of quadrature decoders started using yFirstQuadratureDecoder().
     *
     * @return a pointer to a YQuadratureDecoder object, corresponding to
     *         a quadrature decoder currently online, or a NULL pointer
     *         if there are no more quadrature decoders to enumerate.
     */
           YQuadratureDecoder *nextQuadratureDecoder(void);
    inline YQuadratureDecoder *next(void)
    { return this->nextQuadratureDecoder();}

    /**
     * Starts the enumeration of quadrature decoders currently accessible.
     * Use the method YQuadratureDecoder.nextQuadratureDecoder() to iterate on
     * next quadrature decoders.
     *
     * @return a pointer to a YQuadratureDecoder object, corresponding to
     *         the first quadrature decoder currently online, or a NULL pointer
     *         if there are none.
     */
           static YQuadratureDecoder* FirstQuadratureDecoder(void);
    inline static YQuadratureDecoder* First(void)
    { return YQuadratureDecoder::FirstQuadratureDecoder();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YQuadratureDecoder accessors declaration)
};

//--- (YQuadratureDecoder functions declaration)

/**
 * Retrieves a quadrature decoder for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the quadrature decoder is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YQuadratureDecoder.isOnline() to test if the quadrature decoder is
 * indeed online at a given time. In case of ambiguity when looking for
 * a quadrature decoder by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the quadrature decoder
 *
 * @return a YQuadratureDecoder object allowing you to drive the quadrature decoder.
 */
inline YQuadratureDecoder* yFindQuadratureDecoder(const string& func)
{ return YQuadratureDecoder::FindQuadratureDecoder(func);}
/**
 * Starts the enumeration of quadrature decoders currently accessible.
 * Use the method YQuadratureDecoder.nextQuadratureDecoder() to iterate on
 * next quadrature decoders.
 *
 * @return a pointer to a YQuadratureDecoder object, corresponding to
 *         the first quadrature decoder currently online, or a NULL pointer
 *         if there are none.
 */
inline YQuadratureDecoder* yFirstQuadratureDecoder(void)
{ return YQuadratureDecoder::FirstQuadratureDecoder();}

//--- (end of YQuadratureDecoder functions declaration)

#endif
