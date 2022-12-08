/*********************************************************************
 *
 * $Id: yocto_gyro.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindGyro(), the high-level API for Gyro functions
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


#ifndef YOCTO_GYRO_H
#define YOCTO_GYRO_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (generated code: YQt return codes)
//--- (end of generated code: YQt return codes)
//--- (generated code: YQt definitions)
class YQt; // forward declaration

typedef void (*YQtValueCallback)(YQt *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YQtTimedReportCallback)(YQt *func, YMeasure measure);
//--- (end of generated code: YQt definitions)

//--- (generated code: YQt declaration)
/**
 * YQt Class: Quaternion interface
 *
 * The Yoctopuce API YQt class provides direct access to the Yocto3D attitude estimation
 * using a quaternion. It is usually not needed to use the YQt class directly, as the
 * YGyro class provides a more convenient higher-level interface.
 */
class YOCTO_CLASS_EXPORT YQt: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YQt declaration)
protected:
    //--- (generated code: YQt attributes)
    // Attributes (function value cache)
    YQtValueCallback _valueCallbackQt;
    YQtTimedReportCallback _timedReportCallbackQt;

    friend YQt *yFindQt(const string& func);
    friend YQt *yFirstQt(void);

    // Constructor is protected, use yFindQt factory function to instantiate
    YQt(const string& func);
    //--- (end of generated code: YQt attributes)

public:
    ~YQt();
    //--- (generated code: YQt accessors declaration)


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
    static YQt*         FindQt(string func);

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
    virtual int         registerValueCallback(YQtValueCallback callback);
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
    virtual int         registerTimedReportCallback(YQtTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YQt* Find(string func)
    { return YQt::FindQt(func); }

    /**
     * Continues the enumeration of quaternion components started using yFirstQt().
     *
     * @return a pointer to a YQt object, corresponding to
     *         a quaternion component currently online, or a NULL pointer
     *         if there are no more quaternion components to enumerate.
     */
           YQt             *nextQt(void);
    inline YQt             *next(void)
    { return this->nextQt();}

    /**
     * Starts the enumeration of quaternion components currently accessible.
     * Use the method YQt.nextQt() to iterate on
     * next quaternion components.
     *
     * @return a pointer to a YQt object, corresponding to
     *         the first quaternion component currently online, or a NULL pointer
     *         if there are none.
     */
           static YQt* FirstQt(void);
    inline static YQt* First(void)
    { return YQt::FirstQt();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YQt accessors declaration)
};

//--- (generated code: YQt functions declaration)

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
inline YQt* yFindQt(const string& func)
{ return YQt::FindQt(func);}
/**
 * Starts the enumeration of quaternion components currently accessible.
 * Use the method YQt.nextQt() to iterate on
 * next quaternion components.
 *
 * @return a pointer to a YQt object, corresponding to
 *         the first quaternion component currently online, or a NULL pointer
 *         if there are none.
 */
inline YQt* yFirstQt(void)
{ return YQt::FirstQt();}

//--- (end of generated code: YQt functions declaration)



//--- (generated code: YGyro return codes)
//--- (end of generated code: YGyro return codes)
//--- (generated code: YGyro definitions)
class YGyro; // forward declaration

typedef void (*YGyroValueCallback)(YGyro *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YGyroTimedReportCallback)(YGyro *func, YMeasure measure);
#define Y_BANDWIDTH_INVALID             (YAPI_INVALID_INT)
#define Y_XVALUE_INVALID                (YAPI_INVALID_DOUBLE)
#define Y_YVALUE_INVALID                (YAPI_INVALID_DOUBLE)
#define Y_ZVALUE_INVALID                (YAPI_INVALID_DOUBLE)
//--- (end of generated code: YGyro definitions)

typedef void(*YQuatCallback)(YGyro *yGyro, double w, double x, double y, double z);
typedef void(*YAnglesCallback)(YGyro *yGyro, double roll, double pitch, double head);

//--- (generated code: YGyro declaration)
/**
 * YGyro Class: Gyroscope function interface
 *
 * The YSensor class is the parent class for all Yoctopuce sensors. It can be
 * used to read the current value and unit of any sensor, read the min/max
 * value, configure autonomous recording frequency and access recorded data.
 * It also provide a function to register a callback invoked each time the
 * observed value changes, or at a predefined interval. Using this class rather
 * than a specific subclass makes it possible to create generic applications
 * that work with any Yoctopuce sensor, even those that do not yet exist.
 * Note: The YAnButton class is the only analog input which does not inherit
 * from YSensor.
 */
