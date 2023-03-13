/*********************************************************************
 *
 * $Id: yocto_files.cpp 28753 2017-10-03 11:23:38Z seb $
 *
 * Implements yFindFiles(), the high-level API for Files functions
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


#define _CRT_SECURE_NO_DEPRECATE //do not use windows secure crt
#include "yocto_files.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define  __FILE_ID__  "files"


YFileRecord::YFileRecord(const string& json):
//--- (generated code: YFileRecord initialization)
    _size(0)
    ,_crc(0)
//--- (end of generated code: YFileRecord initialization)
{
    yJsonStateMachine j;

    // Parse JSON data
    j.src = json.c_str();
    j.end = j.src + strlen(j.src);
    j.st = YJSON_START;
    if(yJsonParse(&j) != YJSON_PARSE_AVAIL || j.st != YJSON_PARSE_STRUCT) {
        return ;
    }
    while(yJsonParse(&j) == YJSON_PARSE_AVAIL && j.st == YJSON_PARSE_MEMBNAME) {
        if (!strcmp(j.token, "name")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return ;
            }
            _name = (string)j.token;
            while(j.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j) == YJSON_PARSE_AVAIL) {
                _name =(string)j.token;
            }
        } else if(!strcmp(j.token, "crc")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return;
            }
            _crc = atoi(j.token);;
        } else if(!strcmp(j.token, "size")) {
            if (yJsonParse(&j) != YJSON_PARSE_AVAIL) {
                return;
            }
            _size = atoi(j.token);;
        } else {
            yJsonSkip(&j, 1);
        }
    }
}

//--- (generated code: YFileRecord implementation)
// static attributes


string YFileRecord::get_name(void)
{
    return _name;
}

int YFileRecord::get_size(void)
{
    return _size;
}

int YFileRecord::get_crc(void)
{
    return _crc;
}
//--- (end of generated code: YFileRecord implementation)





YFiles::YFiles(const string& func): YFunction(func)
    //--- (generated code: YFiles initialization)
    ,_filesCount(FILESCOUNT_INVALID)
    ,_freeSpace(FREESPACE_INVALID)
    ,_valueCallbackFiles(NULL)
//--- (end of generated code: YFiles initialization)
{
    _className = "Files";
}

YFiles::~YFiles()
{
    //--- (generated code: YFiles cleanup)
//--- (end of generated code: YFiles cleanup)
}

//--- (generated code: YFiles implementation)
// static attributes

int YFiles::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("filesCount")) {
        _filesCount =  json_val->getInt("filesCount");
    }
    if(json_val->has("freeSpace")) {
        _freeSpace =  json_val->getInt("freeSpace");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the number of files currently loaded in the filesystem.
 *
 * @return an integer corresponding to the number of files currently loaded in the filesystem
 *
 * On failure, throws an exception or returns Y_FILESCOUNT_INVALID.
 */
int YFiles::get_filesCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YFiles::FILESCOUNT_INVALID;
                }
            }
        }
        res = _filesCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the free space for uploading new files to the filesystem, in bytes.
 *
 * @return an integer corresponding to the free space for uploading new files to the filesystem, in bytes
 *
 * On failure, throws an exception or returns Y_FREESPACE_INVALID.
 */
int YFiles::get_freeSpace(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YFiles::FREESPACE_INVALID;
                }
            }
        }
        res = _freeSpace;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
YFiles* YFiles::FindFiles(string func)
{
    YFiles* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YFiles*) YFunction::_FindFromCache("Files", func);
        if (obj == NULL) {
            obj = new YFiles(func);
            YFunction::_AddToCache("Files", func, obj);
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
int YFiles::registerValueCallback(YFilesValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackFiles = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YFiles::_invokeValueCallback(string value)
{
    if (_valueCallbackFiles != NULL) {
        _valueCallbackFiles(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

string YFiles::sendCommand(string command)
{
    string url;
    url = YapiWrapper::ysprintf("files.json?a=%s",command.c_str());

    return this->_download(url);
}

/**
 * Reinitialize the filesystem to its clean, unfragmented, empty state.
 * All files previously uploaded are permanently lost.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YFiles::format_fs(void)
{
    string json;
    string res;
    json = this->sendCommand("format");
    res = this->_json_get_key(json, "res");
    if (!(res == "ok")) {
        _throw(YAPI_IO_ERROR,"format failed");
        return YAPI_IO_ERROR;
    }
    return YAPI_SUCCESS;
}

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
vector<YFileRecord> YFiles::get_list(string pattern)
{
    string json;
    vector<string> filelist;
    vector<YFileRecord> res;
    json = this->sendCommand(YapiWrapper::ysprintf("dir&f=%s",pattern.c_str()));
    filelist = this->_json_get_array(json);
    res.clear();
    for (unsigned ii = 0; ii < filelist.size(); ii++) {
        res.push_back(YFileRecord(filelist[ii]));
    }
    return res;
}

/**
 * Test if a file exist on the filesystem of the module.
 *
 * @param filename : the file name to test.
 *
 * @return a true if the file existe, false ortherwise.
 *
 * On failure, throws an exception.
 */
bool YFiles::fileExist(string filename)
{
    string json;
    vector<string> filelist;
    if ((int)(filename).length() == 0) {
        return false;
    }
    json = this->sendCommand(YapiWrapper::ysprintf("dir&f=%s",filename.c_str()));
    filelist = this->_json_get_array(json);
    if ((int)filelist.size() > 0) {
        return true;
    }
    return false;
}

/**
 * Downloads the requested file and returns a binary buffer with its content.
 *
 * @param pathname : path and name of the file to download
 *
 * @return a binary buffer with the file content
 *
 * On failure, throws an exception or returns an empty content.
 */
string YFiles::download(string pathname)
{
    return this->_download(pathname);
}

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
int YFiles::upload(string pathname,string content)
{
    return this->_upload(pathname, content);
}

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
int YFiles::remove(string pathname)
{
    string json;
    string res;
    json = this->sendCommand(YapiWrapper::ysprintf("del&f=%s",pathname.c_str()));
    res  = this->_json_get_key(json, "res");
    if (!(res == "ok")) {
        _throw(YAPI_IO_ERROR,"unable to remove file");
        return YAPI_IO_ERROR;
    }
    return YAPI_SUCCESS;
}

YFiles *YFiles::nextFiles(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YFiles::FindFiles(hwid);
}

YFiles* YFiles::FirstFiles(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Files", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YFiles::FindFiles(serial+"."+funcId);
}

//--- (end of generated code: YFiles implementation)

//--- (generated code: YFiles functions)
//--- (end of generated code: YFiles functions)
