/*********************************************************************
 *
 * $Id: yocto_gps.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindGps(), the high-level API for Gps functions
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


#ifndef YOCTO_GPS_H
#define YOCTO_GPS_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YGps return codes)
//--- (end of YGps return codes)
//--- (YGps definitions)
class YGps; // forward declaration

typedef void (*YGpsValueCallback)(YGps *func, const string& functionValue);
#ifndef _Y_ISFIXED_ENUM
#define _Y_ISFIXED_ENUM
typedef enum {
    Y_ISFIXED_FALSE = 0,
    Y_ISFIXED_TRUE = 1,
    Y_ISFIXED_INVALID = -1,
} Y_ISFIXED_enum;
#endif
#ifndef _Y_COORDSYSTEM_ENUM
#define _Y_COORDSYSTEM_ENUM
typedef enum {
    Y_COORDSYSTEM_GPS_DMS = 0,
    Y_COORDSYSTEM_GPS_DM = 1,
    Y_COORDSYSTEM_GPS_D = 2,
    Y_COORDSYSTEM_INVALID = -1,
} Y_COORDSYSTEM_enum;
#endif
#define Y_SATCOUNT_INVALID              (YAPI_INVALID_LONG)
#define Y_LATITUDE_INVALID              (YAPI_INVALID_STRING)
#define Y_LONGITUDE_INVALID             (YAPI_INVALID_STRING)
#define Y_DILUTION_INVALID              (YAPI_INVALID_DOUBLE)
#define Y_ALTITUDE_INVALID              (YAPI_INVALID_DOUBLE)
#define Y_GROUNDSPEED_INVALID           (YAPI_INVALID_DOUBLE)
#define Y_DIRECTION_INVALID             (YAPI_INVALID_DOUBLE)
#define Y_UNIXTIME_INVALID              (YAPI_INVALID_LONG)
#define Y_DATETIME_INVALID              (YAPI_INVALID_STRING)
#define Y_UTCOFFSET_INVALID             (YAPI_INVALID_INT)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YGps definitions)

//--- (YGps declaration)
/**
 * YGps Class: GPS function interface
 *
 * The Gps function allows you to extract positionning
 * data from the GPS device. This class can provides
 * complete positionning information: However, if you
 * whish to define callbacks on position changes, you
 * should use the YLatitude et YLongitude classes.
 */
