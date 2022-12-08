/*********************************************************************
 *
 * $Id: yocto_files.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindFiles(), the high-level API for Files functions
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
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
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


#ifndef YOCTO_FILES_H
#define YOCTO_FILES_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (generated code: YFiles definitions)
class YFiles; // forward declaration

typedef void (*YFilesValueCallback)(YFiles *func, const string& functionValue);
#define Y_FILESCOUNT_INVALID            (YAPI_INVALID_UINT)
#define Y_FREESPACE_INVALID             (YAPI_INVALID_UINT)
//--- (end of generated code: YFiles definitions)


//--- (generated code: YFileRecord definitions)
//--- (end of generated code: YFileRecord definitions)


//--- (generated code: YFileRecord declaration)
/**
 * YFileRecord Class: Description of a file on the device filesystem
 *
 *
 */
class YOCTO_CLASS_EXPORT YFileRecord {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YFileRecord declaration)
    //--- (generated code: YFileRecord attributes)
    // Attributes (function value cache)
    string          _name;
    int             _size;
    int             _crc;
    //--- (end of generated code: YFileRecord attributes)
    //--- (generated code: YFileRecord constructor)

    //--- (end of generated code: YFileRecord constructor)
    //--- (generated code: YFileRecord initialization)
    //--- (end of generated code: YFileRecord initialization)

public:
    YFileRecord(const string& json);
    //--- (generated code: YFileRecord accessors declaration)


    virtual string      get_name(void);

    virtual int         get_size(void);

    virtual int         get_crc(void);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YFileRecord accessors declaration)
};





//--- (generated code: YFiles declaration)
/**
 * YFiles Class: Files function interface
 *
 * The filesystem interface makes it possible to store files
 * on some devices, for instance to design a custom web UI
 * (for networked devices) or to add fonts (on display
 * devices).
 */
class YOCTO_CLASS_EXPORT YFiles: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YFiles declaration)
    //--- (generated code: YFiles attributes)
    // Attributes (function value cache)
    int             _filesCount;
    int             _freeSpace;
    YFilesValueCallback _valueCallbackFiles;

    friend YFiles *yFindFiles(const string& func);
    friend YFiles *yFirstFiles(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindFiles factory function to instantiate
    YFiles(const string& func);
    //--- (end of generated code: YFiles attributes)

    //--- (generated code: YFiles initialization)
    //--- (end of generated code: YFiles initialization)

public:
    ~YFiles();
    //--- (generated code: YFiles accessors declaration)

    static const int FILESCOUNT_INVALID = YAPI_INVALID_UINT;
    static const int FREESPACE_INVALID = YAPI_INVALID_UINT;

    /**
     * Returns the number of files currently loaded in the filesystem.
     *
     * @return an integer corresponding to the number of files currently loaded in the filesystem
     *
     * On failure, throws an exception or returns Y_FILESCOUNT_INVALID.
     */
    int                 get_filesCount(void);

    inline int          filesCount(void)
    { return this->get_filesCount(); }

    /**
     * Returns the free space for uploading new files to the filesystem, in bytes.
     *
     * @return an integer corresponding to the free space for uploading new files to the filesystem, in bytes
     *
     * On failure, throws an exception or returns Y_FREESPACE_INVALID.
     */
    int                 get_freeSpace(void);

    inline int          freeSpace(void)
    { return this->get_freeSpace(); }

    /**
     * Retrieves a filesystem for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the filesystem is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YFiles.isOnline() to test if the filesystem is
     * indeed online at a given time. In case of ambiguity when looking for
     * a filesystem by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the filesystem
     *
     * @return a YFiles object allowing you to drive the filesystem.
     */
    static YFiles*      FindFiles(string func);

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
    virtual int         registerValueCallback(YFilesValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    virtual string      sendCommand(string command);

    /**
     * Reinitialize the filesystem to its clean, unfragmented, empty state.
     * All files previously uploaded are permanently lost.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         format_fs(void);

    /**
     * Returns a list of YFileRecord objects that describe files currently loaded
     * in the filesystem.
     *
     * @param pattern : an optional filter pattern, using star and question marks
     *         as wildcards. When an empty pattern is provided, all file records
     *         are returned.
     *
     * @return a list of YFileRecord objects, containing the file path
     *         and name, byte size and 32-bit CRC of the file content.
     *
     * On failure, throws an exception or returns an empty list.
     */
    virtual vector<YFileRecord> get_list(string pattern);

    /**
     * Test if a file exist on the filesystem of the module.
     *
     * @param filename : the file name to test.
     *
     * @return a true if the file existe, false ortherwise.
     *
     * On failure, throws an exception.
     */
    virtual bool        fileExist(string filename);

    /**
     * Downloads the requested file and returns a binary buffer with its content.
     *
     * @param pathname : path and name of the file to download
     *
     * @return a binary buffer with the file content
     *
     * On failure, throws an exception or returns an empty content.
     */
    virtual string      download(string pathname);

    /**
     * Uploads a file to the filesystem, to the specified full path name.
     * If a file already exists with the same path name, its content is overwritten.
     *
     * @param pathname : path and name of the new file to create
     * @param content : binary buffer with the content to set
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         upload(string pathname,string content);

    /**
     * Deletes a file, given by its full path name, from the filesystem.
     * Because of filesystem fragmentation, deleting a file may not always
     * free up the whole space used by the file. However, rewriting a file
     * with the same path name will always reuse any space not freed previously.
     * If you need to ensure that no space is taken by previously deleted files,
     * you can use format_fs to fully reinitialize the filesystem.
     *
     * @param pathname : path and name of the file to remove.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         remove(string pathname);


    inline static YFiles* Find(string func)
    { return YFiles::FindFiles(func); }

    /**
     * Continues the enumeration of filesystems started using yFirstFiles().
     *
     * @return a pointer to a YFiles object, corresponding to
     *         a filesystem currently online, or a NULL pointer
     *         if there are no more filesystems to enumerate.
     */
           YFiles          *nextFiles(void);
    inline YFiles          *next(void)
    { return this->nextFiles();}

    /**
     * Starts the enumeration of filesystems currently accessible.
     * Use the method YFiles.nextFiles() to iterate on
     * next filesystems.
     *
     * @return a pointer to a YFiles object, corresponding to
     *         the first filesystem currently online, or a NULL pointer
     *         if there are none.
     */
           static YFiles* FirstFiles(void);
    inline static YFiles* First(void)
    { return YFiles::FirstFiles();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YFiles accessors declaration)
};

//--- (generated code: YFiles functions declaration)

/**
 * Retrieves a filesystem for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the filesystem is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YFiles.isOnline() to test if the filesystem is
 * indeed online at a given time. In case of ambiguity when looking for
 * a filesystem by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the filesystem
 *
 * @return a YFiles object allowing you to drive the filesystem.
 */
inline YFiles* yFindFiles(const string& func)
{ return YFiles::FindFiles(func);}
/**
 * Starts the enumeration of filesystems currently accessible.
 * Use the method YFiles.nextFiles() to iterate on
 * next filesystems.
 *
 * @return a pointer to a YFiles object, corresponding to
 *         the first filesystem currently online, or a NULL pointer
 *         if there are none.
 */
inline YFiles* yFirstFiles(void)
{ return YFiles::FirstFiles();}

//--- (end of generated code: YFiles functions declaration)

#endif
