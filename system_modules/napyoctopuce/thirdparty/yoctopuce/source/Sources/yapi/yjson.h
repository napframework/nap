/*********************************************************************
 *
 * $Id: yjson.h 27182 2017-04-20 16:25:41Z seb $
 *
 * Simple JSON parser (actually a slightly enhanced lexer)
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

#ifndef YJSON_H
#define YJSON_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ydef.h"

//#define DEBUG_JSON_PARSE
#define YJSON_MAX_DEPTH     6

typedef enum {
    YJSON_HTTP_START,       // about to parse HTTP header, up to first space before return code
    YJSON_HTTP_READ_CODE,   // reading HTTP return code
    YJSON_HTTP_READ_MSG,    // reading HTTP return message
    YJSON_HTTP_SKIP,        // skipping rest of HTTP header until double-CRLF
    YJSON_START,            // about to parse JSON reply
    YJSON_PARSE_ANY,        // parsing anything to come
    YJSON_PARSE_SYMBOL,     // parsing a symbol (boolean)
    YJSON_PARSE_NUM,        // parsing a number
    YJSON_PARSE_STRING,     // parsing a quoted string
    YJSON_PARSE_STRINGQ,    // parsing a quoted string, within quoted character
    YJSON_PARSE_STRINGCONT, // parsing the continuation of a quoted string
    YJSON_PARSE_STRINGCONTQ,// parsing the continuation of a quoted string, within quoted character
    YJSON_PARSE_ARRAY,      // parsing an unnamed array
    YJSON_PARSE_STRUCT,     // parsing a named structure
    YJSON_PARSE_MEMBSTART,  // parsing a structure member (before name)
    YJSON_PARSE_MEMBNAME,   // parsing a structure member name
    YJSON_PARSE_MEMBCOL,    // parsing the colon between member name and value
    YJSON_PARSE_DONE,       // parse completed, end of input data (or end of container)
    YJSON_PARSE_ERROR,      // dead end, parse error encountered
    YJSON_PARSE_SPECIAL = -1
} yJsonState;

#ifdef DEBUG_JSON_PARSE
extern const char* yJsonStateStr[];
#endif



typedef struct {
    _FAR const char *src;               // pointer to source buffer to parse (initialized by caller)
    _FAR const char *end;               // pointer to end of source data (initialized by caller)
    yJsonState  st;                     // current state (initialized by caller)
    yJsonState  next;                   // next state (when returning with parse_avail)
    yJsonState  stack[YJSON_MAX_DEPTH]; // state stack for handling nested structures
    int         depth;                  // state stack depth
    char        token[62];              // parse buffer, also used to return tokens
    char        *pt;                    // pointer in token buffer
    int         skipcnt;                // number of items to skip
    int         skipdepth;              // stack depth at which skipping started
#ifndef YAPI_IN_YDEVICE
    _FAR const char *state_start;       // pointer to the start of the current state
    _FAR const char *state_end;         // pointer to end of of the current state
#endif
} yJsonStateMachine;


typedef enum {
    YJSON_NEED_INPUT,       // caller need to provide fresh input data
    YJSON_PARSE_AVAIL,      // caller need to process current state and token
    YJSON_SUCCESS,          // caller can take a break
    YJSON_FAILED            // caller should consider using better input data
} yJsonRetCode;

// Parse JSON input stream until more data is needed or a token is returned
yJsonRetCode yJsonParse(yJsonStateMachine *j);

// Mark next n JSON items in stream to be skipped (including content, in case items are containers)
void         yJsonSkip(yJsonStateMachine *j, int nitems);

#if 0

    typedef enum {
        JZON_PARSE_SYNCRO = 0,
        JZON_PARSE_ONLY_REF,
        JZON_PARSE_ONLY_YZON
    } yJzonSyncroState;

    typedef struct {
        int         depth;                  // state stack depth
        yJzonSyncroState sst;
        yJzonSyncroState sst_stack[YJSON_MAX_DEPTH];
        yJsonState  st;                     // current state (initialized by caller)
        yJsonState  next;                   // next state (when returning with parse_avail)
        char        token[62];              // parse buffer, also used to return tokens
        yJsonStateMachine  jzon;       // reference state machine (initialized by caller)
        yJsonStateMachine  ref;       // reference state machine (initialized by caller)
    } yJsonStateMachineEx;

    void yJsonInitEx(yJsonStateMachineEx *j, const char *jzon, int jzon_len, const char *ref, int ref_len);

    // Parse JSON input stream until more data is needed or a token is returned
    yJsonRetCode yJsonParseEx(yJsonStateMachineEx *j);

    // Mark next n JSON items in stream to be skipped (including content, in case items are containers)
    void         yJsonSkipEx(yJsonStateMachineEx *j, int nitems);
#endif

#ifdef  __cplusplus
}
#endif
#endif