class YOCTO_CLASS_EXPORT YGyro: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YGyro declaration)
protected:
    //--- (generated code: YGyro attributes)
    // Attributes (function value cache)
    int             _bandwidth;
    double          _xValue;
    double          _yValue;
    double          _zValue;
    YGyroValueCallback _valueCallbackGyro;
    YGyroTimedReportCallback _timedReportCallbackGyro;
    int             _qt_stamp;
    YQt*            _qt_w;
    YQt*            _qt_x;
    YQt*            _qt_y;
    YQt*            _qt_z;
    double          _w;
    double          _x;
    double          _y;
    double          _z;
    int             _angles_stamp;
    double          _head;
    double          _pitch;
    double          _roll;
    YQuatCallback   _quatCallback;
    YAnglesCallback _anglesCallback;

    friend YGyro *yFindGyro(const string& func);
    friend YGyro *yFirstGyro(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindGyro factory function to instantiate
    YGyro(const string& func);
    //--- (end of generated code: YGyro attributes)

public:
    ~YGyro();
    //--- (generated code: YGyro accessors declaration)

    static const int BANDWIDTH_INVALID = YAPI_INVALID_INT;
    static const double XVALUE_INVALID;
    static const double YVALUE_INVALID;
    static const double ZVALUE_INVALID;

    /**
     * Returns the measure update frequency, measured in Hz (Yocto-3D-V2 only).
     *
     * @return an integer corresponding to the measure update frequency, measured in Hz (Yocto-3D-V2 only)
     *
     * On failure, throws an exception or returns Y_BANDWIDTH_INVALID.
     */
    int                 get_bandwidth(void);

    inline int          bandwidth(void)
    { return this->get_bandwidth(); }

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
    int             set_bandwidth(int newval);
    inline int      setBandwidth(int newval)
    { return this->set_bandwidth(newval); }

    /**
     * Returns the angular velocity around the X axis of the device, as a floating point number.
     *
     * @return a floating point number corresponding to the angular velocity around the X axis of the
     * device, as a floating point number
     *
     * On failure, throws an exception or returns Y_XVALUE_INVALID.
     */
    double              get_xValue(void);

    inline double       xValue(void)
    { return this->get_xValue(); }

    /**
     * Returns the angular velocity around the Y axis of the device, as a floating point number.
     *
     * @return a floating point number corresponding to the angular velocity around the Y axis of the
     * device, as a floating point number
     *
     * On failure, throws an exception or returns Y_YVALUE_INVALID.
     */
    double              get_yValue(void);

    inline double       yValue(void)
    { return this->get_yValue(); }

    /**
     * Returns the angular velocity around the Z axis of the device, as a floating point number.
     *
     * @return a floating point number corresponding to the angular velocity around the Z axis of the
     * device, as a floating point number
     *
     * On failure, throws an exception or returns Y_ZVALUE_INVALID.
     */
    double              get_zValue(void);

    inline double       zValue(void)
    { return this->get_zValue(); }

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
    static YGyro*       FindGyro(string func);

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
    virtual int         registerValueCallback(YGyroValueCallback callback);
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
    virtual int         registerTimedReportCallback(YGyroTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);

    virtual int         _loadQuaternion(void);

    virtual int         _loadAngles(void);

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
    virtual double      get_roll(void);

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
    virtual double      get_pitch(void);

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
    virtual double      get_heading(void);

    /**
     * Returns the w component (real part) of the quaternion
     * describing the device estimated orientation, based on the
     * integration of gyroscopic measures combined with acceleration and
     * magnetic field measurements.
     *
     * @return a floating-point number corresponding to the w
     *         component of the quaternion.
     */
    virtual double      get_quaternionW(void);

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
    virtual double      get_quaternionX(void);

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
    virtual double      get_quaternionY(void);

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
    virtual double      get_quaternionZ(void);

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
    virtual int         registerQuaternionCallback(YQuatCallback callback);

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
    virtual int         registerAnglesCallback(YAnglesCallback callback);

    virtual int         _invokeGyroCallbacks(int qtIndex,double qtValue);


    inline static YGyro* Find(string func)
    { return YGyro::FindGyro(func); }

    /**
     * Continues the enumeration of gyroscopes started using yFirstGyro().
     *
     * @return a pointer to a YGyro object, corresponding to
     *         a gyroscope currently online, or a NULL pointer
     *         if there are no more gyroscopes to enumerate.
     */
           YGyro           *nextGyro(void);
    inline YGyro           *next(void)
    { return this->nextGyro();}

    /**
     * Starts the enumeration of gyroscopes currently accessible.
     * Use the method YGyro.nextGyro() to iterate on
     * next gyroscopes.
     *
     * @return a pointer to a YGyro object, corresponding to
     *         the first gyro currently online, or a NULL pointer
     *         if there are none.
     */
           static YGyro* FirstGyro(void);
    inline static YGyro* First(void)
    { return YGyro::FirstGyro();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YGyro accessors declaration)
};

//--- (generated code: YGyro functions declaration)

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
inline YGyro* yFindGyro(const string& func)
{ return YGyro::FindGyro(func);}
/**
 * Starts the enumeration of gyroscopes currently accessible.
 * Use the method YGyro.nextGyro() to iterate on
 * next gyroscopes.
 *
 * @return a pointer to a YGyro object, corresponding to
 *         the first gyro currently online, or a NULL pointer
 *         if there are none.
 */
inline YGyro* yFirstGyro(void)
{ return YGyro::FirstGyro();}

//--- (end of generated code: YGyro functions declaration)

#endif