class YOCTO_CLASS_EXPORT YGps: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YGps declaration)
protected:
    //--- (YGps attributes)
    // Attributes (function value cache)
    Y_ISFIXED_enum  _isFixed;
    s64             _satCount;
    Y_COORDSYSTEM_enum _coordSystem;
    string          _latitude;
    string          _longitude;
    double          _dilution;
    double          _altitude;
    double          _groundSpeed;
    double          _direction;
    s64             _unixTime;
    string          _dateTime;
    int             _utcOffset;
    string          _command;
    YGpsValueCallback _valueCallbackGps;

    friend YGps *yFindGps(const string& func);
    friend YGps *yFirstGps(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindGps factory function to instantiate
    YGps(const string& func);
    //--- (end of YGps attributes)

public:
    ~YGps();
    //--- (YGps accessors declaration)

    static const Y_ISFIXED_enum ISFIXED_FALSE = Y_ISFIXED_FALSE;
    static const Y_ISFIXED_enum ISFIXED_TRUE = Y_ISFIXED_TRUE;
    static const Y_ISFIXED_enum ISFIXED_INVALID = Y_ISFIXED_INVALID;
    static const s64 SATCOUNT_INVALID = YAPI_INVALID_LONG;
    static const Y_COORDSYSTEM_enum COORDSYSTEM_GPS_DMS = Y_COORDSYSTEM_GPS_DMS;
    static const Y_COORDSYSTEM_enum COORDSYSTEM_GPS_DM = Y_COORDSYSTEM_GPS_DM;
    static const Y_COORDSYSTEM_enum COORDSYSTEM_GPS_D = Y_COORDSYSTEM_GPS_D;
    static const Y_COORDSYSTEM_enum COORDSYSTEM_INVALID = Y_COORDSYSTEM_INVALID;
    static const string LATITUDE_INVALID;
    static const string LONGITUDE_INVALID;
    static const double DILUTION_INVALID;
    static const double ALTITUDE_INVALID;
    static const double GROUNDSPEED_INVALID;
    static const double DIRECTION_INVALID;
    static const s64 UNIXTIME_INVALID = YAPI_INVALID_LONG;
    static const string DATETIME_INVALID;
    static const int UTCOFFSET_INVALID = YAPI_INVALID_INT;
    static const string COMMAND_INVALID;

    /**
     * Returns TRUE if the receiver has found enough satellites to work.
     *
     * @return either Y_ISFIXED_FALSE or Y_ISFIXED_TRUE, according to TRUE if the receiver has found
     * enough satellites to work
     *
     * On failure, throws an exception or returns Y_ISFIXED_INVALID.
     */
    Y_ISFIXED_enum      get_isFixed(void);

    inline Y_ISFIXED_enum isFixed(void)
    { return this->get_isFixed(); }

    /**
     * Returns the count of visible satellites.
     *
     * @return an integer corresponding to the count of visible satellites
     *
     * On failure, throws an exception or returns Y_SATCOUNT_INVALID.
     */
    s64                 get_satCount(void);

    inline s64          satCount(void)
    { return this->get_satCount(); }

    /**
     * Returns the representation system used for positioning data.
     *
     * @return a value among Y_COORDSYSTEM_GPS_DMS, Y_COORDSYSTEM_GPS_DM and Y_COORDSYSTEM_GPS_D
     * corresponding to the representation system used for positioning data
     *
     * On failure, throws an exception or returns Y_COORDSYSTEM_INVALID.
     */
    Y_COORDSYSTEM_enum  get_coordSystem(void);

    inline Y_COORDSYSTEM_enum coordSystem(void)
    { return this->get_coordSystem(); }

    /**
     * Changes the representation system used for positioning data.
     *
     * @param newval : a value among Y_COORDSYSTEM_GPS_DMS, Y_COORDSYSTEM_GPS_DM and Y_COORDSYSTEM_GPS_D
     * corresponding to the representation system used for positioning data
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_coordSystem(Y_COORDSYSTEM_enum newval);
    inline int      setCoordSystem(Y_COORDSYSTEM_enum newval)
    { return this->set_coordSystem(newval); }

    /**
     * Returns the current latitude.
     *
     * @return a string corresponding to the current latitude
     *
     * On failure, throws an exception or returns Y_LATITUDE_INVALID.
     */
    string              get_latitude(void);

    inline string       latitude(void)
    { return this->get_latitude(); }

    /**
     * Returns the current longitude.
     *
     * @return a string corresponding to the current longitude
     *
     * On failure, throws an exception or returns Y_LONGITUDE_INVALID.
     */
    string              get_longitude(void);

    inline string       longitude(void)
    { return this->get_longitude(); }

    /**
     * Returns the current horizontal dilution of precision,
     * the smaller that number is, the better .
     *
     * @return a floating point number corresponding to the current horizontal dilution of precision,
     *         the smaller that number is, the better
     *
     * On failure, throws an exception or returns Y_DILUTION_INVALID.
     */
    double              get_dilution(void);

    inline double       dilution(void)
    { return this->get_dilution(); }

    /**
     * Returns the current altitude. Beware:  GPS technology
     * is very inaccurate regarding altitude.
     *
     * @return a floating point number corresponding to the current altitude
     *
     * On failure, throws an exception or returns Y_ALTITUDE_INVALID.
     */
    double              get_altitude(void);

    inline double       altitude(void)
    { return this->get_altitude(); }

    /**
     * Returns the current ground speed in Km/h.
     *
     * @return a floating point number corresponding to the current ground speed in Km/h
     *
     * On failure, throws an exception or returns Y_GROUNDSPEED_INVALID.
     */
    double              get_groundSpeed(void);

    inline double       groundSpeed(void)
    { return this->get_groundSpeed(); }

    /**
     * Returns the current move bearing in degrees, zero
     * is the true (geographic) north.
     *
     * @return a floating point number corresponding to the current move bearing in degrees, zero
     *         is the true (geographic) north
     *
     * On failure, throws an exception or returns Y_DIRECTION_INVALID.
     */
    double              get_direction(void);

    inline double       direction(void)
    { return this->get_direction(); }

    /**
     * Returns the current time in Unix format (number of
     * seconds elapsed since Jan 1st, 1970).
     *
     * @return an integer corresponding to the current time in Unix format (number of
     *         seconds elapsed since Jan 1st, 1970)
     *
     * On failure, throws an exception or returns Y_UNIXTIME_INVALID.
     */
    s64                 get_unixTime(void);

    inline s64          unixTime(void)
    { return this->get_unixTime(); }

    /**
     * Returns the current time in the form "YYYY/MM/DD hh:mm:ss".
     *
     * @return a string corresponding to the current time in the form "YYYY/MM/DD hh:mm:ss"
     *
     * On failure, throws an exception or returns Y_DATETIME_INVALID.
     */
    string              get_dateTime(void);

    inline string       dateTime(void)
    { return this->get_dateTime(); }

    /**
     * Returns the number of seconds between current time and UTC time (time zone).
     *
     * @return an integer corresponding to the number of seconds between current time and UTC time (time zone)
     *
     * On failure, throws an exception or returns Y_UTCOFFSET_INVALID.
     */
    int                 get_utcOffset(void);

    inline int          utcOffset(void)
    { return this->get_utcOffset(); }

    /**
     * Changes the number of seconds between current time and UTC time (time zone).
     * The timezone is automatically rounded to the nearest multiple of 15 minutes.
     * If current UTC time is known, the current time is automatically be updated according to the selected time zone.
     *
     * @param newval : an integer corresponding to the number of seconds between current time and UTC time (time zone)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_utcOffset(int newval);
    inline int      setUtcOffset(int newval)
    { return this->set_utcOffset(newval); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a GPS for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the GPS is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YGps.isOnline() to test if the GPS is
     * indeed online at a given time. In case of ambiguity when looking for
     * a GPS by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the GPS
     *
     * @return a YGps object allowing you to drive the GPS.
     */
    static YGps*        FindGps(string func);

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
    virtual int         registerValueCallback(YGpsValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YGps* Find(string func)
    { return YGps::FindGps(func); }

    /**
     * Continues the enumeration of GPS started using yFirstGps().
     *
     * @return a pointer to a YGps object, corresponding to
     *         a GPS currently online, or a NULL pointer
     *         if there are no more GPS to enumerate.
     */
           YGps            *nextGps(void);
    inline YGps            *next(void)
    { return this->nextGps();}

    /**
     * Starts the enumeration of GPS currently accessible.
     * Use the method YGps.nextGps() to iterate on
     * next GPS.
     *
     * @return a pointer to a YGps object, corresponding to
     *         the first GPS currently online, or a NULL pointer
     *         if there are none.
     */
           static YGps* FirstGps(void);
    inline static YGps* First(void)
    { return YGps::FirstGps();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YGps accessors declaration)
};

//--- (YGps functions declaration)

/**
 * Retrieves a GPS for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the GPS is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YGps.isOnline() to test if the GPS is
 * indeed online at a given time. In case of ambiguity when looking for
 * a GPS by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the GPS
 *
 * @return a YGps object allowing you to drive the GPS.
 */
inline YGps* yFindGps(const string& func)
{ return YGps::FindGps(func);}
/**
 * Starts the enumeration of GPS currently accessible.
 * Use the method YGps.nextGps() to iterate on
 * next GPS.
 *
 * @return a pointer to a YGps object, corresponding to
 *         the first GPS currently online, or a NULL pointer
 *         if there are none.
 */
inline YGps* yFirstGps(void)
{ return YGps::FirstGps();}

//--- (end of YGps functions declaration)

#endif
